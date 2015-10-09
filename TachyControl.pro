QT += core serialport
QT -= gui

TARGET = TachyControl
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = lib

DEFINES += TC_LIB

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/bin/debug
} else {
    DESTDIR = $$PWD/bin/release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

INCLUDEPATH += $$PWD/include

SOURCES += main.cpp \
    tachycontrol.cpp

HEADERS += \
    include/tachycontrol.h

