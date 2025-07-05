#-------------------------------------------------
#
# Project created by QtCreator 2019-10-23T01:09:14
#
#-------------------------------------------------

QT       += core gui

QT += opengl
QT += network

CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SEdit
TEMPLATE = app


#
#  Freetype2
#
INCLUDEPATH += "C:\MDS-series\MDS02\Canopus\Reisim\libs\freetype\include\freetype2"

LIBS += "C:\MDS-series\MDS02\Canopus\Reisim\libs\freetype\lib\libfreetype.a"
LIBS += "C:\MDS-series\MDS02\Canopus\Reisim\libs\libpng\lib\libpng.a"
LIBS += "C:\MDS-series\MDS02\Canopus\Reisim\libs\zlib\lib\libzlibstatic.a"
LIBS += "C:\MDS-series\MDS02\Canopus\Reisim\libs\bzip2\lib\libbz2.a"



# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        basemapimagemanager.cpp \
        configfilemanager.cpp \
        datamanip_base.cpp \
        datamanip_createnode.cpp \
        datamanip_duplicate.cpp \
        datamanip_import.cpp \
        datamanip_insertnode.cpp \
        datamanip_pedestlane.cpp \
        datamanip_roadboundary.cpp \
        datamanip_staticobject.cpp \
        displaycontrol.cpp \
        gc_displaycontrol.cpp \
        gc_mouseoperation.cpp \
        gltransform3d.cpp \
        graphdialog.cpp \
        graphiccanvas.cpp \
        main.cpp \
        mainwindow.cpp \
        objectproperty.cpp \
        objectproperty_lane.cpp \
        objectproperty_node.cpp \
        objectproperty_pedestlane.cpp \
        objectproperty_roadboundary.cpp \
        objectproperty_staticobject.cpp \
        objectproperty_stopline.cpp \
        objectproperty_trafficsignal.cpp \
        odrouteeditor.cpp \
        resimfilesoutput.cpp \
        roadboundary.cpp \
        roaddatafileio.cpp \
        roadinfo.cpp \
        roadlane.cpp \
        roadnode.cpp \
        roadpedestlane.cpp \
        roadroute.cpp \
        roadstaticobject.cpp \
        roadstopline.cpp \
        roadts.cpp \
        roadwp.cpp \
        scenarioeditor.cpp \
        settingdialog.cpp \
        workingthread.cpp

HEADERS += \
        basemapimagemanager.h \
        configfilemanager.h \
        datamanipulator.h \
        displaycontrol.h \
        gltransform3d.h \
        graphdialog.h \
        graphiccanvas.h \
        mainwindow.h \
        objectproperty.h \
        odrouteeditor.h \
        resimfilesoutput.h \
        roadinfo.h \
        roadinfoelement.h \
        scenarioeditor.h \
        settingdialog.h \
        workingthread.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    sedit.qrc

RC_FILE = sedit.rc
