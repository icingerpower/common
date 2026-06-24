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
    // '-' reads the prompt from stdin (fed via setStandardInputFile).
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

QString CliAntigravity::preparePrompt(const QString &prompt) const
{
    // Antigravity is an agentic CLI that explores the filesystem before acting.
    // Without an explicit directive it spends its entire --print-timeout browsing
    // files and running find/Python commands instead of generating the image.
    // Prepend a hard instruction so it calls generate_image immediately.
    return QStringLiteral(
        "IMPORTANT: Use your generate_image tool IMMEDIATELY to create the image described below. "
        "Do NOT list files, view existing images, run shell commands, or search for scripts first. "
        "Generate the image now, save it to the file path specified in the instructions, then stop.\n\n")
        + prompt;
}

DECLARE_CLI(CliAntigravity)
