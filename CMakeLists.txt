cmake_minimum_required(VERSION 3.16)

project(hospital-qt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    src/style/theme.h
    src/style/theme.cpp
    src/models/datatypes.h
    src/api/httpclient.h
    src/api/httpclient.cpp
    src/api/apis.h
    src/api/apis.cpp
    src/widgets/toastwidget.h
    src/widgets/toastwidget.cpp
    src/widgets/loadingoverlay.h
    src/widgets/loadingoverlay.cpp
    src/widgets/chatbubble.h
    src/widgets/chatbubble.cpp
    src/views/loginwidget.h
    src/views/loginwidget.cpp
    src/views/patientwidget.h
    src/views/patientwidget.cpp
    src/views/cswidget.h
    src/views/cswidget.cpp
    src/views/doctorwidget.h
    src/views/doctorwidget.cpp
    src/views/adminwidget.h
    src/views/adminwidget.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(hospital-qt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(hospital-qt SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(hospital-qt
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(hospital-qt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

target_include_directories(hospital-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.hospital-qt)
endif()
set_target_properties(hospital-qt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS hospital-qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(hospital-qt)
endif()
