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
};

#endif // CLIANTIGRAVITY_H
