# Album 完整版实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在现有 Qt Widgets 相册骨架上完成可移动项目、图片导入/浏览/轮播、标签筛选、基础图片编辑和背景音乐，并用自动化测试覆盖核心数据与图像逻辑。

**Architecture:** 用不依赖 QWidget 的 `AlbumProject` 保存项目和照片元数据，用 `PhotoImporter` 负责复制和验证图片，用 `ImageRenderer` 负责非破坏式编辑渲染；主窗口只协调项目树、画廊、预览、编辑器、轮播和音乐控制器。项目目录使用 `album.json` + `photos/` + `edits/` + `music/`，JSON 中只保存相对路径。

**Tech Stack:** Qt 6 Widgets、Qt 6 Multimedia、Qt Test、C++17、CMake、Qt Designer UI；不新增第三方运行时依赖。

## Global Constraints

- 继续使用 C++17、Qt Widgets 和现有深色 QSS 风格。
- 项目文件固定为项目目录下的 `album.json`，资源路径使用相对路径。
- 导入图片必须复制到项目目录，不能修改原始文件，也不能覆盖项目中已有文件。
- 编辑必须保留原始图片，编辑结果写入 `edits/`，支持重置。
- 每个新增业务行为先写一个失败测试并确认失败，再写最小实现并确认通过。
- UI 层不直接拼接或解析项目 JSON；所有持久化通过 `AlbumProject`。
- 当前目录不是 Git 仓库；计划中的提交步骤改为检查工作区差异和记录验证结果，不执行不存在的 Git 提交。

---

## 文件地图

### 新建

- `albumtypes.h/.cpp`：`EditSettings`、`PhotoRecord` 和 JSON 转换。
- `albumproject.h/.cpp`：项目目录、照片记录、标签、筛选、排序和 JSON 持久化。
- `photoimporter.h/.cpp`：图片文件验证、目录扫描、复制和唯一文件名。
- `imagerenderer.h/.cpp`：旋转、翻转、裁剪、亮度和灰度渲染。
- `musiccontroller.h/.cpp`：Qt Multimedia 播放列表、循环、音量和错误信号。
- `photogallerywidget.h/.cpp`：缩略图网格和选中项管理。
- `photoviewerdialog.h/.cpp/.ui`：单张预览和前后切换。
- `imageeditordialog.h/.cpp/.ui`：编辑控件和编辑结果确认。
- `tagdialog.h/.cpp/.ui`：选中图片的标签编辑。
- `slideshowdialog.h/.cpp/.ui`：轮播窗口和计时器。
- `tests/CMakeLists.txt`：Qt Test 目标定义。
- `tests/test_albumproject.cpp`：项目持久化、标签筛选、排序测试。
- `tests/test_photoimporter.cpp`：图片验证、复制、重名测试。
- `tests/test_imagerenderer.cpp`：编辑渲染测试。
- `tests/test_musiccontroller.cpp`：播放列表状态测试。
- `tests/test_gallery_smoke.cpp`：缩略图画廊状态测试。
- `tests/test_mainwindow_smoke.cpp`：主窗口控件和项目加载冒烟测试。
- `README.md`：构建、使用、项目目录和限制说明。

### 修改

- `CMakeLists.txt`：加入新源码、Qt Multimedia、测试子目录和测试注册。
- `mainwindow.h/.cpp/.ui`：组装完整主界面、动作、项目生命周期和业务信号。
- `protree.h/.cpp/.ui`：暴露项目选择、关闭、导入和轮播信号。
- `protreeitem.h/.cpp`：补齐路径访问器，保存项目路径和元数据。
- `protreewidget.h/.cpp`：初始化状态、实现右键菜单动作和项目树信号。
- `prosetpage.cpp`：规范化项目名/路径并修复完成状态刷新。
- `wizard.cpp`：只在向导成功完成时发出项目创建信号。
- `style/style.qss`：补充按钮、画廊、筛选栏、编辑器和空状态样式。

---

### Task 1: 建立 Qt Test 和 CMake 骨架

**Files:**
- Modify: `CMakeLists.txt`
- Create: `tests/CMakeLists.txt`
- Create: `tests/test_albumproject.cpp`

**Interfaces:**
- Produces: 一个能被 CTest 发现的 `album_project_tests` 目标，以及测试可包含根目录业务头文件的构建路径。

- [ ] **Step 1: 写一个能证明测试目标存在的失败测试**

```cpp
#include <QtTest>

class AlbumProjectTest : public QObject {
    Q_OBJECT
private slots:
    void testBuildHarness();
};

void AlbumProjectTest::testBuildHarness()
{
    QVERIFY2(false, "AlbumProject test harness is not implemented yet");
}

QTEST_MAIN(AlbumProjectTest)
#include "test_albumproject.moc"
```

- [ ] **Step 2: 运行测试确认失败原因正确**

Run: `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug -R album_project_tests --output-on-failure`

Expected: 测试目标可被发现，但因为断言 `false` 失败；如果测试目标无法配置，先修正 CMake，不修改测试断言。

- [ ] **Step 3: 配置 Qt 依赖和测试目标**

在 `CMakeLists.txt` 中增加 `Multimedia`，并在文件末尾增加 `enable_testing()` 和 `add_subdirectory(tests)`。测试目标先只链接 `Qt${QT_VERSION_MAJOR}::Test` 和 `Qt${QT_VERSION_MAJOR}::Widgets`，后续由测试任务加入业务源文件。

```cmake
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia Test)

enable_testing()
add_subdirectory(tests)
```

`tests/CMakeLists.txt` 第一阶段只使用 `add_executable(album_project_tests test_albumproject.cpp)`，设置根目录为 include path，链接 `Qt${QT_VERSION_MAJOR}::Test` 和 `Qt${QT_VERSION_MAJOR}::Widgets`，然后调用 `add_test(NAME album_project_tests COMMAND album_project_tests)`；后续任务创建业务源文件后，再把对应源文件和测试目标加入该 CMake 文件。

- [ ] **Step 4: 运行测试确认构建链路正常**

Run: `cmake --build build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --config Debug --parallel 2`

Expected: 能进入测试源码编译；此时测试仍因为 `QVERIFY2(false, ...)` 失败。若出现 `type_traits` 找不到，使用已安装的 VS 64 位开发者环境重新配置/构建，并记录实际构建命令。

- [ ] **Step 5: 将测试改成临时通过并重新运行**

把断言改成 `QVERIFY(true)`，运行同一条 `ctest` 命令，确认 CTest 能报告 1 个测试通过；该临时测试由 Task 2 的真实测试替换。

---

### Task 2: 实现项目数据模型和 JSON 持久化

**Files:**
- Create: `albumtypes.h`, `albumtypes.cpp`
- Create: `albumproject.h`, `albumproject.cpp`
- Modify: `tests/test_albumproject.cpp`

**Interfaces:**
- Produces: `EditSettings`, `PhotoRecord`、`AlbumProject`。
- `AlbumProject::create(parentPath, name, error)` 创建并初始化项目目录。
- `AlbumProject::open(path, error)` 打开项目目录或 `album.json`。
- `AlbumProject::save(error)` 使用 `QSaveFile` 写入 JSON。
- `AlbumProject::photos()`, `filteredPhotos()`, `tags()`, `activeTag()` 返回当前状态。
- `AlbumProject::addPhoto(record)`, `updatePhoto(record)`, `removePhoto(id)` 修改内存状态并标记脏数据。
- `AlbumProject::setActiveTag(tag)`, `setSortMode(mode)` 改变展示条件。

- [ ] **Step 1: 写失败测试覆盖创建、保存和读取**

```cpp
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
```

- [ ] **Step 2: 运行单测确认模型尚未实现**

Run: `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug -R album_project_tests --output-on-failure`

Expected: 编译失败，提示 `AlbumProject` 或 `PhotoRecord` 未定义；如果失败原因是测试环境，先修正 include/构建配置。

- [ ] **Step 3: 实现数据类型和项目目录**

`EditSettings` 包含 `rotation`、`flipHorizontal`、`flipVertical`、`brightness`、`grayscale` 和 `QRect crop`，默认值表示无编辑。`PhotoRecord` 包含 `id`、`relativePath`、`name`、`tags`、`importedAt`、`edit`。

`AlbumProject::create()` 必须拒绝空名称、父目录不存在和目标目录已存在；成功时创建 `photos/`、`edits/`、`music/`，写入 `version: 1` 的初始 JSON。`open()` 允许传入目录或 `album.json`，检查版本、字段类型和路径格式。

- [ ] **Step 4: 实现标签筛选、排序和原子保存**

标签写入前去空格、丢弃空标签并去重；`filteredPhotos()` 在 `activeTag == "全部"` 时返回排序后的全部照片，否则仅返回包含该标签的记录。排序至少支持 `name` 和 `importedAt`。`save()` 使用 `QSaveFile`，失败时返回错误文本且不清除脏状态。

- [ ] **Step 5: 运行单测确认通过**

Run: `cmake --build build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --config Debug --parallel 2`，then `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug -R album_project_tests --output-on-failure`

Expected: 项目创建/读取测试通过，且没有 JSON 解析警告。

---

### Task 3: 实现图片导入和项目内文件管理

**Files:**
- Create: `photoimporter.h`, `photoimporter.cpp`
- Modify: `albumproject.h`, `albumproject.cpp`
- Create: `tests/test_photoimporter.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- `PhotoImportSummary PhotoImporter::importFiles(AlbumProject&, const QStringList&)`。
- `QStringList PhotoImporter::imageFilesInDirectory(const QString&)`。
- `PhotoImportSummary` 包含 `imported`、`failed`、`failureReasons`。

- [ ] **Step 1: 写失败测试验证图片复制和重名策略**

```cpp
static bool createTestImage(const QString &path, const QColor &color)
{
    QImage image(2, 2, QImage::Format_RGB32);
    image.fill(color);
    return image.save(path);
}

void PhotoImporterTest::copiesValidImagesWithoutOverwriting()
{
    QTemporaryDir source;
    QTemporaryDir projectDir;
    QVERIFY(source.isValid() && projectDir.isValid());
    QVERIFY(createTestImage(source.filePath("photo.png"), Qt::red));

    QString error;
    AlbumProject project = AlbumProject::create(projectDir.path(), "相册", &error);
    QVERIFY(project.isValid());

    const auto first = PhotoImporter::importFiles(project, {source.filePath("photo.png")});
    const auto second = PhotoImporter::importFiles(project, {source.filePath("photo.png")});
    QCOMPARE(first.imported.size(), 1);
    QCOMPARE(second.imported.size(), 1);
    QVERIFY(first.imported.first().relativePath != second.imported.first().relativePath);
    QVERIFY(QFileInfo(project.projectDir() + "/" + second.imported.first().relativePath).exists());
}
```

- [ ] **Step 2: 运行单测确认导入接口缺失**

Run: `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug -R photo_importer_tests --output-on-failure`

Expected: 失败原因是导入器尚未定义，而不是测试资源错误。

- [ ] **Step 3: 实现格式验证、唯一文件名和复制**

使用 `QImageReader::supportedImageFormats()` 和 `QImageReader::read()` 验证图片；扫描目录时只取第一层普通文件。目标路径使用项目的 `photos/`，当目标文件存在时追加 `_2`, `_3` 等序号，禁止覆盖。成功复制后生成 UUID、相对路径、显示名和当前时间的 `PhotoRecord`，加入 `AlbumProject` 并保存。

- [ ] **Step 4: 加入失败文件汇总**

不存在、不可读、不是图片、复制失败的文件写入 `failed` 和 `failureReasons`，其他文件继续导入。导入器不弹窗，由 `MainWindow` 负责把结果显示给用户。

- [ ] **Step 5: 运行导入器测试和全量测试**

Run: `cmake --build build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --config Debug --parallel 2`，then `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --output-on-failure`

Expected: 项目模型和图片导入测试全部通过。

---

### Task 4: 实现非破坏式图片渲染和编辑核心

**Files:**
- Create: `imagerenderer.h`, `imagerenderer.cpp`
- Create: `tests/test_imagerenderer.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- `QImage ImageRenderer::render(const QImage&, const EditSettings&)`。
- `QImage ImageRenderer::applyCrop(const QImage&, const QRect&)`。
- `QImage ImageRenderer::applyBrightness(const QImage&, int)`。

- [ ] **Step 1: 写失败测试覆盖每种编辑行为**

```cpp
void ImageRendererTest::rotatesAndFlipsImage()
{
    QImage source(2, 1, QImage::Format_RGB32);
    source.setPixelColor(0, 0, Qt::red);
    source.setPixelColor(1, 0, Qt::blue);

    EditSettings edit;
    edit.rotation = 90;
    const QImage rotated = ImageRenderer::render(source, edit);
    QCOMPARE(rotated.size(), QSize(1, 2));

    edit = {};
    edit.flipHorizontal = true;
    const QImage flipped = ImageRenderer::render(source, edit);
    QCOMPARE(flipped.pixelColor(0, 0), QColor(Qt::blue));
}

void ImageRendererTest::cropsAndAppliesBrightness()
{
    QImage source(4, 4, QImage::Format_RGB32);
    source.fill(QColor(100, 100, 100));
    EditSettings edit;
    edit.crop = QRect(1, 1, 2, 2);
    edit.brightness = 50;
    const QImage result = ImageRenderer::render(source, edit);
    QCOMPARE(result.size(), QSize(2, 2));
    QVERIFY(result.pixelColor(0, 0).red() > 100);
}
```

- [ ] **Step 2: 运行测试确认渲染器缺失或失败**

Run: `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug -R image_renderer_tests --output-on-failure`

Expected: 因 `ImageRenderer` 尚未实现而失败。

- [ ] **Step 3: 实现最小渲染管线**

顺序固定为：有效裁剪区域 -> 旋转 -> 水平/垂直翻转 -> 亮度 -> 灰度。旋转只接受 `0/90/180/270` 并归一化；裁剪越界时与图片矩形求交，空交集则跳过裁剪；亮度使用像素通道加法并限制在 `0..255`。

- [ ] **Step 4: 运行渲染测试并覆盖边界值**

补充空图片、空裁剪、亮度 `-100/100`、灰度和 180 度旋转测试，运行 `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug -R image_renderer_tests --output-on-failure`。

Expected: 所有渲染测试通过，且不会产生空图崩溃。

---

### Task 5: 实现缩略图画廊和单图预览

**Files:**
- Create: `photogallerywidget.h`, `photogallerywidget.cpp`
- Create: `tests/test_gallery_smoke.cpp`
- Create: `photoviewerdialog.h`, `photoviewerdialog.cpp`, `photoviewerdialog.ui`
- Modify: `style/style.qss`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- `PhotoGalleryWidget::setPhotos(const QList<PhotoRecord>&, const QString& projectDir)`。
- `PhotoGalleryWidget::selectedPhotoIds()` 和 `selectedPhotoId()`。
- Signals: `photoActivated(QString)`, `selectionChanged(QStringList)`, `requestDelete(QStringList)`。
- `PhotoViewerDialog` 构造参数为当前记录列表、项目目录和起始索引，提供 `setPhotoIndex(int)`。

- [ ] **Step 1: 写 GUI 失败测试/冒烟测试**

在 `tests/test_gallery_smoke.cpp` 中使用下面的测试类创建 `PhotoGalleryWidget`，调用 `setPhotos()` 后断言 `count() == 2`，并断言第一项的 `Qt::UserRole` 保存正确的照片 ID。测试只验证组件状态，不依赖真实屏幕。

```cpp
class GallerySmokeTest : public QObject {
    Q_OBJECT
private slots:
    void bindsPhotoRecords();
};

void GallerySmokeTest::bindsPhotoRecords()
{
    PhotoGalleryWidget gallery;
    QList<PhotoRecord> photos;
    PhotoRecord first;
    first.id = "one";
    first.name = "one.png";
    first.relativePath = "photos/one.png";
    PhotoRecord second = first;
    second.id = "two";
    second.name = "two.png";
    photos << first << second;
    gallery.setPhotos(photos, QString());
    QCOMPARE(gallery.count(), 2);
    QCOMPARE(gallery.item(0)->data(Qt::UserRole).toString(), QString("one"));
}

QTEST_MAIN(GallerySmokeTest)
#include "test_gallery_smoke.moc"
```

- [ ] **Step 2: 运行冒烟测试确认组件未实现**

Run: `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug -R gallery_smoke_tests --output-on-failure`

Expected: 组件头文件或目标尚未存在导致失败。

- [ ] **Step 3: 实现图标模式画廊**

使用 `QListWidget::IconMode`、固定缩略图尺寸、单击选择和双击/回车激活。每个 item 保存照片 ID 和完整 `PhotoRecord` 的索引，不把路径作为唯一身份。没有图片时显示空状态文本；缩略图加载失败时显示默认图标和文件名。

- [ ] **Step 4: 实现预览窗口**

预览窗口使用 `QLabel` 居中显示渲染后的图片，上一张/下一张按钮在列表范围内循环；“适应窗口”使用 `scaled(..., Qt::KeepAspectRatio, Qt::SmoothTransformation)`，“原始尺寸”显示原始像素尺寸并允许滚动。预览窗口只读，不在其中修改项目。

- [ ] **Step 5: 运行画廊测试并手动检查选择行为**

Run: `cmake --build build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --config Debug --parallel 2`，then `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --output-on-failure`。

Expected: 画廊冒烟测试通过，键盘选择和双击激活信号可用。

---

### Task 6: 实现标签编辑和筛选界面

**Files:**
- Create: `tagdialog.h`, `tagdialog.cpp`, `tagdialog.ui`
- Modify: `albumproject.h`, `albumproject.cpp`
- Modify: `mainwindow.h`, `mainwindow.cpp`, `mainwindow.ui`

**Interfaces:**
- `TagDialog` 输入当前标签列表，确认时返回去重后的 `QStringList`。
- `MainWindow` 的标签栏调用 `AlbumProject::setActiveTag()` 后刷新画廊。
- `AlbumProject::renameTag(oldName, newName)` 和 `removeTag(name)` 保持所有照片关联一致。

- [ ] **Step 1: 写失败测试覆盖标签去重和筛选**

```cpp
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
}
```

- [ ] **Step 2: 运行测试确认接口缺失**

Run: `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug -R album_project_tests --output-on-failure`

Expected: `renameTag` 或筛选行为未实现导致失败。

- [ ] **Step 3: 实现模型标签操作**

`tags()` 始终从项目标签表和照片记录的并集生成排序后的列表；重命名必须拒绝空值、同名和不存在的旧标签；删除标签只从照片记录中移除关联。保存后保留当前筛选条件，若当前标签被删除则回到“全部”。

- [ ] **Step 4: 实现 TagDialog 和筛选栏**

对话框使用 `QListWidget` 勾选已有标签和 `QLineEdit` 新增标签；主窗口筛选栏使用 `QComboBox`，第一项固定为“全部”，标签变更时刷新画廊和状态栏。选中图片后点击“标签”打开对话框，确认后更新每张选中图片并保存。

- [ ] **Step 5: 运行标签测试和 GUI 冒烟检查**

Run: `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --output-on-failure`。

Expected: 标签测试通过；空项目、全部筛选和单标签筛选都能显示正确数量。

---

### Task 7: 实现图片编辑器和编辑结果持久化

**Files:**
- Create: `imageeditordialog.h`, `imageeditordialog.cpp`, `imageeditordialog.ui`
- Modify: `albumproject.h`, `albumproject.cpp`
- Modify: `mainwindow.h`, `mainwindow.cpp`

**Interfaces:**
- `ImageEditorDialog` 构造参数为原图和已有 `EditSettings`。
- Signal: `acceptedEdit(const EditSettings&, const QImage&)`。
- `AlbumProject::editedFilePath(const PhotoRecord&)` 根据照片 ID 生成 `edits/<id>.png`。

- [ ] **Step 1: 写失败测试覆盖编辑结果路径和重置**

```cpp
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
```

- [ ] **Step 2: 运行测试确认编辑路径接口缺失**

Run: `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug -R album_project_tests --output-on-failure`

Expected: 编辑路径接口尚未实现而失败。

- [ ] **Step 3: 实现编辑器控件和交互**

编辑器左侧显示预览，右侧提供旋转、水平/垂直翻转、亮度滑块、灰度复选框、裁剪模式和重置按钮。裁剪模式允许在预览图上拖拽矩形，按图片坐标换算到原图；每次控制变化都通过 `ImageRenderer::render()` 更新预览。

- [ ] **Step 4: 应用编辑并写入 edits/**

点击确定时用 `QImage::save()` 写入 `editedFilePath()`，只有写入成功后才调用 `AlbumProject::updatePhoto()` 和 `save()`。重置时删除编辑文件（若存在）、写回默认 `EditSettings`，原始照片文件保持不变。

- [ ] **Step 5: 运行编辑测试并手动验证重启恢复**

Run: `cmake --build build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --config Debug --parallel 2`，then `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --output-on-failure`。

Expected: 编辑结果尺寸/像素测试通过；关闭并重新打开项目后编辑参数和编辑图仍然存在。

---

### Task 8: 实现音乐控制器

**Files:**
- Create: `musiccontroller.h`, `musiccontroller.cpp`
- Create: `tests/test_musiccontroller.cpp`
- Modify: `albumproject.h`, `albumproject.cpp`
- Modify: `mainwindow.h`, `mainwindow.cpp`, `mainwindow.ui`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- `MusicController::setPlaylist(QStringList absoluteFiles)`。
- `addFiles(QStringList)`, `removeCurrent()`, `play()`, `pause()`, `stop()`, `next()`, `previous()`。
- `setVolume(int)`, `setLoop(bool)`, `currentIndex()`, `isPlaying()`。
- Signals: `stateChanged(QString)`, `trackChanged(int)`, `errorOccurred(QString)`。

- [ ] **Step 1: 写失败测试覆盖播放列表状态机**

测试不依赖实际扬声器，只验证添加、去重、当前索引、循环开关和失效项跳过：

```cpp
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
```

- [ ] **Step 2: 运行测试确认控制器未实现**

Run: `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug -R music_controller_tests --output-on-failure`

Expected: 控制器接口不存在或测试目标未定义。

- [ ] **Step 3: 实现 Qt Multimedia 播放**

使用 `QMediaPlayer` + `QAudioOutput`，控制器自己维护当前索引以兼容 Qt 6。`next()`/`previous()` 在列表边界按循环设置决定是否回到另一端；媒体播放结束时自动调用 `next()`。音量映射到 `QAudioOutput::setVolume()` 的 `0.0..1.0`。

- [ ] **Step 4: 保存和恢复项目音乐列表**

导入音频时复制到项目 `music/` 并保存相对路径；打开项目时只把存在的文件加入控制器，失效路径留在项目 JSON 中并通过 `errorOccurred` 提示。主窗口底部放播放/暂停、上一首、下一首、音量滑块和循环按钮。

- [ ] **Step 5: 运行音乐状态测试和完整构建**

Run: `cmake --build build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --config Debug --parallel 2`，then `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --output-on-failure`。

Expected: 状态测试通过；没有可用编解码器时只报告媒体错误，不影响程序启动。

---

### Task 9: 实现轮播并整合项目树、主窗口和所有操作

**Files:**
- Create: `slideshowdialog.h`, `slideshowdialog.cpp`, `slideshowdialog.ui`
- Create: `tests/test_mainwindow_smoke.cpp`
- Modify: `mainwindow.h`, `mainwindow.cpp`, `mainwindow.ui`
- Modify: `protree.h`, `protree.cpp`, `protreewidget.h`, `protreewidget.cpp`, `protreeitem.h`, `protreeitem.cpp`
- Modify: `wizard.cpp`, `prosetpage.cpp`, `style/style.qss`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- `ProTree` signals: `projectSelected(QString)`, `projectClosed(QString)`, `importRequested(QString)`, `slideshowRequested(QString)`。
- `ProTreeItem::getPath()` 返回项目绝对路径，并通过 `Qt::UserRole` 持久保存。
- `SlideshowDialog` 接收筛选后的照片、项目目录、间隔毫秒数和循环标志。
- `MainWindow::openProjectPath(const QString&)` 作为项目树和离屏冒烟测试共用的项目加载入口。

- [ ] **Step 1: 写失败 GUI 集成测试**

在 `tests/test_mainwindow_smoke.cpp` 中以 `QT_QPA_PLATFORM=offscreen` 创建 `MainWindow`，断言存在项目树、画廊、导入、标签、编辑和轮播动作；创建临时项目后调用公开的项目加载入口，断言状态栏显示项目名称。

```cpp
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
```

- [ ] **Step 2: 运行冒烟测试确认集成入口缺失**

Run: `$env:QT_QPA_PLATFORM='offscreen'; ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug -R mainwindow_smoke_tests --output-on-failure`

Expected: 因主窗口新控件和公开加载接口尚未实现而失败。

- [ ] **Step 3: 修复项目树状态和右键动作**

初始化 `_right_btn_item` 为 `nullptr`；创建项目后把路径放在 `Qt::UserRole`，点击项目发出 `projectSelected`；导入动作通过主窗口处理，不在 `ProTreeWidget` 中直接写业务数据；关闭动作移除树项并发出 `projectClosed`；设置活动项目只改变当前树项样式/状态；轮播动作发出信号。

- [ ] **Step 4: 改造主窗口布局和动作**

将 `_protree` 类型改为 `ProTree*`，添加 `PhotoGalleryWidget`、工具栏、标签 `QComboBox`、状态栏和音乐控制条。实现文件菜单的创建/打开/关闭/退出，图片菜单的导入文件/导入文件夹/编辑/标签/删除/轮播，所有动作根据“无项目/无选择/有选择”设置 enabled 状态。

- [ ] **Step 5: 接通创建、打开、导入、删除、编辑和保存**

创建向导成功后调用 `AlbumProject::create()` 并加入项目树；打开动作解析目录或 `album.json` 并加入项目树；项目选择时加载模型和音乐，刷新标签栏/画廊；导入通过 `PhotoImporter`，删除前确认并删除项目内原图和编辑图；编辑和标签操作完成后保存并刷新。

- [ ] **Step 6: 实现轮播窗口**

使用 `QTimer` 按间隔切换图片，支持空列表保护、暂停/继续、上一张/下一张、循环和 Esc 关闭；轮播显示使用与预览相同的 `ImageRenderer`，不复制编辑逻辑。

- [ ] **Step 7: 运行 GUI 冒烟和完整测试**

Run: `$env:QT_QPA_PLATFORM='offscreen'; cmake --build build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --config Debug --parallel 2`，then `$env:QT_QPA_PLATFORM='offscreen'; ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --output-on-failure`。

Expected: 项目生命周期、画廊数量、标签筛选、编辑保存、轮播入口和音乐控制全部可用，测试无失败。

---

### Task 10: 文档、资源和最终验证

**Files:**
- Create: `README.md`
- Modify: `style/style.qss`, `CMakeLists.txt`
- Modify: all new and changed source files as needed for warnings/errors found by the final build

- [ ] **Step 1: 写 README 使用说明**

说明 Qt 6/CMake/MSVC 构建命令、运行方式、创建项目、导入图片、标签、编辑、轮播和音乐的操作路径；说明项目目录结构和“编辑保留原图”的行为。

- [ ] **Step 2: 扫描占位符和明显错误**

Run: `Select-String -Path .\*.cpp,.\*.h,.\*.ui,.\README.md -Pattern 'project has exists|WizardPage|Dialog'`

Expected: 不存在用户可见的旧占位文案或遗留英文错误提示；若命中合法技术文本，逐项判断并修正用户可见项。

- [ ] **Step 3: 运行 Debug 构建和完整测试**

Run: `cmake --build build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --config Debug --parallel 2`，then `ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --output-on-failure`。

Expected: 构建退出码为 0，CTest 所有测试通过；如果当前 MSVC 环境仍找不到 `type_traits`，用完整 VS 开发者命令环境重跑并记录结果。

- [ ] **Step 4: 做一次手动端到端检查**

创建临时项目，导入两张测试图片和一首音频，验证缩略图、预览、标签筛选、旋转/裁剪/亮度、重启恢复、轮播和音乐按钮；检查项目目录中原图、编辑图和 JSON 均存在。

- [ ] **Step 5: 检查最终变更范围**

Run: `Get-ChildItem -Recurse -File | Where-Object { $_.FullName -notmatch '\\build\\' } | Sort-Object FullName`，确认只包含项目源码、测试、文档和必要资源；当前环境没有 Git 仓库，使用文件清单和最终测试输出作为交付记录。
