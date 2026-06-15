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

QStringList CliGemini::promptArgs() const
{
    // -p "" triggers non-interactive (headless) mode; stdin content is used as the prompt.
    // --skip-trust bypasses the trusted-directory check (needed for temp dirs).
    // -y auto-approves all tool use so the process never blocks on a confirmation.
    // --output-format text gives plain stdout with no wrapping JSON to strip.
    return {
        QStringLiteral("-p"), QStringLiteral(""),
        QStringLiteral("--skip-trust"),
        QStringLiteral("-y"),
        QStringLiteral("--output-format"), QStringLiteral("text"),
    };
}

DECLARE_CLI(CliGemini)
