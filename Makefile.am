SUBDIRS = pics sounds

INCLUDES = -I$(top_srcdir)/libkdegames -I$(top_srcdir)/libkdegames/highscore $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEGAMES) $(LIB_KFILE) -lsoundserver_idl
METASOURCES = AUTO

bin_PROGRAMS = kbounce
kbounce_SOURCES = main.cpp kbounce.cpp game.cpp

appsdir = $(kde_appsdir)/Games/Arcade
apps_DATA = kbounce.desktop

EXTRA_DIST = RULES $(apps_DATA)

rcdir = $(kde_datadir)/kbounce
rc_DATA = kbounceui.rc

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kbounce.pot

install-data-local:
	rm -rf $(DESTDIR)$(kde_appsdir)/Games/kbounce.desktop
