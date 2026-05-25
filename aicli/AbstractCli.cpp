// GCC 13 codegen bug: coroutines with non-trivially-destructible frame locals
// (QPointer, std::function, CliAvailability, QProcess) miscompile at -O2/-O3.
#pragma GCC optimize("O1")
#include "AbstractCli.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QPointer>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>

#include <QCoro/QCoroProcess>

// ---------------------------------------------------------------------------
// Default virtuals
// ---------------------------------------------------------------------------

QStringList AbstractCli::promptArgs() const
{
    return {QStringLiteral("-p"), QStringLiteral("-")};
}

QStringList AbstractCli::availabilityArgs() const
{
    return {QStringLiteral("--version")};
}

QString AbstractCli::parseMembership(const QString &) const
{
    return {};
}

// ---------------------------------------------------------------------------
// Coroutine implementations
// ---------------------------------------------------------------------------

// Resolve an executable name to its full path.
// First checks the current process PATH (works when launched from a terminal).
// If not found, scans common user-local package-manager paths so that
// nvm/npm/yarn-installed tools are found even when the app is launched from a
// desktop shortcut that inherits a minimal PATH.
// NOTE: no subprocess is spawned here — using QProcess::waitForFinished() would
// spin a nested Qt event loop and corrupt in-flight QCoro signal connections.
static QString resolveExecutable(const QString &name)
{
    const QString direct = QStandardPaths::findExecutable(name);
    if (!direct.isEmpty())
        return direct;

    const QString home = QDir::homePath();

    // nvm: enumerate installed node versions and check each bin/ directory.
    const QDir nvmVersions(home + QStringLiteral("/.nvm/versions/node"));
    if (nvmVersions.exists()) {
        for (const QString &ver : nvmVersions.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            const QString candidate =
                nvmVersions.filePath(ver + QStringLiteral("/bin/") + name);
            if (QFileInfo::exists(candidate))
                return candidate;
        }
    }

    // Other common user-local bin directories.
    const QStringList extraDirs = {
        home + QStringLiteral("/.local/bin"),
        home + QStringLiteral("/.npm-global/bin"),
        home + QStringLiteral("/.yarn/bin"),
        home + QStringLiteral("/.pnpm"),
        home + QStringLiteral("/.local/share/pnpm"),
    };
    for (const QString &dir : extraDirs) {
        const QString candidate = dir + QLatin1Char('/') + name;
        if (QFileInfo::exists(candidate))
            return candidate;
    }

    return name; // fall back to bare name; QProcess will report FailedToStart
}

// Set the program on `process` and, when the resolved path lives outside the
// current PATH (e.g. an nvm-installed Node script), inject its directory into
// the process environment so the shebang's runtime (node, python, …) is found.
static void applyResolvedProgram(QProcess *process, const QString &execPath)
{
    process->setProgram(execPath);

    const QFileInfo fi(execPath);
    if (!fi.isAbsolute())
        return; // bare name fallback — PATH already applies

    const QString execDir = fi.absolutePath();
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString currentPath = env.value(QStringLiteral("PATH"));
    // Only prepend if the directory isn't already present.
    const QStringList pathDirs = currentPath.split(QLatin1Char(':'), Qt::SkipEmptyParts);
    if (!pathDirs.contains(execDir)) {
        env.insert(QStringLiteral("PATH"), execDir + QLatin1Char(':') + currentPath);
        process->setProcessEnvironment(env);
    }
}

QCoro::Task<CliRunResult> AbstractCli::runPrompt(const QString &prompt,
                                                   const QString &workingDir) const
{
    CliRunResult res;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        co_return res;
    }

    // Write prompt to a file to avoid per-argument size limits on Linux
    // (MAX_ARG_STRLEN = 128 KB) and to keep argv clean.
    const QString promptPath = tempDir.path() + QStringLiteral("/prompt.txt");
    QFile promptFile(promptPath);
    if (!promptFile.open(QIODevice::WriteOnly)) {
        co_return res;
    }
    promptFile.write(prompt.toUtf8());
    promptFile.close();

    QElapsedTimer timer;
    timer.start();

    // Use the caller-supplied working dir when valid; fall back to the temp dir
    // so the CLI can at least find the prompt file in its working directory.
    const QString processWorkDir =
        (!workingDir.isEmpty() && QDir(workingDir).exists()) ? workingDir : tempDir.path();

    QProcess process;
    process.setWorkingDirectory(processWorkDir);
    applyResolvedProgram(&process, resolveExecutable(getExecutable()));
    process.setArguments(promptArgs());
    process.setStandardInputFile(promptPath);

    co_await qCoro(process).start();
    co_await qCoro(process).waitForFinished(-1); // no timeout: prompts can be slow

    res.durationMs     = timer.elapsed();
    res.exitCode       = process.exitCode();
    res.processStarted = (process.error() != QProcess::FailedToStart);
    res.output         = QString::fromUtf8(process.readAllStandardOutput());
    res.errorOutput    = QString::fromUtf8(process.readAllStandardError());

    co_return res;
}

QCoro::Task<CliAvailability> AbstractCli::checkAvailability() const
{
    CliAvailability avail;

    QProcess process;
    process.setProgram(getExecutable());
    process.setArguments(availabilityArgs());

    co_await qCoro(process).start();
    // 5 s timeout: version checks should be instant; guard against CLIs that
    // launch an interactive REPL and never exit on their own.
    co_await qCoro(process).waitForFinished(5000);

    if (process.error() == QProcess::FailedToStart) {
        co_return avail; // not installed
    }

    // A non-zero exit is unusual for --version, but still means the binary exists.
    avail.available      = true;
    avail.versionOutput  = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    avail.membership     = parseMembership(avail.versionOutput);

    co_return avail;
}

// ---------------------------------------------------------------------------
// Callback wrappers — raw QProcess + signals/slots, no QCoro
// ---------------------------------------------------------------------------
//
// Fire-and-forget QCoro tasks (the previous approach) caused a SIGSEGV:
// the QCoroSignal awaiter lambda captured `this` (pointer into the coroutine
// frame); by the time the queued QMetaCallEvent was dispatched the frame had
// been cleaned up, leaving a dangling `this`.
//
// The fix: heap-allocate QProcess and a shared context struct; connect
// QProcess::finished / errorOccurred directly. No coroutine frame involved,
// no awaiter lifetime issue.

namespace {

// Shared state for runPromptAsync — kept alive by two lambda captures
// (finished + errorOccurred) until one of them fires and deletes the process.
struct RunCtx {
    QTemporaryDir tempDir;   // owns prompt file until the process exits
    QElapsedTimer timer;
    QPointer<QObject> guard;
    std::function<void(CliRunResult)> callback;
    bool fired = false;
};

// Shared state for checkAvailabilityAsync.
struct CheckCtx {
    QPointer<QObject> guard;
    std::function<void(CliAvailability)> callback;
    const AbstractCli *cli;
    bool fired = false;
};

} // namespace

void AbstractCli::runPromptAsync(const QString &prompt,
                                  QObject *context,
                                  std::function<void(CliRunResult)> callback) const
{
    runPromptAsync(prompt, {}, context, std::move(callback));
}

void AbstractCli::runPromptAsync(const QString &prompt,
                                  const QString &workingDir,
                                  QObject *context,
                                  std::function<void(CliRunResult)> callback) const
{
    auto ctx = std::make_shared<RunCtx>();
    if (!ctx->tempDir.isValid())
        return;

    const QString promptPath = ctx->tempDir.path() + QStringLiteral("/prompt.txt");
    QFile promptFile(promptPath);
    if (!promptFile.open(QIODevice::WriteOnly))
        return;
    promptFile.write(prompt.toUtf8());
    promptFile.close();

    const QString processWorkDir =
        (!workingDir.isEmpty() && QDir(workingDir).exists()) ? workingDir : ctx->tempDir.path();

    ctx->guard    = QPointer<QObject>(context);
    ctx->callback = std::move(callback);
    ctx->timer.start();

    auto *process = new QProcess();
    process->setWorkingDirectory(processWorkDir);
    applyResolvedProgram(process, resolveExecutable(getExecutable()));
    process->setArguments(promptArgs());
    process->setStandardInputFile(promptPath);

    QObject::connect(process, &QProcess::finished,
                     process, [process, ctx](int exitCode, QProcess::ExitStatus) mutable {
        if (ctx->fired) return;
        ctx->fired = true;
        CliRunResult res;
        res.durationMs     = ctx->timer.elapsed();
        res.exitCode       = exitCode;
        res.processStarted = (process->error() != QProcess::FailedToStart);
        res.output         = QString::fromUtf8(process->readAllStandardOutput());
        res.errorOutput    = QString::fromUtf8(process->readAllStandardError());
        if (ctx->guard)
            ctx->callback(std::move(res));
        process->deleteLater();
    });

    // finished is NOT emitted on FailedToStart — handle it explicitly.
    QObject::connect(process, &QProcess::errorOccurred,
                     process, [process, ctx](QProcess::ProcessError error) mutable {
        if (error != QProcess::FailedToStart) return;
        if (ctx->fired) return;
        ctx->fired = true;
        CliRunResult res;
        res.durationMs     = ctx->timer.elapsed();
        res.processStarted = false;
        if (ctx->guard)
            ctx->callback(std::move(res));
        process->deleteLater();
    });

    process->start();
}

void AbstractCli::checkAvailabilityAsync(QObject *context,
                                          std::function<void(CliAvailability)> callback) const
{
    auto ctx      = std::make_shared<CheckCtx>();
    ctx->guard    = QPointer<QObject>(context);
    ctx->callback = std::move(callback);
    ctx->cli      = this;

    auto *process = new QProcess();
    applyResolvedProgram(process, resolveExecutable(getExecutable()));
    process->setArguments(availabilityArgs());

    QObject::connect(process, &QProcess::finished,
                     process, [process, ctx](int, QProcess::ExitStatus) mutable {
        if (ctx->fired) return;
        ctx->fired = true;
        CliAvailability avail;
        if (process->error() != QProcess::FailedToStart) {
            avail.available     = true;
            avail.versionOutput = QString::fromUtf8(process->readAllStandardOutput()).trimmed();
            avail.membership    = ctx->cli->parseMembership(avail.versionOutput);
        }
        if (ctx->guard)
            ctx->callback(std::move(avail));
        process->deleteLater();
    });

    QObject::connect(process, &QProcess::errorOccurred,
                     process, [process, ctx](QProcess::ProcessError error) mutable {
        if (error != QProcess::FailedToStart) return;
        if (ctx->fired) return;
        ctx->fired = true;
        if (ctx->guard)
            ctx->callback(CliAvailability{}); // not installed
        process->deleteLater();
    });

    // 5 s timeout: version checks should be instant; kill to avoid a REPL hang.
    auto *timeout = new QTimer(process);
    timeout->setSingleShot(true);
    QObject::connect(timeout, &QTimer::timeout, process, [process]() { process->kill(); });
    timeout->start(5000);

    process->start();
}

// ---------------------------------------------------------------------------
// Registry
// ---------------------------------------------------------------------------

AbstractCli::Recorder::Recorder(AbstractCli *cli)
{
    getClis().append(cli);
}

const QList<AbstractCli *> &AbstractCli::ALL_CLIS()
{
    return getClis();
}

QList<AbstractCli *> &AbstractCli::getClis()
{
    static QList<AbstractCli *> list;
    return list;
}
