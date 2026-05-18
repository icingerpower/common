#include "CliClaude.h"

QString CliClaude::getName() const
{
    return QStringLiteral("Claude");
}

QString CliClaude::getDescription() const
{
    return QStringLiteral("Anthropic Claude Code CLI");
}

bool CliClaude::canGenSvg() const { return true; }
bool CliClaude::canGenImages() const { return false; }
bool CliClaude::canGenVideosFromText() const { return false; }
bool CliClaude::canGenVideoFromImages() const { return false; }

QString CliClaude::getExecutable() const
{
    return QStringLiteral("claude");
}

QStringList CliClaude::promptArgs() const
{
    return {
        QStringLiteral("-p"),
        QStringLiteral("-"),                         // read prompt from stdin
        QStringLiteral("--dangerously-skip-permissions"),
    };
}

QString CliClaude::parseMembership(const QString &versionOutput) const
{
    // `claude --version` currently outputs the version string only (e.g.
    // "claude/1.x.x linux-x64"). Membership information is not included.
    // To retrieve account/plan details, run: claude config list
    // and parse the "subscription" or "plan" key. Left as a future enhancement.
    Q_UNUSED(versionOutput)
    return {};
}

DECLARE_CLI(CliClaude)
