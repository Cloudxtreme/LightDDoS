TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    Covers/error.c \
    Covers/readline.c \
    Covers/wraplib.c \
    Covers/wrapsock.c \
    Covers/wrapstdio.c \
    Covers/wrapunix.c \
    Covers/writen.c \
    daemon_init.c \
    Covers/signal.c

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    unp.h \
    daemon_init.h

