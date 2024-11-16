#ifndef DIALOGSELECTLANGAGE_H
#define DIALOGSELECTLANGAGE_H

#include <QDialog>

namespace Ui {
class DialogSelectLangage;
}

class DialogSelectLangage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectLangage(QWidget *parent = nullptr);
    ~DialogSelectLangage();

    QString getSelLangCode() const;

private:
    Ui::DialogSelectLangage *ui;
    QHash<QString, QString> m_languagesToCode;
};

#endif // DIALOGSELECTLANGAGE_H
