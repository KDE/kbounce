SUBDIRS = pics sounds

INCLUDES = -I$(top_srcdir)/libkdegames -I$(top_srcdir)/libkdegames/highscore $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = kbounce
kbounce_SOURCES = main.cpp kbounce.cpp game.cpp
kbounce_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kbounce_LDADD = $(LIB_KDEGAMES) $(LIB_KFILE) -lsoundserver_idl
kbounce_DEPENDENCIES = $(LIB_KDEGAMES_DEP)

xdg_apps_DATA = kbounce.desktop

EXTRA_DIST = RULES $(xdg_apps_DATA)

rcdir = $(kde_datadir)/kbounce
rc_DATA = kbounceui.rc

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kbounce.pot

