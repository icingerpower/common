#include "CliDeepseek.h"

QString CliDeepseek::getName() const
{
    return QStringLiteral("DeepSeek");
}

QString CliDeepseek::getDescription() const
{
    return QStringLiteral("DeepSeek CLI");
}

bool CliDeepseek::canGenSvg() const { return true; }
bool CliDeepseek::canGenImages() const { return false; }
bool CliDeepseek::canGenVideosFromText() const { return false; }
bool CliDeepseek::canGenVideoFromImages() const { return false; }

QString CliDeepseek::getExecutable() const
{
    return QStringLiteral("deepseek");
}

// TODO: verify the DeepSeek CLI executable name and prompt invocation,
// then override promptArgs() accordingly.

DECLARE_CLI(CliDeepseek)
