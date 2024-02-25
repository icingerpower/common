#include <QAudioOutput>

#include "SoundPlayer.h"

//----------------------------------------
SoundPlayer *SoundPlayer::instance()
{
    static SoundPlayer instance;
    return &instance;
}
//----------------------------------------
SoundPlayer::SoundPlayer()
{
    m_audioOutput = nullptr;
    auto availableVoices = m_textToSpeechDefault.availableVoices();
    m_textToSpeechDefault.setLocale(QLocale("fr_FR"));
    for (auto itVoice = availableVoices.begin();
         itVoice != availableVoices.end(); ++itVoice) {
        auto language = itVoice->locale().language();
        QString langCode = QLocale::languageToCode(language);
        if (!m_textToSpeechByLangCode.contains(langCode)) {
            m_textToSpeechByLangCode[langCode] = new QTextToSpeech;
        } else if (itVoice->age() == QVoice::Teenager
                && itVoice->gender() == QVoice::Female) {
            m_textToSpeechByLangCode[langCode]->setVoice(*itVoice);
        } else if (itVoice->gender() == QVoice::Female
                   && m_textToSpeechByLangCode[langCode]->voice().gender() != QVoice::Female) {
            m_textToSpeechByLangCode[langCode]->setVoice(*itVoice);
        } else if (itVoice->age() == QVoice::Teenager
                   && m_textToSpeechByLangCode[langCode]->voice().age() != QVoice::Teenager
                   && m_textToSpeechByLangCode[langCode]->voice().gender() != QVoice::Female) {
            m_textToSpeechByLangCode[langCode]->setVoice(*itVoice);
        }
    }
}
//----------------------------------------
void SoundPlayer::initLangCodes(const QStringList &langCodes)
{
    QList<QLocale> allLocales
            = QLocale::matchingLocales(
                QLocale::AnyLanguage,
                QLocale::AnyScript,
                QLocale::AnyCountry);
    QSet<QLocale::Language> allLanguages;
    for (auto itLocale = allLocales.begin();
         itLocale != allLocales.end(); ++itLocale) {
        if (!allLanguages.contains(itLocale->language())) {
            allLanguages << itLocale->language();
            QString langCode = QLocale::languageToCode(itLocale->language());
            if (langCode == "cn"
                    || langCode == "zh") {
                int TEMP=10;++TEMP;
            }
            if (langCodes.contains(langCode)) {
                if (!m_textToSpeechByLangCode.contains(langCode)) {
                    m_textToSpeechByLangCode[langCode] = new QTextToSpeech;
                }
                m_textToSpeechByLangCode[langCode]->setLocale(*itLocale);
                m_localByLangCode[langCode] = *itLocale;
                //QLocale temp = m_textToSpeechByLangCode[langCode]->locale();
                //auto lang = temp.language();
                //int TEMP=10;++TEMP;
            }
        }
    }
}
//----------------------------------------
SoundPlayer::~SoundPlayer()
{
    qDeleteAll(m_textToSpeechByLangCode);
}
//----------------------------------------
void SoundPlayer::playSound(const QUrl &url)
{
    m_player.stop();
    if (m_audioOutput != nullptr) {
        m_audioOutput->deleteLater();
    }
    m_audioOutput = new QAudioOutput;
    m_player.setAudioOutput(m_audioOutput);
    m_player.setSource(url);
    m_player.play();
}
//----------------------------------------
void SoundPlayer::sayText(
        const QString &text, const QString &langCode)
{
    QString textToSay = text;
    if (textToSay.contains("+")) {
        if (langCode == "fr") {
            textToSay.replace("+", " plus ");
        } else if (langCode == "de") {
            textToSay.replace("+", " plus ");
        } else if (langCode == "en") {
            textToSay.replace("+", " plus ");
        } else if (langCode == "zh"
                   || langCode == "cn") {
            textToSay.replace("+", " jiā ");
        } else if (langCode == "it") {
            textToSay.replace("+", " più ");
        } else if (langCode == "es") {
            textToSay.replace("+", " más ");
        }
    }
    if (m_textToSpeechByLangCode.contains(langCode)) {
        if (m_textToSpeechByLangCode[langCode]->locale().language() != m_localByLangCode[langCode].language()) {
            m_textToSpeechByLangCode[langCode]->setLocale(m_localByLangCode[langCode]);
        }
        m_textToSpeechByLangCode[langCode]->say(textToSay);
    } else {
        m_textToSpeechDefault.say(textToSay);
    }
}
//----------------------------------------
