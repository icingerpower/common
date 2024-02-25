#ifndef DIALOGOPENCONIFG_H
#define DIALOGOPENCONIFG_H

#include <QDialog>

namespace Ui {
class DialogOpenConifg;
}

class DialogOpenConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOpenConfig(QWidget *parent = nullptr);
    ~DialogOpenConfig();
    bool wasRejected() const;

public slots:
    void clearRecentSelected();
    void openRecent();
    void browseAndOpen();
    void openAndAccept();
    void accept() override;
    void reject() override;

private:
    Ui::DialogOpenConifg *ui;
    void _connectSlots();
    bool m_rejected;
    bool m_opening;
};

#endif // DIALOGOPENCONIFG_H
