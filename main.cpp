#include "mainwindow.h"
#include "appmetadata.h"

#include <QApplication>
#include<QFile>
#include<QString>
#include<QLatin1String>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(AlbumMetadata::softwareName());
    a.setApplicationVersion(AlbumMetadata::version());
    QFile qss(":/style/style.qss");
    if(qss.open(QFile::ReadOnly))
    {
        qDebug()<<"open qss successfully!";
        QString style=QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }else
    {
        qDebug()<<"open qss failed!";
        return 0;
    }
    MainWindow w;
    w.showMaximized();
    return a.exec();
}
