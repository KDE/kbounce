SUBDIRS = pics sounds
bin_PROGRAMS = kjezz
INCLUDES = -I$(top_srcdir)/libkdegames $(all_includes)

kjezz_SOURCES = main.cpp kjezz.cpp game.cpp
kjezz_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(kde_libraries)/libsoundserver_idl.la
kjezz_LDADD = $(top_builddir)/libkdegames/libkdegames.la $(LIB_KDEUI) $(LIB_KFILE)

EXTRA_DIST = kjezz.xpm kjezz.desktop

appdir = $(kde_datadir)/kjezz
app_DATA = kjezzui.rc

appsdir = $(kde_appsdir)/Games/Arcade
apps_DATA = kjezz.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kjezz.pot

install-data-local:
	rm -rf $(kde_appsdir)/Games/kjezz.desktop
