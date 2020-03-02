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

void GraphicCanvas::mousePressEvent(QMouseEvent *e)
{
    mousePressPosition = QVector2D(e->localPos());

    float x,y;
    int ret = Get3DPhysCoordFromPickPoint( e->x(), e->y(), x, y );
    if( ret == 1 ){
        wxMousePress = x;
        wyMousePress = y;
        wxMouseMove = x;
        wyMouseMove = y;
        wxyValid = true;
    }
    else{
        wxyValid = false;
    }

    if( pedestPathPointPickFlag == true ){

        bool addOK = true;
        for(int i=0;i<pedestLanePoints.size();++i){
            float dx = wxMousePress - pedestLanePoints[i]->x();
            float dy = wyMousePress - pedestLanePoints[i]->y();
            float L = dx * dx + dy * dy;
            if( L < 0.5 ){
                addOK = false;
                break;
            }
        }

        if( addOK == true ){
            QVector3D *p = new QVector3D();

            p->setX( wxMousePress );
            p->setY( wyMousePress );
            p->setZ( 0.0 );

            pedestLanePoints.append( p );
        }

        update();

        objectMoveFlag = false;
        mousePressed = true;

        return;
    }

    rotDir = 0;

    if( e->modifiers() & Qt::ControlModifier ){
        if( e->buttons() & Qt::RightButton ){
            selectedObj.selObjKind.clear();
            selectedObj.selObjID.clear();
            numberKeyPressed = -1;
        }
        else{
            SelectObject( e->modifiers() & Qt::ShiftModifier );
            if( selectedObj.selObjKind.size() > 0 ){
                undoInfo.setUndoInfo = false;
                undoInfo.selObjKind.clear();
                undoInfo.selObjID.clear();
                undoInfo.data.clear();
            }
        }
        update();
    }

    objectMoveFlag = false;

    mousePressed = true;

    //qDebug() << "ret="<< ret << " Click Point: (" << e->x() << "," << e->y() << ") -> World: (" << x << "," << y << ",0.0)";
}


void GraphicCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    mousePressed = false;

    if( selectedObj.selObjKind.size() > 0 && objectMoveFlag == true ){

        road->CheckLaneCrossPoints();

        for(int i=0;i<selectedObj.selObjKind.size();++i){
            if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_NODE ){
                int nIdx = road->indexOfNode( selectedObj.selObjID[i] );
                if( nIdx >= 0 ){
                    for(int j=0;j<road->nodes[nIdx]->stopLines.size();++j){
                        int slId = road->nodes[nIdx]->stopLines[j]->id;
                        road->CheckStopLineCrossLanes( slId );
                    }
                }
            }
        }

        for(int i=0;i<selectedObj.selObjKind.size();++i){
            if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_STOPLINE ){
                road->CheckStopLineCrossLanes( selectedObj.selObjID[i] );
            }
        }

        update();
    }

}


void GraphicCanvas::mouseMoveEvent(QMouseEvent *e)
{
    float x,y;
    int ret = Get3DPhysCoordFromPickPoint( e->x(), e->y(), x, y );

    if( mousePressed == false ){
        QString currentCoordinateStr = QString("X=%1, Y=%2").arg(x).arg(y);
        emit UpdateStatusBar( currentCoordinateStr );
    }


    if( e->buttons() & Qt::RightButton ){

        if( selectedObj.selObjKind.size() > 0 && (e->modifiers() & Qt::AltModifier) ){

            if( undoInfo.setUndoInfo == false ){
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    undoInfo.selObjKind.append( selectedObj.selObjKind[i] );
                    undoInfo.selObjID.append( selectedObj.selObjID[i] );
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_NODE ){
                        for(int j=0;j<road->GetNodeNumLeg( selectedObj.selObjID[i] );++j){
                            undoInfo.data.append( road->GetNodeLegAngle(selectedObj.selObjID[i], j) );
                        }
                    }
                    else if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE_EDGE_START ){
                        int lidx = road->indexOfLane( selectedObj.selObjID[i] );
                        if( lidx >= 0 ){
                            undoInfo.data.append( road->lanes[lidx]->shape.derivative.first()->x() );
                            undoInfo.data.append( road->lanes[lidx]->shape.derivative.first()->y() );
                        }
                    }
                    else if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE_EDGE_END ){
                        int lidx = road->indexOfLane( selectedObj.selObjID[i] );
                        if( lidx >= 0 ){
                            undoInfo.data.append( road->lanes[lidx]->shape.derivative.last()->x() );
                            undoInfo.data.append( road->lanes[lidx]->shape.derivative.last()->y() );
                        }
                    }
                }
                undoInfo.setUndoInfo = true;
                undoInfo.operationType = _SEL_OBJ::ROTATE_OBJECT;
//                qDebug() << "Set UndoInfo:";
//                for(int i=0;i<undoInfo.data.size();++i){
//                    qDebug() << "data[" << i << "] = " << undoInfo.data[i];
//                }
            }

            QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;

            diff.setX( diff.x() * sx );
            diff.setY( diff.y() * sy );
            float a = diff.length();
            if( a > 0.0 ){

                QVector2D pos_center;

                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_NODE ){
                        if(i == 0){
                            if( ret == 1 ){
                                pos_center = road->GetNodePosition( selectedObj.selObjID[i] );

                                float dx1 = wxMouseMove - pos_center.x();
                                float dy1 = wyMouseMove - pos_center.y();
                                float dx2 = x - pos_center.x();
                                float dy2 = y - pos_center.y();
                                if( dx2 * (-dy1) + dy2 * dx1 > 0.0 ){
                                    rotDir = 1;
                                }
                                else{
                                    rotDir = 2;
                                }
                                wxMouseMove = x;
                                wyMouseMove = y;
                            }

                            if( rotDir == 2 ){
                                a *= (-1.0);
                            }

                            a /= sx;

                            if( numberKeyPressed < 0 ){
                                road->RotateNode( selectedObj.selObjID[i], a );
                            }
                            else{
                                road->RotateNodeLeg( selectedObj.selObjID[i],
                                                     numberKeyPressed - 1,
                                                     a );
                            }

                        }
                        else{

                            road->RotateNode( selectedObj.selObjID[i], a );

                            QVector2D pos = road->GetNodePosition( selectedObj.selObjID[i] );
                            float dx = pos.x() - pos_center.x();
                            float dy = pos.y() - pos_center.y();

                            float cp = cos( a * 0.017452 );
                            float sp = sin( a * 0.017452 );

                            float rx = dx * cp + dy * (-sp);
                            float ry = dx * sp + dy * cp;

                            float xMove = rx - dx;
                            float yMove = ry - dy;
                            road->MoveNode( selectedObj.selObjID[i], xMove, yMove );
                        }
                    }
                    else if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE_EDGE_START ){

                        QList<int> sMoveLane;
                        QList<int> eMoveLane;
                        sMoveLane.append( selectedObj.selObjID[i] );

                        for(int l=0;l<sMoveLane.size();++l){
                            int lidx = road->indexOfLane( sMoveLane[l] );
                            if( lidx >= 0 ){
                                int nPL = road->lanes[lidx]->previousLanes.size();
                                for(int j=0;j<nPL;++j){
                                    int prevLane = road->lanes[lidx]->previousLanes[j];
                                    if( eMoveLane.contains(prevLane) == false ){
                                        eMoveLane.append( prevLane );
                                    }
                                    int nlidx = road->indexOfLane( prevLane );
                                    if( nlidx >= 0 ){
                                        int nNL = road->lanes[nlidx]->nextLanes.size();
                                        for(int k=0;k<nNL;++k){
                                            int nextLane = road->lanes[nlidx]->nextLanes[k];
                                            if( sMoveLane.contains( nextLane ) == false ){
                                                sMoveLane.append( nextLane );
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if(i == 0){

                            if( ret == 1 ){
                                int lidx = road->indexOfLane( selectedObj.selObjID[i] );
                                if( lidx >= 0 ){

                                    pos_center.setX( road->lanes[lidx]->shape.pos.first()->x() );
                                    pos_center.setY( road->lanes[lidx]->shape.pos.first()->y() );

                                    float dx1 = wxMouseMove - pos_center.x();
                                    float dy1 = wyMouseMove - pos_center.y();
                                    float dx2 = x - pos_center.x();
                                    float dy2 = y - pos_center.y();
                                    if( dx2 * (-dy1) + dy2 * dx1 > 0.0 ){
                                        rotDir = 1;
                                    }
                                    else{
                                        rotDir = 2;
                                    }
                                    wxMouseMove = x;
                                    wyMouseMove = y;
                                }
                            }

                            if( rotDir == 2 ){
                                a *= (-1.0);
                            }

                            a /= sx;
                        }

                        for(int j=0;j<sMoveLane.size();++j){
                            road->RotateLaneEdge( sMoveLane[j], a, 0 );  // Lane Start Point
                        }
                        for(int j=0;j<eMoveLane.size();++j){
                            road->RotateLaneEdge( eMoveLane[j], a, 1 );  // Lane End Point
                        }

                        if(i > 0 ){

                            int lidx = road->indexOfLane( selectedObj.selObjID[i] );
                            if( lidx >= 0 ){

                                float dx = road->lanes[lidx]->shape.pos.first()->x() - pos_center.x();
                                float dy = road->lanes[lidx]->shape.pos.first()->y() - pos_center.y();

                                float cp = cos( a * 0.017452 );
                                float sp = sin( a * 0.017452 );

                                float rx = dx * cp + dy * (-sp);
                                float ry = dx * sp + dy * cp;

                                float xMove = rx - dx;
                                float yMove = ry - dy;

                                for(int j=0;j<sMoveLane.size();++j){
                                    road->MoveLaneEdge( sMoveLane[j], xMove, yMove, 0 );  // Lane Start Point
                                }
                                for(int j=0;j<eMoveLane.size();++j){
                                    road->MoveLaneEdge( eMoveLane[j], xMove, yMove, 1 );  // Lane End Point
                                }
                            }
                        }
                    }
                    else if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE_EDGE_END ){

                        QList<int> sMoveLane;
                        QList<int> eMoveLane;
                        eMoveLane.append( selectedObj.selObjID[i] );

                        for(int l=0;l<eMoveLane.size();++l){
                            int lidx = road->indexOfLane( eMoveLane[l] );
                            if( lidx >= 0 ){
                                int nNL = road->lanes[lidx]->nextLanes.size();
                                for(int j=0;j<nNL;++j){
                                    int nextLane = road->lanes[lidx]->nextLanes[j];
                                    if( sMoveLane.contains(nextLane) == false ){
                                        sMoveLane.append( nextLane );
                                    }
                                    int nlidx = road->indexOfLane( nextLane );
                                    if( nlidx >= 0 ){
                                        int nPL = road->lanes[nlidx]->previousLanes.size();
                                        for(int k=0;k<nPL;++k){
                                            int prevLane = road->lanes[nlidx]->previousLanes[k];
                                            if( eMoveLane.contains( prevLane ) == false ){
                                                eMoveLane.append( prevLane );
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if(i == 0){
                            if( ret == 1 ){
                                int lidx = road->indexOfLane( selectedObj.selObjID[i] );
                                if( lidx >= 0 ){

                                    pos_center.setX( road->lanes[lidx]->shape.pos.last()->x() );
                                    pos_center.setY( road->lanes[lidx]->shape.pos.last()->y() );

                                    float dx1 = wxMouseMove - pos_center.x();
                                    float dy1 = wyMouseMove - pos_center.y();
                                    float dx2 = x - pos_center.x();
                                    float dy2 = y - pos_center.y();
                                    if( dx2 * (-dy1) + dy2 * dx1 > 0.0 ){
                                        rotDir = 1;
                                    }
                                    else{
                                        rotDir = 2;
                                    }
                                    wxMouseMove = x;
                                    wyMouseMove = y;
                                }
                            }

                            if( rotDir == 2 ){
                                a *= (-1.0);
                            }

                            a /= sx;
                        }

                        for(int j=0;j<sMoveLane.size();++j){
                            road->RotateLaneEdge( sMoveLane[j], a, 0 );  // Lane Start Point
                        }
                        for(int j=0;j<eMoveLane.size();++j){
                            road->RotateLaneEdge( eMoveLane[j], a, 1 );  // Lane End Point
                        }

                        if( i > 0 ){
                            int lidx = road->indexOfLane( selectedObj.selObjID[i] );
                            if( lidx >= 0 ){

                                float dx = road->lanes[lidx]->shape.pos.last()->x() - pos_center.x();
                                float dy = road->lanes[lidx]->shape.pos.last()->y() - pos_center.y();

                                float cp = cos( a * 0.017452 );
                                float sp = sin( a * 0.017452 );

                                float rx = dx * cp + dy * (-sp);
                                float ry = dx * sp + dy * cp;

                                float xMove = rx - dx;
                                float yMove = ry - dy;

                                for(int j=0;j<sMoveLane.size();++j){
                                    road->MoveLaneEdge( sMoveLane[j], xMove, yMove, 0 );  // Lane Start Point
                                }
                                for(int j=0;j<eMoveLane.size();++j){
                                    road->MoveLaneEdge( eMoveLane[j], xMove, yMove, 1 );  // Lane End Point
                                }
                            }
                        }
                    }
                }

                objectMoveFlag = true;
            }
        }
        // Normal Map Operation
        else{

            if( isOrthogonal == false){
                QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;

                diff.setX( diff.x() * sx );
                diff.setY( diff.y() * sy );
                float a = diff.length();
                if( a > 0.0 ){

                    cameraYaw += diff.x() * 0.75;
                    cameraPitch += diff.y() * 0.75;

                    cameraQuat = QQuaternion(cos(cameraPitch*0.5), sin(cameraPitch*0.5) , 0.0 , 0.0 ) * QQuaternion(cos(cameraYaw*0.5), 0.0 , 0.0 , sin(cameraYaw*0.5));
                }
            }
        }

        QOpenGLWidget::update();
    }
    else if( e->buttons() & Qt::LeftButton ){

        if( selectedObj.selObjKind.size() > 0 && (e->modifiers() & Qt::AltModifier) ){

            if( undoInfo.setUndoInfo == false ){
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    undoInfo.selObjKind.append( selectedObj.selObjKind[i] );
                    undoInfo.selObjID.append( selectedObj.selObjID[i] );
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_NODE ){
                        QVector2D pos = road->GetNodePosition( selectedObj.selObjID[i] );
                        if( pos != QVector2D() ){
                                undoInfo.data.append( pos.x() );
                                undoInfo.data.append( pos.y() );
                        }
                    }
                    else if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE ){
                        int lidx = road->indexOfLane( selectedObj.selObjID[i] );
                        if( lidx >= 0 ){
                            undoInfo.data.append( road->lanes[lidx]->shape.pos.first()->x() );
                            undoInfo.data.append( road->lanes[lidx]->shape.pos.first()->y() );
                            undoInfo.data.append( road->lanes[lidx]->shape.pos.last()->x() );
                            undoInfo.data.append( road->lanes[lidx]->shape.pos.last()->y() );
                        }
                    }
                    else if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE_EDGE_START ){
                        int lidx = road->indexOfLane( selectedObj.selObjID[i] );
                        if( lidx >= 0 ){
                            undoInfo.data.append( road->lanes[lidx]->shape.pos.first()->x() );
                            undoInfo.data.append( road->lanes[lidx]->shape.pos.first()->y() );
                        }
                    }
                    else if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE_EDGE_END ){
                        int lidx = road->indexOfLane( selectedObj.selObjID[i] );
                        if( lidx >= 0 ){
                            undoInfo.data.append( road->lanes[lidx]->shape.pos.last()->x() );
                            undoInfo.data.append( road->lanes[lidx]->shape.pos.last()->y() );
                        }
                    }
                    else if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_TRAFFIC_SIGNAL ){
                        int rNd = road->indexOfTS( selectedObj.selObjID[i], -1 );
                        int nIdx = road->indexOfNode( rNd );
                        if( nIdx >= 0 ){
                            int tsIdx = road->indexOfTS( selectedObj.selObjID[i], rNd );
                            undoInfo.data.append( road->nodes[nIdx]->trafficSignals[tsIdx]->pos.x() );
                            undoInfo.data.append( road->nodes[nIdx]->trafficSignals[tsIdx]->pos.y() );
                        }
                    }
                    else if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_STOPLINE ){
                        int rNd = road->indexOfSL( selectedObj.selObjID[i], -1 );
                        int nIdx = road->indexOfNode( rNd );
                        if( nIdx >= 0 ){
                            int slIdx = road->indexOfSL( selectedObj.selObjID[i], rNd );
                            undoInfo.data.append( road->nodes[nIdx]->stopLines[slIdx]->leftEdge.x() );
                            undoInfo.data.append( road->nodes[nIdx]->stopLines[slIdx]->leftEdge.y() );
                            undoInfo.data.append( road->nodes[nIdx]->stopLines[slIdx]->rightEdge.x() );
                            undoInfo.data.append( road->nodes[nIdx]->stopLines[slIdx]->rightEdge.y() );
                        }
                    }

                }
                undoInfo.setUndoInfo = true;
                undoInfo.operationType = _SEL_OBJ::MOVE_OBJECT;
            }

            QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;
            float s = 0.02 * Z_eye / (-50.0);
            float xMove = diff.x() * s;
            float yMove = diff.y() * (-s);

            float cy = cos( cameraYaw );
            float sy = sin( cameraYaw );

            float xMy = xMove * cy + yMove * (sy);
            float yMy = xMove * (-sy) + yMove * cy;

            xMove = xMy;
            yMove = yMy;

            // Move Node
            {
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_NODE ){
                        road->MoveNode( selectedObj.selObjID[i], xMove, yMove );
                    }
                }
            }


            // Move Lane
            {
                QList<int> directMoveLane;
                QList<int> sMoveLane;
                QList<int> eMoveLane;
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE && directMoveLane.contains(selectedObj.selObjID[i]) == false ){
                        directMoveLane.append( selectedObj.selObjID[i] );
                    }
                }

                for(int i=0;i<directMoveLane.size();++i){
                    int lidx = road->indexOfLane( directMoveLane[i] );
                    if( lidx >= 0 ){
                        int nNL = road->lanes[lidx]->nextLanes.size();
                        for(int j=0;j<nNL;++j){
                            int nextLane = road->lanes[lidx]->nextLanes[j];
                            if( directMoveLane.contains( nextLane ) == true ){
                                continue;
                            }
                            if( sMoveLane.contains(nextLane) == false ){
                                sMoveLane.append( nextLane );
                            }
                            int nlidx = road->indexOfLane( nextLane );
                            if( nlidx >= 0 ){
                                int nPL = road->lanes[nlidx]->previousLanes.size();
                                for(int k=0;k<nPL;++k){
                                    int prevLane = road->lanes[nlidx]->previousLanes[k];
                                    if( directMoveLane.contains( prevLane ) == true ){
                                        continue;
                                    }
                                    if( eMoveLane.contains( prevLane ) == false ){
                                        eMoveLane.append( prevLane );
                                    }
                                }
                            }
                        }
                        int nPL = road->lanes[lidx]->previousLanes.size();
                        for(int j=0;j<nPL;++j){
                            int prevLane = road->lanes[lidx]->previousLanes[j];
                            if( directMoveLane.contains( prevLane ) == true ){
                                continue;
                            }
                            if( eMoveLane.contains( prevLane ) == false ){
                                eMoveLane.append( prevLane );
                            }
                            int plidx = road->indexOfLane( prevLane );
                            if( plidx >= 0 ){
                                int nNL = road->lanes[plidx]->nextLanes.size();
                                for(int k=0;k<nNL;++k){
                                    int nextLane = road->lanes[plidx]->nextLanes[k];
                                    if( directMoveLane.contains( nextLane ) == true ){
                                        continue;
                                    }
                                    if( sMoveLane.contains( nextLane ) == false ){
                                        sMoveLane.append( nextLane );
                                    }
                                }
                            }
                        }
                    }
                }


                for(int i=0;i<directMoveLane.size();++i){
                    road->MoveLane( directMoveLane[i], xMove, yMove, true);
                }
                for(int i=0;i<sMoveLane.size();++i){
                    road->MoveLaneEdge( sMoveLane[i], xMove, yMove, 0 );  // Lane Start Point
                }
                for(int i=0;i<eMoveLane.size();++i){
                    road->MoveLaneEdge( eMoveLane[i], xMove, yMove, 1 );  // Lane End Point
                }
            }


            // Move Lane Edge START
            {
                QList<int> sMoveLane;
                QList<int> eMoveLane;
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE_EDGE_START && sMoveLane.contains(selectedObj.selObjID[i]) == false ){
                        sMoveLane.append( selectedObj.selObjID[i] );
                    }
                }

                for(int i=0;i<sMoveLane.size();++i){
                    int lidx = road->indexOfLane( sMoveLane[i] );
                    if( lidx >= 0 ){
                        int nPL = road->lanes[lidx]->previousLanes.size();
                        for(int j=0;j<nPL;++j){
                            int prevLane = road->lanes[lidx]->previousLanes[j];
                            if( eMoveLane.contains(prevLane) == false ){
                                eMoveLane.append( prevLane );
                            }
                            int nlidx = road->indexOfLane( prevLane );
                            if( nlidx >= 0 ){
                                int nNL = road->lanes[nlidx]->nextLanes.size();
                                for(int k=0;k<nNL;++k){
                                    int nextLane = road->lanes[nlidx]->nextLanes[k];
                                    if( sMoveLane.contains( nextLane ) == false ){
                                        sMoveLane.append( nextLane );
                                    }
                                }
                            }
                        }
                    }
                }

                for(int i=0;i<sMoveLane.size();++i){
                    road->MoveLaneEdge( sMoveLane[i], xMove, yMove, 0 );  // Lane Start Point
                }
                for(int i=0;i<eMoveLane.size();++i){
                    road->MoveLaneEdge( eMoveLane[i], xMove, yMove, 1 );  // Lane End Point
                }
            }

            // Move Lane Edge END
            {
                QList<int> sMoveLane;
                QList<int> eMoveLane;
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE_EDGE_END && eMoveLane.contains(selectedObj.selObjID[i]) == false ){
                        eMoveLane.append( selectedObj.selObjID[i] );
                    }
                }

                for(int i=0;i<eMoveLane.size();++i){
                    int lidx = road->indexOfLane( eMoveLane[i] );
                    if( lidx >= 0 ){
                        int nNL = road->lanes[lidx]->nextLanes.size();
                        for(int j=0;j<nNL;++j){
                            int nextLane = road->lanes[lidx]->nextLanes[j];
                            if( sMoveLane.contains(nextLane) == false ){
                                sMoveLane.append( nextLane );
                            }
                            int nlidx = road->indexOfLane( nextLane );
                            if( nlidx >= 0 ){
                                int nPL = road->lanes[nlidx]->previousLanes.size();
                                for(int k=0;k<nPL;++k){
                                    int prevLane = road->lanes[nlidx]->previousLanes[k];
                                    if( eMoveLane.contains( prevLane ) == false ){
                                        eMoveLane.append( prevLane );
                                    }
                                }
                            }
                        }
                    }
                }

                for(int i=0;i<sMoveLane.size();++i){
                    road->MoveLaneEdge( sMoveLane[i], xMove, yMove, 0 );  // Lane Start Point
                }
                for(int i=0;i<eMoveLane.size();++i){
                    road->MoveLaneEdge( eMoveLane[i], xMove, yMove, 1 );  // Lane End Point
                }
            }

            // Move Traffic Signal
            {
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_TRAFFIC_SIGNAL ){
                        road->MoveTrafficSignal( selectedObj.selObjID[i], xMove, yMove );
                    }
                }
            }

            // Move StopLine
            {
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_STOPLINE ){
                        road->MoveStopLine( selectedObj.selObjID[i], xMove, yMove );
                    }
                }
            }

            // Move PedestLane Point
            {
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_PEDEST_LANE ){
                        road->MovePedestLane( selectedObj.selObjID[i], xMove, yMove );
                    }
                }
            }

            // Move PedestLane Point
            {
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_PEDEST_LANE_POINT ){

                        int modID = selectedObj.selObjID[i];
                        int laneID = modID / 100;
                        int sectIdx = modID - laneID * 100;
                        road->MovePedestLanePoint( laneID, sectIdx, xMove, yMove );
                    }
                }
            }


            objectMoveFlag = true;
        }
        // Normal Map Operation
        else {
            QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;

            float s = 0.04 * Z_eye / (-50.0);
            if( e->modifiers() & Qt::ControlModifier ){
                s *= 4.0;
            }

            float xMove = diff.x() * s;
            float yMove = diff.y() * (-s);

            X_eye += xMove;
            Y_eye += yMove;
        }

        QOpenGLWidget::update();
    }


    mousePressPosition = QVector2D(e->localPos());

    //qDebug() << "Eye: (" << X_eye << "," << Y_eye << "," << Z_eye << ") ptich=" << cameraPitch << " yaw=" << cameraYaw;
}


void GraphicCanvas::wheelEvent(QWheelEvent *e)
{
    float s = 1.0;
    if( e->modifiers() & Qt::ControlModifier ){
        s *= 2.2;
    }
    if( e->delta() > 0.0 ){
        if( Z_eye < -0.2 ){
            Z_eye /= (1.05 * s);
        }
    }
    else if( e->delta() < 0.0 ){
        Z_eye *= 1.05 * s;
    }

    if( isOrthogonal == true ){
        float nearDist = 0.1;
        float farDist = 10000.0;
        float t = Z_eye * tan(45.0f / 2.0f * 0.017452f) * (-1.0);
        float r = t * aspectRatio;
        projection.setToIdentity();
        projection.ortho( -r, r, -t, t, nearDist, farDist );
    }

    QOpenGLWidget::update();

    //qDebug() << "Eye: (" << X_eye << "," << Y_eye << "," << Z_eye << ") ptich=" << cameraPitch << " yaw=" << cameraYaw;
}


int GraphicCanvas::Get3DPhysCoordFromPickPoint(int xp,int yp, float &x,float &y)
{
    //
    // View coordinate -1 < sx,sy < 1
    float lsx = 2.0 / currentWidth * xp - 1.0;
    float lsy = 1.0 - 2.0 / currentHeight * yp;

    float Kx = -lsx * aspectRatio * tan( fieldOfView * 0.5 * 0.017452 );
    float Ky = -lsy * tan( fieldOfView * 0.5 * 0.017452 );

    float qx, qy, qz, qw;
    cameraQuat.getAxisAndAngle( &qx, &qy, &qz, &qw );

    float cqw = cos( qw * 0.017452 );
    float sqw = sin( qw * 0.017452 );

    //
    // Rodrigues' rotation formula
    float q11 = qx * qx * (1.0 - cqw) + cqw;
    float q12 = qy * qx * (1.0 - cqw) - qz * sqw;
    float q21 = qx * qy * (1.0 - cqw) + qz * sqw;
    float q22 = qy * qy * (1.0 - cqw) + cqw;
    float q31 = qx * qz * (1.0 - cqw) - qy * sqw;
    float q32 = qy * qz * (1.0 - cqw) + qx * sqw;

    //
    // By assuming z = 0, world coordinate (x, y, 0) can be calculated by
    // solving following Simultaneous equations:
    //      A11 * x + A12 * y = B11
    //      A21 * x + A22 * y = B21
    float A11 = q11 - Kx * q31;
    float A12 = q12 - Kx * q32;
    float A21 = q21 - Ky * q31;
    float A22 = q22 - Ky * q32;
    float B11 = Kx * Z_eye - X_eye;
    float B21 = Ky * Z_eye - Y_eye;

    float det = A11 * A22 - A12 * A21;
    if( fabs(det) > 1.0e-5 ){

        x = (A22 * B11 - A12 * B21) / det;
        y = (A11 * B21 - A21 * B11) / det;

        return 1;
    }
    else{
        x = 0.0;
        y = 0.0;

        return -1;
    }
}


void GraphicCanvas::SelectObject(bool shiftModifier)
{
    float Dmin;
    int selID = -1;
    int selID2 = -1;
    int kind = -1;

    if( nodePickModeFlag == true ){
        for(int i=0;i<road->nodes.size();++i){
            float dx = road->nodes[i]->pos.x() - wxMousePress;
            float dy = road->nodes[i]->pos.y() - wyMousePress;
            float D = dx * dx + dy * dy;
            if( D > 25.0 ){
                continue;
            }
            if( selID < 0 || Dmin > D ){
                selID = road->nodes[i]->id;
                Dmin = D;
                kind = _SEL_OBJ::SEL_NODE_ROUTE_PICK;
            }
        }

        for(int i=1;i<selectedObj.selObjID.size()-1;i++){
            if( selectedObj.selObjID[i] == selID ){
                selectedObj.selObjID.removeAt(i);
                selectedObj.selObjKind.removeAt(i);
                return;
            }
        }

        if( selID == selectedObj.selObjID.last() ){
            emit SetNodeListForRoute( selectedObj.selObjID );
            return;
        }

        selectedObj.selObjKind.append( kind );
        selectedObj.selObjID.insert( selectedObj.selObjID.size() - 1, selID );
        update();
    }
    else{
        if( selectNodeFlag == true ){
            for(int i=0;i<road->nodes.size();++i){
                float dx = road->nodes[i]->pos.x() - wxMousePress;
                float dy = road->nodes[i]->pos.y() - wyMousePress;
                float D = dx * dx + dy * dy;
                if( D > 25.0 ){
                    continue;
                }
                if( selID < 0 || Dmin > D ){
                    selID = road->nodes[i]->id;
                    Dmin = D;
                    kind = _SEL_OBJ::SEL_NODE;
                }
            }
        }

        if( selectLaneFlag == true ){
            int nearLaneID = road->GetNearestLane( QVector2D(wxMouseMove, wyMousePress) );
            if( nearLaneID >= 0 ){
                float dist = 0.0;
                int isEdge = -1;
                road->GetDistanceLaneFromPoint( nearLaneID, QVector2D(wxMouseMove, wyMousePress), dist, isEdge );
                qDebug() << "isEdge = " << isEdge;

                if( selID < 0 || Dmin > dist ){
                    selID = nearLaneID;
                    Dmin = dist;
                    if( isEdge == -1 ){
                        kind = _SEL_OBJ::SEL_LANE;
                    }
                    else if( isEdge == 0 ){
                        kind = _SEL_OBJ::SEL_LANE_EDGE_START;
                    }
                    else if( isEdge == 1 ){
                        kind = _SEL_OBJ::SEL_LANE_EDGE_END;
                    }
                }
            }
        }

        if( selectTrafficSignalFlag == true ){
            float dist = 0.0;
            int nearTSID = road->GetNearestTrafficSignal( QVector2D(wxMouseMove, wyMousePress), dist );
            if( nearTSID >= 0 ){

                if( selID < 0 || Dmin > dist ){
                    selID = nearTSID;
                    Dmin = dist;
                    kind = _SEL_OBJ::SEL_TRAFFIC_SIGNAL;
                }
            }
        }

        if( selectStopLineFlag == true ){
            float dist = 0.0;
            int nearTSID = road->GetNearestStopLine( QVector2D(wxMouseMove, wyMousePress), dist );
            if( nearTSID >= 0 ){

                if( selID < 0 || Dmin > dist ){
                    selID = nearTSID;
                    Dmin = dist;
                    kind = _SEL_OBJ::SEL_STOPLINE;
                }
            }
        }

        if( selectPedestLaneFlag == true ){
            float dist = 0.0;
            int nearPedestLaneID = -1;
            int nearPedestLanePointIndex = -1;

            road->GetNearestPedestLanePoint( QVector2D(wxMouseMove, wyMousePress), dist, nearPedestLaneID, nearPedestLanePointIndex );
            qDebug() << "nearPedestLaneID = " << nearPedestLaneID << " nearPedestLanePointIndex = " << nearPedestLanePointIndex;

            if( nearPedestLaneID >= 0 ){

                if( selID < 0 || Dmin > dist ){

                    selID = nearPedestLaneID;
                    Dmin = dist;
                    kind = _SEL_OBJ::SEL_PEDEST_LANE;

                    if( nearPedestLanePointIndex >= 0 ){
                        selID2 = nearPedestLanePointIndex;
                        kind = _SEL_OBJ::SEL_PEDEST_LANE_POINT;
                    }
                }
            }
        }

        if( selID >= 0 ){
            if( kind == _SEL_OBJ::SEL_NODE ){
                bool alreadySelected = false;
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_NODE ){
                        if( selectedObj.selObjID[i] == selID ){
                            alreadySelected = true;
                            if( shiftModifier == true ){
                                selectedObj.selObjKind.removeAt(i);
                                selectedObj.selObjID.removeAt(i);
                            }
                            break;
                        }
                    }
                }
                if( alreadySelected == false ){
                    if( shiftModifier == false ){
                        selectedObj.selObjKind.clear();
                        selectedObj.selObjID.clear();
                    }
                    selectedObj.selObjKind.append( _SEL_OBJ::SEL_NODE );
                    selectedObj.selObjID.append( selID );
                    qDebug() << "Node Selected: ID = " << selID;
                }
                else{

                    // If selected node has terminal nodes, add them
                    int nidx = road->indexOfNode( selID );
                    if( nidx >= 0 ){
                        for(int i=0;i<road->nodes[nidx]->legInfo.size();++i){
                            int cn = road->nodes[nidx]->legInfo[i]->connectingNode;
                            if( road->GetNodeNumLeg(cn) == 1 ){
                                alreadySelected = false;
                                for(int j=0;j<selectedObj.selObjKind.size();++j){
                                    if( selectedObj.selObjKind[j] == _SEL_OBJ::SEL_NODE ){
                                        if( selectedObj.selObjID[j] == cn ){
                                            alreadySelected = true;
                                            break;
                                        }
                                    }
                                }
                                if( alreadySelected == false ){
                                    selectedObj.selObjKind.append( _SEL_OBJ::SEL_NODE );
                                    selectedObj.selObjID.append( cn );
                                    qDebug() << "Add Terminal Node: ID = " << cn;
                                }
                            }

                            int dn = road->nodes[nidx]->legInfo[i]->connectedNode;
                            if( road->GetNodeNumLeg(dn) == 1 ){
                                alreadySelected = false;
                                for(int j=0;j<selectedObj.selObjKind.size();++j){
                                    if( selectedObj.selObjKind[j] == _SEL_OBJ::SEL_NODE ){
                                        if( selectedObj.selObjID[j] == dn ){
                                            alreadySelected = true;
                                            break;
                                        }
                                    }
                                }
                                if( alreadySelected == false ){
                                    selectedObj.selObjKind.append( _SEL_OBJ::SEL_NODE );
                                    selectedObj.selObjID.append( dn );
                                    qDebug() << "Add Terminal Node: ID = " << dn;
                                }
                            }
                        }
                    }
                }

                QString propStr = road->GetNodeProperty( selID );
                QStringList divPropStr = propStr.split("\n");
                for(int i=0;i<divPropStr.size();++i){
                    qDebug() << QString( divPropStr[i] );
                }
            }
            else if( kind == _SEL_OBJ::SEL_LANE ){
                bool alreadySelected = false;
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE ){
                        if( selectedObj.selObjID[i] == selID ){
                            alreadySelected = true;
                            if( shiftModifier == true ){
                                selectedObj.selObjKind.removeAt(i);
                                selectedObj.selObjID.removeAt(i);
                            }
                            break;
                        }
                    }
                }
                if( alreadySelected == false ){
                    if( shiftModifier == false ){
                        selectedObj.selObjKind.clear();
                        selectedObj.selObjID.clear();
                    }
                    selectedObj.selObjKind.append( _SEL_OBJ::SEL_LANE );
                    selectedObj.selObjID.append( selID );
                    qDebug() << "Lane Selected: ID = " << selID;
                }

                QString propStr = road->GetLaneProperty( selID );
                QStringList divPropStr = propStr.split("\n");
                for(int i=0;i<divPropStr.size();++i){
                    qDebug() << QString( divPropStr[i] );
                }
            }
            else if( kind == _SEL_OBJ::SEL_LANE_EDGE_START ){
                bool alreadySelected = false;
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE_EDGE_START ){
                        if( selectedObj.selObjID[i] == selID ){
                            alreadySelected = true;
                            if( shiftModifier == true ){
                                selectedObj.selObjKind.removeAt(i);
                                selectedObj.selObjID.removeAt(i);
                            }
                            break;
                        }
                    }
                }
                if( alreadySelected == false ){
                    if( shiftModifier == false ){
                        selectedObj.selObjKind.clear();
                        selectedObj.selObjID.clear();
                    }
                    selectedObj.selObjKind.append( _SEL_OBJ::SEL_LANE_EDGE_START );
                    selectedObj.selObjID.append( selID );
                    qDebug() << "Lane Edge Selected: ID = " << selID;
                }
            }
            else if( kind == _SEL_OBJ::SEL_LANE_EDGE_END ){
                bool alreadySelected = false;
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_LANE_EDGE_END ){
                        if( selectedObj.selObjID[i] == selID ){
                            alreadySelected = true;
                            if( shiftModifier == true ){
                                selectedObj.selObjKind.removeAt(i);
                                selectedObj.selObjID.removeAt(i);
                            }
                            break;
                        }
                    }
                }
                if( alreadySelected == false ){
                    if( shiftModifier == false ){
                        selectedObj.selObjKind.clear();
                        selectedObj.selObjID.clear();
                    }
                    selectedObj.selObjKind.append( _SEL_OBJ::SEL_LANE_EDGE_END );
                    selectedObj.selObjID.append( selID );
                    qDebug() << "Lane Edge Selected: ID = " << selID;
                }
            }
            else if( kind == _SEL_OBJ::SEL_TRAFFIC_SIGNAL ){
                bool alreadySelected = false;
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_TRAFFIC_SIGNAL ){
                        if( selectedObj.selObjID[i] == selID ){
                            alreadySelected = true;
                            if( shiftModifier == true ){
                                selectedObj.selObjKind.removeAt(i);
                                selectedObj.selObjID.removeAt(i);
                            }
                            break;
                        }
                    }
                }
                if( alreadySelected == false ){
                    if( shiftModifier == false ){
                        selectedObj.selObjKind.clear();
                        selectedObj.selObjID.clear();
                    }
                    selectedObj.selObjKind.append( _SEL_OBJ::SEL_TRAFFIC_SIGNAL );
                    selectedObj.selObjID.append( selID );
                    qDebug() << "TS Selected: ID = " << selID;
                }
            }
            else if( kind == _SEL_OBJ::SEL_STOPLINE ){
                bool alreadySelected = false;
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_STOPLINE ){
                        if( selectedObj.selObjID[i] == selID ){
                            alreadySelected = true;
                            if( shiftModifier == true ){
                                selectedObj.selObjKind.removeAt(i);
                                selectedObj.selObjID.removeAt(i);
                            }
                            break;
                        }
                    }
                }
                if( alreadySelected == false ){
                    if( shiftModifier == false ){
                        selectedObj.selObjKind.clear();
                        selectedObj.selObjID.clear();
                    }
                    selectedObj.selObjKind.append( _SEL_OBJ::SEL_STOPLINE );
                    selectedObj.selObjID.append( selID );
                    qDebug() << "SL Selected: ID = " << selID;
                }
            }
            else if( kind == _SEL_OBJ::SEL_PEDEST_LANE ){
                bool alreadySelected = false;
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_PEDEST_LANE ){
                        if( selectedObj.selObjID[i] == selID ){
                            alreadySelected = true;
                            if( shiftModifier == true ){
                                selectedObj.selObjKind.removeAt(i);
                                selectedObj.selObjID.removeAt(i);
                            }
                            break;
                        }
                    }
                }
                if( alreadySelected == false ){
                    if( shiftModifier == false ){
                        selectedObj.selObjKind.clear();
                        selectedObj.selObjID.clear();
                    }
                    selectedObj.selObjKind.append( _SEL_OBJ::SEL_PEDEST_LANE );
                    selectedObj.selObjID.append( selID );
                    qDebug() << "Pedest Lane Selected: ID = " << selID;
                }
            }
            else if( kind == _SEL_OBJ::SEL_PEDEST_LANE_POINT ){

                int modId = selID * 100 + selID2;

                bool alreadySelected = false;
                for(int i=0;i<selectedObj.selObjKind.size();++i){
                    if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_PEDEST_LANE_POINT ){
                        if( selectedObj.selObjID[i] == modId ){
                            alreadySelected = true;
                            if( shiftModifier == true ){
                                selectedObj.selObjKind.removeAt(i);
                                selectedObj.selObjID.removeAt(i);
                            }
                            break;
                        }
                    }
                }
                if( alreadySelected == false ){
                    if( shiftModifier == false ){
                        selectedObj.selObjKind.clear();
                        selectedObj.selObjID.clear();
                    }
                    selectedObj.selObjKind.append( _SEL_OBJ::SEL_PEDEST_LANE_POINT );
                    selectedObj.selObjID.append( modId );
                    qDebug() << "Pedest Lane Point Selected: ID = " << modId;
                }
            }


            if( selectedObj.selObjKind.size() == 1 ){
                if( selectedObj.selObjKind[0] == _SEL_OBJ::SEL_NODE ){
                    if( roadProperty ){
                        roadProperty->ChangeTabPage(0);
                        roadProperty->nodeIDSB->setValue( selectedObj.selObjID[0] );
                    }
                }
                else if( selectedObj.selObjKind[0] == _SEL_OBJ::SEL_LANE ){
                    if( roadProperty ){
                        roadProperty->ChangeTabPage(1);
                        roadProperty->laneIDSB->setValue( selectedObj.selObjID[0] );
                    }
                }
                else if( selectedObj.selObjKind[0] == _SEL_OBJ::SEL_TRAFFIC_SIGNAL ){
                    if( roadProperty ){
                        roadProperty->ChangeTabPage(2);
                        if( roadProperty->tsIDSB->value() == selectedObj.selObjID[0] ){
                            roadProperty->ChangeTrafficSignalInfo(selectedObj.selObjID[0]);
                        }
                        roadProperty->tsIDSB->setValue( selectedObj.selObjID[0] );
                    }
                }
                else if( selectedObj.selObjKind[0] == _SEL_OBJ::SEL_STOPLINE ){
                    if( roadProperty ){
                        roadProperty->ChangeTabPage(3);
                        if( roadProperty->slIDSB->value() == selectedObj.selObjID[0] ){
                            roadProperty->ChangeStopLineInfo(selectedObj.selObjID[0]);
                        }
                        roadProperty->slIDSB->setValue( selectedObj.selObjID[0] );
                    }
                }
                else if( selectedObj.selObjKind[0] == _SEL_OBJ::SEL_PEDEST_LANE ){
                    if( roadProperty ){
                        roadProperty->ChangeTabPage(4);
                        qDebug() << "Change Tab Page 4";

                        if( roadProperty->pedestLaneIDSB->value() == selectedObj.selObjID[0] ){
                            roadProperty->ChangePedestLaneInfo(selectedObj.selObjID[0], -1);
                        }
                        roadProperty->pedestLaneIDSB->setValue( selectedObj.selObjID[0] );
                        roadProperty->pedestLaneSectionSB->setValue(0);
                    }
                }
                else if( selectedObj.selObjKind[0] == _SEL_OBJ::SEL_PEDEST_LANE_POINT ){
                    if( roadProperty ){
                        roadProperty->ChangeTabPage(4);
                        qDebug() << "Change Tab Page 4";

                        int modId = selectedObj.selObjID[0];
                        int pedestLaneId = modId / 100;
                        int pedestLanePointIndex = modId - pedestLaneId * 100;

                        if( roadProperty->pedestLaneIDSB->value() == pedestLaneId ){
                            roadProperty->ChangePedestLaneInfo(pedestLaneId, pedestLanePointIndex);
                        }
                        roadProperty->pedestLaneIDSB->setValue( pedestLaneId );
                        roadProperty->pedestLaneSectionSB->setValue( pedestLanePointIndex );
                    }
                }
            }
        }
    }
}

