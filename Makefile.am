SUBDIRS = pics sounds
bin_PROGRAMS = kjezzball
INCLUDES = -I$(top_srcdir)/libkdegames $(all_includes)

kjezzball_SOURCES = main.cpp kjezzball.cpp game.cpp
kjezzball_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(kde_libraries)/libsoundserver_idl.la
kjezzball_LDADD = $(top_builddir)/libkdegames/libkdegames.la $(LIB_KDEUI) $(LIB_KFILE)

EXTRA_DIST = kjezzball.desktop game.h kjezzball.h RULES

appdir = $(kde_datadir)/kjezzball
app_DATA = kjezzballui.rc

appsdir = $(kde_appsdir)/Games/Arcade
apps_DATA = kjezzball.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kjezzball.pot

install-data-local:
	rm -rf $(kde_appsdir)/Games/kjezzball.desktop
