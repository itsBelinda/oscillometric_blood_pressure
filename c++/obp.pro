MOC_DIR = moc

OBJECTS_DIR = obj

LIBS += \
    -L/usr/local/lib \
    -lqwt-qt5 \
    -lcomedi \
    -liir

TMAKE_CXXFLAGS += -fno-exceptions

SOURCES = \
    obp.cpp \
    dataplot.cpp \
    main.cpp \

HEADERS = \
    obp.h \
    dataplot.h

CONFIG		+= qt release c++11

QT             	+= widgets
QT += printsupport
