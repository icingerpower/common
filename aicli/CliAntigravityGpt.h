#ifndef CLIANTIGRAVITYGPT_H
#define CLIANTIGRAVITYGPT_H

#include "CliAntigravity.h"

class CliAntigravityGpt : public CliAntigravity
{
public:
    QString getName() const override;
    QString getDescription() const override;
    QStringList promptArgs() const override;
};

#endif // CLIANTIGRAVITYGPT_H
