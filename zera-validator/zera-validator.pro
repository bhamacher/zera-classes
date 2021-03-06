#-------------------------------------------------
#
# Project created by QtCreator 2015-06-15T16:31:42
#
#-------------------------------------------------

TEMPLATE = lib




#do not copy the target into the $$VEIN_BASEDIR/libs directory
VF_NO_DEPLOY = 1

include(../zera-modules/zera-modules.pri)
include(../zera-classes.pri)

QT       -= gui

TARGET = zera-validator


DEFINES += ZERAVALIDATOR_LIBRARY

SOURCES += \
    commandvalidator.cpp \
    groupvalidator.cpp

HEADERS += \
    zera-validator_global.h \
    commandvalidator.h \
    groupvalidator.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

