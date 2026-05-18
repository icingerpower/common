#ifndef CLIDEEPSEEK_H
#define CLIDEEPSEEK_H

#include "AbstractCli.h"

class CliDeepseek : public AbstractCli
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

#endif // CLIDEEPSEEK_H
