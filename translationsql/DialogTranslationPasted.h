#ifndef DIALOGTRANSLATIONPASTED_H
#define DIALOGTRANSLATIONPASTED_H

#include <QDialog>

namespace Ui {
class DialogTranslationPasted;
}

class TranslateTableModel;

class DialogTranslationPasted : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTranslationPasted(
            TranslateTableModel *translateTableModel,
            QWidget *parent = nullptr);
    ~DialogTranslationPasted();
    QString getPrompt() const;

public slots:
    void copyPrompt();

private:
    Ui::DialogTranslationPasted *ui;
};

#endif // DIALOGTRANSLATIONPASTED_H
