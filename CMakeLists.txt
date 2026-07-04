cmake_minimum_required(VERSION 3.16)

project(QmlTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Quick OpenGL)

qt_standard_project_setup(REQUIRES 6.10)
set(CMAKE_AUTORCC ON)

qt_add_executable(appQmlTest
    src/main.cpp
    src/chartitem.h
    src/chartitem.cpp
    src/viewportmanager.h
    src/viewportmanager.cpp
    src/onlinestream.h
    src/onlinestream.cpp
    src/chartstrategy.h
    src/strategies/linechartstrategy.h
    src/strategies/linechartstrategy.cpp
    src/strategies/barchartstrategy.h
    src/strategies/barchartstrategy.cpp
    src/strategies/piechartstrategy.h
    src/strategies/piechartstrategy.cpp
)
qt_add_qml_module(appQmlTest
    URI QmlTest
    QML_FILES
        ui/Main.qml
        RESOURCES resources.qrc
        SOURCES src/datamanager.h src/datamanager.cpp
        SOURCES src/fileloader.h src/fileloader.cpp
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
