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


#include "graphiccanvas.h"


void GraphicCanvas::ResetRotate()
{
    cameraYaw   = 0.0;
    cameraPitch = 0.0;
    cameraQuat  = QQuaternion(cos(cameraPitch*0.5), sin(cameraPitch*0.5) , 0.0 , 0.0 ) * QQuaternion(cos(cameraYaw*0.5), 0.0 , 0.0 , sin(cameraYaw*0.5));
    update();
}

void GraphicCanvas::MoveTo(float x,float y)
{
    X_eye = -x;
    Y_eye = -y;
    update();
}

void GraphicCanvas::SetMapVisibility(bool b)
{
    showMapImageFlag = b;
    update();
}

void GraphicCanvas::SetBackMap(bool b)
{
    backMapImageFlag = b;
    update();
}

void GraphicCanvas::SetNodeVisibility(bool b)
{
    showNodesFlag = b;
    update();
}

void GraphicCanvas::SetNodeLabelVisibility(bool b)
{
    showNodeLabelsFlag = b;
    update();
}

void GraphicCanvas::SetNodeLaneListlVisibility(bool b)
{
    LaneListFlag = b;
    update();
}

void GraphicCanvas::SetRelatedLaneslVisibility(bool b)
{
    RelatedLanesFlag = b;
    update();
}

void GraphicCanvas::SetRouteLaneListlVisibility(bool b)
{
    RouteLaneListFlag = b;
    update();
}

void GraphicCanvas::ResetLaneListIndex()
{
    laneListIndex = 0;
    update();
}

void GraphicCanvas::ShowPrevLaneList()
{
    laneListIndex--;
    if( laneListIndex < 0 ){
        laneListIndex = 0;
    }
    update();
}

void GraphicCanvas::ShowNextLaneList()
{
    if( selectedObj.selObjKind.size() > 0 ){
        if( selectedObj.selObjKind[0] == _SEL_OBJ::SEL_NODE ){
            int ndIdx = road->indexOfNode( selectedObj.selObjID[0] );
            if( ndIdx >= 0 ){

                int maxListNum = 0;
                for(int i=0;i<road->nodes[ndIdx]->laneList.size();++i){
                    maxListNum += road->nodes[ndIdx]->laneList[i]->lanes.size();
                }

                if( laneListIndex < maxListNum - 1 ){
                    laneListIndex++;
                    update();
                }
            }
        }
    }
}

void GraphicCanvas::SetLaneVisibility(bool b)
{
    showLanesFlag = b;
    update();
}

void GraphicCanvas::SetLaneLabelVisibility(bool b)
{
    showLaneLabelsFlag = b;
    update();
}

void GraphicCanvas::SetLaneDrawWidth(int w)
{
    laneDrawWidth = w;
    update();
}

void GraphicCanvas::SetTrafficSignalVisibility(bool b)
{
    showTrafficSignalsFlag = b;
    update();
}

void GraphicCanvas::SetTrafficSignalLabelVisibility(bool b)
{
    showTrafficSignalLabelsFlag = b;
    update();
}

void GraphicCanvas::SetStopLineVisibility(bool b)
{
    showStopLinesFlag = b;
    update();
}

void GraphicCanvas::SetStopLineLabelVisibility(bool b)
{
    showStopLineLabelsFlag = b;
    update();
}

void GraphicCanvas::SetPedestLaneVisibility(bool b)
{
    showPedestLaneFlag = b;
    update();
}

void GraphicCanvas::SetPedestLaneLabelVisibility(bool b)
{
    showPedestLaneLabelsFlag = b;
    update();
}

void GraphicCanvas::SetProjectionOrthogonal(bool b)
{
    isOrthogonal = b;

    qDebug() << "[SetProjectionOrthogonal] isOrthogonal = " << isOrthogonal;

    projection.setToIdentity();
    if( isOrthogonal == true ){
        float nearDist = 0.1;
        float farDist = 10000.0;
        float t = Z_eye * tan(45.0f / 2.0f * 0.017452f) * (-1.0);
        float r = t * aspectRatio;
        projection.ortho( -r, r, -t, t, nearDist, farDist );

        cameraYawSave = cameraYaw;
        cameraPitchSave = cameraPitch;

        cameraYaw = 0.0;
        cameraPitch = 0.0;
        cameraQuat = QQuaternion(cos(cameraPitch*0.5), sin(cameraPitch*0.5) , 0.0 , 0.0 ) * QQuaternion(cos(cameraYaw*0.5), 0.0 , 0.0 , sin(cameraYaw*0.5));
    }
    else{
        projection.perspective( fieldOfView, aspectRatio, 0.1, 100000.0 );

        cameraYaw = cameraYawSave;
        cameraPitch = cameraPitchSave;
        cameraQuat = QQuaternion(cos(cameraPitch*0.5), sin(cameraPitch*0.5) , 0.0 , 0.0 ) * QQuaternion(cos(cameraYaw*0.5), 0.0 , 0.0 , sin(cameraYaw*0.5));
    }

    update();
}

void GraphicCanvas::SetLabelVisibility(bool b)
{
    showLabelsFlag = b;
    update();
}

void GraphicCanvas::SetNodeSelection(bool b)
{
    selectNodeFlag = b;
}

void GraphicCanvas::SetLaneSelection(bool b)
{
    selectLaneFlag = b;
}

void GraphicCanvas::SetTrafficSignalSelection(bool b)
{
    selectTrafficSignalFlag = b;
}

void GraphicCanvas::SetStopLineSelection(bool b)
{
    selectStopLineFlag = b;
}

void GraphicCanvas::SetPedestLaneSelection(bool b)
{
    selectPedestLaneFlag = b;
}

