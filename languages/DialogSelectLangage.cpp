#include <QSettings>
#include <QLocale>

#include "LangManager.h"

#include "DialogSelectLangage.h"
#include "ui_DialogSelectLangage.h"

DialogSelectLangage::DialogSelectLangage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelectLangage)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    QLocale systemLocale = QLocale::system();
    const QString &langCodeLocale = QLocale::languageToCode(systemLocale.language());
    LangManager langManager;
    int indexToSel = -1;
    const auto &langCodes = langManager.langCodesTo();
    QString languageNameToSel;
    if (langCodes.size() > 0)
    {
        for (int i=0; i<langCodes.size(); ++i)
        {
            auto language = QLocale::codeToLanguage(langCodes[i]);
            const auto &languageName = QLocale::languageToString(language).trimmed();
            if (langCodes[i].compare(langCodeLocale, Qt::CaseInsensitive) == 0)
            {
                languageNameToSel = languageName;
            }
            else if (langCodes[i].compare("en", Qt::CaseInsensitive) == 0 && indexToSel == -1)
            {
                languageNameToSel = languageName;
            }
            auto item = new QListWidgetItem{languageName};
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setData(Qt::TextAlignmentRole, Qt::AlignCenter);
            ui->listWidgetLangs->addItem(item);
            auto font = ui->listWidgetLangs->font();
            font.setPixelSize(30);
            ui->listWidgetLangs->setFont(font);
            m_languagesToCode[languageName] = langCodes[i];
        }
        if (!languageNameToSel.isEmpty())
        {
            for (int i=0; i<langCodes.size(); ++i)
            {
                if (ui->listWidgetLangs->item(i)->text() == languageNameToSel)
                {
                    indexToSel = i;
                    ui->listWidgetLangs->setCurrentRow(indexToSel);
                    break;
                }
            }
        }
    }
}

DialogSelectLangage::~DialogSelectLangage()
{
    delete ui;
}

QString DialogSelectLangage::getSelLangCode() const
{
    const auto &selItems = ui->listWidgetLangs->selectedItems();
    if (selItems.size() > 0)
    {
        const QString &languageName = selItems.first()->text();
        return m_languagesToCode[languageName];
    }
    return QString{};
}
