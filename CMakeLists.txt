project(kbounce)

add_subdirectory( themes ) 
add_subdirectory( sounds ) 

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
    wall.cpp )

kde4_add_kcfg_files(kbounce_SRCS settings.kcfgc)

set(kbounce_icons
	${CMAKE_CURRENT_SOURCE_DIR}/pics/hi16-app-kbounce.png
	${CMAKE_CURRENT_SOURCE_DIR}/pics/hi32-app-kbounce.png
	${CMAKE_CURRENT_SOURCE_DIR}/pics/hi48-app-kbounce.png
	${CMAKE_CURRENT_SOURCE_DIR}/pics/hi64-app-kbounce.png
	${CMAKE_CURRENT_SOURCE_DIR}/pics/hi128-app-kbounce.png
)
kde4_add_win32_app_icon(kbounce_SRCS ${kbounce_icons})
kde4_add_executable(kbounce ${kbounce_SRCS})

if(KDE4_ENABLE_FPIE)
    target_link_libraries(kbounce  ${KDE4_KDEUI_LIBS}  kdegames ${KDE4_PHONON_LIBS} ${KDE4_PIE_LDFLAGS})
else(KDE4_ENABLE_FPIE)
    target_link_libraries(kbounce  ${KDE4_KDEUI_LIBS}  kdegames ${KDE4_PHONON_LIBS})
endif(KDE4_ENABLE_FPIE)


install(TARGETS kbounce  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kbounce.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kbounceui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kbounce )


