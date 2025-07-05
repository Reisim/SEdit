/************************************************************************
**                                SEdit
**
**   Copyright (C) 2020 Misaki Design.LLC
**   Copyright (C) 2020 Jun Tajima <tajima@misaki-design.co.jp>
**
**   This file is part of the SEdit, Simulation data Editor for Re:sim
**
**   This software is released under the GNU Lesser General Public
**   License version 3, see LICENSE.
*************************************************************************/


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>

#include <QUdpSocket>
#include <QTimerEvent>

#include "graphiccanvas.h"
#include "settingdialog.h"
#include "roadinfo.h"
#include "datamanipulator.h"
#include "basemapimagemanager.h"
#include "displaycontrol.h"
#include "objectproperty.h"
#include "odrouteeditor.h"
#include "resimfilesoutput.h"
#include "configfilemanager.h"
#include "scenarioeditor.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();


    GraphicCanvas *canvas;
    SettingDialog *setDlg;
    RoadInfo *road;
    DataManipulator *dtManip;
    BaseMapImageManager *mapImageMng;
    DisplayControl *dispCtrl;
    RoadObjectProperty *roadObjProp;
    ODRouteEditor *odRoute;
    ResimFilesOutput *resimOut;
    ConfigFileManager *configMgr;
    ScenarioEditor *scenarioEdit;


    void SetRecentDataFile();
    void GetRecentDataFile();
    void OpenRoadDataWithFilename(QString);

protected:
    void closeEvent(QCloseEvent *event);
    bool okToContinue();
    void keyPressEvent(QKeyEvent *);
    // void timerEvent(QTimerEvent *e) override;

public slots:
    void UpdateStatusBar(QString);

    void NewFile();
    void OpenFile();
    bool SaveFile();
    bool SaveAsFile();
    void OpenRecentFile();
    void ImportOtherData();
    void MigrateData();

    void SetTrafficDirection(int dir);
    void WrapWinModified(){ setWindowModified(true); }
    void GetHeightDataFromUE();


private:
    QMenu *createObjectPopup;
    QMenu *insertNodePopup;
    QMenu *recentFileOpen;
    QMenu *utilityPopup;
    QMenu *searchPopup;

    QString currentSEditFilename;
    QStringList recentOpenFiles;
};

#endif // MAINWINDOW_H
