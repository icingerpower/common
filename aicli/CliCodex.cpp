#include "CliCodex.h"

QString CliCodex::getName() const
{
    return QStringLiteral("Codex");
}

QString CliCodex::getDescription() const
{
    return QStringLiteral("OpenAI Codex CLI");
}

bool CliCodex::canGenSvg() const { return true; }
bool CliCodex::canGenImages() const { return true; }
bool CliCodex::canGenVideosFromText() const { return false; }
bool CliCodex::canGenVideoFromImages() const { return false; }

QString CliCodex::getExecutable() const
{
    return QStringLiteral("codex");
}

QStringList CliCodex::promptArgs() const
{
    // codex requires the `exec` subcommand for non-interactive use.
    // `-` tells it to read the prompt from stdin (same pipe as Claude's `-p -`).
    // --dangerously-bypass-approvals-and-sandbox grants full file-system access
    // so Codex can create image files and other outputs in the working directory.
    return {
        QStringLiteral("exec"),
        QStringLiteral("--dangerously-bypass-approvals-and-sandbox"),
        QStringLiteral("-"),
    };
}

DECLARE_CLI(CliCodex)
