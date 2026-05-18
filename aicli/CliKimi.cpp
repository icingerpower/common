#include "CliKimi.h"

QString CliKimi::getName() const
{
    return QStringLiteral("Kimi");
}

QString CliKimi::getDescription() const
{
    return QStringLiteral("Moonshot Kimi CLI");
}

bool CliKimi::canGenSvg() const { return true; }
bool CliKimi::canGenImages() const { return false; }
bool CliKimi::canGenVideosFromText() const { return false; }
bool CliKimi::canGenVideoFromImages() const { return false; }

QString CliKimi::getExecutable() const
{
    return QStringLiteral("kimi");
}

// TODO: verify the Kimi CLI executable name and prompt invocation,
// then override promptArgs() accordingly.

DECLARE_CLI(CliKimi)
