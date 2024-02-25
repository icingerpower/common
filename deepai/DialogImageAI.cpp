#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QSettings>
#include <QFile>
#include <QPixmap>

#include "../common/deepai/TextToImage.h"

#include "DialogImageAI.h"
#include "ui_DialogImageAI.h"

//----------------------------------------------
DialogImageAI::DialogImageAI(
        const QString &text,
        QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogImageAI)
{
    ui->setupUi(this);
    m_textForImage = text;
    ui->lineEditText->setText(text);
    *_deleted() = false;
    m_textToImage = new TextToImage();
    m_generated = false;
    m_generating = false;
    m_wasAccepted = true;
    ui->toolBox->setItemEnabled(0, false);
    //ui->pageExisting->setVisible(false);
    connect(ui->buttonGenerate,
            &QPushButton::clicked,
            this,
            &DialogImageAI::generate);
    connect(ui->buttonBrowseImage,
            &QPushButton::clicked,
            this,
            &DialogImageAI::browseExisting);
}
//----------------------------------------------
DialogImageAI::~DialogImageAI()
{
    *_deleted() = true;
    delete ui;
    delete m_textToImage;
}
//----------------------------------------------
void DialogImageAI::setCurrentImage(
        const QString &filePath)
{
    if (QFile::exists(filePath)) {
        //m_imageFileName = QFileInfo(filePath).fileName();
        ui->toolBox->setItemEnabled(0, true);
        m_imageFilePath = filePath;
        QPixmap pixmap(m_imageFilePath);
        ui->labelCurrentPixmap->setPixmap(pixmap);
        ui->toolBox->setCurrentIndex(0);
    }
}
//----------------------------------------------
const QImage &DialogImageAI::getSelectedGenImage() const
{
    int indSel = 0;
    if (ui->radioImage2->isChecked()) {
        indSel = 1;
    } else if (ui->radioImage3->isChecked()) {
        indSel = 2;
    } else if (ui->radioImage4->isChecked()) {
        indSel = 3;
    }
    indSel = qMin(indSel, m_images.size()-1);
    return m_images[indSel];
}
//----------------------------------------------
const QList<QImage> &DialogImageAI::getGenImages() const
{
    return m_images;
}
//----------------------------------------------
QString DialogImageAI::getImageFilePath() const
{
    return m_imageFilePath;
}
//----------------------------------------------
bool DialogImageAI::wasAccepted() const
{
    return m_wasAccepted;
}
//----------------------------------------------
void DialogImageAI::generate()
{
    QString text = ui->lineEditText->text();
    if (text.size() < 2) {
        QMessageBox::information(
                    this,
                    tr("No text"),
                    tr("You need to enter a text for image generation."));
    } else {
        m_generating = true;
        //setCursor(Qt::WaitCursor);
        //*
        auto *progressDialog = new QProgressDialog(this);
        progressDialog->setCancelButton(nullptr);
        progressDialog->setWindowTitle(("Loading"));
        progressDialog->setLabelText(
                    tr("Images are being generated. It can take up to one minute."));
        progressDialog->setRange(0, 0);
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->setAutoClose(true);
        progressDialog->show();
        //*/

        //auto *timer * new QTim
        m_textToImage->retrieveImage(
                    text,
                    [this, progressDialog](const QList<QImage> &images){
            if (!*_deleted()) {
                if (isVisible()) {
                    progressDialog->close();
                }
                m_images = images;
                QList<QLabel *> labels;
                labels << ui->labelPixmap1;
                labels << ui->labelPixmap2;
                labels << ui->labelPixmap3;
                labels << ui->labelPixmap4;
                for (int i=0; i<labels.size(); ++i) {
                    QPixmap pixmap = QPixmap::fromImage(images[i]);
                    labels[i]->setPixmap(pixmap);
                }
                m_generated = true;
                m_generating = false;
                m_wasAccepted = true;
            }
        }, [this, progressDialog](const QString &error){
            if (!*_deleted()) {
                if (isVisible()) {
                    progressDialog->close();
                }
                m_generating = false;
                QMessageBox::warning(
                            this,
                            tr("Error generating image"),
                            error);
            }
        }
        , "78a1f59c-3973-4eb0-92d7-9df64b79eaa5"
        );
    }
}
//----------------------------------------------
void DialogImageAI::browseExisting()
{
    QSettings settings;
    static QString key = "DialogImage::browseNewImage";
    QString lastDir = settings.value(key, QString()).toString();
    QString filePath = QFileDialog::getOpenFileName(
                this, tr("Choose an image"), lastDir);
    if (!filePath.isEmpty()) {
        settings.setValue(key, QFileInfo(filePath).dir().path());
        QPixmap imageQt(filePath);
        ui->labelBrowsedPixmap->setPixmap(imageQt);
        ui->lineEditImage->setText(filePath);
        QString sizeText = QString::number(imageQt.width())
                + " - " + QString::number(imageQt.height());
        ui->labelImageInfo->setText(sizeText);
    }
}
//----------------------------------------------
void DialogImageAI::accept()
{
    if (m_generating) {
        QMessageBox::information(
                    this,
                    tr("Generation"),
                    tr("Image generation in progress."));
    } else if (!m_generated && ui->lineEditImage->text().isEmpty()) {
        QMessageBox::information(
                    this,
                    tr("No image"),
                    tr("You need to generate an image."));
    } else if (m_generated) {
        bool imageChecked1 = ui->radioImage1->isChecked();
        bool imageChecked2 = ui->radioImage2->isChecked();
        bool imageChecked3 = ui->radioImage3->isChecked();
        bool imageChecked4 = ui->radioImage4->isChecked();
        if (imageChecked1 || imageChecked2 || imageChecked3 || imageChecked4) {
            auto imageSel = getSelectedGenImage();
            QString tempImageFileName = "temp-dialog-empire-image.jpg";
            imageSel.save(tempImageFileName);
            QString altText = m_textForImage;
            if (altText.isEmpty()) {
                altText = ui->lineEditText->text();
            }
            m_imageFilePath = tempImageFileName;
            /*
            m_imageId = "food-"
                    + Link::titleToPermalink(altText)
                    + "-" + QDateTime::currentDateTime().toString(
                        "yyyymmdd-hhmmsszzz");
            QString tempFilePath = QDir().absoluteFilePath(tempImageFileName);
            Image image(m_imageId,
                        tempFilePath,
                        imageSel.width(),
                        imageSel.height(),
                        altText,
                        altText,
                        m_imageDir);
            image.copyToInternalDirIfNotInternal();
                        //*/
            QDialog::accept();
        } else {
            QMessageBox::information(
                        this,
                        tr("No image selected"),
                        tr("You need to select a generated image."));
        }
    } else if (!ui->lineEditImage->text().isEmpty()) {
        /*
        QString altText = m_textForImage;
        if (altText.isEmpty()) {
            altText = ui->lineEditText->text();
        }
        m_imageId = "foodload-"
                + Link::titleToPermalink(altText)
                + "-" + QDateTime::currentDateTime().toString(
                    "yyyymmdd-hhmmsszzz");
        QString imageFilePath = ui->lineEditImage->text();
        QImage imageBrowsed(imageFilePath);
        Image image(m_imageId,
                    imageFilePath,
                    imageBrowsed.width(),
                    imageBrowsed.height(),
                    altText,
                    altText,
                    m_imageDir);
        image.copyToInternalDirIfNotInternal();
                    //*/
        m_imageFilePath = ui->lineEditImage->text();
        QDialog::accept();
    }

}
//----------------------------------------------
void DialogImageAI::reject()
{
    if (m_generating) {
        QMessageBox::information(
                    this,
                    tr("Generation in progress"),
                    tr("You need to wait the generation to be finished before closing"));
    } else {
        m_generated = false;
        m_wasAccepted = false;
        //m_imageId.clear();
        ui->lineEditText->clear();
        ui->radioImage1->setChecked(false);
        ui->radioImage2->setChecked(false);
        ui->radioImage3->setChecked(false);
        ui->radioImage4->setChecked(false);
        QDialog::reject();
    }
}
//----------------------------------------------
bool *DialogImageAI::_deleted()
{
    static bool deleted = false;
    return &deleted;
}
//----------------------------------------------
