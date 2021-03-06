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

INCLUDEPATH += ../../zera-xml-config/src \
    ../../zera-misc \
    ../../zera-comm/zera-proxy \
    ../../zera-basemodule \
    ../../zera-validator \
    ../interface

TARGET = $$qtLibraryTarget(thdnmodule)


INCLUDEPATH += "src"

SOURCES += \
    src/thdnmodule.cpp \
    src/thdnmoduleconfiguration.cpp \
    src/thdnmodulefactory.cpp \
    src/thdnmodulemeasprogram.cpp \
    src/thdnmoduleobservation.cpp

HEADERS += \
    src/thdnmodule.h \
    src/thdnmoduleconfigdata.h \
    src/thdnmoduleconfiguration.h \
    src/thdnmodulefactory.h \
    src/thdnmodulemeasprogram.h \
    src/thdnmoduleobservation.h \
    src/debug.h

OTHER_FILES += \
    src/thdnmodule.xsd \
    src/com5003-thdnmodule.xml \
    src/com5003-thdnmodule2.xml \
    src/mt310s2-thdnmodule.xml \
    src/mt310s2-thdnmodule2.xml

RESOURCES += \
    resources.qrc

include(../../zera-classes.pri)



