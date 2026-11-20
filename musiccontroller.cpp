#include "musiccontroller.h"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QUrl>

MusicController::MusicController(QObject *parent)
    : QObject(parent), _player(new QMediaPlayer(this)), _audioOutput(new QAudioOutput(this))
{
    _audioOutput->setVolume(_volume / 100.0);
    _player->setAudioOutput(_audioOutput);

    connect(_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            next();
        }
    });
    connect(_player, &QMediaPlayer::playbackStateChanged, this,
            [this](QMediaPlayer::PlaybackState state) {
                switch (state) {
                case QMediaPlayer::PlayingState:
                    emit stateChanged(QStringLiteral("playing"));
                    break;
                case QMediaPlayer::PausedState:
                    emit stateChanged(QStringLiteral("paused"));
                    break;
                case QMediaPlayer::StoppedState:
                    emit stateChanged(QStringLiteral("stopped"));
                    break;
                }
            });
    connect(_player, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &message) {
                emit errorOccurred(message.isEmpty() ? tr("音乐播放失败。") : message);
            });
}

void MusicController::setPlaylist(const QStringList &files)
{
    _playlist.clear();
    addFiles(files);
    if (_playlist.isEmpty()) {
        _currentIndex = -1;
        _player->stop();
        _player->setSource(QUrl());
    } else {
        selectIndex(0);
    }
}

void MusicController::addFiles(const QStringList &files)
{
    for (const QString &file : files) {
        if (!file.trimmed().isEmpty() && !_playlist.contains(file)) {
            _playlist.append(file);
        }
    }
    if (_currentIndex < 0 && !_playlist.isEmpty()) {
        selectIndex(0);
    }
}

void MusicController::removeCurrent()
{
    if (_currentIndex < 0 || _currentIndex >= _playlist.size()) {
        return;
    }
    _playlist.removeAt(_currentIndex);
    if (_playlist.isEmpty()) {
        _currentIndex = -1;
        _player->stop();
        _player->setSource(QUrl());
        emit trackChanged(-1);
        return;
    }
    if (_currentIndex >= _playlist.size()) {
        _currentIndex = _playlist.size() - 1;
    }
    selectIndex(_currentIndex);
}

void MusicController::play()
{
    if (_playlist.isEmpty()) {
        emit errorOccurred(tr("播放列表为空。"));
        return;
    }
    if (_currentIndex < 0) {
        selectIndex(0);
    }
    _player->play();
}

void MusicController::pause()
{
    _player->pause();
}

void MusicController::stop()
{
    _player->stop();
}

void MusicController::next()
{
    if (_playlist.isEmpty()) {
        return;
    }
    const int nextIndex = _currentIndex + 1;
    if (nextIndex < _playlist.size()) {
        selectIndex(nextIndex, _player->playbackState() == QMediaPlayer::PlayingState);
    } else if (_loop) {
        selectIndex(0, _player->playbackState() == QMediaPlayer::PlayingState);
    } else {
        stop();
    }
}

void MusicController::previous()
{
    if (_playlist.isEmpty()) {
        return;
    }
    const int previousIndex = _currentIndex - 1;
    if (previousIndex >= 0) {
        selectIndex(previousIndex, _player->playbackState() == QMediaPlayer::PlayingState);
    } else if (_loop) {
        selectIndex(_playlist.size() - 1, _player->playbackState() == QMediaPlayer::PlayingState);
    }
}

void MusicController::setVolume(int volume)
{
    _volume = qBound(0, volume, 100);
    _audioOutput->setVolume(_volume / 100.0);
}

void MusicController::setLoop(bool enabled)
{
    _loop = enabled;
}

QStringList MusicController::playlist() const
{
    return _playlist;
}

int MusicController::currentIndex() const
{
    return _currentIndex;
}

bool MusicController::isPlaying() const
{
    return _player->playbackState() == QMediaPlayer::PlayingState;
}

int MusicController::volume() const
{
    return _volume;
}

bool MusicController::loop() const
{
    return _loop;
}

void MusicController::selectIndex(int index, bool autoplay)
{
    if (index < 0 || index >= _playlist.size()) {
        return;
    }
    _currentIndex = index;
    _player->setSource(QUrl::fromLocalFile(_playlist.at(index)));
    emit trackChanged(index);
    if (autoplay) {
        _player->play();
    }
}
