#-------------------------------------------------
#
# Project created by QtCreator 2014-10-15T00:07:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = img_compressor
TEMPLATE = app


SOURCES += main.cxx\
        mainwindow.cxx \
    image_helper.cxx

HEADERS  += mainwindow.hxx \
    image_helper.hxx

FORMS    += mainwindow.ui

DISTFILES += \
    README \
    LICENCE
