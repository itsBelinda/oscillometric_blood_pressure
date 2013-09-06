MOC_DIR = moc

OBJECTS_DIR = obj

LIBS += \
    -L/usr/local/lib \
    -lqwt \
    -lcomedi \
    -liir

TMAKE_CXXFLAGS += -fno-exceptions

SOURCES = \
    psthplot.cpp \
    dataplot.cpp \
    main.cpp \
    psth-vep.cpp

HEADERS = \
    psth-vep.h \
    psthplot.h \
    dataplot.h
