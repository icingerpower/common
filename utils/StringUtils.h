#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <QString>

class StringUtils
{
public:
    StringUtils();
    /// Check if file name or content exists already
    static void trimString(QString &string);
};

#endif // STRINGUTILS_H
