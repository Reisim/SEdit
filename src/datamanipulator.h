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


    QStringList operationHistory;

    void UndoOperation();
    void SelectAllNode();
    void DeleteSelectedObject();
    void MergeSelectedObject();
    void SplitSelectedLane();
    void SplitSelectedPedestLane();
    void SplitSelectedRoadBoundary();

    int CreateNode_4(float x,float y,QList<int> inlanes, QList<int> outlanes, QList<bool> createTurnLane,QStringList turnRestriction = QStringList());
    int CreateNode_3(float x,float y,QList<int> inlanes, QList<int> outlanes, QList<bool> createTurnLane,QStringList turnRestriction = QStringList());
    int CreateNode_5(float x,float y,QList<int> inlanes, QList<int> outlanes, QList<bool> createTurnLane,QStringList turnRestriction = QStringList());
    int CreateNode_6(float x,float y,QList<int> inlanes, QList<int> outlanes, QList<bool> createTurnLane,QStringList turnRestriction = QStringList());
    int CreateStraight(float x,float y,int nLane);

    int CreateNode_4x1x1();
    int CreateNode_4x1x1_r();
    int CreateNode_4x2x1();
    int CreateNode_4x2x2();
    int CreateNode_4x2x1_r();
    int CreateNode_4x2x2_r();

    int CreateNode_3x1x1();
    int CreateNode_3x2x1();
    int CreateNode_3x1x1_r();
    int CreateNode_3x1x1_tr();
    int CreateNode_3x2x1_tr();
    int CreateNode_3x3x1_tr();
    int CreateNode_3x2x1_rm();
    int CreateNode_3x2x1_r();

    int depthCount;
    void ChangeRelatedLanesForPreviousLanes(int lidx,int addNode,int relDir,int rmvNode);
    void ChangeRelatedLanesForNextLanes(int lidx,int addNode,int relDir,int rmvNode);


signals:
    void UpdateStatusBar(QString);


public slots:
    int CreateNode_4x1x1_noTS();
    int CreateNode_4x2x1_noTS();
    int CreateNode_3x1x1_noTS();
    int CreateNode_3x2x1_noTS();
    int CreateNode_3x1x1_tr_noTS();
    int CreateNode_3x2x1_tr_noTS();
    int CreateNode_3x3x1_tr_noTS();

    int CreateNode_4x1x1_TS();
    int CreateNode_4x1x1_r_TS();
    int CreateNode_4x2x1_TS();
    int CreateNode_4x2x1_r_TS();
    int CreateNode_4x2x2_TS();
    int CreateNode_4x2x2_r_TS();

    int CreateNode_3x1x1_TS();
    int CreateNode_3x2x1_TS();
    int CreateNode_3x1x1_r_TS();
    int CreateNode_3x2x1_rm_TS();
    int CreateNode_3x2x1_r_TS();

    int CreateNode_2L_exist();
    int CreateNode_2L_merge();
    int CreateNode_3L_exist();
    int CreateNode_3L_merge();

    int CreateNode_straight_1();
    int CreateNode_straight_2();
    int CreateNode_straight_3();

    void CreateNode_Dialog();

    void CreateTrafficSignalForVehicle();
    void CreateTrafficSignalForPedestrian();
    void CreateStopLineForInDir();

    void StartCreatePedestPath();
    int CreatePedestPath();

    int CreateStaticObject();

    void StartCreateRoadBoundary();
    int CreateRoadBoundary();


    int CreateNode(float x,float y,int nLeg,QList<int> inlanes,QList<int> outlanes);
    int CreateTrafficSignal(int nodeID,int nodeDir,int type);
    int CreateStopLine(int nodeID,int nodeDir,int type);

    void DuplicateNodes();
    void CheckLaneConnectionFull();
    void FindInconsistentData();
    void CreateWPData();
    void SetODFlagOfTerminalNode();
    void SetAllLaneLists();
    void SetSelectedNodeLaneLists();
    void SetTurnDirectionInfo();
    void CheckAllStopLineCrossLane();
    void CheckLaneCrossPoints();
    void CheckCrossPointsOfSelectedLane();
    void SetLaneHeightOfSelectedLane();
    void ChangeSpeedLimitOfSelectedLanes();
    void ChangeActualSpeedOfSelectedLanes();
    void CheckLaneAndPedestLaneCrossPoint();
    void SetSignalsNodeByCommand();

    void GetConnectionDirection(int nd1,int nd2, int& dir1, int &dir2);
    void InsertNode_4x1_noTS();
    void InsertNode_4x2_noTS();
    void InsertNode_3Lx1_noTS();
    void InsertNode_3Lx2_noTS();
    void InsertNode_3Rx1_noTS();
    void InsertNode_3Rx2_noTS();
    void CheckRouteChangeByInsertNode(int, QStringList);

    void ImportERIS3Data(QString filename);
    void ImportERIS3TrafficSignalData(QString);
    void ImportERIS3ODData(QString);

    void MigrateData(QString);


    void SearchNode();
    void SearchLane();
    void SearchTrafficSignal();
    void SearchStaticObject();
    void MoveXY();
    void SetNodePos();
    void SelectAllLanes();

    void ReadLineCSV();
    void ClearLineData();
    void ChangeLineCoordInfo();
};

#endif // DATAMANIPULATPR_H
