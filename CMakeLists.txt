cmake_minimum_required(VERSION 3.16)

project(QmlTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Quick OpenGL)

qt_standard_project_setup(REQUIRES 6.10)
set(CMAKE_AUTORCC ON)

qt_add_executable(appQmlTest
    main.cpp
    chartitem.cpp
    chartitem.h
)

qt_add_qml_module(appQmlTest
    URI QmlTest
    QML_FILES
        Main.qml
        SOURCES chartitem.h chartitem.cpp
        RESOURCES resources.qrc
        SOURCES chartstrategy.h
        SOURCES linechartstrategy.h linechartstrategy.cpp
        SOURCES barchartstrategy.h barchartstrategy.cpp
        SOURCES piechartstrategy.h piechartstrategy.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appQmlTest PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appQmlTest
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appQmlTest
    PRIVATE Qt6::Quick
    Qt6::Gui
    Qt6::OpenGL
)

install(TARGETS appQmlTest
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
