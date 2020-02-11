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

#include "roadinfo.h"
#include "objectproperty.h"
#include <QDebug>


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


    void SelectObject(bool shiftModifier);
    enum _SEL_OBJ{
        SEL_NOOBJ,
        SEL_NODE,
        SEL_LANE,
        SEL_LANE_EDGE_START,
        SEL_LANE_EDGE_END,
        SEL_TRAFFIC_SIGNAL,
        SEL_STOPLINE,
        MOVE_OBJECT,
        ROTATE_OBJECT,
        SEL_NODE_ROUTE_PICK,
    };

    RoadInfo *road;
    RoadObjectProperty *roadProperty;


    // Object Selection
    struct ObjectSelect selectedObj;
    struct UndoInfo undoInfo;

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);

signals:
    void UpdateStatusBar(QString);
    void SetNodeListForRoute(QList<int>);

public slots:
    void SetNodeVisibility(bool);
    void SetNodeLabelVisibility(bool);
    void SetNodeLaneListlVisibility(bool);
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
    void ResetRotate();
    void MoveTo(float x,float y);
    void SetProjectionOrthogonal(bool);
    void SetLabelVisibility(bool);
    void SetNodeSelection(bool);
    void SetLaneSelection(bool);
    void SetTrafficSignalSelection(bool);
    void SetStopLineSelection(bool);
    void SetNodePickMode(int,int);
    void ResetNodePickMode();


private:
    float X_eye;
    float Y_eye;
    float Z_eye;

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



    // Flags
    bool isOrthogonal;

    bool showNodesFlag;
    bool showLanesFlag;
    bool showTrafficSignalsFlag;
    bool showStopLinesFlag;

    bool showNodeLabelsFlag;
    bool showLaneLabelsFlag;
    bool showTrafficSignalLabelsFlag;
    bool showStopLineLabelsFlag;
    bool showLabelsFlag;

    bool selectNodeFlag;
    bool selectLaneFlag;
    bool selectTrafficSignalFlag;
    bool selectStopLineFlag;

    bool LaneListFlag;
    bool objectMoveFlag;
    bool nodePickModeFlag;

    int laneListIndex;
};


#endif // GRAPHICCANVAS_H