#include <QRegularExpression>

#include "StringUtils.h"

//----------------------------------------
StringUtils::StringUtils()
{

}
//----------------------------------------
void StringUtils::trimString(QString &string)
{
    static QRegularExpression regEx("\\S");
    int start = string.indexOf(regEx);
    int end = string.lastIndexOf(regEx);
    if (end > -1 && end < string.size() - 1) {
        string.remove(end + 1, string.size() - end - 1);
    }
    if (start > 0) {
        string.remove(0, start);
    }
}
//----------------------------------------


