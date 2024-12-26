#ifndef DIALOGPRETRANSLATIONCHECK_H
#define DIALOGPRETRANSLATIONCHECK_H

#include <QDialog>

namespace Ui {
class DialogPreTranslationCheck;
}

class TranslateTableModel;

class DialogPreTranslationCheck : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPreTranslationCheck(
            TranslateTableModel *translationTableModel, QWidget *parent = nullptr);
    ~DialogPreTranslationCheck();

public slots:
    void paste();
    void accept() override;

signals:
    void uploadAsked();

private:
    Ui::DialogPreTranslationCheck *ui;
    void _connectSlots();
};

#endif // DIALOGPRETRANSLATIONCHECK_H
