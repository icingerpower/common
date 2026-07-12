#include "CliAntigravityClaude.h"

QString CliAntigravityClaude::getName() const
{
    return QStringLiteral("Antigravity - Claude");
}

QString CliAntigravityClaude::getDescription() const
{
    return QStringLiteral("Antigravity AI CLI (Claude Sonnet backend)");
}

QStringList CliAntigravityClaude::promptArgs() const
{
    return CliAntigravity::promptArgs()
           << QStringLiteral("--model")
           << QStringLiteral("Claude Sonnet 4.6 (Thinking)");
}

DECLARE_CLI(CliAntigravityClaude)
