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
    CATEGORY_NAME log_kbounce
    DESCRIPTION "KBounce game"
    EXPORT KBOUNCE
)

file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/pics/*-apps-kbounce.png")
ecm_add_app_icon(kbounce ICONS ${ICONS_SRCS})

target_link_libraries(kbounce 
    KF5KDEGames
    KF${KF_MAJOR_VERSION}::XmlGui
    KF${KF_MAJOR_VERSION}::I18n 
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KF${KF_MAJOR_VERSION}::DBusAddons
    KF${KF_MAJOR_VERSION}::Crash
    Qt::Svg 
)

install(TARGETS kbounce  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.kbounce.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kbounce.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ecm_qt_install_logging_categories(
    EXPORT KBOUNCE
    FILE kbounce.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
