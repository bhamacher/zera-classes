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
LIBS +=  -lzera-validator


include(../zera-modules.pri)


INCLUDEPATH += ../../zera-basemodule \
    ../../zera-xml-config/src \
    ../../zera-misc \
    ../../zera-comm/zera-proxy \
    ../../zera-validator \
    ../interface


TARGET = $$qtLibraryTarget(rangemodule)

INCLUDEPATH += "src"

SOURCES += \
    src/rangemodulefactory.cpp \
    src/rangemoduleconfiguration.cpp \
    src/rangemeaschannel.cpp \
    src/rangemodulemeasprogram.cpp \
    src/rangeobsermatic.cpp \
    src/adjustment.cpp \
    src/rangemoduleobservation.cpp \
    src/rangemodule.cpp

HEADERS += \
    src/rangemodulefactory.h \
    src/rangemodule.h \
    src/rangemoduleconfiguration.h \
    src/rangemoduleconfigdata.h \
    src/rangemeaschannel.h \
    src/rangemodulemeasprogram.h \
    src/rangeobsermatic.h \
    src/adjustment.h \
    src/rangemoduleobservation.h \
    src/debug.h \

OTHER_FILES += \
    src/rangemodule.xsd \
    src/com5003-rangemodule.xml \
    src/com5003-rangemoduleREF.xml \
    src/mt310s2-rangemodule.xml

RESOURCES += \
    resources.qrc

include(../../zera-classes.pri)



