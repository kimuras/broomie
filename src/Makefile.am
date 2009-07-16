lib_LTLIBRARIES    = libbrm.la
libbrm_la_CPPFLAGS = -Wall
libbrm_la_LDFLAGS  = -I/usr/local/include -L/usr/local/lib `tcucodec conf -l` -loll -lfcgi -ltinysvm
libbrm_la_SOURCES  = libbrm.hpp libbrm.cpp brmalgorithm.hpp brmalgorithm.cpp brmNB.hpp brmNB.cpp

pkginclude_HEADERS = libbrm.hpp brmalgorithm.hpp brmutil.hpp brmNB.hpp

bin_PROGRAMS    = classifier.fcgi brmtest brmtrain brmclassify brmNBtest

brmtrain_CPPFLAGS = -Wall
brmtrain_LDADD    = libbrm.la
brmtrain_LDFLAGS  = -L/usr/local/lib `tcucodec conf -l`
brmtrain_SOURCES  = brmtrain.cpp

brmclassify_CPPFLAGS = -Wall
brmclassify_LDADD    = libbrm.la
brmclassify_LDFLAGS  = -L/usr/local/lib `tcucodec conf -l`
brmclassify_SOURCES  = brmclassify.cpp

classifier_fcgi_CPPFLAGS = -Wall
classifier_fcgi_LDADD    = libbrm.la
classifier_fcgi_LDFLAGS  = -L/usr/local/lib `tcucodec conf -l` -lfcgi
classifier_fcgi_SOURCES  = brminterface.cpp

check_PROGRAMS = brmtest

brmtest_CPPFLAGS = -Wall
brmtest_LDADD    = libbrm.la
brmtest_LDFLAGS  = -L/usr/local/lib -lgtest
brmtest_SOURCES  = brmtest.cpp

brmNBtest_CPPFLAGS = -Wall
brmNBtest_LDADD    = libbrm.la
brmNBtest_LDFLAGS  = -L/usr/local/lib -lgtest
brmNBtest_SOURCES  = brmNBtest.cpp


TESTS = brmtest


#brmNBtest.cpp