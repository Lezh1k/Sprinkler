TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/avr/include
INCLUDEPATH += inc

DEFINES += __AVR_ATtiny85__

DISTFILES += \
    Makefile


SOURCES += src/main.c
HEADERS += \

