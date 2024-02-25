#ifndef DIALOGIMAGEAI_H
#define DIALOGIMAGEAI_H

#include <QDialog>
#include <QDir>

class TextToImage;

namespace Ui {
class DialogImageAI;
}

class DialogImageAI : public QDialog
{
    Q_OBJECT

public:
    explicit DialogImageAI(
            const QString &text,
            QWidget *parent = nullptr);
    ~DialogImageAI();
    void setCurrentImage(const QString &filePath);
    const QImage &getSelectedGenImage() const;
    const QList<QImage> &getGenImages() const;
    QString getImageFilePath() const;

    bool wasAccepted() const;

public slots:
    void generate();
    void browseExisting();
    void accept();
    void reject();

private:
    Ui::DialogImageAI *ui;
    QString m_imageFilePath;
    static bool *_deleted();
    bool m_generated;
    bool m_generating;
    TextToImage *m_textToImage;
    QDir m_imageDir;
    QList<QImage> m_images;
    QString m_textForImage;
    bool m_wasAccepted;
};

#endif // DIALOGIMAGEAI_H
