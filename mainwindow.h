#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "albumproject.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QAction;
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class ProTree;
class PhotoGalleryWidget;
class MusicController;
struct PhotoImportSummary;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool openProjectPath(const QString &path);

private slots:
    void SlotCreatepro(bool checked = false);
    void openProject();
    void closeProject();
    void onProjectSelected(const QString &path);
    void onProjectClosed(const QString &path);
    void onProjectImportRequested(const QString &path);
    void onProjectSlideshowRequested(const QString &path);
    void importImages();
    void importImageFolder();
    void importMusic();
    void editSelectedPhoto();
    void editSelectedTags();
    void deleteSelectedPhotos();
    void previewSelectedPhoto(const QString &id);
    void startSlideshow();
    void changeTagFilter(int index);
    void playOrPauseMusic();
    void nextMusic();
    void previousMusic();
    void setMusicVolume(int value);
    void updateMusicState(const QString &state);

private:
    void setupActions();
    void setupPhotoControls();
    void refreshProjectView();
    void updateActionState();
    void importFiles(const QStringList &files);
    QList<PhotoRecord> selectedPhotos() const;
    void showFailureSummary(const PhotoImportSummary &summary);
    void updateMusicPlaylist();

    Ui::MainWindow *ui = nullptr;
    ProTree *_protree = nullptr;
    PhotoGalleryWidget *_gallery = nullptr;
    QComboBox *_tagFilter = nullptr;
    QLabel *_musicState = nullptr;
    QSlider *_volumeSlider = nullptr;
    QPushButton *_musicLoopButton = nullptr;
    MusicController *_music = nullptr;
    AlbumProject _project;
    bool _hasProject = false;

    QAction *_actionCreate = nullptr;
    QAction *_actionOpen = nullptr;
    QAction *_actionClose = nullptr;
    QAction *_actionImport = nullptr;
    QAction *_actionImportFolder = nullptr;
    QAction *_actionTags = nullptr;
    QAction *_actionEdit = nullptr;
    QAction *_actionDelete = nullptr;
    QAction *_actionSlideshow = nullptr;
    QAction *_actionMusic = nullptr;
    QAction *_actionAbout = nullptr;
};

#endif // MAINWINDOW_H
