#ifndef CLICODEX_H
#define CLICODEX_H

#include "AbstractCli.h"

class CliCodex : public AbstractCli
{
public:
    QString getName() const override;
    QString getDescription() const override;
    bool canGenSvg() const override;
    bool canGenImages() const override;
    bool canGenVideosFromText() const override;
    bool canGenVideoFromImages() const override;
    QString getExecutable() const override;
};

#endif // CLICODEX_H
