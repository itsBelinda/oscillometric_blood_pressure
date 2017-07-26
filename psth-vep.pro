MOC_DIR = moc

OBJECTS_DIR = obj

LIBS += \
    -L/usr/local/lib \
    -lqwt-qt5 \
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

CONFIG		+= qt release c++11

QT             	+= widgets
QT += printsupport
