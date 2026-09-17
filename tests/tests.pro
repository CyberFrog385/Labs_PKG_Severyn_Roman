QT += widgets testlib

CONFIG += c++17 testcase console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = runtests

INCLUDEPATH += $$PWD/../src

SOURCES += TestColorModel.cpp \
    ../src/ColorModel.cpp

HEADERS += ../src/ColorModel.h