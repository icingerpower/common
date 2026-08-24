#ifndef CLIANTIGRAVITY_H
#define CLIANTIGRAVITY_H

#include "AbstractCli.h"

class CliAntigravity : public AbstractCli
{
public:
    QString getName() const override;
    QString getDescription() const override;
    bool canGenSvg() const override;
    bool canGenImages() const override;
    bool canGenVideosFromText() const override;
    bool canGenVideoFromImages() const override;
    QString getExecutable() const override;
    QStringList promptArgs() const override;
    // false: agy has no stdin prompt mode — `--print` takes the prompt text as
    // its flag value.  `--print -` would make the model answer the literal
    // prompt "-" while stdin is silently ignored.
    bool promptViaStdin() const override;
    QString preparePrompt(const QString &prompt) const override;
    CliErrorKind classifyError(const QString &errorOutput) const override;
};

#endif // CLIANTIGRAVITY_H
