#include "CliGemini.h"

QString CliGemini::getName() const
{
    return QStringLiteral("Gemini");
}

QString CliGemini::getDescription() const
{
    return QStringLiteral("Google Gemini CLI");
}

bool CliGemini::canGenSvg() const { return true; }
// Gemini has access to Imagen; update if the CLI exposes image generation.
bool CliGemini::canGenImages() const { return false; }
bool CliGemini::canGenVideosFromText() const { return false; }
bool CliGemini::canGenVideoFromImages() const { return false; }

QString CliGemini::getExecutable() const
{
    return QStringLiteral("gemini");
}

// TODO: verify prompt invocation for the Gemini CLI and override promptArgs().

DECLARE_CLI(CliGemini)
