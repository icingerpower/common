#include "CliAntigravityGpt.h"

QString CliAntigravityGpt::getName() const
{
    return QStringLiteral("Antigravity - GPT");
}

QString CliAntigravityGpt::getDescription() const
{
    return QStringLiteral("Antigravity AI CLI (GPT-OSS backend)");
}

QStringList CliAntigravityGpt::promptArgs() const
{
    return CliAntigravity::promptArgs()
           << QStringLiteral("--model")
           << QStringLiteral("GPT-OSS 120B (Medium)");
}

DECLARE_CLI(CliAntigravityGpt)
