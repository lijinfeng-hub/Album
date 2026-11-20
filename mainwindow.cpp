#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "appmetadata.h"
#include "imageeditordialog.h"
#include "musiccontroller.h"
#include "photoimporter.h"
#include "photogallerywidget.h"
#include "photoviewerdialog.h"
#include "protree.h"
#include "slideshowdialog.h"
#include "tagdialog.h"
#include "wizard.h"

#include <QAction>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMenu>
#include <QPushButton>
#include <QSlider>
#include <QStatusBar>
#include <QVBoxLayout>

namespace {
QString uniqueMusicName(const QDir &directory, const QString &baseName, const QString &suffix)
{
    QString candidate = baseName + suffix;
    int index = 2;
    while (directory.exists(candidate)) {
        candidate = QStringLiteral("%1_%2%3").arg(baseName).arg(index++).arg(suffix);
    }
    return candidate;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), _music(new MusicController(this))
{
    ui->setupUi(this);
    setWindowTitle(AlbumMetadata::windowTitle());
    setWindowIcon(QIcon(":/Icon/Album.png"));
    resize(1200, 760);

    _protree = new ProTree(this);
    ui->proLayout->addWidget(_protree);
    _gallery = new PhotoGalleryWidget(this);
    ui->picLayout->addWidget(_gallery, 1);

    setupActions();
    setupPhotoControls();

    connect(_protree, &ProTree::projectSelected, this, &MainWindow::onProjectSelected);
    connect(_protree, &ProTree::projectClosed, this, &MainWindow::onProjectClosed);
    connect(_protree, &ProTree::importRequested, this, &MainWindow::onProjectImportRequested);
    connect(_protree, &ProTree::slideshowRequested, this, &MainWindow::onProjectSlideshowRequested);
    connect(_gallery, &PhotoGalleryWidget::selectionChanged, this,
            [this](const QStringList &) { updateActionState(); });
    connect(_gallery, &PhotoGalleryWidget::photoActivated, this, &MainWindow::previewSelectedPhoto);
    connect(_gallery, &PhotoGalleryWidget::requestDelete, this,
            [this](const QStringList &) { deleteSelectedPhotos(); });
    connect(_music, &MusicController::stateChanged, this, &MainWindow::updateMusicState);
    connect(_music, &MusicController::errorOccurred, this, [this](const QString &message) {
        statusBar()->showMessage(message, 5000);
    });
    updateActionState();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("文件(&F)"));
    _actionCreate = new QAction(QIcon(":/Icon/create.png"), tr("创建项目"), this);
    _actionCreate->setObjectName("actionCreate");
    _actionCreate->setShortcut(QKeySequence::New);
    _actionOpen = new QAction(QIcon(":/Icon/open.png"), tr("打开项目"), this);
    _actionOpen->setObjectName("actionOpen");
    _actionOpen->setShortcut(QKeySequence::Open);
    _actionClose = new QAction(QIcon(":/Icon/关闭 (1).png"), tr("关闭项目"), this);
    _actionClose->setObjectName("actionClose");
    fileMenu->addActions({_actionCreate, _actionOpen, _actionClose});
    fileMenu->addSeparator();
    fileMenu->addAction(tr("退出"), this, &QWidget::close, QKeySequence::Quit);

    QMenu *photoMenu = menuBar()->addMenu(tr("图片(&P)"));
    _actionImport = new QAction(QIcon(":/Icon/import.png"), tr("导入图片"), this);
    _actionImport->setObjectName("actionImport");
    _actionImportFolder = new QAction(QIcon(":/Icon/dir.png"), tr("导入文件夹"), this);
    _actionImportFolder->setObjectName("actionImportFolder");
    _actionEdit = new QAction(tr("编辑图片"), this);
    _actionEdit->setObjectName("actionEdit");
    _actionTags = new QAction(tr("编辑标签"), this);
    _actionTags->setObjectName("actionTags");
    _actionDelete = new QAction(QIcon(":/Icon/关闭 (1).png"), tr("删除图片"), this);
    _actionDelete->setObjectName("actionDelete");
    _actionSlideshow = new QAction(QIcon(":/Icon/slide.png"), tr("开始轮播"), this);
    _actionSlideshow->setObjectName("actionSlideshow");
    photoMenu->addActions({_actionImport, _actionImportFolder});
    photoMenu->addSeparator();
    photoMenu->addActions({_actionEdit, _actionTags, _actionDelete, _actionSlideshow});

    QMenu *settingsMenu = menuBar()->addMenu(tr("设置(&S)"));
    _actionMusic = new QAction(QIcon(":/Icon/Music.png"), tr("导入背景音乐"), this);
    _actionMusic->setObjectName("actionMusic");
    settingsMenu->addAction(_actionMusic);

    QMenu *helpMenu = menuBar()->addMenu(tr("帮助(&H)"));
    _actionAbout = new QAction(tr("关于软件"), this);
    _actionAbout->setObjectName("actionAbout");
    helpMenu->addAction(_actionAbout);

    connect(_actionCreate, &QAction::triggered, this, &MainWindow::SlotCreatepro);
    connect(_actionOpen, &QAction::triggered, this, &MainWindow::openProject);
    connect(_actionClose, &QAction::triggered, this, &MainWindow::closeProject);
    connect(_actionImport, &QAction::triggered, this, &MainWindow::importImages);
    connect(_actionImportFolder, &QAction::triggered, this, &MainWindow::importImageFolder);
    connect(_actionEdit, &QAction::triggered, this, &MainWindow::editSelectedPhoto);
    connect(_actionTags, &QAction::triggered, this, &MainWindow::editSelectedTags);
    connect(_actionDelete, &QAction::triggered, this, &MainWindow::deleteSelectedPhotos);
    connect(_actionSlideshow, &QAction::triggered, this, &MainWindow::startSlideshow);
    connect(_actionMusic, &QAction::triggered, this, &MainWindow::importMusic);
    connect(_actionAbout, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("关于软件"), AlbumMetadata::copyrightText());
    });
}

void MainWindow::setupPhotoControls()
{
    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel(tr("标签筛选："), this));
    _tagFilter = new QComboBox(this);
    _tagFilter->setObjectName("tagFilter");
    filterLayout->addWidget(_tagFilter);
    filterLayout->addWidget(new QLabel(tr("排序："), this));
    auto *sortCombo = new QComboBox(this);
    sortCombo->addItem(tr("导入时间"), "importedAt");
    sortCombo->addItem(tr("文件名"), "name");
    filterLayout->addWidget(sortCombo);
    filterLayout->addStretch();
    ui->picLayout->insertLayout(0, filterLayout);
    connect(_tagFilter, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::changeTagFilter);
    connect(sortCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, sortCombo](int index) {
        if (_hasProject) {
            _project.setSortMode(sortCombo->itemData(index).toString());
            _project.save();
            refreshProjectView();
        }
    });

    auto *musicLayout = new QHBoxLayout;
    _musicState = new QLabel(tr("没有背景音乐"), this);
    _musicState->setObjectName("musicState");
    auto *previous = new QPushButton(tr("上一首"), this);
    auto *play = new QPushButton(tr("播放/暂停"), this);
    auto *next = new QPushButton(tr("下一首"), this);
    auto *importButton = new QPushButton(tr("添加音乐"), this);
    _musicLoopButton = new QPushButton(tr("循环"), this);
    _musicLoopButton->setCheckable(true);
    _musicLoopButton->setChecked(true);
    _volumeSlider = new QSlider(Qt::Horizontal, this);
    _volumeSlider->setRange(0, 100);
    _volumeSlider->setValue(_music->volume());
    _volumeSlider->setMaximumWidth(140);
    musicLayout->addWidget(_musicState, 1);
    musicLayout->addWidget(previous);
    musicLayout->addWidget(play);
    musicLayout->addWidget(next);
    musicLayout->addWidget(importButton);
    musicLayout->addWidget(_musicLoopButton);
    musicLayout->addWidget(new QLabel(tr("音量"), this));
    musicLayout->addWidget(_volumeSlider);
    ui->picLayout->addLayout(musicLayout);
    connect(previous, &QPushButton::clicked, this, &MainWindow::previousMusic);
    connect(play, &QPushButton::clicked, this, &MainWindow::playOrPauseMusic);
    connect(next, &QPushButton::clicked, this, &MainWindow::nextMusic);
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importMusic);
    connect(_volumeSlider, &QSlider::valueChanged, this, &MainWindow::setMusicVolume);
    connect(_musicLoopButton, &QPushButton::toggled, this, [this](bool enabled) {
        _music->setLoop(enabled);
        if (_hasProject) {
            _project.setMusicLoop(enabled);
            _project.save();
        }
    });
}

bool MainWindow::openProjectPath(const QString &path)
{
    AlbumProject loaded;
    QString error;
    if (!loaded.open(path, &error)) {
        QMessageBox::warning(this, tr("打开项目失败"), error);
        return false;
    }
    _project = loaded;
    _hasProject = true;
    _protree->AddProjectPath(_project.name(), _project.projectDir());
    updateMusicPlaylist();
    refreshProjectView();
    statusBar()->showMessage(tr("当前项目：%1").arg(_project.name()));
    return true;
}

void MainWindow::SlotCreatepro(bool)
{
    Wizard wizard(this);
    wizard.setWindowTitle(tr("创建项目"));
    if (wizard.exec() != QDialog::Accepted) {
        return;
    }
    const QString name = wizard.field("proName").toString().trimmed();
    const QString parent = wizard.field("proPath").toString();
    QString error;
    const AlbumProject created = AlbumProject::create(parent, name, &error);
    if (!created.isValid()) {
        QMessageBox::warning(this, tr("创建项目失败"), error);
        return;
    }
    openProjectPath(created.projectDir());
}

void MainWindow::openProject()
{
    const QString file = QFileDialog::getOpenFileName(this, tr("打开 Album 项目"),
                                                      QDir::homePath(),
                                                      tr("Album 项目 (album.json)"));
    if (!file.isEmpty()) {
        openProjectPath(file);
    }
}

void MainWindow::closeProject()
{
    if (!_hasProject) return;
    if (_project.isDirty()) {
        const auto answer = QMessageBox::question(this, tr("保存项目"), tr("项目有未保存更改，是否保存？"),
                                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (answer == QMessageBox::Cancel) return;
        if (answer == QMessageBox::Save) _project.save();
    }
    _music->stop();
    _protree->RemoveProjectPath(_project.projectDir());
    _project = AlbumProject();
    _hasProject = false;
    _gallery->setPhotos({}, QString());
    _tagFilter->clear();
    _musicState->setText(tr("没有背景音乐"));
    statusBar()->clearMessage();
    updateActionState();
}

void MainWindow::onProjectSelected(const QString &path)
{
    if (!path.isEmpty()) openProjectPath(path);
}

void MainWindow::onProjectClosed(const QString &path)
{
    if (_hasProject && QDir::cleanPath(_project.projectDir()) == QDir::cleanPath(path)) {
        closeProject();
    }
}

void MainWindow::onProjectImportRequested(const QString &path)
{
    if (!_hasProject || QDir::cleanPath(_project.projectDir()) != QDir::cleanPath(path)) {
        if (!openProjectPath(path)) return;
    }
    importImages();
}

void MainWindow::onProjectSlideshowRequested(const QString &path)
{
    if (!_hasProject || QDir::cleanPath(_project.projectDir()) != QDir::cleanPath(path)) {
        if (!openProjectPath(path)) return;
    }
    startSlideshow();
}

void MainWindow::importImages()
{
    if (!_hasProject) return;
    const QStringList files = QFileDialog::getOpenFileNames(this, tr("导入图片"), QDir::homePath(),
                                                            tr("图片 (*.png *.jpg *.jpeg *.bmp *.gif *.webp)"));
    if (!files.isEmpty()) importFiles(files);
}

void MainWindow::importImageFolder()
{
    if (!_hasProject) return;
    const QString directory = QFileDialog::getExistingDirectory(this, tr("选择图片文件夹"), QDir::homePath());
    if (directory.isEmpty()) return;
    const QStringList names = PhotoImporter::imageFilesInDirectory(directory);
    QStringList files;
    for (const QString &name : names) files.append(QDir(directory).filePath(name));
    importFiles(files);
}

void MainWindow::importFiles(const QStringList &files)
{
    const PhotoImportSummary summary = PhotoImporter::importFiles(_project, files);
    refreshProjectView();
    showFailureSummary(summary);
    statusBar()->showMessage(tr("已导入 %1 张图片").arg(summary.imported.size()), 5000);
}

void MainWindow::importMusic()
{
    if (!_hasProject) return;
    const QStringList files = QFileDialog::getOpenFileNames(this, tr("添加背景音乐"), QDir::homePath(),
                                                            tr("音频 (*.mp3 *.wav *.flac *.m4a *.ogg)"));
    if (files.isEmpty()) return;
    const QDir musicDir(_project.projectDir() + QStringLiteral("/music"));
    QStringList relativeFiles = _project.musicFiles();
    for (const QString &file : files) {
        const QFileInfo info(file);
        const QString name = uniqueMusicName(musicDir, info.completeBaseName(),
                                             QStringLiteral(".") + info.suffix().toLower());
        const QString target = musicDir.filePath(name);
        if (QFile::copy(file, target)) relativeFiles.append(QDir(_project.projectDir()).relativeFilePath(target).replace('\\', '/'));
    }
    _project.setMusicFiles(relativeFiles);
    _project.save();
    updateMusicPlaylist();
}

void MainWindow::editSelectedPhoto()
{
    const QList<PhotoRecord> records = selectedPhotos();
    if (records.size() != 1) return;
    const PhotoRecord record = records.first();
    const QImage source(QDir(_project.projectDir()).filePath(record.relativePath));
    if (source.isNull()) {
        QMessageBox::warning(this, tr("编辑失败"), tr("图片无法读取。"));
        return;
    }
    ImageEditorDialog editor(source, record.edit, this);
    connect(&editor, &ImageEditorDialog::acceptedEdit, this,
            [this, record](const EditSettings &settings, const QImage &image) {
                PhotoRecord updated = record;
                const QString output = _project.editedFilePath(updated);
                if (settings.isDefault()) {
                    QFile::remove(output);
                } else if (!image.save(output, "PNG")) {
                    QMessageBox::warning(this, tr("保存编辑失败"), tr("无法写入编辑结果。"));
                    return;
                }
                updated.edit = settings;
                _project.updatePhoto(updated);
                _project.save();
                refreshProjectView();
            });
    editor.exec();
}

void MainWindow::editSelectedTags()
{
    const QList<PhotoRecord> records = selectedPhotos();
    if (records.isEmpty()) return;
    TagDialog dialog(_project.tags(), records.first().tags, this);
    if (dialog.exec() != QDialog::Accepted) return;
    const QStringList tags = dialog.selectedTags();
    for (const PhotoRecord &record : records) {
        PhotoRecord updated = record;
        updated.tags = tags;
        _project.updatePhoto(updated);
    }
    _project.save();
    refreshProjectView();
}

void MainWindow::deleteSelectedPhotos()
{
    const QList<PhotoRecord> records = selectedPhotos();
    if (records.isEmpty()) return;
    if (QMessageBox::question(this, tr("删除图片"), tr("确定删除选中的 %1 张图片吗？").arg(records.size()))
        != QMessageBox::Yes) return;
    for (const PhotoRecord &record : records) {
        QFile::remove(QDir(_project.projectDir()).filePath(record.relativePath));
        QFile::remove(_project.editedFilePath(record));
        _project.removePhoto(record.id);
    }
    _project.save();
    refreshProjectView();
}

void MainWindow::previewSelectedPhoto(const QString &id)
{
    const QList<PhotoRecord> records = _project.filteredPhotos();
    for (int index = 0; index < records.size(); ++index) {
        if (records.at(index).id == id) {
            PhotoViewerDialog dialog(records, _project.projectDir(), index, this);
            dialog.exec();
            return;
        }
    }
}

void MainWindow::startSlideshow()
{
    if (!_hasProject || _project.filteredPhotos().isEmpty()) return;
    SlideshowDialog dialog(_project.filteredPhotos(), _project.projectDir(), 3000, true, this);
    dialog.exec();
}

void MainWindow::changeTagFilter(int index)
{
    if (!_hasProject || index < 0) return;
    _project.setActiveTag(_tagFilter->itemText(index));
    _project.save();
    refreshProjectView();
}

void MainWindow::playOrPauseMusic()
{
    if (_music->isPlaying()) _music->pause(); else _music->play();
}

void MainWindow::nextMusic() { _music->next(); }
void MainWindow::previousMusic() { _music->previous(); }
void MainWindow::setMusicVolume(int value) { _music->setVolume(value); }

void MainWindow::updateMusicState(const QString &state)
{
    _musicState->setText(state == "playing" ? tr("正在播放") : state == "paused" ? tr("已暂停") : tr("已停止"));
}

void MainWindow::refreshProjectView()
{
    if (!_hasProject) {
        _gallery->setPhotos({}, QString());
        return;
    }
    _tagFilter->blockSignals(true);
    _tagFilter->clear();
    _tagFilter->addItem(tr("全部"));
    _tagFilter->addItems(_project.tags());
    const int index = qMax(0, _tagFilter->findText(_project.activeTag()));
    _tagFilter->setCurrentIndex(index);
    _tagFilter->blockSignals(false);
    _gallery->setPhotos(_project.filteredPhotos(), _project.projectDir());
    updateActionState();
}

void MainWindow::updateActionState()
{
    const bool hasSelection = _hasProject && !_gallery->selectedItems().isEmpty();
    _actionClose->setEnabled(_hasProject);
    _actionImport->setEnabled(_hasProject);
    _actionImportFolder->setEnabled(_hasProject);
    _actionMusic->setEnabled(_hasProject);
    _actionEdit->setEnabled(hasSelection && _gallery->selectedItems().size() == 1);
    _actionTags->setEnabled(hasSelection);
    _actionDelete->setEnabled(hasSelection);
    _actionSlideshow->setEnabled(_hasProject && !_project.filteredPhotos().isEmpty());
}

QList<PhotoRecord> MainWindow::selectedPhotos() const
{
    QList<PhotoRecord> result;
    for (const QString &id : _gallery->selectedPhotoIds()) {
        for (const PhotoRecord &record : _project.photos()) {
            if (record.id == id) result.append(record);
        }
    }
    return result;
}

void MainWindow::showFailureSummary(const PhotoImportSummary &summary)
{
    if (!summary.hasFailures()) return;
    QStringList messages;
    for (int index = 0; index < summary.failed.size(); ++index) {
        messages.append(QFileInfo(summary.failed.at(index)).fileName() + QStringLiteral("：")
                        + summary.failureReasons.value(index));
    }
    QMessageBox::warning(this, tr("部分文件未导入"), messages.join('\n'));
}

void MainWindow::updateMusicPlaylist()
{
    QStringList absoluteFiles;
    if (_hasProject) {
        for (const QString &relative : _project.musicFiles()) {
            const QString absolute = QDir(_project.projectDir()).filePath(relative);
            if (QFileInfo::exists(absolute)) absoluteFiles.append(absolute);
        }
    }
    _music->setPlaylist(absoluteFiles);
    _music->setLoop(_hasProject ? _project.musicLoop() : true);
    if (_musicLoopButton) {
        _musicLoopButton->blockSignals(true);
        _musicLoopButton->setChecked(_hasProject ? _project.musicLoop() : true);
        _musicLoopButton->blockSignals(false);
    }
    _musicState->setText(absoluteFiles.isEmpty() ? tr("没有背景音乐") : tr("音乐已就绪"));
}
