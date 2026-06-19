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
bool CliAntigravity::canGenImages() const { return false; }
bool CliAntigravity::canGenVideosFromText() const { return false; }
bool CliAntigravity::canGenVideoFromImages() const { return false; }

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
    return {
        QStringLiteral("--print"),
        QStringLiteral("-"),
        QStringLiteral("--dangerously-skip-permissions"),
    };
}

DECLARE_CLI(CliAntigravity)
