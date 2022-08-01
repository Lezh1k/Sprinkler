TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/avr/include
INCLUDEPATH += inc

DEFINES += __AVR_ATtiny2313__ \
    __SECONDS_TO_00__=$$system(./seconds_to_00.sh) \
    __VALVE_DEBUG__


DISTFILES += \
    Makefile


SOURCES += src/main.c \
    src/valve.c
HEADERS += \
    inc/valve.h

