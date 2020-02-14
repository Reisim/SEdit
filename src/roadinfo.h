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


#ifndef ROADINFO_H
#define ROADINFO_H

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QPointF>
#include <math.h>

#include "roadinfoelement.h"
#include "settingdialog.h"
#include "basemapimagemanager.h"


class RoadInfo
{
public:
    RoadInfo();

    // Node
    int CreateNode(int assignId, float x, float y, QList<int> inLanes, QList<int> outLanes);
    void DeleteNode(int id);
    void DeleteNodeLeg(int id,int legID);
    void SetNodePosition(int id,float atX,float atY);
    void MoveNode(int id,float moveX,float moveY);
    void RotateNode(int id,float rotate);
    void RotateNodeLeg(int id,int leg,float rotate);
    void AddStopLineToNode(int nodeId,int assignStopLintId,int type,int relatedDirection);
    void AddTrafficSignalToNode(int nodeId,int assignTSId,int type, int relatedDirection);
    void SetNodeConnectInfo(int id, int leg, int connectInfo, QString type);
    void SetNodeRelatedLane(int id, int laneID);
    void ClearNodeRelatedLane(int id,int laneID);
    QVector2D GetNodePosition(int id);
    int GetNodeNumLeg(int id);
    float GetNodeLegAngle(int id,int legID);
    int GetNodeLegIDByConnectedNode(int id,int connectedNode);
    int GetNodeLegIDByConnectingNode(int id,int connectingNode);
    QString GetNodeProperty(int id);

    void SetAllLaneLists();
    void SetLaneLists(int id,bool showConsoleOutput=false);
    void SetODFlagOfTerminalNode();
    void SetTurnDirectionInfo();


    // Lane
    int CreateLane(int assignId,QVector4D startPoint, int sWPInNode, int sWPNodeDir, int departureNode, bool sWPBoundary, QVector4D endPoint,int eWPInNode, int eWPNodeDir,int connectedNode, bool eWPBoundary);
    void DeleteLane(int id);
    void MoveLane(int id,float moveX,float moveY,bool onlyThisLane);
    void MoveLaneEdge(int id,float moveX,float moveY,int edgeFlag);
    void RotateLane(int id,float rotate,QVector2D rotCenter);
    void RotateLaneEdge(int id,float rotate,int edgeFlag);
    void SetLaneNumber(int id,int laneNumner);
    void CalculateShape(struct LaneShapeInfo*);
    void CheckLaneConnection();

    void DivideLaneHalf(int id);
    void DivideLaneAtPos(int id,QVector4D atPoint);

    int GetNearestLane(QVector2D pos);
    int GetDistanceLaneFromPoint(int id,QVector2D pos,float &dist,int &isEdge);
    void CheckLaneCrossPoints();
    struct CrossPointInfo* CheckLaneCrossPoint(int id,QPointF p1,QPointF p2, bool debugFlag=false);
    QString GetLaneProperty(int id);


    // WP
    void CreateWPData();
    int CreateWP(int assignID, QVector3D pos, float dir,QList<int> relatedLanes);
    void ClearWPs();


    // Traffic Signal
    int CreateTrafficSignal(int assignId, int relatedNodeID, int relatedNodeDir, int TSType);
    int GetNearestTrafficSignal(QVector2D pos,float &dist);
    void MoveTrafficSignal(int id,float moveX,float moveY);



    // Stop Lines
    int CreateStopLine(int assignId, int relatedNodeID, int relatedNodeDir, int SLType);
    int GetNearestStopLine(QVector2D pos,float &dist);
    void CheckStopLineCrossLanes(int id);
    void MoveStopLine(int id,float moveX,float moveY);
    void CheckAllStopLineCrossLane();


    // Route
    void CheckRouteInOutDirection();



    // List / Vector Index
    int indexOfNode(int id);
    int indexOfLane(int id);
    int indexOfTS(int id,int relatedNodeID);
    int indexOfSL(int id,int relatedNodeID);
    int indexOfWP(int id);


    bool SaveRoadData(QString filename);
    bool LoadRoadData(QString filename);
    QString GetCurrentRoadDataFileName(){ return roadDataFileName; }

    bool outputResimScenarioFiles( QString outputfolder, QString outputfilename, int maxAgent );
    bool outputResimRoadFiles( QString outputfolder, QString outputfilename );
    bool outputResimTrafficSignalFiles( QString outputfolder, QString outputfilename );

    int LeftOrRight;
    QList<NodeInfo*> nodes;
    QList<LaneInfo*> lanes;
    QList<WayPoint*> wps;

    float tbl_cos[DEFAULT_LANE_SHAPE_POINTS+1];
    float tbl_sin[DEFAULT_LANE_SHAPE_POINTS+1];


    QString roadDataFileName;

    SettingDialog *setDlg;
    BaseMapImageManager *mapImageMng;

    QList<struct TreeSearchElem*> treeSeachHelper;
    void ForwardTreeSearch(int nodeId,int nextLane,int currentLane);
    void ClearSearchHelper();
};

#endif // ROADINFO_H
