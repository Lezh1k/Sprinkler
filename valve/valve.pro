TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/avr/include
INCLUDEPATH += inc

DEFINES += __AVR_ATtiny2313__


DISTFILES += \
    Makefile


SOURCES += src/main.c
HEADERS += \

