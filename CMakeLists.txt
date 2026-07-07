cmake_minimum_required(VERSION 3.16)

project(QmlTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Quick OpenGL)

qt_standard_project_setup(REQUIRES 6.10)
set(CMAKE_AUTORCC ON)

# Thiết lập thư mục xuất các module QML theo đúng chuẩn URI (tránh lỗi module is not installed)
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# -------------------------------------------------------------------
# 1. TARGET THƯ VIỆN ĐỘNG / QT PLUGIN (ChartPlotterPlugin)
# -------------------------------------------------------------------
qt_add_library(ChartPlotterPlugin SHARED)

# Đảm bảo tự động xuất toàn bộ symbols trên môi trường Windows DLL (MinGW / MSVC)
set_target_properties(ChartPlotterPlugin PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

qt_add_qml_module(ChartPlotterPlugin
    URI MyChartLibrary
    VERSION 1.0
    QML_FILES
        ui/components/TopNavBar.qml
        ui/components/ChartTooltip.qml
        ui/components/CrosshairOverlay.qml
        ui/components/AxisLabels.qml
        ui/components/FooterStatusBar.qml
    RESOURCES resources.qrc
    SOURCES
        src/chartitem.h
        src/chartitem.cpp
        src/charthittester.h
        src/charthittester.cpp
        src/chartaxiscalculator.h
        src/chartaxiscalculator.cpp
        src/chartrenderer.h
        src/chartrenderer.cpp
        src/dataprocessor.h
        src/dataprocessor.cpp
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
        src/strategies/chartstrategyfactory.h
        src/strategies/chartstrategyfactory.cpp
        src/datamanager.h
        src/datamanager.cpp
        src/fileloader.h
        src/fileloader.cpp
)

target_link_libraries(ChartPlotterPlugin
    PRIVATE Qt6::Quick
    Qt6::Gui
    Qt6::OpenGL
)

target_include_directories(ChartPlotterPlugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# -------------------------------------------------------------------
# 2. TARGET ỨNG DỤNG DEMO (appQmlTest)
# -------------------------------------------------------------------
qt_add_executable(appQmlTest
    src/main.cpp
)

qt_add_qml_module(appQmlTest
    URI QmlTest
    QML_FILES
        ui/Main.qml
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
    ChartPlotterPlugin
)

install(TARGETS ChartPlotterPlugin appQmlTest
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
