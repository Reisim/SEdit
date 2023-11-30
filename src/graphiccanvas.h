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


#ifndef GRAPHICCANVAS_H
#define GRAPHICCANVAS_H

#include <QWidget>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include <QMouseEvent>
#include <QWheelEvent>

#include <QVector3D>
#include <QMatrix4x4>
#include <QMap>
#include "gltransform3d.h"
#include <math.h>

#include "basemapimagemanager.h"
#include "roadinfo.h"
#include "objectproperty.h"
#include "odrouteeditor.h"
#include "scenarioeditor.h"
#include "displaycontrol.h"


#include <QAction>
#include <QMenu>

#include <QDebug>

#include <windows.h>


#define NODE_CIRCLE_DIV  (30)


struct Character {
    GLuint TextureID;
    QSize Size;
    QSize Bearing;
    GLuint Advance;
};


struct AxisPoly
{
    bool isValid;
    QOpenGLVertexArrayObject array;
    QOpenGLBuffer *buffer;
    QVector<GLfloat> vertex;
};


struct TextPoly
{
    bool isTextValid;
    QOpenGLVertexArrayObject textArray;
    QOpenGLBuffer *textBuffer;
};


struct CirclePoly
{
    bool isValid;
    QOpenGLVertexArrayObject array;
    QOpenGLBuffer *buffer;
    QVector<GLfloat> vertex;
};


struct LinePoly
{
    bool isValid;
    QOpenGLVertexArrayObject array;
    QOpenGLBuffer *buffer;
    QVector<GLfloat> vertex;
};


struct RectPoly
{
    bool isValid;
    QOpenGLVertexArrayObject array;
    QOpenGLBuffer *buffer;
    QVector<GLfloat> vertex;
};


struct TrianglePoly
{
    bool isValid;
    QOpenGLVertexArrayObject array;
    QOpenGLBuffer *buffer;
    QVector<GLfloat> vertex;
};


struct VehiclePoly
{
    bool isValid;
    QOpenGLVertexArrayObject array;
    QOpenGLBuffer *buffer;
    QVector<GLfloat> vertex;
};


struct BoxPoly
{
    bool isValid;
    QOpenGLVertexArrayObject array;
    QOpenGLBuffer *buffer;
    QVector<GLfloat> vertex;
};


struct ObjectSelect
{
    QList<int> selObjKind;
    QList<int> selObjID;
};


struct UndoInfo
{
    bool setUndoInfo;
    QList<int> selObjKind;
    QList<int> selObjID;
    QList<float> data;
    int operationType;
};


struct LineObject
{
    int color;
    QList<QPointF> coord;
    QList<QPointF> p;
};

struct LineObjectCoordInfo
{
    QPointF center;
    float scale_x;
    float scale_y;
};


class GraphicCanvas : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit GraphicCanvas(QOpenGLWidget *parent = nullptr);
    ~GraphicCanvas();

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    int Get3DPhysCoordFromPickPoint(int xp,int yp, float &x,float &y);
    QVector2D GetMouseClickPosition(){ return mousePressPosition; }

    void SetNumberKeyPressed(int key);
    int GetNumberKeyPressed(){ return numberKeyPressed; }


    void SelectObject(bool shiftModifier);
    enum _SEL_OBJ{
        SEL_NOOBJ,
        SEL_NODE,
        SEL_LANE,
        SEL_LANE_EDGE_START,
        SEL_LANE_EDGE_END,
        SEL_TRAFFIC_SIGNAL,
        SEL_STOPLINE,
        SEL_PEDEST_LANE,
        SEL_PEDEST_LANE_POINT,
        MOVE_OBJECT,
        ROTATE_OBJECT,
        SEL_NODE_ROUTE_PICK,
        SEL_STATIC_OBJECT,
    };

    RoadInfo *road;
    RoadObjectProperty *roadProperty;
    BaseMapImageManager *mapImageMng;
    ODRouteEditor *odRoute;
    ScenarioEditor *scnrEdit;
    DisplayControl *dispCtrl;

    QMenu *addObjToNodePopup;
    QAction *createVTS;
    QAction *createPTS;
    QAction *createSL;


    // Object Selection
    struct ObjectSelect selectedObj;
    struct UndoInfo undoInfo;

    // Line Object
    QList<struct LineObject *> lineObj;
    struct LineObjectCoordInfo lineObjCoordInfo;

    // Point Data for create Pedest Lane
    QList<QVector3D*> pedestLanePoints;


protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);


//    void PutText(float x,float y,float z,char*);
//    void DrawLine(float x,float y,float z,float len,float angle,float thickness,float cr,float cg,float cb);
//    void DrawCircle(float x,float y,float z,float R,float cr,float cg,float cb,bool fill);



signals:
    void UpdateStatusBar(QString);
    void SetNodeListForRoute(QList<int>);
    void PedestLanePointPicked();
    void PointsPickedForScenario(int,float,float,float,float);
    void PointListForScenario(int,QList<QPointF>);

public slots:
    void SetMapVisibility(bool);
    void SetBackMap(bool);
    void SetNodeVisibility(bool);
    void SetNodeLabelVisibility(bool);
    void SetNodeLaneListlVisibility(bool);
    void SetRelatedLaneslVisibility(bool);
    void SetRouteLaneListlVisibility(bool);
    void ShowPrevLaneList();
    void ShowNextLaneList();
    void ResetLaneListIndex();
    void SetLaneVisibility(bool);
    void SetLaneLabelVisibility(bool);
    void SetLaneDrawWidth(int);
    void SetTrafficSignalVisibility(bool);
    void SetTrafficSignalLabelVisibility(bool);
    void SetStopLineVisibility(bool);
    void SetStopLineLabelVisibility(bool);
    void SetPedestLaneVisibility(bool);
    void SetPedestLaneLabelVisibility(bool);
    void SetStaticObjectVisibility(bool);
    void SetStaticObjectLabelVisibility(bool);
    void ResetRotate();
    void MoveTo(float x,float y);
    void SetProjectionOrthogonal(bool);
    void SetLabelVisibility(bool);
    void SetNodeSelection(bool);
    void SetLaneSelection(bool);
    void SetTrafficSignalSelection(bool);
    void SetStopLineSelection(bool);
    void SetPedestLaneSelection(bool);
    void SetStaticObjectSelection(bool);
    void SetNodePickMode(int,int);
    void ResetNodePickMode();
    void LoadMapImage(struct baseMapImage *);
    void DeleteMapImage(struct baseMapImage *);
    void SetPedestLanePointPickMode();
    void ResetPedestLanePointPickMode();
    void RemovePickedPedestLanePoint();
    void SetNodeSelected(int);
    void SetLaneColorBySpeedLimitFlag(bool);
    void SetLaneColorByActualSpeedFlag(bool);
    void SetScenarioPickMode(int);
    void ChangeSelectionRequest(int,int);
    void SetLaneColorByODDFlag(bool);


private:
    float X_eye;
    float Y_eye;
    float Z_eye;

    float X_trans;
    float Y_trans;
    float Z_trans;

    float fieldOfView;
    float aspectRatio;
    float currentWidth;
    float currentHeight;

    QVector2D mousePressPosition;
    bool mousePressed;
    float sx;
    float sy;
    float wxMousePress;
    float wyMousePress;
    float wxMouseMove;
    float wyMouseMove;
    bool wxyValid;
    int rotDir;
    int numberKeyPressed;

    int laneDrawWidth;

    QOpenGLShaderProgram *program;

    int u_modelToCamera;
    int u_CameraToView;
    int u_isText;
    int u_letterPos;
    int u_useTex;
    int u_vColor;
    int u_texture;

    QMatrix4x4 projection;
    GLTransform3D model2World;
    QQuaternion cameraQuat;

    float cameraYaw;
    float cameraPitch;
    float cameraYawSave;
    float cameraPitchSave;


    // Shader Files
    QString vertexShaderProgramFile;
    QString fragmentShaderProgramFile;


    // Font
    QMap<GLchar, Character*> Characters;


    // Polygons
    struct AxisPoly axisPoly;
    struct TextPoly textPoly;
    struct CirclePoly circlePoly;
    struct LinePoly linePoly;
    struct RectPoly rectPoly;
    struct TrianglePoly trianglePoly;
    struct VehiclePoly vhclPoly;
    struct BoxPoly boxPoly;

    // Flags
    bool isOrthogonal;

    bool showMapImageFlag;
    bool backMapImageFlag;
    bool showNodesFlag;
    bool showLanesFlag;
    bool showTrafficSignalsFlag;
    bool showStopLinesFlag;
    bool showPedestLaneFlag;
    bool showStaticObjectFlag;

    bool showNodeLabelsFlag;
    bool showLaneLabelsFlag;
    bool showTrafficSignalLabelsFlag;
    bool showStopLineLabelsFlag;
    bool showPedestLaneLabelsFlag;
    bool showStaticObjectLabelsFlag;
    bool showLabelsFlag;

    bool selectNodeFlag;
    bool selectLaneFlag;
    bool selectTrafficSignalFlag;
    bool selectStopLineFlag;
    bool selectPedestLaneFlag;
    bool selectStaticObjectFlag;

    bool LaneListFlag;
    bool RelatedLanesFlag;
    bool RouteLaneListFlag;
    bool objectMoveFlag;
    bool nodePickModeFlag;
    bool pedestPathPointPickFlag;

    bool colorLaneBySpeedLimitFlag;
    bool colorLaneByActualSpeedFlag;
    bool colorLaneByODDFlag;

    int laneListIndex;

    int scenarioPickMode;
    int scenarioPickCount;
    QPointF pickedPoint1;
    QList<QPointF> pathRoutePointStock;

    bool selectByArea;
    QPointF selectByArearPoints[4];

    bool cutLaneByLine;
    QPointF cutLaneByLinePoints[2];

    // Folders
    QStringList imageFilePathFolders;
    QString lastImageLoadFolder;


    LARGE_INTEGER start, end;
    LARGE_INTEGER freq;
};


#endif // GRAPHICCANVAS_H
