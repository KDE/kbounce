SUBDIRS = pics
bin_PROGRAMS = kjezz
INCLUDES = $(all_includes)

kjezz_SOURCES = main.cpp kjezz.cpp game.cpp
kjezz_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kjezz_LDADD = $(LIB_KDEUI)

EXTRA_DIST = kjezz.xpm kjezz.desktop

appdir = $(kde_datadir)/kjezz
app_DATA = kjezzui.rc

KDE_ICON = kjezz

appsdir = $(kde_appsdir)/Games
apps_DATA = kjezz.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kjezz.pot
