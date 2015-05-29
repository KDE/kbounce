project(kbounce)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.2.0")

find_package(ECM 1.3.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Svg Test)
find_package(KF5 REQUIRED COMPONENTS 
    CoreAddons 
    Config 
    WidgetsAddons 
    DBusAddons 
    I18n 
    GuiAddons 
    ConfigWidgets 
    IconThemes 
    Completion 
    TextWidgets
    XmlGui
    KIO
    NotifyConfig
    )
    
find_package(KF5KDEGames 4.9.0 REQUIRED)
find_package(Phonon4Qt5 REQUIRED)

include(FeatureSummary)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

add_subdirectory( themes ) 
add_subdirectory( sounds ) 
add_subdirectory( pics ) 
add_subdirectory( doc )

include_directories(${CMAKE_SOURCE_DIR}/KF5KDEGames/highscore)
 
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
ki18n_wrap_ui(kbounce_SRCS  backgroundselector.ui )
kde4_add_app_icon(kbounce_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/hi*-app-kbounce.png") 
add_executable(kbounce ${kbounce_SRCS})

target_link_libraries(kbounce 
  Qt5::Svg 
  KF5::XmlGui
  KF5::I18n 
  Phonon::phonon4qt5 
  KF5::KIOWidgets
  KF5::DBusAddons
  KF5KDEGames )

install(TARGETS kbounce  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS org.kde.kbounce.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kbounceui.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/kbounce )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

