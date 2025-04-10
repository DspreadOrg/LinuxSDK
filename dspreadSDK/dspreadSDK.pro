#-------------------------------------------------
#
# Project created by QtCreator 2016-05-26T22:28:41
#
#-------------------------------------------------

QT       += core gui

QT += widgets

QT += serialport

#QT += Bluetooth

TARGET = dspreadSDK
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dotrade.cpp

HEADERS  += mainwindow.h \
    pos_sdk.h \
    dotrade.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libs/release/ -ldspreadsdk
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libs/debug/ -ldspreadsdk
else:unix: LIBS += -L$$PWD/../Libs/ -ldspreadsdk

INCLUDEPATH += $$PWD/../Libs
DEPENDPATH += $$PWD/../Libs

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libs/release/libdspreadsdk.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libs/debug/libdspreadsdk.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libs/release/dspreadsdk.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libs/debug/dspreadsdk.lib
else:unix: PRE_TARGETDEPS += $$PWD/../Libs/libdspreadsdk.a


#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libs/release/ -ldspreadsdk-for-arm-arch
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libs/debug/ -ldspreadsdk-for-arm-arch
#else:unix: LIBS += -L$$PWD/../Libs/ -ldspreadsdk-for-arm-arch

#INCLUDEPATH += $$PWD/../Libs
#DEPENDPATH += $$PWD/../Libs

#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libs/release/libdspreadsdk-for-arm-arch.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libs/debug/libdspreadsdk-for-arm-arch.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libs/release/libdspreadsdk-for-arm-arch.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libs/debug/libdspreadsdk-for-arm-arch.lib
#else:unix: PRE_TARGETDEPS += $$PWD/../Libs/libdspreadsdk-for-arm-arch.a

