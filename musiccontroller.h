#ifndef MUSICCONTROLLER_H
#define MUSICCONTROLLER_H

#include <QObject>
#include <QStringList>

class QAudioOutput;
class QMediaPlayer;

class MusicController : public QObject
{
    Q_OBJECT

public:
    explicit MusicController(QObject *parent = nullptr);

    void setPlaylist(const QStringList &files);
    void addFiles(const QStringList &files);
    void removeCurrent();
    void play();
    void pause();
    void stop();
    void next();
    void previous();
    void setVolume(int volume);
    void setLoop(bool enabled);

    QStringList playlist() const;
    int currentIndex() const;
    bool isPlaying() const;
    int volume() const;
    bool loop() const;

signals:
    void stateChanged(const QString &state);
    void trackChanged(int index);
    void errorOccurred(const QString &message);

private:
    void selectIndex(int index, bool autoplay = false);

    QMediaPlayer *_player = nullptr;
    QAudioOutput *_audioOutput = nullptr;
    QStringList _playlist;
    int _currentIndex = -1;
    int _volume = 70;
    bool _loop = true;
};

#endif // MUSICCONTROLLER_H
