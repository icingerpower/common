#ifndef DIALOGOPENSETTINGSFILE_H
#define DIALOGOPENSETTINGSFILE_H

#include <QDialog>

namespace Ui {
class DialogOpenSettingsFile;
}

class DialogOpenSettingsFile : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOpenSettingsFile(QWidget *parent = nullptr);
    ~DialogOpenSettingsFile();
    bool wasAccepted();
    QString getFilePath() const;

public slots:
    void createSettingsFile();
    void browseSettingsFile();
    void accept();
    void reject();

private:
    Ui::DialogOpenSettingsFile *ui;
    bool m_wasAccepted;
};

#endif // DIALOGOPENSETTINGSFILE_H
