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
#include "scenarioeditor.h"


class RoadInfo
{
public:
    RoadInfo();


    void ClearAllData();


    // Node
    int CreateNode(int assignId, float x, float y, QList<int> inLanes, QList<int> outLanes);
    void DeleteNode(int id);
    void DeleteNodeLeg(int id,int legID);
    void SetNodePosition(int id,float atX,float atY);
    void MoveNode(int id,float moveX,float moveY,bool moveNonEdge=false);
    void RotateNode(int id,float rotate);
    void RotateNodeLeg(int id,int leg,float rotate);
    void SetAngleNodeLeg(int id,int leg,float rotate);
    void AddStopLineToNode(int nodeId,int assignStopLintId,int relatedDirection);
    void AddTrafficSignalToNode(int nodeId,int assignTSId,int type, int relatedDirection);
    void SetNodeConnectInfo(int id, int leg, int connectInfo, QString type);
    void SetNodeRelatedLane(int id, int laneID);
    void ClearNodeRelatedLane(int id,int laneID);
    void SetNodeConnectInOutDirect();

    QVector2D GetNodePosition(int id);
    int GetNodeNumLeg(int id);
    float GetNodeLegAngle(int id,int legID);
    int GetNodeLegIDByConnectedNode(int id,int connectedNode);
    int GetNodeLegIDByConnectingNode(int id,int connectingNode);
    QString GetNodeProperty(int id);

    void SetAllLaneLists();
    void SetLaneLists(int id,int hIdx,bool showConsoleOutput=false);
    void SetODFlagOfTerminalNode();
    void SetTurnDirectionInfo(QList<int> nodeList, bool verbose=false);

    int GetNearestNode(QVector2D pos);
    void ClearNodes();


    // Lane
    int CreateLane(int assignId,QVector4D startPoint, int sWPInNode, int sWPNodeDir, bool sWPBoundary, QVector4D endPoint,int eWPInNode, int eWPNodeDir,bool eWPBoundary);
    void DeleteLane(int id);
    void MoveLane(int id,float moveX,float moveY,bool onlyThisLane);
    void MoveLaneEdge(int id,float moveX,float moveY,int edgeFlag);
    void HeightChangeLaneEdge(int id,float moveZ,int edgeFlag);
    void RotateLane(int id,float rotate,QVector2D rotCenter);
    void RotateLaneEdge(int id,float rotate,int edgeFlag);
    void SetLaneEdgeAngle(int id,float angle,int edgeFlag);
    void SetLaneNumber(int id,int laneNumner);
    void CalculateShape(struct LaneShapeInfo*);
    bool CheckLaneConnectionOfNode(int nodeID);
    bool CheckLaneConnection();
    bool CheckLaneConnectionFull();
    void CheckLaneRelatedNode(int laneID);
    void CheckLaneRelatedNodeAllLanes();

    void DivideLaneHalf(int id);
    void DivideLaneAtPos(int id,QVector4D atPoint);
    int DivideLaneAndMove(int id,float xm,float ym,float angle,bool showInfo = false);
    void CutLanesByLine(QPointF p1, QPointF p2);

    int GetNearestLane(QVector2D pos);
    int GetDistanceLaneFromPoint(int id,QVector2D pos,float &dist,int &isEdge);
    int GetNearestLanePoint( int id,
                             float xp,
                             float yp,
                             float &xt,
                             float &yt,
                             float &angle );
    int GetNearestLanePointShapeInfo(struct LaneShapeInfo *s,
                                     float xp,
                                     float yp,
                                     float &xt,
                                     float &yt,
                                     float &angle);
    void CheckIfTwoLanesCross(int lID1,int lID2,bool verbose=false);
    void CheckLaneCrossWithPedestLane(int lID,int pedestID);
    bool CheckLaneCrossPoints();
    void CheckLaneCrossPointsInsideNode(QList<int> nodeList);
    struct CrossPointInfo* CheckLaneCrossPoint(int id,QPointF p1,QPointF p2, bool debugFlag=false);
    QString GetLaneProperty(int id);

    void ClearLanes();
    void ClearLaneShape(struct LaneShapeInfo *);

    bool updateCPEveryOperation;
    bool updateWPDataEveryOperation;


    // WP
    void CreateWPData();
    int CreateWP(int assignID, QVector3D pos, float dir,QList<int> relatedLanes);
    void ClearWPs();



    // Pedest Lane
    int CreatePedestLane(int assignId, QList<QVector3D*> posData);
    void DeletePedestLane(int id);
    void UpdatePedestLaneShapeParams(int id);
    void MovePedestLane(int id, float moveX,float moveY);
    void MovePedestLanePoint(int id,int pIdx,float moveX,float moveY);
    void SetPedestLaneIsCrossWalk(int id,int pIdx,bool flag);
    void SetPedestLaneRunOutData(int id,int pIdx,float prob,int dir);
    void SetPedestLaneWidth(int id,int pIdx,float w);
    void SetPedestLaneTrafficVolumne(int id,QList<int> volume);
    int GetNearestPedestLane(QVector2D pos,float &dist);
    void GetNearestPedestLanePoint(QVector2D pos, float &dist,int &nearPedestLaneID,int &nearPedestLanePointIndex);
    void FindPedestSignalFroCrossWalk();
    bool CheckPedestLaneCrossPoints();

    void DividePedestLaneHalf(int id,int sect);
    void DividePedestLaneAtPos(int id,int sect, QVector3D atPoint);
    void ChangePedestLaneWidthByWheel(int id,int sect,float diff);

    void ClearPedestLanes();


    // Traffic Signal
    int CreateTrafficSignal(int assignId, int relatedNodeID, int relatedNodeDir, int TSType);
    int GetNearestTrafficSignal(QVector2D pos,float &dist);
    void MoveTrafficSignal(int id,float moveX,float moveY);
    void DeleteTrafficSignal(int id);



    // Stop Lines
    int CreateStopLine(int assignId, int relatedNodeID, int relatedNodeDir, int SLType);
    int CreateStopLineAtLAne(int assignId,int onLane,float X,float Y,float angle,int SLType);
    int GetNearestStopLine(QVector2D pos,float &dist);
    void CheckStopLineCrossLanes(int id);
    void MoveStopLine(int id,float moveX,float moveY);
    void ChangeStopLineLength(int id,float fact);
    void CheckAllStopLineCrossLane();
    void DeleteStopLine(int id);


    // Static Ocjects
    int CreateStaticObject(int assignId);
    int GetNearestStaticObject(QVector2D pos,float &dist);
    void MoveStaticObject(int id,float moveX,float moveY,float moveZ);
    void RotateStaticObject(int id,float rot);
    void SetSizeStaticObject(int id,float lenx,float leny,float height);
    void SetCornerPointsStaticObject(int id);
    void DeleteStaticObject(int id);
    void ClearStaticObject();


    // Road Boundary Info
    int CreateRoadBoundaryInfo(int assignId, QList<QVector3D *> posData, QList<float> height);
    void ClearRoadBoundaryInfo();
    void DeleteRoadBoundaryInfo(int id);
    void MoveRoadBoundaryInfo(int id,float moveX,float moveY,float moveZ);
    void DivideRoadBoundarySection(int id, int pIdx1, int pIdx2);
    void MergeRoadBoundarySection(int id, int pIdx);
    void MoveRoadBoundaryPoint(int id,int pIdx,float moveX,float moveY,float moveZ);
    int GetNearestRoadBoundaryInfo(QVector2D pos,float &dist);
    void GetNearestRoadBoundaryPoint(QVector2D pos, float &dist,int &nearRoadBaundaryID,int &nearRoadBoundaryPointIndex);
    void UpdateRoadBoundaryInfo(int id);
    void SetRoadBoudaryInfoRoadSide(int id,int roadSide);


    // Route
    void CheckRouteInOutDirection();
    void CheckRouteInOutDirectionGivenODNode(int origNodeId,int destNodeID);
    void GetLaneListForRoute(int origNodeId,int destNodeID,int hIdx);
    void SetAllRouteLaneList();
    void GetLaneListForScenarioNodeRoute(struct RouteData *);
    void ClearODData(struct ODData *);


    // List / Vector Index
    int indexOfNode(int id);
    int indexOfLane(int id);
    int indexOfTS(int id,int relatedNodeID);
    int indexOfSL(int id,int relatedNodeID);
    int indexOfWP(int id);
    int indexOfPedestLane(int id);
    int indexOfStaticObject(int id);
    int indexOfRoadBoundary(int id);

    bool SaveRoadData(QString filename);
    bool LoadRoadData(QString filename);
    QString GetCurrentRoadDataFileName(){ return roadDataFileName; }

    bool outputResimScenarioFiles( QString outputfolder,
                                   QString outputfilename,
                                   int maxAgent,
                                   bool onlyFilename,
                                   QString scenariofilename);
    bool outputResimRoadFiles( QString outputfolder, QString outputfilename );
    bool outputResimTrafficSignalFiles( QString outputfolder, QString outputfilename );

    int LeftOrRight;
    bool useRelativePath;

    QList<NodeInfo*> nodes;
    QList<LaneInfo*> lanes;
    QList<WayPoint*> wps;
    QList<PedestrianLane*> pedestLanes;
    QList<StaticObject*> staticObj;
    QList<RoadBoundaryInfo*> roadBoundary;

    float tbl_cos[DEFAULT_LANE_SHAPE_POINTS+1];
    float tbl_sin[DEFAULT_LANE_SHAPE_POINTS+1];


    QString roadDataFileName;

    SettingDialog *setDlg;
    BaseMapImageManager *mapImageMng;

    QList<struct TreeSearchElem*> treeSeachHelper[8];
    void ForwardTreeSearch(int nodeId,int nextLane,int currentLane,int);
    void ClearSearchHelper(int);

    void ForwardTreeSearchForRouteLaneList(struct RouteData* route,int nextLane,int currentLane, int);
    void FindInconsistentData();
};

#endif // ROADINFO_H
