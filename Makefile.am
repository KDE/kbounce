SUBDIRS = pics sounds
bin_PROGRAMS = kbounce
INCLUDES = -I$(top_srcdir)/libkdegames $(all_includes)

kbounce_SOURCES = main.cpp kbounce.cpp game.cpp
kbounce_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(kde_libraries)/libsoundserver_idl.la
kbounce_LDADD = $(top_builddir)/libkdegames/libkdegames.la $(LIB_KDEUI) $(LIB_KFILE)

EXTRA_DIST = kbounce.desktop game.h kbounce.h RULES

appdir = $(kde_datadir)/kbounce
app_DATA = kbounceui.rc

appsdir = $(kde_appsdir)/Games/Arcade
apps_DATA = kbounce.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kbounce.pot

install-data-local:
	rm -rf $(DESTDIR)$(kde_appsdir)/Games/kbounce.desktop
