#ifndef DIALOGPICKLANGUAGE_H
#define DIALOGPICKLANGUAGE_H

#include <QDialog>

namespace Ui {
class DialogPickLanguage;
}

class DialogPickLanguage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPickLanguage(QWidget *parent = nullptr);
    ~DialogPickLanguage();

    QString getLangCode() const;
    QString getLangName() const;

public slots:
    void filter();
    void filterReset();

private:
    Ui::DialogPickLanguage *ui;
    void _connectSlots();
    void _loadLangages();
};

#endif // DIALOGPICKLANGUAGE_H
