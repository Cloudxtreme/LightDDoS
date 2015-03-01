TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -pthread
SOURCES += main.c \
    Covers/wrapunix.c \
    Covers/wrapsock.c \
    Covers/error.c \
    Covers/wraplib.c \
    Methods/getmassive.c \
    Covers/readline.c \
    Covers/writen.c \
    Covers/wrapstdio.c \
    Methods/synflood.c

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    methods.h \
    unp.h \
    Methods/getmassive.h \
    Methods/synflood.h

