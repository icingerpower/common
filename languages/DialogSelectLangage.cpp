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
    if (langCodes.size() > 0)
    {
        for (int i=0; i<langCodes.size(); ++i)
        {
            if (langCodes[i].compare(langCodeLocale, Qt::CaseInsensitive) == 0)
            {
                indexToSel = i;
            }
            else if (langCodes[i].compare("en", Qt::CaseInsensitive) == 0 && indexToSel == -1)
            {
                indexToSel = i;
            }
            auto language = QLocale::codeToLanguage(langCodes[i]);
            const auto &languageName = QLocale::languageToString(language).trimmed();
            auto item = new QListWidgetItem{languageName};
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setData(Qt::TextAlignmentRole, Qt::AlignCenter);
            ui->listWidgetLangs->addItem(item);
            auto font = ui->listWidgetLangs->font();
            font.setPixelSize(30);
            ui->listWidgetLangs->setFont(font);
            m_languagesToCode[languageName] = langCodes[i];
        }
        if (indexToSel > -1)
        {
            ui->listWidgetLangs->setCurrentRow(indexToSel);
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
