#include "CliClaude.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

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

QStringList CliClaude::translationPromptArgs() const
{
    return {
        QStringLiteral("-p"),
        QStringLiteral("-"),
        QStringLiteral("--dangerously-skip-permissions"),
        QStringLiteral("--no-session-persistence"),
        QStringLiteral("--tools"), QStringLiteral(""),
        QStringLiteral("--output-format"), QStringLiteral("stream-json"),
        QStringLiteral("--verbose"),
    };
}

QString CliClaude::extractTextFromOutput(const QByteArray &rawOutput) const
{
    // Output is stream-json: one JSON object per line.
    // Collect text from two sources and use whichever is longer:
    //   {"type":"result","result":"..."}  — final compiled response (may be tail-truncated)
    //   {"type":"assistant","message":{"content":[{"type":"text","text":"..."}]}}  — full turn
    QString resultText;
    QString assistantText;

    for (const QByteArray &line : rawOutput.split('\n')) {
        const QByteArray trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(trimmed);
        if (!doc.isObject()) {
            continue;
        }
        const QJsonObject obj = doc.object();
        const QString type = obj.value(QStringLiteral("type")).toString();

        if (type == QStringLiteral("result")) {
            resultText = obj.value(QStringLiteral("result")).toString().trimmed();
            break;
        }

        if (type == QStringLiteral("assistant")) {
            const QJsonArray content = obj.value(QStringLiteral("message"))
                                           .toObject()
                                           .value(QStringLiteral("content"))
                                           .toArray();
            for (const QJsonValue &block : std::as_const(content)) {
                const QJsonObject blk = block.toObject();
                if (blk.value(QStringLiteral("type")).toString() == QStringLiteral("text")) {
                    const QString t = blk.value(QStringLiteral("text")).toString().trimmed();
                    if (!t.isEmpty()) {
                        if (!assistantText.isEmpty()) {
                            assistantText += QLatin1Char('\n');
                        }
                        assistantText += t;
                    }
                }
            }
        }
    }

    return (assistantText.size() > resultText.size()) ? assistantText : resultText;
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
