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


#ifndef ROADINFOELEMENT_H
#define ROADINFOELEMENT_H

#include <QList>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QOpenGLTexture>


#define   DEFAULT_LANE_SHAPE_POINTS   (10)
#define   PATH_ADJUST_PARAM   (0.707)

#define   LEFT_HAND_TRAFFIC   (0)
#define   RIGHT_HAND_TRAFFIC  (1)


enum _STOPLINE_KIND
{
    STOPLINE_SIGNAL,
    STOPLINE_TEMPSTOP,
    STOPLINE_TURNWAIT,
    STOPLINE_CROSSWALK
};


struct SearchHelpInfo
{
    float xmin;
    float ymin;
    float xmax;
    float ymax;
};


struct LaneShapeInfo
{
    QList<QVector3D*> pos;
    QList<QVector2D*> derivative;
    QList<QVector2D*> diff;

    QList<float> curvature;
    QList<float> length;
    QList<float> segmentLength;
    float pathLength;

    struct SearchHelpInfo searchHelper;
    QList<float> angles;
};


struct StopLineInfo
{
    int id;
    QVector2D leftEdge;
    QVector2D rightEdge;
    QList<int> crossLanes;
    int stopLineType;
    int relatedNode;
    int relatedNodeDir;
};


struct StopPointInfo
{
    int stoplineID;
    int stoplineType;
    QVector3D pos;
    QVector2D derivative;
    float distanceFromLaneStartPoint;
};


struct CrossPointInfo
{
    int crossLaneID;
    int crossSectIndex;
    QVector3D pos;
    QVector2D derivative;
    float distanceFromLaneStartPoint;
};


struct LaneInfo
{
    int id;
    struct LaneShapeInfo shape;

    QList<int> nextLanes;
    QList<int> previousLanes;

    float laneWidth;
    float speedInfo;        // speed limit
    float actualSpeed;      // actual speed

    float driverErrorProb;
    bool automaticDrivingEnabled;

    int connectedNode;
    int connectedNodeInDirect;
    int connectedNodeOutDirect;
    int departureNode;
    int departureNodeOutDirect;
    int laneNum;

    int sWPInNode;
    int sWPNodeDir;
    bool sWPBoundary;

    int eWPInNode;
    int eWPNodeDir;
    bool eWPBoundary;

    bool hasStopLine;
    QList< struct StopPointInfo*> stopPoints;
    bool insideIntersection;

    QList<struct CrossPointInfo*> crossPoints;

    QList<struct CrossPointInfo*> pedestCrossPoints;

    int startWPID;
    int endWPID;
};


struct PedestrianLaneShapeElement
{
    QVector3D pos;
    float angleToNextPos;  // in [rad]
    float width;
    float distanceToNextPos;
    bool isCrossWalk;
    int controlPedestSignalID;
    int runOutDirect;
    float runOutProb;
};


struct PedestrianLane
{
    int id;
    QList<PedestrianLaneShapeElement*> shape;
    QList<int> trafficVolume;
};


struct SignalPatternData
{
    int signal;
    int duration;
};


struct TrafficSignalInfo
{
    int id;
    int type;

    QVector3D pos;
    float facingDirect;

    int relatedNode;
    int controlNodeDirection;
    int controlNodeLane;
    int controlCrossWalk;

    QList<struct SignalPatternData*> sigPattern;
    int startOffset;
};


struct LaneLists
{
    int relatedNodeOutDirection;
    int relatedNodeInDirection;
    QList<QList<int>> lanes;
};


struct TreeSearchElem
{
    int nextLane;
    int currentLane;
    bool isEnd;
};


struct LegInfo
{
    int legID;
    float angle;
    int connectingNode;
    int connectingNodeInDirect;
    int connectedNode;
    int connectedNodeOutDirect;
    int nLaneIn;
    int nLaneOut;
    QList<int> inWPs;
    QList<int> outWPs;
    int oncomingLegID;
    QList<int> leftTurnLegID;
    QList<int> rightTurnLegID;
};


struct WayPoint
{
    int id;
    QVector3D pos;
    float angle;
    QList<int> relatedLanes;
};


struct RouteElem
{
    int inDir;
    int node;
    int outDir;
};


struct TrafficVolumeData
{
    int vehicleKind;
    int trafficVolume;    // count per 1-hour
};


struct RouteData
{
    float selProb;
    QList<RouteElem*> nodeList;
    QList<struct TrafficVolumeData*> volume;
};


struct ODData
{
    int destinationNode;
    QList<struct RouteData*> route;
};


struct NodeInfo
{
    int id;
    QVector2D pos;

    int nLeg;
    QList<struct LegInfo*> legInfo;

    QList<struct LaneLists*> laneList;

    bool hasTS;
    QList<struct TrafficSignalInfo*> trafficSignals;

    QList<struct StopLineInfo*> stopLines;

    QList<int> relatedLanes;

    bool isOriginNode;
    bool isDestinationNode;
    QList<ODData*> odData;
};


struct baseMapImage
{
    QString path;
    QString filename;
    float scale;
    float x;
    float y;
    float rotate;
    float halfWidth;
    float halfHeight;
    GLuint textureID;
    bool isValid;
};

#endif // ROADINFOELEMENT_H
