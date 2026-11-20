#include <QDir>
#include <QTemporaryDir>
#include <QtTest>

#include "albumproject.h"
#include "albumtypes.h"

class AlbumProjectTest : public QObject
{
    Q_OBJECT

private slots:
    void createsAndReloadsProject();
    void filtersByTagAndRenamesTags();
    void usesStableEditPath();
};

void AlbumProjectTest::createsAndReloadsProject()
{
    QTemporaryDir temp;
    QVERIFY(temp.isValid());

    QString error;
    AlbumProject project = AlbumProject::create(temp.path(), "旅行相册", &error);
    QVERIFY2(project.isValid(), qPrintable(error));
    QVERIFY(QDir(project.projectDir() + "/photos").exists());

    PhotoRecord record;
    record.id = "photo-1";
    record.relativePath = "photos/a.jpg";
    record.name = "a.jpg";
    record.tags = {"旅行", "夏天"};
    QVERIFY(project.addPhoto(record));
    QVERIFY(project.save(&error));

    AlbumProject loaded;
    QVERIFY2(loaded.open(project.projectDir(), &error), qPrintable(error));
    QCOMPARE(loaded.name(), QString("旅行相册"));
    QCOMPARE(loaded.photos().size(), 1);
    QCOMPARE(loaded.photos().first().tags, QStringList({"旅行", "夏天"}));
}

void AlbumProjectTest::filtersByTagAndRenamesTags()
{
    QTemporaryDir temp;
    QVERIFY(temp.isValid());
    QString error;
    AlbumProject project = AlbumProject::create(temp.path(), "标签测试", &error);
    QVERIFY2(project.isValid(), qPrintable(error));
    auto add = [&project](const QString &id, const QStringList &tags) {
        PhotoRecord record;
        record.id = id;
        record.relativePath = "photos/" + id + ".png";
        record.name = id + ".png";
        record.tags = tags;
        QVERIFY(project.addPhoto(record));
    };
    add("one", {"旅行", "夏天"});
    add("two", {"工作"});

    project.setActiveTag("旅行");
    QCOMPARE(project.filteredPhotos().size(), 1);
    QVERIFY(project.renameTag("旅行", "出游"));
    QCOMPARE(project.filteredPhotos().size(), 1);
    QCOMPARE(project.activeTag(), QString("出游"));
}

void AlbumProjectTest::usesStableEditPath()
{
    QTemporaryDir temp;
    QVERIFY(temp.isValid());
    QString error;
    AlbumProject project = AlbumProject::create(temp.path(), "编辑测试", &error);
    QVERIFY2(project.isValid(), qPrintable(error));
    PhotoRecord record;
    record.id = "photo-42";
    record.relativePath = "photos/a.jpg";
    record.name = "a.jpg";
    QVERIFY(project.addPhoto(record));
    QCOMPARE(project.editedFilePath(record), project.projectDir() + "/edits/photo-42.png");
}

QTEST_MAIN(AlbumProjectTest)
#include "test_albumproject.moc"
