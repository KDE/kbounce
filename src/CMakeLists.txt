ecm_setup_version(${RELEASE_SERVICE_VERSION}
    VARIABLE_PREFIX KBOUNCE
    VERSION_HEADER kbounce_version.h
)

add_executable(kbounce)

target_sources(kbounce PRIVATE
    backgroundselector.cpp
    backgroundselector.h
    ball.cpp
    ball.h
    board.cpp
    board.h
    gameobject.cpp
    gameobject.h
    gamewidget.cpp
    gamewidget.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    renderer.cpp
    renderer.h
    wall.cpp
    wall.h

    kbounce.qrc
)

kconfig_add_kcfg_files(kbounce settings.kcfgc)

ki18n_wrap_ui(kbounce  backgroundselector.ui )

ecm_qt_declare_logging_category(kbounce
    HEADER debug.h
    IDENTIFIER KBOUNCE_LOG
    CATEGORY_NAME kbounce
    OLD_CATEGORY_NAMES log_kbounce
    DESCRIPTION "KBounce game"
    EXPORT KBOUNCE
)

file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/pics/*-apps-kbounce.png")
ecm_add_app_icon(kbounce ICONS ${ICONS_SRCS})

if(APPLE)
    set_target_properties(kbounce PROPERTIES
        MACOSX_BUNDLE_DISPLAY_NAME "KBounce"
        MACOSX_BUNDLE_BUNDLE_NAME "KBounce"
        MACOSX_BUNDLE_LONG_VERSION_STRING "KBounce ${RELEASE_SERVICE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${RELEASE_SERVICE_VERSION}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.kbounce"
        MACOSX_BUNDLE_COPYRIGHT "2024 The KBounce Developers")
endif()


target_link_libraries(kbounce 
    KDEGames6
    KF6::XmlGui
    KF6::I18n
    KF6::KIOWidgets
    KF6::DBusAddons
    KF6::Crash
    KF6::IconThemes
    Qt6::Svg
)

install(TARGETS kbounce  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.kbounce.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kbounce.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ecm_qt_install_logging_categories(
    EXPORT KBOUNCE
    FILE kbounce.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
