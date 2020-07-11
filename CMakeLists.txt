cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "20")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "80")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kbounce VERSION ${RELEASE_SERVICE_VERSION})

set (QT_MIN_VERSION "5.7.0")
set (KF5_MIN_VERSION "5.30.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Svg)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons 
    Config 
    Crash
    WidgetsAddons 
    DBusAddons 
    I18n 
    ConfigWidgets 
    Completion 
    XmlGui
    KIO
    DocTools
)

find_package(KF5KDEGames 4.9.0 REQUIRED)

include(FeatureSummary)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMAddAppIcon)
include(ECMSetupVersion)


add_definitions(
    -DQT_USE_FAST_CONCATENATION
    -DQT_USE_FAST_OPERATOR_PLUS
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_USE_QSTRINGBUILDER
)

add_subdirectory(themes)
add_subdirectory(sounds)
add_subdirectory(pics)
add_subdirectory(doc)

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
    backgroundselector.cpp
)

ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX KBOUNCE VERSION_HEADER kbounce_version.h)

kconfig_add_kcfg_files(kbounce_SRCS settings.kcfgc)
ki18n_wrap_ui(kbounce_SRCS  backgroundselector.ui )
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/*-apps-kbounce.png")
ecm_add_app_icon(kbounce_SRCS ICONS ${ICONS_SRCS})

add_executable(kbounce ${kbounce_SRCS})

target_link_libraries(kbounce 
  Qt5::Svg 
  KF5::XmlGui
  KF5::I18n 
  KF5::KIOWidgets
  KF5::DBusAddons
  KF5::Crash
  KF5KDEGames
)

install(TARGETS kbounce  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(PROGRAMS org.kde.kbounce.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kbounce.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kbounceui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/kbounce)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

