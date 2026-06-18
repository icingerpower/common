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
    return QStringLiteral("antigravity");
}

QStringList CliAntigravity::promptArgs() const
{
    // 'chat' subcommand with ask mode: non-agentic, no tool use, plain response.
    // '-' reads the prompt from stdin (fed via setStandardInputFile).
    return {
        QStringLiteral("chat"),
        QStringLiteral("--mode"), QStringLiteral("ask"),
        QStringLiteral("-"),
    };
}

DECLARE_CLI(CliAntigravity)
