add_executable(album_project_tests
    test_albumproject.cpp
    ../albumtypes.cpp
    ../albumproject.cpp
)

add_executable(photo_importer_tests
    test_photoimporter.cpp
    ../albumtypes.cpp
    ../albumproject.cpp
    ../photoimporter.cpp
)

add_executable(image_renderer_tests
    test_imagerenderer.cpp
    ../albumtypes.cpp
    ../imagerenderer.cpp
)

add_executable(gallery_smoke_tests
    test_gallery_smoke.cpp
    ../albumtypes.cpp
    ../imagerenderer.cpp
    ../photogallerywidget.cpp
)

add_executable(music_controller_tests
    test_musiccontroller.cpp
    ../musiccontroller.cpp
)

add_executable(mainwindow_smoke_tests
    test_mainwindow_smoke.cpp
    ../mainwindow.cpp
    ../albumtypes.cpp
    ../albumproject.cpp
    ../photoimporter.cpp
    ../imagerenderer.cpp
    ../photogallerywidget.cpp
    ../photoviewerdialog.cpp
    ../imageeditordialog.cpp
    ../tagdialog.cpp
    ../musiccontroller.cpp
    ../slideshowdialog.cpp
    ../wizard.cpp
    ../prosetpage.cpp
    ../confirmpage.cpp
    ../protree.cpp
    ../protreewidget.cpp
    ../protreeitem.cpp
    ../res.qrc
    ../mainwindow.ui
    ../wizard.ui
    ../prosetpage.ui
    ../confirmpage.ui
    ../protree.ui
)

target_include_directories(album_project_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(album_project_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(photo_importer_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(photo_importer_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(image_renderer_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(image_renderer_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(gallery_smoke_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(gallery_smoke_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(music_controller_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(music_controller_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Multimedia
)

target_include_directories(mainwindow_smoke_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(mainwindow_smoke_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
)

add_test(NAME album_project_tests COMMAND album_project_tests)
add_test(NAME photo_importer_tests COMMAND photo_importer_tests)
add_test(NAME image_renderer_tests COMMAND image_renderer_tests)
add_test(NAME gallery_smoke_tests COMMAND gallery_smoke_tests)
add_test(NAME music_controller_tests COMMAND music_controller_tests)
add_test(NAME mainwindow_smoke_tests COMMAND mainwindow_smoke_tests)
set_tests_properties(mainwindow_smoke_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
