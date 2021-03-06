#-------------------------------------------------
#
# Project created by QtCreator 2013-09-04T14:38:09
#
#-------------------------------------------------
TEMPLATE = lib
CONFIG += plugin

QT       += network core
QT       -= gui

LIBS +=  -lzera-proxy
LIBS +=  -lzeramisc
LIBS +=  -lzeraxmlconfig
LIBS +=  -lzerabasemodule
LIBS +=  -lMeasurementModuleInterface

include(../zera-modules.pri)

INCLUDEPATH += ../../zera-basemodule \
    ../../zera-xml-config/src \
    ../../zera-misc \
    ../../zera-comm/zera-proxy \
    ../../zera-validator \
    ../interface/


TARGET = $$qtLibraryTarget(fftmodule)


INCLUDEPATH += "src"

SOURCES += \
    src/fftmodule.cpp \
    src/fftmodulefactory.cpp \
    src/fftmoduleconfiguration.cpp \
    src/fftmodulemeasprogram.cpp \
    src/fftmoduleobservation.cpp

HEADERS += \
    src/fftmodule.h \
    src/fftmoduleconfigdata.h \
    src/fftmoduleconfiguration.h \
    src/fftmodulefactory.h \
    src/fftmodulemeasprogram.h \
    src/fftmoduleobservation.h \
    src/debug.h

OTHER_FILES += \
    src/fftmodule.xsd \
    src/com5003-fftmodule.xml \
    src/mt310s2-fftmodule.xml

RESOURCES += \
    resources.qrc

include(../../zera-classes.pri)


