#ifndef ABSTRACTCLI_H
#define ABSTRACTCLI_H

#include <functional>

#include <QList>
#include <QString>
#include <QStringList>

#include <QCoro/QCoroTask>

// Result of a prompt run through a CLI.
struct CliRunResult {
    QString output;           // Full stdout from the CLI process.
    QString errorOutput;      // Stderr from the CLI process.
    int     exitCode       = -1;
    qint64  durationMs     = 0;
    bool    processStarted = false; // false when the executable was not found in PATH.
};

// Result of an availability check.
struct CliAvailability {
    bool    available      = false;
    QString membership;         // Account/plan info if detectable; empty otherwise.
    QString versionOutput;      // Raw stdout from the availability check command.
};

// Base class for AI command-line tools (Claude, Gemini, Codex, …).
//
// Subclasses register themselves at startup via DECLARE_CLI() and are
// accessible through ALL_CLIS().
//
// Two async APIs are provided for each operation:
//   - QCoro::Task<> (for callers already in a coroutine context)
//   - Callback/lambda (fire-and-forget; context-guarded via QPointer)
//
// Requirements: caller's project must have QCoro set up (setup_qcoro(COMPONENTS Core)).
class AbstractCli
{
public:
    virtual ~AbstractCli() = default;

    // -------------------------------------------------------------------------
    // Identity & capabilities
    // -------------------------------------------------------------------------

    virtual QString getName() const = 0;
    virtual QString getDescription() const = 0;

    // Whether this CLI can generate SVG markup.
    virtual bool canGenSvg() const = 0;

    // Whether this CLI can generate raster images.
    virtual bool canGenImages() const = 0;

    // Whether this CLI can generate a video from a text prompt.
    virtual bool canGenVideosFromText() const = 0;

    // Whether this CLI can generate a video from one or more images.
    virtual bool canGenVideoFromImages() const = 0;

    // -------------------------------------------------------------------------
    // Process configuration
    // -------------------------------------------------------------------------

    // Executable name looked up in PATH (e.g. "claude", "gemini").
    virtual QString getExecutable() const = 0;

    // Arguments passed when running a prompt. The prompt text is always
    // written to a temporary file and fed via setStandardInputFile().
    // Default: {"-p", "-"}
    virtual QStringList promptArgs() const;

    // Arguments used to verify the CLI is installed and optionally retrieve
    // version/account information. Default: {"--version"}
    virtual QStringList availabilityArgs() const;

    // Parse membership / plan info from the stdout of the availability check.
    // Default implementation returns an empty string.
    virtual QString parseMembership(const QString &versionOutput) const;

    // -------------------------------------------------------------------------
    // Coroutine-based API (co_await friendly)
    // -------------------------------------------------------------------------

    // Run prompt and return result. Prompt is passed via stdin.
    // workingDir: directory the CLI process runs in (empty = use a temp dir).
    // Set it to the config's files/ dir so the CLI can read input files by name.
    QCoro::Task<CliRunResult> runPrompt(const QString &prompt,
                                        const QString &workingDir = {}) const;

    // Check whether the CLI is installed and optionally retrieve membership.
    QCoro::Task<CliAvailability> checkAvailability() const;

    // -------------------------------------------------------------------------
    // Callback-based API (fire-and-forget; safe to use outside coroutines)
    // -------------------------------------------------------------------------

    // Starts a prompt run asynchronously. Invokes callback(result) on the Qt
    // event-loop thread when the process finishes. The callback is NOT called
    // if context is destroyed before the process completes.
    void runPromptAsync(const QString &prompt,
                        QObject *context,
                        std::function<void(CliRunResult)> callback) const;

    // Same as above for the availability check.
    void checkAvailabilityAsync(QObject *context,
                                std::function<void(CliAvailability)> callback) const;

    // -------------------------------------------------------------------------
    // Registry
    // -------------------------------------------------------------------------

    static const QList<AbstractCli *> &ALL_CLIS();

    // Used by DECLARE_CLI to register a subclass instance at startup.
    class Recorder
    {
    public:
        explicit Recorder(AbstractCli *cli);
    };

private:
    static QList<AbstractCli *> &getClis();
};

// Place this macro once, in the .cpp file of every AbstractCli subclass.
// It creates a static registry instance that is registered before main().
#define DECLARE_CLI(NEW_CLASS)                                   \
    NEW_CLASS instance##NEW_CLASS;                               \
    AbstractCli::Recorder recorder##NEW_CLASS{&instance##NEW_CLASS};

#endif // ABSTRACTCLI_H
