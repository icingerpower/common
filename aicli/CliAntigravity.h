#ifndef CLIANTIGRAVITY_H
#define CLIANTIGRAVITY_H

#include "AbstractCli.h"

class CliAntigravity : public AbstractCli
{
public:
    explicit CliAntigravity(QString modelName = {}, QString displayName = {});

    QString getName() const override;
    QString getDescription() const override;
    QString getModel() const override;
    bool canGenSvg() const override;
    bool canGenImages() const override;
    bool canGenVideosFromText() const override;
    bool canGenVideoFromImages() const override;
    // The one CLI with a live web-search measurement (a 31-day daily-close
    // lookup answered in 1 m 52 s as one flat JSON object).
    bool canWebSearch() const override;
    QString getExecutable() const override;
    QStringList promptArgs() const override;
    // false: agy has no stdin prompt mode — `--print` takes the prompt text as
    // its flag value.  `--print -` would make the model answer the literal
    // prompt "-" while stdin is silently ignored.
    bool promptViaStdin() const override;
    QString preparePrompt(const QString &prompt) const override;
    CliErrorKind classifyError(const QString &errorOutput) const override;
    QStringList outputFallbackDirs() const override;

private:
    QString m_modelName;
    QString m_displayName;
};

#endif // CLIANTIGRAVITY_H
