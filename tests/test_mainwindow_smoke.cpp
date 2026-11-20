#include <QTemporaryDir>
#include <QStatusBar>
#include <QtTest>

#include "mainwindow.h"
#include "photogallerywidget.h"
#include "protree.h"

class MainWindowSmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void loadsProjectAndExposesActions();
};

void MainWindowSmokeTest::loadsProjectAndExposesActions()
{
    MainWindow window;
    QVERIFY(window.findChild<ProTree*>() != nullptr);
    QVERIFY(window.findChild<PhotoGalleryWidget*>() != nullptr);
    QVERIFY(window.findChild<QAction*>("actionImport") != nullptr);
    QVERIFY(window.findChild<QAction*>("actionTags") != nullptr);
    QVERIFY(window.findChild<QAction*>("actionEdit") != nullptr);
    QVERIFY(window.findChild<QAction*>("actionSlideshow") != nullptr);

    QTemporaryDir temp;
    QString error;
    AlbumProject project = AlbumProject::create(temp.path(), "冒烟项目", &error);
    QVERIFY2(project.isValid(), qPrintable(error));
    QVERIFY(window.openProjectPath(project.projectDir()));
    QVERIFY(window.statusBar()->currentMessage().contains("冒烟项目"));
}

QTEST_MAIN(MainWindowSmokeTest)
#include "test_mainwindow_smoke.moc"
