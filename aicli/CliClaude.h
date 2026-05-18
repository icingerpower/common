#ifndef CLICLAUDE_H
#define CLICLAUDE_H

#include "AbstractCli.h"

class CliClaude : public AbstractCli
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
    QString parseMembership(const QString &versionOutput) const override;
};

#endif // CLICLAUDE_H
