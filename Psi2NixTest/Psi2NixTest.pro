QT += testlib
QT += gui core serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qt warn_on depend_includepath testcase c++17

TEMPLATE = app

include(../psi2nix-src.pri)
APPPATH=../Psi2Nix
INCLUDEPATH += $$APPPATH
DEPENDPATH += $$APPPATH

SOURCES +=  tst_testlink.cpp \
    mockserial.cpp

HEADERS += \
    mockserial.hpp
