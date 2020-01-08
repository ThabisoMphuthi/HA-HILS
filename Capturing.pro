QT       += core

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
#CONFIG -= qt

SOURCES += main.cpp \
    spincapture.cpp

HEADERS += \
    spincapture.h


QMAKE_CXXFLAGS += -std=gnu++0x
INCLUDEPATH += "/usr/include/spinnaker"
INCLUDEPATH += "usr/local/include/"
LIBS += `pkg-config --libs opencv`

LIBS += -L/usr/lib
LIBS += -lSpinnaker
LIBS += -lGenApi_gcc540_v3_0
LIBS += -lGCBase_gcc540_v3_0
LIBS += -L/usr/local/lib
LIBS += -lopencv_core
LIBS += -lopencv_highgui
LIBS += -lopencv_imgproc
LIBS += -lopencv_video
LIBS += -lopencv_videoio
