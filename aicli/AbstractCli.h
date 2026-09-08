#ifndef ABSTRACTCLI_H
#define ABSTRACTCLI_H

#include <functional>

#include <QList>
#include <QString>
#include <QStringList>

#include <QCoro/QCoroTask>

class QProcess;

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

// Coarse classification of a failed CLI invocation, derived from its stderr
// text. Lets callers react to recoverable, human-fixable conditions (rate
// limits, expired login) differently from an ordinary failure, without the
// caller having to know each CLI's specific error strings.
enum class CliErrorKind {
    Other,          // no special classification — treat as an ordinary failure.
    QuotaExceeded,  // rate limit / subscription quota hit; typically resets shortly.
    AuthRequired,   // login session expired; needs a human to re-authenticate.
};

// Marker line that a raster-image generation prompt (built by
// GenPageQueue::buildRasterImagePrompt()) includes so CLI-specific
// preparePrompt() overrides can detect "this prompt wants a real image file
// written to disk" without colliding with the SVG prompt, which also mentions
// an "Image filename" but must never trigger a file-write directive (it
// explicitly requires text-only output). Keep this literal in sync between
// the prompt builder and every preparePrompt() override that checks for it.
static constexpr const char *RASTER_IMAGE_PROMPT_MARKER = "Task type         : RASTER_IMAGE_GENERATION";

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

    // Arguments passed when running a prompt via AbstractCli::runPrompt[Async].
    // The '-' entry is a placeholder: for stdin-based CLIs it is passed through
    // verbatim ("read prompt from stdin"); for argv-based CLIs (see
    // promptViaStdin()) configurePromptProcess() replaces it with the prompt
    // text itself.  Default: {"-p", "-"}
    virtual QStringList promptArgs() const;

    // Whether the CLI reads the prompt from stdin when given the '-' placeholder
    // (Claude Code convention).  Default: true.
    // Override to return false for CLIs whose prompt flag consumes the prompt
    // text as its argument value and that ignore stdin entirely.  Antigravity's
    // `agy` is one: `--print` is a Go string flag, so `--print -` makes the model
    // answer the literal prompt "-" ("your request was blank") while the real
    // prompt fed via setStandardInputFile() is never read.
    virtual bool promptViaStdin() const;

    // Configure how `process` receives the prompt.  `baseArgs` is the argv
    // template (promptArgs() or translationPromptArgs()), `prompt` is the exact
    // text that was written to `promptPath`.
    //   - stdin CLIs: arguments set unchanged, promptPath fed via
    //     setStandardInputFile().
    //   - argv CLIs (promptViaStdin() == false): the '-' placeholder is replaced
    //     by the prompt text and stdin is bound to the null device.
    // Caveat for argv CLIs: Linux caps one argv entry at 128 KB (MAX_ARG_STRLEN);
    // a larger prompt makes the process fail to start.  There is no workaround
    // for CLIs without stdin support — callers see FailedToStart.
    void configurePromptProcess(QProcess *process,
                                const QStringList &baseArgs,
                                const QString &prompt,
                                const QString &promptPath) const;

    // Arguments used by PageTranslator when launching a translation process.
    // May differ from promptArgs() when the CLI needs special flags (e.g. streaming
    // JSON output format for Claude).  Default: same as promptArgs().
    virtual QStringList translationPromptArgs() const;

    // Extract the final translated text from raw process stdout.
    // Default: return the output trimmed (plain-text CLIs).
    // Claude override: parse stream-json event stream.
    virtual QString extractTextFromOutput(const QByteArray &rawOutput) const;

    // Arguments used to verify the CLI is installed and optionally retrieve
    // version/account information. Default: {"--version"}
    virtual QStringList availabilityArgs() const;

    // Parse membership / plan info from the stdout of the availability check.
    // Default implementation returns an empty string.
    virtual QString parseMembership(const QString &versionOutput) const;

    // Optionally transform the prompt before it is written to the temp file and
    // fed to the CLI process.  Default: return the prompt unchanged.
    // Override to inject CLI-specific preambles (e.g. force Antigravity to call
    // generate_image immediately instead of exploring the filesystem first).
    virtual QString preparePrompt(const QString &prompt) const;

    // Classify a failed process's stderr text (see CliErrorKind). Default:
    // always Other — this CLI has no known recoverable-error strings.
    // Override for CLIs with distinctive quota/auth error text so callers can
    // offer a "fix it and retry" path instead of treating every failure the
    // same way.
    virtual CliErrorKind classifyError(const QString &errorOutput) const;

    // Extra directories where a file-writing tool call for this CLI can land
    // instead of workingDir, when the CLI's own idea of "current directory"
    // does not track the process's actual one. Default: none (assume the
    // CLI's file tools honor workingDir). Override for CLIs known to fall
    // back to a fixed internal location — see CliAntigravity, whose
    // generate_image/file tools were observed writing into a persistent
    // `~/.gemini/antigravity-cli/scratch/` directory regardless of the
    // process's working directory, most likely because the file tool
    // resolves against its own "project"/session concept rather than the
    // literal OS cwd. Callers that expect a specific output file should
    // check these directories too before concluding the CLI produced
    // nothing.
    virtual QStringList outputFallbackDirs() const;

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

    // Overload that forwards a working directory to the CLI process.
    void runPromptAsync(const QString &prompt,
                        const QString &workingDir,
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
