#ifndef CLIANTIGRAVITYCLAUDE_H
#define CLIANTIGRAVITYCLAUDE_H

#include "CliAntigravity.h"

class CliAntigravityClaude : public CliAntigravity
{
public:
    QString getName() const override;
    QString getDescription() const override;
    QStringList promptArgs() const override;
};

#endif // CLIANTIGRAVITYCLAUDE_H
