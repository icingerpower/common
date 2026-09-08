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

CliErrorKind CliCodex::classifyError(const QString &errorOutput) const
{
    // Exact strings observed live (codex_login::auth::manager): "Failed to
    // refresh token: 401 Unauthorized: ... refresh_token_expired ... Please
    // log out and sign in again." — the CLI manages its own OAuth session
    // and needs a human to run `codex login` again.
    if (errorOutput.contains(QStringLiteral("refresh_token_expired"))
        || errorOutput.contains(QStringLiteral("Please try signing in again"))
        || errorOutput.contains(QStringLiteral("log out and sign in"), Qt::CaseInsensitive))
    {
        return CliErrorKind::AuthRequired;
    }
    if (errorOutput.contains(QStringLiteral("rate limit"), Qt::CaseInsensitive)
        || errorOutput.contains(QStringLiteral("usage limit"), Qt::CaseInsensitive)
        || errorOutput.contains(QStringLiteral("429")))
    {
        return CliErrorKind::QuotaExceeded;
    }
    return CliErrorKind::Other;
}

DECLARE_CLI(CliCodex)
