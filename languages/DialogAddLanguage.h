#ifndef DIALOGAddLANGUAGE_H
#define DIALOGAddLANGUAGE_H

#include <QDialog>

namespace Ui {
class DialogAddLanguage;
}

class DialogAddLanguage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddLanguage(QWidget *parent = nullptr);
    ~DialogAddLanguage();

    QString getLangCode() const;
    QString getLangName() const;

public slots:
    void filter();
    void filterReset();

private:
    Ui::DialogAddLanguage *ui;
    void _connectSlots();
    void _loadLanguages();
};

#endif // DIALOGAddLANGUAGE_H
