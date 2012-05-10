project(kbounce)

if(NOT COMPILING_KDEGAMES)
	find_package(KDE4 REQUIRED)
	include(KDE4Defaults)
	include(MacroLibrary)
	find_package(KDEGames REQUIRED)

	add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
	add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
	include_directories(${QDBUS_INCLUDE_DIRS} ${KDE4_INCLUDES} ${KDEGAMES_INCLUDE_DIRS})
endif(NOT COMPILING_KDEGAMES)

add_subdirectory( themes ) 
add_subdirectory( sounds ) 
add_subdirectory( pics ) 

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
	sound.cpp
    wall.cpp 
    backgroundselector.cpp)

kde4_add_kcfg_files(kbounce_SRCS settings.kcfgc)
kde4_add_ui_files(kbounce_SRCS  backgroundselector.ui )
kde4_add_app_icon(kbounce_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/hi*-app-kbounce.png")
kde4_add_executable(kbounce ${kbounce_SRCS})

target_link_libraries(kbounce ${KDE4_KDEUI_LIBS} kdegames kdegamesprivate ${KDE4_PHONON_LIBS} ${KDE4_KIO_LIBS})
if(KDE4_ENABLE_FPIE)
    target_link_libraries(kbounce ${KDE4_PIE_LDFLAGS})
endif(KDE4_ENABLE_FPIE)


install(TARGETS kbounce  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS kbounce.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kbounceui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kbounce )



