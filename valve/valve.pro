TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/avr/include
INCLUDEPATH += inc

DEFINES += __AVR_ATtiny2313__ \
    __SECONDS_TO_00__=$$system(./seconds_to_00.sh) \


DISTFILES += \
    Makefile


SOURCES += src/main.c
HEADERS += \

