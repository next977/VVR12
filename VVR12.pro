#-------------------------------------------------
#
# Project created by QtCreator 2019-12-09T10:43:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

CONFIG += c++11
TARGET = VVR12
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainsystem.cpp \
    workerthread.cpp \
    sysparm.cpp \
    rfoutputmodes.cpp \
    outputthread.cpp \
    outputmode.cpp \
    monitoringtask.cpp \
    gpiotools.cpp \
    mrfa_rf_table.c \
    setting.cpp \
    sysmessage.cpp \
    initializing.cpp \
    qgaugedrawfuncions.cpp \
    qgauge.cpp \
    calibration.cpp \
    gpiobutton.cpp \
    audiooutput.cpp \
    sounds.cpp

HEADERS  += mainsystem.h \
    workerthread.h \
    sysparm.h \
    rfoutputmodes.h \
    outputthread.h \
    outputmode.h \
    mrfa_rf_table.h \
    monitoringtask.h \
    include.h \
    gpiotools.h \
    global.h \
    setting.h \
    sysmessage.h \
    initializing.h \
    QGauge/qgauge.h \
    calibration.h \
    gpiobutton.h \
    audiooutput.h \
    sounds.h

FORMS    += mainsystem.ui \
    setting.ui \
    sysmessage.ui \
    initializing.ui \
    calibration.ui

RESOURCES += \
    fonts.qrc \
    image.qrc \
    sound.qrc
