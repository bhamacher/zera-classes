#-------------------------------------------------
#
# Project created by QtCreator 2013-11-27T13:25:24
#
#-------------------------------------------------
TEMPLATE = lib

QT       -= gui

TARGET = MeasurementModuleInterface
#CONFIG += staticlib

HEADERS += \
    virtualmodule.h \
    abstractmodulefactory.h

header_files.files = $$HEADERS
header_files.path = /usr/include/zera-modules
INSTALLS += header_files

include(../../zera-classes.pri)
include(../zera-modules.pri)
