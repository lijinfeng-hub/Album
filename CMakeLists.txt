cmake_minimum_required(VERSION 3.16)

project(Album VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia Test)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        appmetadata.h
        mainwindow.ui
        confirmpage.h
        prosetpage.h
        albumtypes.h
        albumtypes.cpp
        albumproject.h
        albumproject.cpp
        photoimporter.h
        photoimporter.cpp
        imagerenderer.h
        imagerenderer.cpp
        photogallerywidget.h
        photogallerywidget.cpp
        photoviewerdialog.h
        photoviewerdialog.cpp
        tagdialog.h
        tagdialog.cpp
        imageeditordialog.h
        imageeditordialog.cpp
        musiccontroller.h
        musiccontroller.cpp
        slideshowdialog.h
        slideshowdialog.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Album
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        res.qrc
        wizard.h wizard.cpp wizard.ui
        prosetpage.h prosetpage.cpp prosetpage.ui
        confirmpage.h confirmpage.cpp confirmpage.ui
        protree.h protree.cpp protree.ui
        protreewidget.h protreewidget.cpp
        protreeitem.h protreeitem.cpp
        const.h



    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Album APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Album SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Album
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Album PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
)
target_include_directories(Album PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Album)
endif()
set_target_properties(Album PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Album
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Album)
endif()

enable_testing()
add_subdirectory(tests)
