#include "CliMistral.h"

QString CliMistral::getName() const
{
    return QStringLiteral("Mistral");
}

QString CliMistral::getDescription() const
{
    return QStringLiteral("Mistral AI CLI");
}

bool CliMistral::canGenSvg() const { return true; }
bool CliMistral::canGenImages() const { return false; }
bool CliMistral::canGenVideosFromText() const { return false; }
bool CliMistral::canGenVideoFromImages() const { return false; }

QString CliMistral::getExecutable() const
{
    return QStringLiteral("mistral");
}

// TODO: verify the Mistral CLI executable name and prompt invocation,
// then override promptArgs() accordingly.

DECLARE_CLI(CliMistral)
