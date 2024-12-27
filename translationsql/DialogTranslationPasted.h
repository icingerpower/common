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
            const QString &imageFilePath,
            QWidget *parent = nullptr);
    ~DialogTranslationPasted();
    QString getPrompt() const;

public slots:
    void copyPrompt();
    void accept() override;

private:
    Ui::DialogTranslationPasted *ui;
    QString m_imageFilePath;
};

#endif // DIALOGTRANSLATIONPASTED_H
