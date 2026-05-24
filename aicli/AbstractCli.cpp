// GCC 13 codegen bug: coroutines with non-trivially-destructible frame locals
// (QPointer, std::function, CliAvailability, QProcess) miscompile at -O2/-O3.
#pragma GCC optimize("O1")
#include "AbstractCli.h"

#include <QElapsedTimer>
#include <QFile>
#include <QPointer>
#include <QProcess>
#include <QTemporaryDir>

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
    process.setProgram(getExecutable());
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
// Callback wrappers (fire-and-forget coroutines)
// ---------------------------------------------------------------------------
//
// IMPORTANT: do NOT use lambda coroutines here. A lambda's closure object
// lives on the stack of the enclosing function. When the function returns the
// coroutine is suspended, and the frame only holds a *pointer* to the now-
// destroyed closure. When the coroutine later resumes it reads dangling memory
// (captured callback / guard), causing a crash ~N seconds later when the
// child process finishes.
//
// Fix: free coroutine functions whose parameters are copied into the frame by
// the C++ standard ("copies all function parameters to the coroutine state").

static QCoro::Task<void> doRunPromptAsync(const AbstractCli *cli,
                                           QString prompt,
                                           QString workingDir,
                                           QPointer<QObject> guard,
                                           std::function<void(CliRunResult)> callback)
{
    CliRunResult result = co_await cli->runPrompt(prompt, workingDir);
    if (guard) {
        callback(std::move(result));
    }
}

static QCoro::Task<void> doCheckAvailabilityAsync(const AbstractCli *cli,
                                                   QPointer<QObject> guard,
                                                   std::function<void(CliAvailability)> callback)
{
    CliAvailability avail = co_await cli->checkAvailability();
    if (guard) {
        callback(std::move(avail));
    }
}

void AbstractCli::runPromptAsync(const QString &prompt,
                                  QObject *context,
                                  std::function<void(CliRunResult)> callback) const
{
    doRunPromptAsync(this, prompt, {}, QPointer<QObject>(context), std::move(callback));
}

void AbstractCli::runPromptAsync(const QString &prompt,
                                  const QString &workingDir,
                                  QObject *context,
                                  std::function<void(CliRunResult)> callback) const
{
    doRunPromptAsync(this, prompt, workingDir, QPointer<QObject>(context), std::move(callback));
}

void AbstractCli::checkAvailabilityAsync(QObject *context,
                                          std::function<void(CliAvailability)> callback) const
{
    doCheckAvailabilityAsync(this, QPointer<QObject>(context), std::move(callback));
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
