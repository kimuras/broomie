lib_LTLIBRARIES    = libbrm.la
libbrm_la_CPPFLAGS = -Wall  $(BRM_USE_OLL) $(BRM_USE_TINYSVM) $(BRM_DEBUG)
libbrm_la_LDFLAGS  = -I/usr/local/include -L/usr/local/lib `tcucodec conf -l` $(BRM_LIB_OLL) $(BRM_LIB_TINYSVM)
libbrm_la_SOURCES  = libbrm.hpp libbrm.cpp brmalgorithm.hpp brmalgorithm.cpp brmNB.hpp brmNB.cpp

pkginclude_HEADERS = libbrm.hpp brmalgorithm.hpp brmutil.hpp brmNB.hpp

bin_PROGRAMS = brmtrain brmclassify

if ENABLE_FCGI
    bin_PROGRAMS += classifier.fcgi
endif

if ENABLE_TEST
    bin_PROGRAMS += brmtest
    bin_PROGRAMS += brmNBtest
endif

brmtrain_CPPFLAGS = -Wall $(BRM_USE_OLL) $(BRM_USE_TINYSVM) $(BRM_DEBUG)
brmtrain_LDADD    = libbrm.la
brmtrain_LDFLAGS  =
brmtrain_SOURCES  = brmtrain.cpp

brmclassify_CPPFLAGS = -Wall $(BRM_USE_OLL) $(BRM_USE_TINYSVM) $(BRM_DEBUG)
brmclassify_LDADD    = libbrm.la
brmclassify_LDFLAGS  =
brmclassify_SOURCES  = brmclassify.cpp

if ENABLE_FCGI
    classifier_fcgi_CPPFLAGS = -Wall $(BRM_USE_OLL) $(BRM_USE_TINYSVM) $(BRM_DEBUG)
    classifier_fcgi_LDADD    = libbrm.la
    classifier_fcgi_LDFLAGS  = -L/usr/local/lib -lfcgi
    classifier_fcgi_SOURCES  = brminterface.cpp
endif

if ENABLE_TEST
    check_PROGRAMS     = brmtest brmNBtest
    brmtest_CPPFLAGS   = -Wall $(BRM_USE_OLL) $(BRM_USE_TINYSVM) $(BRM_DEBUG)
    brmtest_LDADD      = libbrm.la
    brmtest_LDFLAGS    = -L/usr/local/lib -lgtest
    brmtest_SOURCES    = brmtest.cpp
    brmNBtest_CPPFLAGS = -Wall $(BRM_USE_OLL) $(BRM_USE_TINYSVM) $(BRM_DEBUG)
    brmNBtest_LDADD    = libbrm.la
    brmNBtest_LDFLAGS  = -L/usr/local/lib -lgtest
    brmNBtest_SOURCES  = brmNBtest.cpp
    TESTS = brmtest brmNBtest
endif
