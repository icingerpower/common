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

// TODO: verify prompt invocation for the Codex CLI and override promptArgs().

DECLARE_CLI(CliCodex)
