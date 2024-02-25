#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVoice>
#include <QTextToSpeech>

class SoundPlayer
{
public:
    static SoundPlayer *instance();
    void playSound(const QUrl &url);
    void sayText(const QString &text, const QString &langCode);
    void initLangCodes(const QStringList &langCodes);

private:
    SoundPlayer();
    ~SoundPlayer();
    QMediaPlayer m_player;
    QAudioOutput *m_audioOutput;
    //QHash<QString, const QVoice *> m_voiceByLangCode;
    QHash<QString, QTextToSpeech *> m_textToSpeechByLangCode;
    QTextToSpeech m_textToSpeechDefault;
    //QVoice m_defaultVoice;
    QHash<QString, QLocale> m_localByLangCode;
};

#endif // SOUNDPLAYER_H
