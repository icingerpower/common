#ifndef CLIGEMINI_H
#define CLIGEMINI_H

#include "AbstractCli.h"

class CliGemini : public AbstractCli
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

#endif // CLIGEMINI_H
