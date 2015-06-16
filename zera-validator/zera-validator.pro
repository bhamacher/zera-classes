#-------------------------------------------------
#
# Project created by QtCreator 2015-06-15T16:31:42
#
#-------------------------------------------------

TEMPLATE = lib

#dependencies
VEIN_DEP_EVENT = 1
VEIN_DEP_COMP = 1
VEIN_DEP_HASH = 1

include(../zera-classes.pri)

QT       -= gui

TARGET = zera-validator


DEFINES += ZERAVALIDATOR_LIBRARY

SOURCES += \
    commandvalidator.cpp \
    groupvalidator.cpp

HEADERS +=\
        zera-validator_global.h \
    commandvalidator.h \
    groupvalidator.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

