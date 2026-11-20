#include <QtTest>

#include "musiccontroller.h"

class MusicControllerTest : public QObject
{
    Q_OBJECT

private slots:
    void tracksPlaylistState();
};

void MusicControllerTest::tracksPlaylistState()
{
    MusicController controller;
    controller.setPlaylist({"a.mp3", "b.mp3"});
    QCOMPARE(controller.currentIndex(), 0);
    controller.next();
    QCOMPARE(controller.currentIndex(), 1);
    controller.setLoop(true);
    controller.next();
    QCOMPARE(controller.currentIndex(), 0);
}

QTEST_MAIN(MusicControllerTest)
#include "test_musiccontroller.moc"
