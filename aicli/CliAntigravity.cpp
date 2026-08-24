#include "CliAntigravity.h"

QString CliAntigravity::getName() const
{
    return QStringLiteral("Antigravity");
}

QString CliAntigravity::getDescription() const
{
    return QStringLiteral("Antigravity AI CLI");
}

bool CliAntigravity::canGenSvg() const { return true; }
bool CliAntigravity::canGenImages() const { return true; }
bool CliAntigravity::canGenVideosFromText() const { return true; }
bool CliAntigravity::canGenVideoFromImages() const { return true; }

QString CliAntigravity::getExecutable() const
{
    return QStringLiteral("agy");
}

QStringList CliAntigravity::promptArgs() const
{
    // --print runs a single prompt non-interactively and prints the response.
    // It is a Go string flag: its VALUE is the prompt text.  agy has no stdin
    // prompt mode, so the '-' below is only the placeholder that
    // configurePromptProcess() replaces with the prompt text (promptViaStdin()
    // returns false).
    // --dangerously-skip-permissions auto-approves tool requests so the process
    // never blocks on a confirmation prompt.
    // --print-timeout 20m: the default 5 min is too short when the model needs
    // to view several reference images before generating output.
    return {
        QStringLiteral("--print"),
        QStringLiteral("-"),
        QStringLiteral("--dangerously-skip-permissions"),
        QStringLiteral("--print-timeout"),
        QStringLiteral("20m"),
    };
}

bool CliAntigravity::promptViaStdin() const
{
    return false;
}

QString CliAntigravity::preparePrompt(const QString &prompt) const
{
    // Antigravity is an agentic CLI that explores the filesystem before acting.
    // Branch on prompt type so it never receives the wrong instruction.
    //
    // Image/SVG prompts always contain "Image filename" (from buildSvgPrompt).
    // All other prompts are text (article generation, update, translation) and
    // only need a "start immediately" guard — not a generate_image directive.
    if (prompt.contains(QStringLiteral("Image filename"))) {
        return QStringLiteral(
            "IMPORTANT: Use your generate_image tool IMMEDIATELY to create the image described below. "
            "Do NOT list files, view existing images, run shell commands, or search for scripts first. "
            "Generate the image now, save it to the file path specified in the instructions, then stop.\n\n")
            + prompt;
    }
    // Text prompt — prevent agentic filesystem exploration, output immediately.
    return QStringLiteral(
        "IMPORTANT: Output your response as plain text IMMEDIATELY. "
        "Do NOT use any tools, list files, run shell commands, or explore the filesystem. "
        "Start your response now.\n\n")
        + prompt;
}

CliErrorKind CliAntigravity::classifyError(const QString &errorOutput) const
{
    // Exact strings observed live: "Authentication required. Please visit the
    // URL to log in:", "Individual quota reached. Please upgrade your
    // subscription...", "Eligibility check failed: RESOURCE_EXHAUSTED (code
    // 429): ...".
    if (errorOutput.contains(QStringLiteral("Authentication required"), Qt::CaseInsensitive)) {
        return CliErrorKind::AuthRequired;
    }
    if (errorOutput.contains(QStringLiteral("quota"), Qt::CaseInsensitive)
            || errorOutput.contains(QStringLiteral("RESOURCE_EXHAUSTED"))) {
        return CliErrorKind::QuotaExceeded;
    }
    return CliErrorKind::Other;
}

DECLARE_CLI(CliAntigravity)
