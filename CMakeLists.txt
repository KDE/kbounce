project(kbounce)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.2.0")

find_package(ECM 0.0.9 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(DATA_INSTALL_DIR "/usr/local/share")

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Qml Quick Svg Test)
find_package(KF5 REQUIRED COMPONENTS CoreAddons Config ItemModels WidgetsAddons WindowSystem Codecs Archive
    Config DBusAddons DNSSD Declarative
    I18n GuiAddons Service ConfigWidgets ItemViews IconThemes Completion JobWidgets TextWidgets GlobalAccel XmlGui Crash
    Bookmarks KIO NotifyConfig NewStuff KDELibs4Support)
    
find_package(KDEGames 4.9.0 REQUIRED)
find_package(Phonon4Qt5 REQUIRED)

include(FeatureSummary)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${KDEGAMES_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QDBUS_INCLUDE_DIRS})

add_subdirectory( themes ) 
add_subdirectory( sounds ) 
add_subdirectory( pics ) 
add_subdirectory( doc )

include_directories(${CMAKE_SOURCE_DIR}/libkdegames/highscore)
 
if(KDE4_ENABLE_FPIE)
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_CXX_FPIE_FLAGS}")
endif(KDE4_ENABLE_FPIE)

########### next target ###############

set(kbounce_SRCS
    ball.cpp
    board.cpp
    gameobject.cpp
    gamewidget.cpp
    main.cpp 
    mainwindow.cpp 
    renderer.cpp
    wall.cpp 
    backgroundselector.cpp)

kconfig_add_kcfg_files(kbounce_SRCS settings.kcfgc)
qt5_wrap_ui(kbounce_SRCS  backgroundselector.ui )
# kde4_add_app_icon(kbounce_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/hi*-app-kbounce.png") TODO Port!
add_executable(kbounce ${kbounce_SRCS})

target_link_libraries(kbounce KF5:KDELibs4Support kdegames Phonon:phonon4qt5 KF5:KIOWidgets )

if(KDE4_ENABLE_FPIE)
    target_link_libraries(kbounce ${KDE4_PIE_LDFLAGS})
endif(KDE4_ENABLE_FPIE)


install(TARGETS kbounce  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS kbounce.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kbounceui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kbounce )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

