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


#include "datamanipulator.h"
#include <QInputDialog>


DataManipulator::DataManipulator(QObject *parent) : QObject(parent)
{
    canvas = NULL;

    insertMode = 0;
    insertNode1 = -1;
    insertNode2 = -1;
}


void DataManipulator::UndoOperation()
{
    if( canvas->undoInfo.setUndoInfo == false ||
            canvas->undoInfo.selObjKind.size() == 0 ){
        qDebug() << "No undo info.";
        return;
    }

    if( canvas->undoInfo.operationType == canvas->MOVE_OBJECT ){

        for(int i=0;i<canvas->undoInfo.selObjKind.size();++i){
            qDebug() << "[" << i << "] kind = " << canvas->undoInfo.selObjKind[i]
                        << " id = " << canvas->undoInfo.selObjID[i];
        }


        // Node
        {
            float xMove = 0.0;
            float yMove = 0.0;

            int dIdx = 0;
            for(int i=0;i<canvas->undoInfo.selObjKind.size();++i){
                if( canvas->undoInfo.selObjKind[i] == canvas->SEL_NODE ){

                    int nIdx = road->indexOfNode( canvas->undoInfo.selObjID[i] );
                    xMove = canvas->undoInfo.data[dIdx  ] - road->nodes[nIdx]->pos.x();
                    yMove = canvas->undoInfo.data[dIdx+1] - road->nodes[nIdx]->pos.y();

                    road->SetNodePosition( canvas->undoInfo.selObjID[i],
                                           canvas->undoInfo.data[dIdx],
                                           canvas->undoInfo.data[dIdx+1] );
                    dIdx += 2;
                    qDebug() << "Undo move node " << canvas->undoInfo.selObjID[i];
                }
                else if( canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE ||
                         canvas->undoInfo.selObjKind[i] == canvas->SEL_STOPLINE ){
                    dIdx += 4;
                }
                else if(canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_START ||
                        canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_END ||
                        canvas->undoInfo.selObjKind[i] == canvas->SEL_TRAFFIC_SIGNAL ){
                    dIdx += 2;
                }
            }
        }

        // Lane
        {
            QList<int> directMoveLane;
            QList<int> sMoveLane;
            QList<int> eMoveLane;

            bool setReverseMove = false;
            float xMove = 0.0;
            float yMove = 0.0;

            int dIdx = 0;
            for(int i=0;i<canvas->undoInfo.selObjKind.size();++i){
                if( canvas->undoInfo.selObjKind[i] == canvas->SEL_NODE ||
                        canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_START ||
                        canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_END ||
                        canvas->undoInfo.selObjKind[i] == canvas->SEL_TRAFFIC_SIGNAL ){
                    dIdx += 2;
                }
                else if( canvas->undoInfo.selObjKind[i] == canvas->SEL_STOPLINE ){
                    dIdx += 4;
                }
                else if( canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE ){
                    if( directMoveLane.contains(canvas->undoInfo.selObjID[i]) == false ){
                        directMoveLane.append( canvas->undoInfo.selObjID[i] );
                    }
                    if( setReverseMove == false ){
                        int lidx = road->indexOfLane( canvas->undoInfo.selObjID[i] );
                        if( lidx >= 0 ){
                            float dx1 = canvas->undoInfo.data[dIdx] - road->lanes[lidx]->shape.pos.first()->x();
                            float dy1 = canvas->undoInfo.data[dIdx+1] - road->lanes[lidx]->shape.pos.first()->y();
                            float dx2 = canvas->undoInfo.data[dIdx+2] - road->lanes[lidx]->shape.pos.last()->x();
                            float dy2 = canvas->undoInfo.data[dIdx+3] - road->lanes[lidx]->shape.pos.last()->y();

                            if( fabs(dx1) > fabs(dx2) ){
                                xMove = dx1;
                            }
                            else{
                                xMove = dx2;
                            }

                            if( fabs(dy1) > fabs(dy2) ){
                                yMove = dy1;
                            }
                            else{
                                yMove = dy2;
                            }

                            setReverseMove = true;
                        }
                    }
                    dIdx += 4;
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

            if( setReverseMove == true ){
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
        }

        // Lane Edge START
        {
            QList<int> sMoveLane;
            QList<int> eMoveLane;

            bool setReverseMove = false;
            float xMove = 0.0;
            float yMove = 0.0;

            int dIdx = 0;
            for(int i=0;i<canvas->undoInfo.selObjKind.size();++i){
                if( canvas->undoInfo.selObjKind[i] == canvas->SEL_NODE ||
                    canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_END ||
                    canvas->undoInfo.selObjKind[i] == canvas->SEL_TRAFFIC_SIGNAL ){
                    dIdx += 2;
                }
                else if( canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE ||
                         canvas->undoInfo.selObjKind[i] == canvas->SEL_STOPLINE ){
                    dIdx += 4;
                }
                else if( canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_START ){
                    if( sMoveLane.contains(canvas->undoInfo.selObjID[i]) == false ){
                        sMoveLane.append( canvas->undoInfo.selObjID[i] );
                    }
                    if( setReverseMove == false ){
                        int lidx = road->indexOfLane( canvas->undoInfo.selObjID[i] );
                        if( lidx >= 0 ){
                            xMove = canvas->undoInfo.data[dIdx] - road->lanes[lidx]->shape.pos.first()->x();
                            yMove = canvas->undoInfo.data[dIdx+1] - road->lanes[lidx]->shape.pos.first()->y();
                            setReverseMove = true;
                        }
                    }
                    dIdx += 2;
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

        // Lane Edge END
        {
            QList<int> sMoveLane;
            QList<int> eMoveLane;

            bool setReverseMove = false;
            float xMove = 0.0;
            float yMove = 0.0;

            int dIdx = 0;
            for(int i=0;i<canvas->undoInfo.selObjKind.size();++i){
                if( canvas->undoInfo.selObjKind[i] == canvas->SEL_NODE ||
                    canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_START ||
                    canvas->undoInfo.selObjKind[i] == canvas->SEL_TRAFFIC_SIGNAL ){
                    dIdx += 2;
                }
                else if( canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE ||
                         canvas->undoInfo.selObjKind[i] == canvas->SEL_STOPLINE ){
                    dIdx += 4;
                }
                else if( canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_END ){
                    if( eMoveLane.contains(canvas->undoInfo.selObjID[i]) == false ){
                        eMoveLane.append( canvas->undoInfo.selObjID[i] );
                    }
                    if( setReverseMove == false ){
                        int lidx = road->indexOfLane( canvas->undoInfo.selObjID[i] );
                        if( lidx >= 0 ){
                            xMove = canvas->undoInfo.data[dIdx] - road->lanes[lidx]->shape.pos.last()->x();
                            yMove = canvas->undoInfo.data[dIdx+1] - road->lanes[lidx]->shape.pos.last()->y();
                            setReverseMove = true;
                        }
                    }
                    dIdx += 2;
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

        // Traffic Signal
        {
            float xMove = 0.0;
            float yMove = 0.0;

            int dIdx = 0;
            for(int i=0;i<canvas->undoInfo.selObjKind.size();++i){
                if( canvas->undoInfo.selObjKind[i] == canvas->SEL_TRAFFIC_SIGNAL ){

                    int rNd = road->indexOfTS( canvas->undoInfo.selObjID[i], -1 );
                    int nIdx = road->indexOfNode( rNd );
                    if( nIdx >= 0 ){
                        int tsIdx = road->indexOfTS( canvas->undoInfo.selObjID[i], rNd );

                        xMove = canvas->undoInfo.data[dIdx  ] - road->nodes[nIdx]->trafficSignals[tsIdx]->pos.x();
                        yMove = canvas->undoInfo.data[dIdx+1] - road->nodes[nIdx]->trafficSignals[tsIdx]->pos.y();

                        road->MoveTrafficSignal( canvas->undoInfo.selObjID[i], xMove, yMove );
                    }
                    dIdx += 2;
                    qDebug() << "Undo move traffic signal " << canvas->undoInfo.selObjID[i];
                }
                else if( canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE ||
                         canvas->undoInfo.selObjKind[i] == canvas->SEL_STOPLINE ){
                    dIdx += 4;
                }
                else if(canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_START ||
                        canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_END ||
                        canvas->undoInfo.selObjKind[i] == canvas->SEL_NODE ){
                    dIdx += 2;
                }
            }
        }

        // Stop Line
        {
            float xMove = 0.0;
            float yMove = 0.0;

            int dIdx = 0;
            for(int i=0;i<canvas->undoInfo.selObjKind.size();++i){
                if( canvas->undoInfo.selObjKind[i] == canvas->SEL_STOPLINE ){

                    int rNd = road->indexOfSL( canvas->undoInfo.selObjID[i], -1 );
                    int nIdx = road->indexOfNode( rNd );
                    if( nIdx >= 0 ){
                        int slIdx = road->indexOfSL( canvas->undoInfo.selObjID[i], rNd );

                        xMove = canvas->undoInfo.data[dIdx  ] - road->nodes[nIdx]->stopLines[slIdx]->leftEdge.x();
                        yMove = canvas->undoInfo.data[dIdx+1] - road->nodes[nIdx]->stopLines[slIdx]->leftEdge.y();

                        road->MoveStopLine( canvas->undoInfo.selObjID[i], xMove, yMove );
                    }
                    dIdx += 4;
                    qDebug() << "Undo move stop line " << canvas->undoInfo.selObjID[i];
                }
                else if( canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE ){
                    dIdx += 4;
                }
                else if(canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_START ||
                        canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_END ||
                        canvas->undoInfo.selObjKind[i] == canvas->SEL_NODE ||
                        canvas->undoInfo.selObjKind[i] == canvas->SEL_TRAFFIC_SIGNAL ){
                    dIdx += 2;
                }
            }
        }


        if( road->updateCPEveryOperation == true ){
            road->CheckLaneCrossPoints();
        }


        for(int i=0;i<canvas->undoInfo.selObjKind.size();++i){
            if( canvas->undoInfo.selObjKind[i] == canvas->SEL_NODE ){
                int nIdx = road->indexOfNode( canvas->undoInfo.selObjID[i] );
                for(int i=0;i<road->nodes[nIdx]->stopLines.size();++i){
                    road->CheckStopLineCrossLanes( road->nodes[nIdx]->stopLines[i]->id );
                }
            }
        }

        for(int i=0;i<canvas->undoInfo.selObjKind.size();++i){
            if( canvas->undoInfo.selObjKind[i] == canvas->SEL_STOPLINE ){
                road->CheckStopLineCrossLanes( canvas->undoInfo.selObjID[i] );
            }
        }

    }
    else if( canvas->undoInfo.operationType == canvas->ROTATE_OBJECT ){
        float rotAngle = 0.0;
        bool isRotAngleSet = false;
        QVector2D pos_center;
        int didx = 0;
        for(int i=0;i<canvas->undoInfo.selObjKind.size();++i){
            if( canvas->undoInfo.selObjKind[i] == canvas->SEL_NODE ){
                int idx = road->indexOfNode( canvas->undoInfo.selObjID[i] );
                if( idx >= 0 ){
                    int nLeg = road->nodes[idx]->nLeg;
                    for(int j=0;j<nLeg;++j){

                        float angle = canvas->undoInfo.data[didx++];
                        float diffAngle = angle - road->nodes[idx]->legInfo[j]->angle;

                        if( i == 0 && isRotAngleSet == false ){

                            pos_center = road->GetNodePosition( canvas->undoInfo.selObjID[0] );

                            if( fabs( diffAngle ) > 0.5 ){
                                rotAngle = diffAngle;
                                isRotAngleSet = true;
                                qDebug() << "rotAngle = " << rotAngle << " by Leg = " << j;
                            }
                        }
                        else if( i > 0 ){
                            diffAngle = rotAngle;
                        }
                        road->RotateNodeLeg( canvas->undoInfo.selObjID[i],
                                             j,
                                             diffAngle );
                    }

                    if( i > 0 ){
                        QVector2D pos = road->GetNodePosition( canvas->undoInfo.selObjID[i] );
                        float dx = pos.x() - pos_center.x();
                        float dy = pos.y() - pos_center.y();

                        float cp = cos( rotAngle * 0.017452 );
                        float sp = sin( rotAngle * 0.017452 );

                        float rx = dx * cp + dy * (-sp);
                        float ry = dx * sp + dy * cp;

                        float xMove = rx - dx;
                        float yMove = ry - dy;
                        road->MoveNode( canvas->undoInfo.selObjID[i], xMove, yMove );
                    }
                }
            }
            else if( canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_START ){
                int lidx = road->indexOfLane( canvas->undoInfo.selObjID[i] );
                if( lidx >= 0 ){
                    if( i == 0 && isRotAngleSet == false ){

                        pos_center.setX( road->lanes[lidx]->shape.pos.first()->x() );
                        pos_center.setY( road->lanes[lidx]->shape.pos.first()->y() );

                        float dx = canvas->undoInfo.data[didx++];
                        float dy = canvas->undoInfo.data[didx++];
                        float angle_orig = atan2( dy, dx ) * 57.3;

                        dx = road->lanes[lidx]->shape.derivative.first()->x();
                        dy = road->lanes[lidx]->shape.derivative.first()->y();
                        float angle_curr = atan2( dy, dx ) * 57.3;

                        rotAngle = angle_orig - angle_curr;
                    }

                    QList<int> sMoveLane;
                    QList<int> eMoveLane;
                    sMoveLane.append( canvas->undoInfo.selObjID[i] );

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

                    for(int j=0;j<sMoveLane.size();++j){
                        road->RotateLaneEdge( sMoveLane[j], rotAngle, 0 );  // Lane Start Point
                    }
                    for(int j=0;j<eMoveLane.size();++j){
                        road->RotateLaneEdge( eMoveLane[j], rotAngle, 1 );  // Lane End Point
                    }

                    if(i > 0 ){

                        int lidx = road->indexOfLane( canvas->undoInfo.selObjID[i] );
                        if( lidx >= 0 ){

                            float dx = road->lanes[lidx]->shape.pos.first()->x() - pos_center.x();
                            float dy = road->lanes[lidx]->shape.pos.first()->y() - pos_center.y();

                            float cp = cos( rotAngle * 0.017452 );
                            float sp = sin( rotAngle * 0.017452 );

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
            else if( canvas->undoInfo.selObjKind[i] == canvas->SEL_LANE_EDGE_END ){
                int lidx = road->indexOfLane( canvas->undoInfo.selObjID[i] );
                if( lidx >= 0 ){
                    if( i == 0 && isRotAngleSet == false ){

                        pos_center.setX( road->lanes[lidx]->shape.pos.last()->x() );
                        pos_center.setY( road->lanes[lidx]->shape.pos.last()->y() );

                        float dx = canvas->undoInfo.data[didx++];
                        float dy = canvas->undoInfo.data[didx++];
                        float angle_orig = atan2( dy, dx ) * 57.3;

                        dx = road->lanes[lidx]->shape.derivative.last()->x();
                        dy = road->lanes[lidx]->shape.derivative.last()->y();
                        float angle_curr = atan2( dy, dx ) * 57.3;

                        rotAngle = angle_orig - angle_curr;
                    }

                    QList<int> sMoveLane;
                    QList<int> eMoveLane;
                    eMoveLane.append( canvas->undoInfo.selObjID[i] );

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

                    for(int j=0;j<sMoveLane.size();++j){
                        road->RotateLaneEdge( sMoveLane[j], rotAngle, 0 );  // Lane Start Point
                    }
                    for(int j=0;j<eMoveLane.size();++j){
                        road->RotateLaneEdge( eMoveLane[j], rotAngle, 1 );  // Lane End Point
                    }

                    if(i > 0 ){

                        int lidx = road->indexOfLane( canvas->undoInfo.selObjID[i] );
                        if( lidx >= 0 ){

                            float dx = road->lanes[lidx]->shape.pos.last()->x() - pos_center.x();
                            float dy = road->lanes[lidx]->shape.pos.last()->y() - pos_center.y();

                            float cp = cos( rotAngle * 0.017452 );
                            float sp = sin( rotAngle * 0.017452 );

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
        }


        if( road->updateCPEveryOperation == true ){
            road->CheckLaneCrossPoints();
        }


        for(int i=0;i<canvas->undoInfo.selObjKind.size();++i){
            if( canvas->undoInfo.selObjKind[i] == canvas->SEL_NODE ){
                int nIdx = road->indexOfNode( canvas->undoInfo.selObjID[i] );
                for(int i=0;i<road->nodes[nIdx]->stopLines.size();++i){
                    road->CheckStopLineCrossLanes( road->nodes[nIdx]->stopLines[i]->id );
                }
            }
        }
    }

    canvas->undoInfo.setUndoInfo = false;
    canvas->undoInfo.selObjKind.clear();
    canvas->undoInfo.selObjID.clear();
    canvas->undoInfo.data.clear();

    road->CheckLaneConnection();

    canvas->update();
}


void DataManipulator::SelectAllNode()
{
    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();

    for(int i=0;i<road->nodes.size();++i){
        canvas->selectedObj.selObjKind.append( canvas->SEL_NODE );
        canvas->selectedObj.selObjID.append( road->nodes[i]->id );
    }

    canvas->update();
}


void DataManipulator::DeleteSelectedObject()
{
    if( canvas->selectedObj.selObjKind.size() == 0 ){
        return;
    }

    for(int i=0;i<canvas->selectedObj.selObjKind.size();++i){

        if( canvas->selectedObj.selObjKind[i] == canvas->SEL_NODE ){

            QList<int> delTermNode;
            QList<int> modNode;

            int delNDID = canvas->selectedObj.selObjID[i];
            int idx = road->indexOfNode( delNDID );
            if( idx >= 0 ){

                for(int j=0;j<road->nodes[idx]->relatedLanes.size();++j){

                    int tLaneID = road->nodes[idx]->relatedLanes[j];
                    int lidx = road->indexOfLane( tLaneID );
                    if( lidx >= 0 ){

                        if( road->lanes[lidx]->connectedNode == delNDID && road->lanes[lidx]->departureNode == delNDID ){
                            // This is lane inside node
                            road->DeleteLane( tLaneID );
                        }
                        else{

                            if( road->lanes[lidx]->connectedNode != delNDID ){
                                int cnIdx = road->indexOfNode( road->lanes[lidx]->connectedNode );
                                if( cnIdx >= 0 ){
                                    if( road->nodes[cnIdx]->nLeg < 3 ){
                                        if( delTermNode.contains(road->lanes[lidx]->connectedNode) == false ){
                                            delTermNode.append(road->lanes[lidx]->connectedNode);
                                        }
                                    }
                                    else{
                                        if( modNode.contains(road->lanes[lidx]->connectedNode) == false ){
                                            modNode.append(road->lanes[lidx]->connectedNode);
                                        }
                                    }
                                    int rIdx = road->nodes[cnIdx]->relatedLanes.indexOf( tLaneID );
                                    if( rIdx >= 0 && rIdx < road->nodes[cnIdx]->relatedLanes.size() ){
                                        road->nodes[cnIdx]->relatedLanes.removeAt( rIdx );
                                    }
                                }
                            }
                            else if( road->lanes[lidx]->departureNode != delNDID ){
                                int cnIdx = road->indexOfNode( road->lanes[lidx]->departureNode );
                                if( cnIdx >= 0 ){
                                    if( road->nodes[cnIdx]->nLeg < 3 ){
                                        if( delTermNode.contains(road->lanes[lidx]->departureNode) == false ){
                                            delTermNode.append(road->lanes[lidx]->departureNode);
                                        }
                                    }
                                    else{
                                        if( modNode.contains(road->lanes[lidx]->connectedNode) == false ){
                                            modNode.append(road->lanes[lidx]->connectedNode);
                                        }
                                    }
                                    int rIdx = road->nodes[cnIdx]->relatedLanes.indexOf( tLaneID );
                                    if( rIdx >= 0 && rIdx < road->nodes[cnIdx]->relatedLanes.size() ){
                                        road->nodes[cnIdx]->relatedLanes.removeAt( rIdx );
                                    }
                                }
                            }

                            road->DeleteLane( tLaneID );
                        }
                    }
                }

                // Delete the node
                road->DeleteNode( delNDID );


                // Delete connected terminal node
                for(int j=0;j<delTermNode.size();++j){
                    road->DeleteNode( delTermNode[j] );
                }


                // Delete lanes and node-legs of connected node
                for(int j=modNode.size()-1;j>=0;--j){

                    int cnIdx = road->indexOfNode( modNode[j] );
                    if( cnIdx >= 0 ){

                        int connectedDir = -1;
                        for(int k=0;k<road->nodes[cnIdx]->nLeg;++k){
                            if( road->nodes[cnIdx]->legInfo[k]->connectingNode == delNDID ){
                                connectedDir = road->nodes[cnIdx]->legInfo[k]->legID;
                                break;
                            }
                        }

                        if( connectedDir >= 0 ){

                            for(int k=road->nodes[cnIdx]->relatedLanes.size()-1;k>=0;k--){

                                int tLaneID = road->nodes[cnIdx]->relatedLanes[k];
                                int lidx = road->indexOfLane( tLaneID );
                                if( lidx >= 0 ){
                                    if( road->lanes[lidx]->connectedNodeOutDirect == connectedDir ){
                                        road->DeleteLane( tLaneID );
                                        road->nodes[cnIdx]->relatedLanes.removeAt(k);
                                    }
                                    else if( road->lanes[lidx]->connectedNodeInDirect == connectedDir ){
                                        road->DeleteLane( tLaneID );
                                        road->nodes[cnIdx]->relatedLanes.removeAt(k);
                                    }
                                }
                            }

                            road->DeleteNodeLeg( modNode[j], connectedDir );

                            // If the number of the node leg becomes 2, delete the node
                            if( road->nodes[cnIdx]->nLeg == 2 ){

                                for(int k=0;k<2;++k){
                                    int cND1    = road->nodes[cnIdx]->legInfo[k]->connectedNode;
                                    int cND1out = road->nodes[cnIdx]->legInfo[k]->connectedNodeOutDirect;
                                    int cND2    = road->nodes[cnIdx]->legInfo[(k+1)%2]->connectingNode;
                                    int cND2in  = road->nodes[cnIdx]->legInfo[(k+1)%2]->connectingNodeInDirect;
                                    if( cND1 >= 0 && cND1out >= 0 && cND2 >= 0 && cND2in >= 0 ){

                                        int legID1 = road->GetNodeLegIDByConnectingNode( cND1, modNode[j] );
                                        int legID2 = road->GetNodeLegIDByConnectedNode( cND2, modNode[j] );

                                        road->SetNodeConnectInfo( cND1, legID1, cND2, QString("OutNode") );
                                        road->SetNodeConnectInfo( cND1, legID1, cND2in, QString("OutNodeInDirect") );
                                        road->SetNodeConnectInfo( cND2, legID2, cND1, QString("InNode") );
                                        road->SetNodeConnectInfo( cND2, legID2, cND1out, QString("InNodeOutDirect") );

                                        for(int l=road->nodes[cnIdx]->relatedLanes.size()-1;l>=0;l--){
                                            int tLaneID = road->nodes[cnIdx]->relatedLanes[l];
                                            road->SetNodeRelatedLane( cND1, tLaneID );
                                            road->SetNodeRelatedLane( cND2, tLaneID );

                                            int lidx = road->indexOfLane( tLaneID );
                                            if( lidx >= 0 ){
                                                if( road->lanes[lidx]->connectedNode == modNode[j] &&
                                                    road->lanes[lidx]->connectedNodeInDirect == road->nodes[cnIdx]->legInfo[k]->legID &&
                                                    road->lanes[lidx]->connectedNodeOutDirect == road->nodes[cnIdx]->legInfo[(k+1)%2]->legID ){

                                                    road->lanes[lidx]->connectedNodeOutDirect = -1;
                                                    road->lanes[lidx]->connectedNode = cND2;
                                                    road->lanes[lidx]->connectedNodeInDirect = cND2in;
                                                    road->lanes[lidx]->departureNode = cND1;
                                                    road->lanes[lidx]->departureNodeOutDirect = cND1out;

                                                    road->lanes[lidx]->sWPInNode  = -1;
                                                    road->lanes[lidx]->sWPNodeDir = -1;
                                                    road->lanes[lidx]->eWPInNode  = -1;
                                                    road->lanes[lidx]->eWPNodeDir = -1;
                                                }
                                                else if( road->lanes[lidx]->connectedNode == modNode[j] &&
                                                         road->lanes[lidx]->connectedNodeOutDirect == -1 &&
                                                         road->lanes[lidx]->departureNode == cND1 ){

                                                    road->lanes[lidx]->connectedNode = cND2;
                                                    road->lanes[lidx]->connectedNodeInDirect = cND2in;

                                                    road->lanes[lidx]->eWPInNode  = -1;
                                                    road->lanes[lidx]->eWPNodeDir = -1;
                                                }
                                                else if( road->lanes[lidx]->connectedNode == cND2 &&
                                                         road->lanes[lidx]->connectedNodeOutDirect == -1 &&
                                                         road->lanes[lidx]->departureNode == modNode[j] ){

                                                    road->lanes[lidx]->departureNode = cND1;
                                                    road->lanes[lidx]->departureNodeOutDirect = cND1out;

                                                    road->lanes[lidx]->sWPInNode  = -1;
                                                    road->lanes[lidx]->sWPNodeDir = -1;
                                                }
                                            }
                                        }
                                    }
                                }

                                road->DeleteNode( modNode[j] );
                            }
                        }
                    }
                }
            }
        }
        else if( canvas->selectedObj.selObjKind[i] == canvas->SEL_LANE ){

            int delLaneID = canvas->selectedObj.selObjID[i];
            int idx = road->indexOfLane( delLaneID );
            if( idx >= 0 ){

                int toNode = road->lanes[idx]->connectedNode;
                int tnIdx = road->indexOfNode( toNode );
                if( tnIdx >= 0 ){
                    int at = road->nodes[tnIdx]->relatedLanes.indexOf( delLaneID );
                    if( at >= 0 ){
                        road->nodes[tnIdx]->relatedLanes.removeAt( at );
                    }
                }
                int fromNode = road->lanes[idx]->departureNode;
                int fnIdx = road->indexOfNode( fromNode );
                if( fnIdx >= 0 ){
                    int at = road->nodes[fnIdx]->relatedLanes.indexOf( delLaneID );
                    if( at >= 0 ){
                        road->nodes[fnIdx]->relatedLanes.removeAt( at );
                    }
                }

                road->DeleteLane( delLaneID );
            }

        }
    }

    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();

    road->CheckLaneConnection();

    canvas->update();
}


void DataManipulator::MergeSelectedObject()
{
    if( canvas->selectedObj.selObjKind.size() == 0 ){
        return;
    }

    if( canvas->selectedObj.selObjKind.size() == 2 ){
        if( canvas->selectedObj.selObjKind[0] == canvas->SEL_NODE && canvas->selectedObj.selObjKind[1] == canvas->SEL_NODE ){
            int nd1 = canvas->selectedObj.selObjID[0];
            int nd2 = canvas->selectedObj.selObjID[1];
            if( road->GetNodeNumLeg(nd1) == 1 && road->GetNodeNumLeg(nd2) == 1 ){

                int nd1Idx = road->indexOfNode( nd1 );
                int nd2Idx = road->indexOfNode( nd2 );
                if( nd1Idx >= 0 && nd2Idx >= 0){

                    // Check Lane numbers
                    int nLane1In  = road->nodes[nd1Idx]->legInfo[0]->nLaneIn;
                    int nLane1Out = road->nodes[nd1Idx]->legInfo[0]->nLaneOut;

                    int nLane2In  = road->nodes[nd2Idx]->legInfo[0]->nLaneIn;
                    int nLane2Out = road->nodes[nd2Idx]->legInfo[0]->nLaneOut;

                    if( nLane1Out >= nLane2In && nLane2Out >= nLane1In ){

                        QList<int> nd1OutLane;
                        QList<int> nd1InLane;
                        // Determine Lanes to connect
                        for(int i=0;i<road->nodes[nd1Idx]->relatedLanes.size();++i){
                            int lidx = road->indexOfLane( road->nodes[nd1Idx]->relatedLanes[i] );
                            if( lidx >= 0 ){
                                if( road->lanes[lidx]->sWPInNode == nd1 && road->lanes[lidx]->sWPNodeDir == 0 && road->lanes[lidx]->sWPBoundary == true ){
                                    nd1OutLane.append( road->nodes[nd1Idx]->relatedLanes[i] );
                                }
                                else if( road->lanes[lidx]->eWPInNode == nd1 && road->lanes[lidx]->eWPNodeDir == 0 && road->lanes[lidx]->eWPBoundary == true ){
                                    nd1InLane.append( road->nodes[nd1Idx]->relatedLanes[i] );
                                }
                            }
                        }

                        QList<int> nd2OutLane;
                        QList<int> nd2InLane;
                        for(int i=0;i<road->nodes[nd2Idx]->relatedLanes.size();++i){
                            int lidx = road->indexOfLane( road->nodes[nd2Idx]->relatedLanes[i] );
                            if( lidx >= 0 ){
                                if( road->lanes[lidx]->sWPInNode == nd2 && road->lanes[lidx]->sWPNodeDir == 0 && road->lanes[lidx]->sWPBoundary == true ){
                                    nd2OutLane.append( road->nodes[nd2Idx]->relatedLanes[i] );
                                }
                                else if( road->lanes[lidx]->eWPInNode == nd2 && road->lanes[lidx]->eWPNodeDir == 0 && road->lanes[lidx]->eWPBoundary == true ){
                                    nd2InLane.append( road->nodes[nd2Idx]->relatedLanes[i] );
                                }
                            }
                        }

                        // Sort
                        if( nd1OutLane.size() > 0 ){
                            QList<float> dist;
                            dist.append( 0.0 );
                            int lidx = road->indexOfLane( nd1OutLane[0] );
                            if(lidx >= 0){
                                float xt = road->lanes[lidx]->shape.pos.first()->x();
                                float yt = road->lanes[lidx]->shape.pos.first()->y();
                                float ct = road->lanes[lidx]->shape.derivative.first()->x();
                                float st = road->lanes[lidx]->shape.derivative.first()->y();

                                for(int i=1;i<nd1OutLane.size();++i){
                                    lidx = road->indexOfLane( nd1OutLane[i] );
                                    if(lidx >= 0){
                                        float rx = road->lanes[lidx]->shape.pos.first()->x() - xt;
                                        float ry = road->lanes[lidx]->shape.pos.first()->y() - yt;
                                        float e = rx * (-st) + ry * ct;
                                        dist.append( e );
                                    }
                                }

                                QList<int> tmpLanes = nd1OutLane;
                                nd1OutLane.clear();
                                while( dist.size() > 0 ){
                                    int minIdx = 0;
                                    float minDist = dist[0];
                                    for(int i=1;i<dist.size();++i){
                                        if( dist[i] < minDist ){
                                            minIdx = i;
                                            minDist = dist[i];
                                        }
                                    }
                                    nd1OutLane.append( tmpLanes[minIdx] );
                                    tmpLanes.removeAt(minIdx);
                                    dist.removeAt(minIdx);
                                };
                            }
                        }

                        if( nd2OutLane.size() > 0 ){
                            QList<float> dist;
                            dist.append( 0.0 );
                            int lidx = road->indexOfLane( nd2OutLane[0] );
                            if(lidx >= 0){
                                float xt = road->lanes[lidx]->shape.pos.first()->x();
                                float yt = road->lanes[lidx]->shape.pos.first()->y();
                                float ct = road->lanes[lidx]->shape.derivative.first()->x();
                                float st = road->lanes[lidx]->shape.derivative.first()->y();

                                for(int i=1;i<nd2OutLane.size();++i){
                                    lidx = road->indexOfLane( nd2OutLane[i] );
                                    if(lidx >= 0){
                                        float rx = road->lanes[lidx]->shape.pos.first()->x() - xt;
                                        float ry = road->lanes[lidx]->shape.pos.first()->y() - yt;
                                        float e = rx * (-st) + ry * ct;
                                        dist.append( e );
                                    }
                                }

                                QList<int> tmpLanes = nd2OutLane;
                                nd2OutLane.clear();
                                while( dist.size() > 0 ){
                                    int minIdx = 0;
                                    float minDist = dist[0];
                                    for(int i=1;i<dist.size();++i){
                                        if( dist[i] < minDist ){
                                            minIdx = i;
                                            minDist = dist[i];
                                        }
                                    }
                                    nd2OutLane.append( tmpLanes[minIdx] );
                                    tmpLanes.removeAt(minIdx);
                                    dist.removeAt(minIdx);
                                };
                            }
                        }

                        if( nd1InLane.size() > 0 ){
                            QList<float> dist;
                            dist.append( 0.0 );
                            int lidx = road->indexOfLane( nd1InLane[0] );
                            if(lidx >= 0){
                                float xt = road->lanes[lidx]->shape.pos.last()->x();
                                float yt = road->lanes[lidx]->shape.pos.last()->y();
                                float ct = road->lanes[lidx]->shape.derivative.last()->x();
                                float st = road->lanes[lidx]->shape.derivative.last()->y();

                                for(int i=1;i<nd1InLane.size();++i){
                                    lidx = road->indexOfLane( nd1InLane[i] );
                                    if(lidx >= 0){
                                        float rx = road->lanes[lidx]->shape.pos.last()->x() - xt;
                                        float ry = road->lanes[lidx]->shape.pos.last()->y() - yt;
                                        float e = rx * (-st) + ry * ct;
                                        dist.append( e );
                                    }
                                }

                                QList<int> tmpLanes = nd1InLane;
                                nd1InLane.clear();
                                while( dist.size() > 0 ){
                                    int minIdx = 0;
                                    float minDist = dist[0];
                                    for(int i=1;i<dist.size();++i){
                                        if( dist[i] < minDist ){
                                            minIdx = i;
                                            minDist = dist[i];
                                        }
                                    }
                                    nd1InLane.append( tmpLanes[minIdx] );
                                    tmpLanes.removeAt(minIdx);
                                    dist.removeAt(minIdx);
                                };
                            }
                        }

                        if( nd2InLane.size() > 0 ){
                            QList<float> dist;
                            dist.append( 0.0 );
                            int lidx = road->indexOfLane( nd2InLane[0] );
                            if(lidx >= 0){
                                float xt = road->lanes[lidx]->shape.pos.last()->x();
                                float yt = road->lanes[lidx]->shape.pos.last()->y();
                                float ct = road->lanes[lidx]->shape.derivative.last()->x();
                                float st = road->lanes[lidx]->shape.derivative.last()->y();

                                for(int i=1;i<nd2InLane.size();++i){
                                    lidx = road->indexOfLane( nd2InLane[i] );
                                    if(lidx >= 0){
                                        float rx = road->lanes[lidx]->shape.pos.last()->x() - xt;
                                        float ry = road->lanes[lidx]->shape.pos.last()->y() - yt;
                                        float e = rx * (-st) + ry * ct;
                                        dist.append( e );
                                    }
                                }

                                QList<int> tmpLanes = nd2InLane;
                                nd2InLane.clear();
                                while( dist.size() > 0 ){
                                    int minIdx = 0;
                                    float minDist = dist[0];
                                    for(int i=1;i<dist.size();++i){
                                        if( dist[i] < minDist ){
                                            minIdx = i;
                                            minDist = dist[i];
                                        }
                                    }
                                    nd2InLane.append( tmpLanes[minIdx] );
                                    tmpLanes.removeAt(minIdx);
                                    dist.removeAt(minIdx);
                                };
                            }
                        }


                        // Get Current Node connection info
                        int nd1DepartureNode = road->nodes[nd1Idx]->legInfo[0]->connectedNode;
                        int nd1DepartureNodeOutDirect = road->nodes[nd1Idx]->legInfo[0]->connectedNodeOutDirect;

                        int nd2ConnectingNode = road->nodes[nd2Idx]->legInfo[0]->connectingNode;
                        int nd2ConnectingNodeInDirect = road->nodes[nd2Idx]->legInfo[0]->connectingNodeInDirect;

                        int nd2DepartureNode = road->nodes[nd2Idx]->legInfo[0]->connectedNode;
                        int nd2DepartureNodeOutDirect = road->nodes[nd2Idx]->legInfo[0]->connectedNodeOutDirect;

                        int nd1ConnectingNode = road->nodes[nd1Idx]->legInfo[0]->connectingNode;
                        int nd1ConnectingNodeInDirect = road->nodes[nd1Idx]->legInfo[0]->connectingNodeInDirect;


                        // Update Node Connection Info
                        road->SetNodeConnectInfo( nd1DepartureNode, nd1DepartureNodeOutDirect, nd2ConnectingNode, QString("OutNode") );
                        road->SetNodeConnectInfo( nd1DepartureNode, nd1DepartureNodeOutDirect, nd2ConnectingNodeInDirect, QString("OutNodeInDirect") );
                        road->SetNodeConnectInfo( nd1ConnectingNode, nd1ConnectingNodeInDirect, nd2DepartureNode, QString("InNode") );
                        road->SetNodeConnectInfo( nd1ConnectingNode, nd1ConnectingNodeInDirect, nd2DepartureNodeOutDirect, QString("InNodeOutDirect") );

                        road->SetNodeConnectInfo( nd2DepartureNode, nd2DepartureNodeOutDirect, nd1ConnectingNode, QString("OutNode") );
                        road->SetNodeConnectInfo( nd2DepartureNode, nd2DepartureNodeOutDirect, nd1ConnectingNodeInDirect, QString("OutNodeInDirect") );
                        road->SetNodeConnectInfo( nd2ConnectingNode, nd2ConnectingNodeInDirect, nd1DepartureNode, QString("InNode") );
                        road->SetNodeConnectInfo( nd2ConnectingNode, nd2ConnectingNodeInDirect, nd1DepartureNodeOutDirect, QString("InNodeOutDirect") );


                        // Create Lanes
                        for(int i=0;i<nd1InLane.size();++i){

                            QVector4D startPoint;
                            QVector4D endPoint;

                            int lidx = road->indexOfLane( nd1InLane[i] );
                            if( lidx >= 0 ){
                                startPoint.setX( road->lanes[lidx]->shape.pos.last()->x() );
                                startPoint.setY( road->lanes[lidx]->shape.pos.last()->y() );
                                startPoint.setZ( road->lanes[lidx]->shape.pos.last()->z() );
                                startPoint.setW( road->lanes[lidx]->shape.angles.last() );

                                road->lanes[lidx]->eWPInNode   = -1;
                                road->lanes[lidx]->eWPNodeDir  = -1;
                                road->lanes[lidx]->eWPBoundary = false;

                                lidx = road->indexOfLane( nd2OutLane[i] );
                                if( lidx >= 0 ){
                                    endPoint.setX( road->lanes[lidx]->shape.pos.first()->x() );
                                    endPoint.setY( road->lanes[lidx]->shape.pos.first()->y() );
                                    endPoint.setZ( road->lanes[lidx]->shape.pos.first()->z() );
                                    endPoint.setW( road->lanes[lidx]->shape.angles.first() );

                                    road->lanes[lidx]->sWPInNode   = -1;
                                    road->lanes[lidx]->sWPNodeDir  = -1;
                                    road->lanes[lidx]->sWPBoundary = false;

                                    int lId = road->CreateLane( -1, startPoint, -1, -1, nd1DepartureNode, false, endPoint, -1, -1, nd2ConnectingNode, false );
                                    road->SetNodeRelatedLane( nd1DepartureNode, lId );
                                    road->SetNodeRelatedLane( nd2ConnectingNode, lId );

                                    lidx = road->indexOfLane( lId );
                                    if( lidx >= 0 ){
                                        road->lanes[lidx]->connectedNodeInDirect = nd2ConnectingNodeInDirect;
                                        road->lanes[lidx]->departureNodeOutDirect = nd1DepartureNodeOutDirect;
                                    }
                                }
                            }
                        }

                        for(int i=0;i<nd2InLane.size();++i){

                            QVector4D startPoint;
                            QVector4D endPoint;

                            int lidx = road->indexOfLane( nd2InLane[i] );
                            if( lidx >= 0 ){
                                startPoint.setX( road->lanes[lidx]->shape.pos.last()->x() );
                                startPoint.setY( road->lanes[lidx]->shape.pos.last()->y() );
                                startPoint.setZ( road->lanes[lidx]->shape.pos.last()->z() );
                                startPoint.setW( road->lanes[lidx]->shape.angles.last() );

                                road->lanes[lidx]->eWPInNode   = -1;
                                road->lanes[lidx]->eWPNodeDir  = -1;
                                road->lanes[lidx]->eWPBoundary = false;

                                lidx = road->indexOfLane( nd1OutLane[i] );
                                if( lidx >= 0 ){
                                    endPoint.setX( road->lanes[lidx]->shape.pos.first()->x() );
                                    endPoint.setY( road->lanes[lidx]->shape.pos.first()->y() );
                                    endPoint.setZ( road->lanes[lidx]->shape.pos.first()->z() );
                                    endPoint.setW( road->lanes[lidx]->shape.angles.first() );

                                    road->lanes[lidx]->sWPInNode   = -1;
                                    road->lanes[lidx]->sWPNodeDir  = -1;
                                    road->lanes[lidx]->sWPBoundary = false;

                                    int lId = road->CreateLane( -1, startPoint, -1, -1, nd2DepartureNode, false, endPoint, -1, -1, nd1ConnectingNode, false );
                                    road->SetNodeRelatedLane( nd2DepartureNode, lId );
                                    road->SetNodeRelatedLane( nd1ConnectingNode, lId );

                                    lidx = road->indexOfLane( lId );
                                    if( lidx >= 0 ){
                                        road->lanes[lidx]->connectedNodeInDirect = nd1ConnectingNodeInDirect;
                                        road->lanes[lidx]->departureNodeOutDirect = nd2DepartureNodeOutDirect;
                                    }
                                }
                            }
                        }

                        // Update existing Lane connection data
                        for(int i=0;i<road->nodes[nd1Idx]->relatedLanes.size();++i){
                            int lidx = road->indexOfLane( road->nodes[nd1Idx]->relatedLanes[i] );
                            if( lidx >= 0 ){
                                if( road->lanes[lidx]->connectedNode == nd1  ){
                                    road->SetNodeRelatedLane( nd2ConnectingNode, road->nodes[nd1Idx]->relatedLanes[i] );
                                    road->lanes[lidx]->connectedNode = nd2ConnectingNode;
                                    road->lanes[lidx]->connectedNodeInDirect = nd2ConnectingNodeInDirect;
                                    road->lanes[lidx]->connectedNodeOutDirect = -1;
                                }
                                else if( road->lanes[lidx]->departureNode == nd1 ){
                                    road->SetNodeRelatedLane( nd2DepartureNode, road->nodes[nd1Idx]->relatedLanes[i] );
                                    road->lanes[lidx]->departureNode = nd2DepartureNode;
                                    road->lanes[lidx]->departureNodeOutDirect = nd2DepartureNodeOutDirect;
                                }
                            }
                        }

                        for(int i=0;i<road->nodes[nd2Idx]->relatedLanes.size();++i){
                            int lidx = road->indexOfLane( road->nodes[nd2Idx]->relatedLanes[i] );
                            if( lidx >= 0 ){
                                if( road->lanes[lidx]->connectedNode == nd2 ){
                                    road->SetNodeRelatedLane( nd1ConnectingNode, road->nodes[nd2Idx]->relatedLanes[i] );
                                    road->lanes[lidx]->connectedNode = nd1ConnectingNode;
                                    road->lanes[lidx]->connectedNodeInDirect = nd1ConnectingNodeInDirect;
                                    road->lanes[lidx]->connectedNodeOutDirect = -1;
                                }
                                if( road->lanes[lidx]->departureNode == nd2 ){
                                    road->SetNodeRelatedLane( nd1DepartureNode, road->nodes[nd2Idx]->relatedLanes[i] );
                                    road->lanes[lidx]->departureNode = nd1DepartureNode;
                                    road->lanes[lidx]->departureNodeOutDirect = nd1DepartureNodeOutDirect;
                                }
                            }
                        }

                        // Delete Terminal Nodes
                        road->DeleteNode( nd1 );
                        road->DeleteNode( nd2 );
                    }
                }
            }
        }
        else if( canvas->selectedObj.selObjKind[0] == canvas->SEL_LANE &&
                 canvas->selectedObj.selObjKind[1] == canvas->SEL_LANE ){

            int eLaneID = canvas->selectedObj.selObjID[0];
            int sLaneID = canvas->selectedObj.selObjID[1];
            int elidx = road->indexOfLane( eLaneID );
            int slidx = road->indexOfLane( sLaneID );
            if( elidx >= 0 && slidx >= 0 ){

                int eWPinNode = road->lanes[elidx]->eWPInNode;
                int eWPinDir  = road->lanes[elidx]->eWPNodeDir;
                int sWPinNode = road->lanes[slidx]->sWPInNode;
                int sWPinDir  = road->lanes[slidx]->sWPNodeDir;

                int dNode = -1;
                int cNode = -1;

                int cN1 = road->lanes[elidx]->connectedNode;
                int cN2 = road->lanes[slidx]->connectedNode;
                int dN1 = road->lanes[elidx]->departureNode;
                int dN2 = road->lanes[slidx]->departureNode;

                // Case 1
                if( dN1 != cN1 && dN1 == dN2 && cN1 && cN2 ){
                    dNode = dN1;
                    cNode = cN1;
                }
                // Case 2
                else if( dN1 != cN1 && cN1 == dN2 && dN2 == cN2 ){
                    if( eWPinNode < 0 ){
                        dNode = dN1;
                        cNode = cN1;
                    }
                    else{
                        dNode = cN1;
                        cNode = cN1;
                    }
                }
                // Case 3
                else if( dN1 == cN1 && cN1 == dN2 && dN2 != cN2 ){
                    if( eWPinNode < 0 ){
                        dNode = cN1;
                        cNode = cN1;
                    }
                    else{
                        dNode = dN2;
                        cNode = cN2;
                    }
                }
                // Case 4
                else if( dN1 != dN2 && dN1 == cN1 && dN2 == cN2 ){
                    dNode = dN1;
                    cNode = cN2;
                }
                // Case 5
                else if( dN1 != cN1 && cN1 == dN2 && dN2 != cN2 && dN1 != cN2 ){
                    dNode = cN1;
                    cNode = cN1;
                }
                // Case 6
                else if( dN1 == cN1 && cN1 == dN2 && dN2 == cN2 ){
                    dNode = dN1;
                    cNode = cN1;
                }


                QVector4D startPoint;
                QVector4D endPoint;

                startPoint.setX( road->lanes[elidx]->shape.pos.last()->x() );
                startPoint.setY( road->lanes[elidx]->shape.pos.last()->y() );
                startPoint.setZ( road->lanes[elidx]->shape.pos.last()->z() );
                startPoint.setW( road->lanes[elidx]->shape.angles.last() );

                endPoint.setX( road->lanes[slidx]->shape.pos.first()->x() );
                endPoint.setY( road->lanes[slidx]->shape.pos.first()->y() );
                endPoint.setZ( road->lanes[slidx]->shape.pos.first()->z() );
                endPoint.setW( road->lanes[slidx]->shape.angles.first() );

                int lId = road->CreateLane( -1, startPoint, eWPinNode, eWPinDir, dNode, road->lanes[elidx]->eWPBoundary, endPoint, sWPinNode, sWPinDir, cNode, road->lanes[slidx]->sWPBoundary );

                road->SetNodeRelatedLane( dNode, lId );
                road->SetNodeRelatedLane( cNode, lId );

                // Set Lane Info
                int lidx = road->indexOfNode( lId );
                if( lidx >= 0 ){

                    // TO BE CODED

                }

            }

        }

    }

    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();

    road->CheckLaneConnection();

    canvas->update();
}


void DataManipulator::CheckLaneConnectionFull()
{
    road->CheckLaneConnectionFull();
    canvas->update();
}


void DataManipulator::CreateWPData()
{
    road->CreateWPData();
    canvas->update();
}


void DataManipulator::SetODFlagOfTerminalNode()
{
    road->SetODFlagOfTerminalNode();
}


void DataManipulator::SetAllLaneLists()
{
    road->SetAllLaneLists();
}


void DataManipulator::SetSelectedNodeLaneLists()
{
    for(int i=0;i<canvas->selectedObj.selObjKind.size();++i){
        if( canvas->selectedObj.selObjKind[i] == canvas->SEL_NODE ){

            road->SetLaneLists( canvas->selectedObj.selObjID[i] , 0, true );
        }
    }
}


void DataManipulator::SetTurnDirectionInfo()
{
    road->SetTurnDirectionInfo();
}


void DataManipulator::CheckAllStopLineCrossLane()
{
    road->CheckAllStopLineCrossLane();
}


void DataManipulator::CheckLaneCrossPoints()
{
    road->CheckLaneCrossPoints();
}


void DataManipulator::CheckCrossPointsOfSelectedLane()
{
    if( canvas->selectedObj.selObjKind.first() != canvas->SEL_LANE ){
        return;
    }

    int selLaneID = canvas->selectedObj.selObjID.first();
    int lIdx = road->indexOfLane( selLaneID );

    {
        int i = lIdx;

        for(int j=0;j<road->lanes[i]->crossPoints.size();++j){
            delete road->lanes[i]->crossPoints[j];
        }
        road->lanes[i]->crossPoints.clear();

        for(int j=0;j<road->lanes[i]->pedestCrossPoints.size();++j){
            delete road->lanes[i]->pedestCrossPoints[j];
        }
        road->lanes[i]->pedestCrossPoints.clear();

        for(int j=0;j<road->lanes.size();++j){
            if( j == i ){
                continue;
            }
            if( road->lanes[j]->crossPoints.size() == 0 ){
                continue;
            }
            QList<int> delCPInfo;
            for(int k=0;k<road->lanes[j]->crossPoints.size();++k){
                if( road->lanes[j]->crossPoints[k]->crossLaneID == selLaneID ){
                    delCPInfo.prepend( k );
                }
            }
            if( delCPInfo.size() == 0 ){
                continue;
            }
            for(int k=0;k<delCPInfo.size();++k){
                delete road->lanes[j]->crossPoints[ delCPInfo[k] ];
                road->lanes[j]->crossPoints.removeAt( delCPInfo[k] );
            }
        }
    }

    for(int j=0;j<road->lanes.size();++j){
        if( lIdx == j ){
            continue;
        }

        //qDebug() << "Checking lane " << road->lanes[j]->id;

        road->CheckIfTwoLanesCross( road->lanes[lIdx]->id, road->lanes[j]->id );
    }

    for(int j=0;j<road->pedestLanes.size();++j){

        //qDebug() << "Checking pedestLane " << road->pedestLanes[j]->id;

        road->CheckLaneCrossWithPedestLane( road->lanes[lIdx]->id, road->pedestLanes[j]->id );
    }

    UpdateStatusBar(QString("Cross Point Checked."));
    canvas->update();
}


void DataManipulator::SearchNode()
{
    int nodeId = QInputDialog::getInt(NULL,"Find Node","Node ID");
    int nIdx = road->indexOfNode( nodeId );
    if( nIdx >= 0 ){

        float x = road->nodes[nIdx]->pos.x();
        float y = road->nodes[nIdx]->pos.y();

        canvas->ResetRotate();
        canvas->MoveTo( x, y );

        canvas->selectedObj.selObjKind.clear();
        canvas->selectedObj.selObjID.clear();

        canvas->selectedObj.selObjKind.append( canvas->SEL_NODE );
        canvas->selectedObj.selObjID.append( nodeId );

        roadObjProp->ChangeTabPage(0);
        roadObjProp->ChangeLaneInfo(nodeId);

        canvas->update();
    }
}


void DataManipulator::SearchLane()
{
    int laneId = QInputDialog::getInt(NULL,"Find Lane","Lane ID");
    int lIdx = road->indexOfLane( laneId );

    qDebug() << "Lane ID = " << laneId << "  lIdx = " << lIdx;

    if( lIdx >= 0 ){

        float x = road->lanes[lIdx]->shape.pos.first()->x();
        float y = road->lanes[lIdx]->shape.pos.first()->y();

        qDebug() << "x = " << x << "  y = " << y;

        canvas->ResetRotate();
        canvas->MoveTo( x, y );

        canvas->selectedObj.selObjKind.clear();
        canvas->selectedObj.selObjID.clear();

        canvas->selectedObj.selObjKind.append( canvas->SEL_LANE );
        canvas->selectedObj.selObjID.append( laneId );

        roadObjProp->ChangeTabPage(1);
        roadObjProp->ChangeLaneInfo(laneId);

        canvas->update();
    }
}


void DataManipulator::SearchTrafficSignal()
{
    int tsId = QInputDialog::getInt(NULL,"Find Traffic Signal","TS ID");

    int rNd = road->indexOfTS( tsId, -1 );
    int ndIdx = road->indexOfNode( rNd );
    int tsIdx = road->indexOfTS( tsId, rNd );

    if( ndIdx >= 0 && tsIdx >= 0 ){

        float x = road->nodes[ndIdx]->trafficSignals[tsIdx]->pos.x();
        float y = road->nodes[ndIdx]->trafficSignals[tsIdx]->pos.y();

        canvas->ResetRotate();
        canvas->MoveTo( x, y );

        canvas->selectedObj.selObjKind.clear();
        canvas->selectedObj.selObjID.clear();

        canvas->selectedObj.selObjKind.append( canvas->SEL_TRAFFIC_SIGNAL );
        canvas->selectedObj.selObjID.append( tsId );

        roadObjProp->ChangeTabPage(2);
        roadObjProp->ChangeLaneInfo(tsId);

        canvas->update();
    }
}


void DataManipulator::MoveXY()
{
    QString xyTxt = QInputDialog::getText(NULL,"Enter Coordinate","X,Y");
    QStringList xyDiv = xyTxt.split(",");
    if( xyDiv.size() != 2 ){
        emit UpdateStatusBar( QString("Invalid coordinate data") );
        return;
    }

    bool isOK;
    float x = QString(xyDiv[0]).trimmed().toFloat(&isOK);
    if( isOK == true ){
        float y = QString(xyDiv[1]).trimmed().toFloat(&isOK);
        if( isOK == true ){
            canvas->ResetRotate();
            canvas->MoveTo( x, y );
            canvas->update();
        }
    }
}


void DataManipulator::SelectAllLanes()
{
    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();

    for(int i=0;i<road->lanes.size();++i){
        canvas->selectedObj.selObjKind.append( canvas->SEL_LANE );
        canvas->selectedObj.selObjID.append( road->lanes[i]->id );
    }

    canvas->update();
}



void DataManipulator::ReadLineCSV()
{
    QString fileName = QFileDialog::getOpenFileName(NULL,
                                                    tr("Choose Line Data CSV"),
                                                    ".",
                                                    tr("Data file(*.csv)"));

    if( fileName.isNull() || fileName.isEmpty() ){
        return;
    }

    QFile file(fileName);
    if( file.open(QIODevice::ReadOnly | QIODevice::Text) == false ){
        return;
    }

    QTextStream in(&file);

    QString lineStr = in.readLine();
    QStringList lineStrDiv = lineStr.split(",");
    if( lineStrDiv.size() != 4 ){
        return;
    }

    canvas->lineObjCoordInfo.center.setY( QString(lineStrDiv[0]).trimmed().toFloat() );
    canvas->lineObjCoordInfo.center.setX( QString(lineStrDiv[1]).trimmed().toFloat() );
    canvas->lineObjCoordInfo.scale_x = QString(lineStrDiv[2]).trimmed().toFloat();
    canvas->lineObjCoordInfo.scale_y = QString(lineStrDiv[3]).trimmed().toFloat();


    while(in.atEnd() == false ){

        lineStr = in.readLine();

        lineStrDiv = lineStr.split(",");
        if( lineStrDiv.size() < 2 ){
            continue;
        }

        struct LineObject *l = new LineObject;

        l->color = QString(lineStrDiv[0]).trimmed().toInt();

        for(int i=1;i<lineStrDiv.size();++i){

            QStringList pointDiv = QString(lineStrDiv[i]).trimmed().split(":");

            QPointF p;
            p.setY( QString(pointDiv[0]).trimmed().toFloat() );
            p.setX( QString(pointDiv[1]).trimmed().toFloat() );

            l->coord.append( p );

            QPointF p2;
            p2.setX( (p.x() - canvas->lineObjCoordInfo.center.x()) * canvas->lineObjCoordInfo.scale_x );
            p2.setY( (p.y() - canvas->lineObjCoordInfo.center.y()) * canvas->lineObjCoordInfo.scale_y );

            l->p.append( p2 );
        }


        canvas->lineObj.append( l );
    }


    file.close();
}


void DataManipulator::ClearLineData()
{
    for(int i=0;i<canvas->lineObj.size();++i){
        canvas->lineObj[i]->p.clear();
        delete canvas->lineObj[i];
    }
    canvas->lineObj.clear();

    canvas->update();
}


void DataManipulator::ChangeLineCoordInfo()
{
    QString currentData = QString("Lat:%1, Long:%2, ScaleX:%3, ScaleY:%4")
            .arg( canvas->lineObjCoordInfo.center.y(), 0, 'g', 10 )
            .arg( canvas->lineObjCoordInfo.center.x(), 0, 'g', 10 )
            .arg( canvas->lineObjCoordInfo.scale_x )
            .arg( canvas->lineObjCoordInfo.scale_y );

    QString modData = QInputDialog::getText(NULL,"Set Coord Info",QString(),QLineEdit::Normal, currentData);
    if( modData.isNull() || modData.isEmpty() ){
        return;
    }

    QStringList divStr = modData.split(",");
    if( divStr.size() != 4 ){
        return;
    }

    float _lat = QString(divStr[0]).remove("Lat:").trimmed().toFloat();
    float _long = QString(divStr[1]).remove("Long:").trimmed().toFloat();
    float _sx = QString(divStr[2]).remove("ScaleX:").trimmed().toFloat();
    float _sy = QString(divStr[3]).remove("ScaleY:").trimmed().toFloat();

    canvas->lineObjCoordInfo.center.setX( _long );
    canvas->lineObjCoordInfo.center.setY( _lat );
    canvas->lineObjCoordInfo.scale_x = _sx;
    canvas->lineObjCoordInfo.scale_y = _sy;

    canvas->update();
}


void DataManipulator::SplitSelectedLane()
{
    qDebug() << "[SplitSelectedLane]";

    QList<int> targetLanes;

    for(int i=0;i<canvas->selectedObj.selObjKind.size();++i){
        if( canvas->selectedObj.selObjKind[i] == canvas->SEL_LANE ){
            targetLanes.append( canvas->selectedObj.selObjID[i] );
        }
    }

    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();

    qDebug() << "targetLanes: " << targetLanes;

    for(int i=0;i<targetLanes.size();++i){
        road->DivideLaneHalf( targetLanes[i] );
    }

    canvas->update();
}
