kde4_header()

add_subdirectory( pics ) 
add_subdirectory( sounds ) 

include_directories( ${CMAKE_SOURCE_DIR}/libkdegames ${CMAKE_SOURCE_DIR}/libkdegames/highscore ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kbounce_SRCS 
   main.cpp 
   kbounce.cpp 
   game.cpp 
   highscores.cpp )

kde4_automoc(${kbounce_SRCS})

kde4_add_executable(kbounce ${kbounce_SRCS})

target_link_libraries(kbounce  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kbounce )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kbounce.desktop )
install_files( ${DATA_INSTALL_DIR}/kbounce FILES kbounceui.rc )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = pics sounds
#
#INCLUDES = -I$(top_srcdir)/libkdegames -I$(top_srcdir)/libkdegames/highscore $(all_includes)
#METASOURCES = AUTO
#KDE_CXXFLAGS = $(KDE_USE_FPIE)
#
#bin_PROGRAMS = kbounce
#kbounce_SOURCES = main.cpp kbounce.cpp game.cpp highscores.cpp
#kbounce_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(KDE_USE_PIE)
#kbounce_LDADD = $(LIB_KDEGAMES) $(LIB_KFILE) $(LIB_ARTS) 
#kbounce_DEPENDENCIES = $(LIB_KDEGAMES_DEP)
#
#xdg_apps_DATA = kbounce.desktop
#
#EXTRA_DIST = RULES $(xdg_apps_DATA)
#
#rcdir = $(kde_datadir)/kbounce
#rc_DATA = kbounceui.rc
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kbounce.pot
#
## for system-wide highscore file
#DESTBIN = $(DESTDIR)$(bindir)/$(bin_PROGRAMS)
#DESTHIGHSCORES = $(DESTDIR)$(HIGHSCORE_DIRECTORY)
#DESTSCORES = $(DESTDIR)$(HIGHSCORE_DIRECTORY)/$(bin_PROGRAMS).scores
#
#install-data-local:
#	@if test x$(HIGHSCORE_DIRECTORY) != x; then \
#	  echo "********************************************************" ;\
#	  echo "" ;\
#	  echo "This game is installed sgid \"games\" to use the" ;\
#	  echo "system-wide highscore file (in "$(HIGHSCORE_DIRECTORY)")." ;\
#	  echo "" ;\
#	  echo "If the system-wide highscore file does not exist, it is" ;\
#          echo "created with the correct ownership and permissions. See the" ;\
#          echo "INSTALL file in \"kdegames/libkdegames/highscore\" for details." ;\
#	  echo "" ;\
#	  echo "********************************************************" ;\
#	fi 
#
#install-exec-hook:
#	@if test x$(HIGHSCORE_DIRECTORY) != x; then \
#	  chown $(highscore_user):$(highscore_group) $(DESTBIN) \
#	  || echo "Error: Could not install the game with correct permissions !!" ;\
#	fi
#
#	@if test x$(HIGHSCORE_DIRECTORY) != x; then \
#		mkdir -p $(DESTHIGHSCORES) && \
#		chown $(highscore_user):$(highscore_group) $(DESTHIGHSCORES) \
#		&& chmod 750 $(DESTHIGHSCORES) \
#	        || echo "Error: Could not create the highscore directory with correct permissions !!" ;\
#	fi
#
#	@if test x$(HIGHSCORE_DIRECTORY) != x; then \
#	   chown $(highscore_user):$(highscore_group) $(DESTBIN) \
#	   || echo "Error: Could not install the game with correct permissions !!" ;\
#	fi
#
#	@if test ${setgid} = true; then \
#	   chmod 2755  $(DESTBIN) \
#	   || echo "Error: Could not install the game with correct permissions !!" ;\
#	fi
#
#	@if test x$(HIGHSCORE_DIRECTORY) != x; then \
#	   touch $(DESTSCORES) && chown $(highscore_user):$(highscore_group) $(DESTSCORES) \
#	   && chmod 0660 $(DESTSCORES) \
#	   || echo "Error: Could not create system-wide highscore file with correct permissions !!" ;\
#	fi
#
