lib_LTLIBRARIES = persistentMute.la

persistentMute_la_SOURCES = persistentMute.cpp
persistentMute_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
persistentMute_la_LDFLAGS = -module -avoid-version -shared
persistentMute_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.persistentMute.txt \
	README.md \
	persistentMute.def \
	persistentMute.sln \
	persistentMute.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
