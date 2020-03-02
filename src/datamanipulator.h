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


#ifndef DATAMANIPULATPR_H
#define DATAMANIPULATPR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QApplication>
#include <QDebug>


#include "roadinfo.h"
#include "graphiccanvas.h"
#include "basemapimagemanager.h"
#include "objectproperty.h"
#include "settingdialog.h"


class DataManipulator : public QObject
{
    Q_OBJECT
public:
    explicit DataManipulator(QObject *parent = nullptr);

    RoadInfo *road;
    GraphicCanvas *canvas;
    BaseMapImageManager *mapImageMng;
    RoadObjectProperty *roadObjProp;
    SettingDialog *setDlg;


    int insertMode;
    int insertNode1;
    int insertNode2;

    QStringList operationHistory;

    void UndoOperation();
    void SelectAllNode();
    void DeleteSelectedObject();
    void MergeSelectedObject();

    int CreateNode_4x1x1();
    int CreateNode_4x1x1_r();
    int CreateNode_4x2x1();
    int CreateNode_4x2x2();
    int CreateNode_4x2x1_r();
    int CreateNode_4x2x2_r();
    int CreateNode_3x1x1();


signals:
    void UpdateStatusBar(QString);


public slots:
    int CreateNode_4x1x1_noTS();
    int CreateNode_3x1x1_noTS();

    int CreateNode_4x1x1_TS();
    int CreateNode_4x1x1_r_TS();
    int CreateNode_4x2x1_TS();
    int CreateNode_4x2x1_r_TS();
    int CreateNode_4x2x2_TS();
    int CreateNode_4x2x2_r_TS();


    void StartCreatePedestPath();
    int CreatePedestPath();



    int CreateNode(float x,float y,int nLeg,QList<int> inlanes,QList<int> outlanes);
    int CreateTrafficSignal(int nodeID,int nodeDir,int type);
    int CreateStopLine(int nodeID,int nodeDir,int type);

    void CreateWPData();
    void SetODFlagOfTerminalNode();
    void SetAllLaneLists();
    void SetSelectedNodeLaneLists();
    void SetTurnDirectionInfo();

    void InsertNode_4x1_noTS();
    void InsertNode_4x2_noTS();
    void InsertNode_3Lx1_noTS();
    void InsertNode_3Lx2_noTS();
    void InsertNode_3Rx1_noTS();
    void InsertNode_3Rx2_noTS();

    void ImportERIS3Data(QString filename);
    void ImportERIS3TrafficSignalData(QString);
    void ImportERIS3ODData(QString);


    void SearchNode();
    void SearchLane();
    void SearchTrafficSignal();
    void MoveXY();
    void SelectAllLanes();

    void ReadLineCSV();
    void ClearLineData();
    void ChangeLineCoordInfo();
};

#endif // DATAMANIPULATPR_H
