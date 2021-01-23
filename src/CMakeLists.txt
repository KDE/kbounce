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

ecm_setup_version(${RELEASE_SERVICE_VERSION}
    VARIABLE_PREFIX KBOUNCE
    VERSION_HEADER kbounce_version.h
)

kconfig_add_kcfg_files(kbounce_SRCS settings.kcfgc)

ki18n_wrap_ui(kbounce_SRCS  backgroundselector.ui )

ecm_qt_declare_logging_category(kbounce_SRCS
    HEADER debug.h
    IDENTIFIER KBOUNCE_LOG
    CATEGORY_NAME log_kbounce
    DESCRIPTION "KBounce game"
    EXPORT KBOUNCE
)

file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/pics/*-apps-kbounce.png")
ecm_add_app_icon(kbounce_SRCS ICONS ${ICONS_SRCS})

add_executable(kbounce ${kbounce_SRCS})

target_link_libraries(kbounce 
    KF5KDEGames
    KF5::XmlGui
    KF5::I18n 
    KF5::KIOWidgets
    KF5::DBusAddons
    KF5::Crash
    Qt5::Svg 
)

install(TARGETS kbounce  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.kbounce.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kbounce.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kbounceui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/kbounce)

ecm_qt_install_logging_categories(
    EXPORT KBOUNCE
    FILE kbounce.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
