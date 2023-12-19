#-------------------------------------------------
#
# Project created by QtCreator 2023-10-09T20:36:00
#
#-------------------------------------------------

QT       += core gui opengl sql

LIBS    += -lopengl32

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lab7
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++1z
CONFIG += resources_big

SOURCES += \
        asdloader.cpp \
        camera.cpp \
        celler.cpp \
        launcherform.cpp \
        main.cpp \
        mywidget.cpp \
        objloader.cpp \
        obstacles.cpp \
        resultator.cpp \
        sboxer.cpp

HEADERS += \
        asdloader.h \
        camera.h \
        celler.h \
        launcherform.h \
        lzma/Lzma2Decoder.hpp \
        lzma/LzmaDecoderCore.hpp \
        myutils.h \
        mywidget.h \
        objloader.h \
        obstacles.h \
        resultator.h \
        sboxer.h

FORMS += \
        launcherform.ui \
        mywidget.ui \
        resultator.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    lzma/liblzma/common/Makefile.inc
