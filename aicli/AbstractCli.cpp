// GCC 13 codegen bug: coroutines with non-trivially-destructible frame locals
// (QPointer, std::function, CliAvailability, QProcess) miscompile at -O2/-O3.
#pragma GCC optimize("O1")
#include "AbstractCli.h"

#include <chrono>

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QPointer>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>

#include <QCoro/QCoroTimer>

// ---------------------------------------------------------------------------
// Default virtuals
// ---------------------------------------------------------------------------

QString AbstractCli::getModel() const
{
    const QStringList args = promptArgs();
    for (int i = 0; i < args.size() - 1; ++i) {
        if (args.at(i) == QStringLiteral("--model")) {
            return args.at(i + 1);
        }
    }
    return {};
}

QStringList AbstractCli::promptArgs() const
{
    return {QStringLiteral("-p"), QStringLiteral("-")};
}

QStringList AbstractCli::translationPromptArgs() const
{
    return promptArgs();
}

bool AbstractCli::promptViaStdin() const
{
    return true;
}

void AbstractCli::configurePromptProcess(QProcess *process,
                                         const QStringList &baseArgs,
                                         const QString &prompt,
                                         const QString &promptPath) const
{
    if (promptViaStdin()) {
        process->setArguments(baseArgs);
        process->setStandardInputFile(promptPath);
        return;
    }

    // argv-based CLI (e.g. Antigravity): the prompt text replaces the '-'
    // placeholder.  Keep the placeholder position so value-taking flags such
    // as `--print <prompt>` stay correctly paired.
    QStringList arguments = baseArgs;
    const int placeholder = arguments.indexOf(QStringLiteral("-"));
    if (placeholder >= 0) {
        arguments[placeholder] = prompt;
    } else {
        arguments.append(prompt);
    }
    process->setArguments(arguments);
    // Bind stdin to the null device so a CLI that unexpectedly polls stdin
    // can never block waiting for input.
    process->setStandardInputFile(QProcess::nullDevice());
}

QString AbstractCli::extractTextFromOutput(const QByteArray &rawOutput) const
{
    return QString::fromUtf8(rawOutput).trimmed();
}

QString AbstractCli::preparePrompt(const QString &prompt) const
{
    return prompt;
}

QStringList AbstractCli::outputFallbackDirs() const
{
    return {};
}

// ---------------------------------------------------------------------------
// TRANSPORT FAILURES, CLASSIFIED ONCE FOR EVERY CLI
// ---------------------------------------------------------------------------
//
// Every CLI in this registry is an HTTPS client wrapped in a terminal command,
// so the layer that breaks when there is no network prints the SAME family of
// messages whichever binary is in front of it: libc resolver errors
// ("Name or service not known", "Temporary failure in name resolution"), Node's
// errno spellings ("ENOTFOUND", "ECONNRESET", "EAI_AGAIN", "socket hang up",
// "fetch failed"), curl/Go phrasings ("connection refused", "no route to host")
// and gateway statuses (502/503/504). Recognising them here rather than in each
// subclass means a caller's "is this worth retrying?" question has ONE answer
// for all of them, and a newly added CLI inherits it without writing anything.
//
// ⚠️ WHY THIS IS DELIBERATELY CONSERVATIVE. The cost of a FALSE NetworkError is
// that a caller waits and retries something that will never work; the cost of a
// missed one is that it gives up on an outage that would have cleared. So the
// matched strings are transport words only — no bare "timeout" (a slow model
// times out too, and re-running it costs another full timeout), no bare "error",
// and auth/quota are NOT guessed at here because their wording is genuinely
// vendor-specific and a subclass override owns it (and is checked first, since
// an override runs before it chains down to this).
CliErrorKind AbstractCli::classifyError(const QString &errorOutput) const
{
    if (errorOutput.isEmpty())
        return CliErrorKind::Other;

    static const char *const kNetworkNeedles[] = {
        // libc / getaddrinfo
        "getaddrinfo",
        "name or service not known",
        "temporary failure in name resolution",
        "could not resolve host",
        "could not resolve",
        "nodename nor servname provided",
        "dns lookup failed",
        // errno spellings (Node, Go, Python clients)
        "enotfound",
        "eai_again",
        "econnrefused",
        "econnreset",
        "ehostunreach",
        "enetunreach",
        "enetdown",
        "epipe",
        // prose forms of the same
        "connection refused",
        "connection reset",
        "connection aborted",
        "connection closed",
        "network is unreachable",
        "network unreachable",
        "network error",
        "no route to host",
        "socket hang up",
        "fetch failed",
        "failed to connect",
        "unable to connect",
        "remote end closed connection",
        "server disconnected",
        "broken pipe",
        // proxy / TLS transport
        "proxy connect",
        "proxyconnect",
        "tls handshake",
        "ssl handshake",
        "handshake failure",
        // gateways answering for a service that is momentarily unreachable
        "502 bad gateway",
        "bad gateway",
        "503 service unavailable",
        "service unavailable",
        "504 gateway time-out",
        "504 gateway timeout",
        "gateway timeout",
        "upstream connect error",
        // the tools' own summaries
        "you appear to be offline",
        "offline",
    };
    for (const char *needle : kNetworkNeedles) {
        if (errorOutput.contains(QLatin1String(needle), Qt::CaseInsensitive))
            return CliErrorKind::NetworkError;
    }
    return CliErrorKind::Other;
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

// Resolve an executable name to its full path, or an EMPTY string when it is
// not installed anywhere this resolver looks.
//
// First checks the current process PATH (works when launched from a terminal).
// If not found, scans common user-local package-manager paths so that
// nvm/npm/yarn-installed tools are found even when the app is launched from a
// desktop shortcut that inherits a minimal PATH.
// NOTE: no subprocess is spawned here — using QProcess::waitForFinished() would
// spin a nested Qt event loop and corrupt in-flight QCoro signal connections.
static QString resolveExecutableOrEmpty(const QString &name)
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

    return {};
}

// Same lookup, but with the historical fallback for the LAUNCH path: a bare
// name is still handed to QProcess, which reports FailedToStart. Kept so a
// mis-detection can never turn a runnable CLI into a silent no-op.
static QString resolveExecutable(const QString &name)
{
    const QString found = resolveExecutableOrEmpty(name);
    return found.isEmpty() ? name : found;
}

// ---------------------------------------------------------------------------
// Installation probe
// ---------------------------------------------------------------------------
//
// ⚠️ THE SAME RESOLVER THAT LAUNCHES THE PROCESS ANSWERS "IS IT INSTALLED?".
// Two separate lookups would eventually disagree, and the disagreement is
// exactly the failure a caller uses this to avoid: a UI offering a CLI it
// cannot launch. No subprocess is spawned, so this is cheap enough to call for
// every registered CLI while building a widget, and deterministic — unlike
// AvailableCliTable's `--version` probe, which is asynchronous and reports
// membership too.

QString AbstractCli::resolvedExecutablePath() const
{
    return resolveExecutableOrEmpty(getExecutable());
}

bool AbstractCli::isInstalled() const
{
    return !resolvedExecutablePath().isEmpty();
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
    const QString prepared = preparePrompt(prompt);
    const QString promptPath = tempDir.path() + QStringLiteral("/prompt.txt");
    QFile promptFile(promptPath);
    if (!promptFile.open(QIODevice::WriteOnly)) {
        co_return res;
    }
    promptFile.write(prepared.toUtf8());
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
    configurePromptProcess(&process, promptArgs(), prepared, promptPath);

    process.start();
    if (process.waitForStarted(30'000)) {
        // QCoroProcess resumes the coroutine from QProcess::finished. The
        // local process can then be destroyed while its signal is still being
        // delivered. Wait via timer ticks so the event loop stays responsive
        // and the process outlives its own finished callback.
        while (process.state() != QProcess::NotRunning)
            co_await QCoro::sleepFor(std::chrono::milliseconds(100));
    }

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
    // THE SAME RESOLVER EVERY OTHER ENTRY POINT USES. This line used to be a
    // bare setProgram(getExecutable()), which only ever searched the process
    // PATH — so when the app is launched from a desktop shortcut (minimal PATH,
    // no ~/.local/bin, no nvm bin dir) this coroutine reported an INSTALLED CLI
    // as unavailable, while checkAvailabilityAsync() and runPrompt[Async]() —
    // all three going through applyResolvedProgram(resolveExecutable(...)) —
    // found and ran it. Which API a caller happens to use must not change the
    // answer to "is this CLI here?".
    applyResolvedProgram(&process, resolveExecutable(getExecutable()));
    process.setArguments(availabilityArgs());

    process.start();
    if (!process.waitForStarted(30'000)) {
        co_return avail;
    }
    // 5 s timeout: version checks should be instant; guard against CLIs that
    // launch an interactive REPL and never exit on their own.
    QElapsedTimer timeout;
    timeout.start();
    while (process.state() != QProcess::NotRunning && timeout.elapsed() < 5000)
        co_await QCoro::sleepFor(std::chrono::milliseconds(100));
    if (process.state() != QProcess::NotRunning) {
        process.kill();
        process.waitForFinished(2000);
        co_return avail;
    }

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

QProcess *AbstractCli::runPromptAsync(const QString &prompt,
                                  QObject *context,
                                  std::function<void(CliRunResult)> callback) const
{
    return runPromptAsync(prompt, {}, context, std::move(callback));
}

QProcess *AbstractCli::runPromptAsync(const QString &prompt,
                                  const QString &workingDir,
                                  QObject *context,
                                  std::function<void(CliRunResult)> callback) const
{
    auto ctx = std::make_shared<RunCtx>();
    if (!ctx->tempDir.isValid())
        return nullptr;

    const QString prepared = preparePrompt(prompt);
    const QString promptPath = ctx->tempDir.path() + QStringLiteral("/prompt.txt");
    QFile promptFile(promptPath);
    if (!promptFile.open(QIODevice::WriteOnly))
        return nullptr;
    promptFile.write(prepared.toUtf8());
    promptFile.close();

    const QString processWorkDir =
        (!workingDir.isEmpty() && QDir(workingDir).exists()) ? workingDir : ctx->tempDir.path();

    ctx->guard    = QPointer<QObject>(context);
    ctx->callback = std::move(callback);
    ctx->timer.start();

    auto *process = new QProcess();
    process->setWorkingDirectory(processWorkDir);
    applyResolvedProgram(process, resolveExecutable(getExecutable()));
    configurePromptProcess(process, promptArgs(), prepared, promptPath);

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
    return process;
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
