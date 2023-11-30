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


void DataManipulator::GetConnectionDirection(int nd1,int nd2, int& dir1, int &dir2)
{
    int nd1Idx = road->indexOfNode( nd1 );
    int nd2Idx = road->indexOfNode( nd2 );

    for(int i=0;i<road->nodes[nd1Idx]->legInfo.size();++i){
        if( road->nodes[nd1Idx]->legInfo[i]->connectingNode == nd2 ){
            dir1 = i;
            break;
        }
    }
    for(int i=0;i<road->nodes[nd2Idx]->legInfo.size();++i){
        if( road->nodes[nd2Idx]->legInfo[i]->connectedNode == nd1 &&
                road->nodes[nd2Idx]->legInfo[i]->connectedNodeOutDirect != dir1 ){
            dir1 = -1;
            break;
        }
    }


    for(int i=0;i<road->nodes[nd2Idx]->legInfo.size();++i){
        if( road->nodes[nd2Idx]->legInfo[i]->connectingNode == nd1 ){
            dir2 = i;
            break;
        }
    }
    for(int i=0;i<road->nodes[nd1Idx]->legInfo.size();++i){
        if( road->nodes[nd1Idx]->legInfo[i]->connectedNode == nd2 &&
                road->nodes[nd1Idx]->legInfo[i]->connectedNodeOutDirect != dir2 ){
            dir2 = -1;
            break;
        }
    }
}


void DataManipulator::ChangeRelatedLanesForPreviousLanes(int lidx,int addNode,int relDir,int rmvNode)
{
    if( lidx < 0 || lidx >= road->lanes.size() ){
        return;
    }

    depthCount++;

    int anIdx = road->indexOfNode( addNode );
    int rnIdx = road->indexOfNode( rmvNode );

    for(int j=0;j<road->lanes[lidx]->previousLanes.size();j++){
        int plane = road->lanes[lidx]->previousLanes[j];

        road->nodes[anIdx]->relatedLanes.append( plane );

        int rl = road->nodes[rnIdx]->relatedLanes.indexOf( plane );
        if( rl >= 0 ){
            road->nodes[rnIdx]->relatedLanes.removeAt( rl );
        }

        int plidx = road->indexOfLane( plane );

        if( depthCount == 1 ){
            road->lanes[plidx]->eWPBoundary = true;
        }
        road->lanes[plidx]->eWPInNode = addNode;
        road->lanes[plidx]->eWPNodeDir = relDir;

        if( road->lanes[plidx]->sWPBoundary == false ){
            ChangeRelatedLanesForPreviousLanes( plidx, addNode, relDir, rmvNode );
            depthCount--;
        }
    }
}

void DataManipulator::ChangeRelatedLanesForNextLanes(int lidx,int addNode,int relDir,int rmvNode)
{
    if( lidx < 0 || lidx >= road->lanes.size() ){
        return;
    }

    depthCount++;

    int anIdx = road->indexOfNode( addNode );
    int rnIdx = road->indexOfNode( rmvNode );

    for(int j=0;j<road->lanes[lidx]->nextLanes.size();j++){
        int nlane = road->lanes[lidx]->nextLanes[j];

        road->nodes[anIdx]->relatedLanes.append( nlane );

        int rl = road->nodes[rnIdx]->relatedLanes.indexOf( nlane );
        if( rl >= 0 ){
            road->nodes[rnIdx]->relatedLanes.removeAt( rl );
        }

        int nlidx = road->indexOfLane( nlane );

        if( depthCount == 1 ){
            road->lanes[nlidx]->sWPBoundary = true;
        }
        road->lanes[nlidx]->sWPInNode = addNode;
        road->lanes[nlidx]->sWPNodeDir = relDir;

        if( road->lanes[nlidx]->eWPBoundary == false ){
            ChangeRelatedLanesForPreviousLanes( nlidx, addNode, relDir, rmvNode );
            depthCount--;
        }
    }
}


void DataManipulator::InsertNode_4x1_noTS()
{
    int nPoint = 0;
    float xc = 0.0;
    float yc = 0.0;

    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){

            xc += road->lanes[lidx]->shape.pos.first()->x();
            yc += road->lanes[lidx]->shape.pos.first()->y();
            nPoint++;

            xc += road->lanes[lidx]->shape.pos.last()->x();
            yc += road->lanes[lidx]->shape.pos.last()->y();
            nPoint++;
        }
    }
    if( nPoint == 0 ){
        return;
    }
    xc /= nPoint;
    yc /= nPoint;

    qDebug() << " xc = " << xc << " yc = " << yc;

    QStringList edgeNodeInfo;
    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){

            QString eni = QString("%1,%2,%3,%4")
                    .arg(road->lanes[lidx]->sWPInNode)
                    .arg(road->lanes[lidx]->sWPNodeDir)
                    .arg(road->lanes[lidx]->eWPInNode)
                    .arg(road->lanes[lidx]->eWPNodeDir);

            if( edgeNodeInfo.contains(eni) == false ){
                edgeNodeInfo.append( eni );
            }
        }
    }


    QStringList nodeConnectInfo;

    QList<int> d0StartLanes;

    float angle0 = 0.0;
    int nIn0 = 0;
    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){
            float rx = road->lanes[lidx]->shape.pos.first()->x() - xc;
            if( rx > 0.0 ){

                d0StartLanes.append( lidx );

                if( nIn0 == 0 ){
                    angle0 = atan2( -road->lanes[lidx]->shape.derivative.first()->y(), -road->lanes[lidx]->shape.derivative.first()->x() );

                    QString ncStr = QString("%1,%2").arg( road->lanes[lidx]->sWPInNode ).arg( road->lanes[lidx]->sWPNodeDir );
                    nodeConnectInfo.append( ncStr );
                }
                nIn0++;
            }
        }
    }

    nodeConnectInfo.append( QString("-1,0") );

    QList<int> d2StartLanes;

    float angle2 = 0.0;
    int nIn2 = 0;
    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){
            float rx = road->lanes[lidx]->shape.pos.first()->x() - xc;
            if( rx < 0.0 ){

                d2StartLanes.append( lidx );

                if( nIn2 == 0 ){
                    angle2 = atan2( -road->lanes[lidx]->shape.derivative.first()->y(), -road->lanes[lidx]->shape.derivative.first()->x() );

                    QString ncStr = QString("%1,%2").arg( road->lanes[lidx]->sWPInNode ).arg( road->lanes[lidx]->sWPNodeDir );
                    nodeConnectInfo.append( ncStr );
                }
                nIn2++;
            }
        }
    }

    nodeConnectInfo.append( QString("-1,0") );

    if( cos(angle0) * cos(angle2) + sin(angle0) * sin(angle2) > 0.0 ){
        angle2 = angle0 + 3.141592;
        if( angle2 > 3.141592 ){
            angle2 -= 6.28272;
        }
        else if( angle2 < -3.141592 ){
            angle2 += 6.28272;
        }
    }

    qDebug() << " angle0 = " << angle0 * 57.3;
    qDebug() << " angle2 = " << angle2 * 57.3;

    float angle1 = angle0 + 1.5708;
    if( angle1 > 3.141592 ){
        angle1 -= 6.28272;
    }
    else if( angle1 < -3.141592 ){
        angle1 += 6.28272;
    }

    float angle3 = angle2 + 1.5708;
    if( angle3 > 3.141592 ){
        angle3 -= 6.28272;
    }
    else if( angle3 < -3.141592 ){
        angle3 += 6.28272;
    }

    qDebug() << " angle1 = " << angle1 * 57.3;
    qDebug() << " angle3 = " << angle3 * 57.3;

    qDebug() << " nodeConnectInfo = " << nodeConnectInfo;

    QList<int> inLanes;
    QList<int> outLanes;

    inLanes << nIn0 << 1 << nIn2 << 1;
    outLanes << nIn2 << 1 << nIn0 << 1;

    int nID = road->CreateNode( -1, xc, yc, inLanes, outLanes );

    int nIdx = road->indexOfNode( nID );
    if( nIdx >= 0 ){

        road->nodes[nIdx]->legInfo[0]->angle = angle0 * 57.3;
        road->nodes[nIdx]->legInfo[1]->angle = angle1 * 57.3;
        road->nodes[nIdx]->legInfo[2]->angle = angle2 * 57.3;
        road->nodes[nIdx]->legInfo[3]->angle = angle3 * 57.3;

        QStringList divStr0 = QString(nodeConnectInfo[0]).split(",");
        road->nodes[nIdx]->legInfo[0]->connectedNode = QString(divStr0[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[0]->connectedNodeOutDirect = QString(divStr0[1]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[0]->connectingNode = QString(divStr0[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[0]->connectingNodeInDirect = QString(divStr0[1]).trimmed().toInt();

        int cN0idx = road->indexOfNode( road->nodes[nIdx]->legInfo[0]->connectedNode );
        if( cN0idx >= 0 ){
            int dir = road->nodes[nIdx]->legInfo[0]->connectedNodeOutDirect;
            road->nodes[cN0idx]->legInfo[dir]->connectedNode = nID;
            road->nodes[cN0idx]->legInfo[dir]->connectedNodeOutDirect = 0;
            road->nodes[cN0idx]->legInfo[dir]->connectingNode = nID;
            road->nodes[cN0idx]->legInfo[dir]->connectingNodeInDirect = 0;

            for(int i=0;i<d0StartLanes.size();++i){
                int rlIdx = road->nodes[cN0idx]->relatedLanes.indexOf( road->lanes[d0StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN0idx]->relatedLanes.removeAt( rlIdx );
                }
            }

            for(int i=0;i<d2StartLanes.size();++i){
                int rlIdx = road->nodes[cN0idx]->relatedLanes.indexOf( road->lanes[d2StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN0idx]->relatedLanes.removeAt( rlIdx );
                }
            }
        }


        QStringList divStr2 = QString(nodeConnectInfo[2]).split(",");
        road->nodes[nIdx]->legInfo[2]->connectedNode = QString(divStr2[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[2]->connectedNodeOutDirect = QString(divStr2[1]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[2]->connectingNode = QString(divStr2[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[2]->connectingNodeInDirect = QString(divStr2[1]).trimmed().toInt();

        int cN2idx = road->indexOfNode( road->nodes[nIdx]->legInfo[2]->connectedNode );
        if( cN2idx >= 0 ){
            int dir = road->nodes[nIdx]->legInfo[2]->connectedNodeOutDirect;
            road->nodes[cN2idx]->legInfo[dir]->connectedNode = nID;
            road->nodes[cN2idx]->legInfo[dir]->connectedNodeOutDirect = 2;
            road->nodes[cN2idx]->legInfo[dir]->connectingNode = nID;
            road->nodes[cN2idx]->legInfo[dir]->connectingNodeInDirect = 2;

            for(int i=0;i<d0StartLanes.size();++i){
                int rlIdx = road->nodes[cN2idx]->relatedLanes.indexOf( road->lanes[d0StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN2idx]->relatedLanes.removeAt( rlIdx );
                }
            }

            for(int i=0;i<d2StartLanes.size();++i){
                int rlIdx = road->nodes[cN2idx]->relatedLanes.indexOf( road->lanes[d2StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN2idx]->relatedLanes.removeAt( rlIdx );
                }
            }
        }

        for(int i=0;i<d0StartLanes.size();++i){
            int lidx = d0StartLanes[i];
            road->lanes[lidx]->sWPInNode = nID;
            road->lanes[lidx]->sWPNodeDir = 0;
            road->lanes[lidx]->sWPBoundary = true;
            road->lanes[lidx]->eWPInNode = nID;
            road->lanes[lidx]->eWPNodeDir = 2;
            road->lanes[lidx]->eWPBoundary = true;

            road->nodes[nIdx]->relatedLanes.append( road->lanes[lidx]->id );

            depthCount = 0;
            ChangeRelatedLanesForPreviousLanes(lidx, nID, 0, road->nodes[nIdx]->legInfo[2]->connectedNode );

            depthCount = 0;
            ChangeRelatedLanesForNextLanes(lidx, nID, 2, road->nodes[nIdx]->legInfo[0]->connectedNode );
        }


        for(int i=0;i<d2StartLanes.size();++i){
            int lidx = d2StartLanes[i];
            road->lanes[lidx]->sWPInNode = nID;
            road->lanes[lidx]->sWPNodeDir = 2;
            road->lanes[lidx]->sWPBoundary = true;
            road->lanes[lidx]->eWPInNode = nID;
            road->lanes[lidx]->eWPNodeDir = 0;
            road->lanes[lidx]->eWPBoundary = true;

            road->nodes[nIdx]->relatedLanes.append( road->lanes[lidx]->id );

            depthCount = 0;
            ChangeRelatedLanesForPreviousLanes(lidx, nID, 2, road->nodes[nIdx]->legInfo[0]->connectedNode );

            depthCount = 0;
            ChangeRelatedLanesForNextLanes(lidx, nID, 0, road->nodes[nIdx]->legInfo[2]->connectedNode );
        }


        // Create Terminal Nodes
        float distToTN = 50.0;

        inLanes.clear();
        outLanes.clear();

        inLanes << 1;
        outLanes << 1;

        float xt1 = xc + cos( angle1 ) * distToTN;
        float yt1 = yc + sin( angle1 ) * distToTN;

        int tID1 = road->CreateNode( -1, xt1, yt1, inLanes, outLanes );
        int tIdx1 = road->indexOfNode( tID1 );
        if( tIdx1 >= 0 ){
            road->nodes[tIdx1]->legInfo[0]->angle = angle3 * 57.3;
            road->nodes[tIdx1]->legInfo[0]->connectedNode = nID;
            road->nodes[tIdx1]->legInfo[0]->connectedNodeOutDirect = 1;
            road->nodes[tIdx1]->legInfo[0]->connectingNode = nID;
            road->nodes[tIdx1]->legInfo[0]->connectingNodeInDirect = 1;

            road->nodes[nIdx]->legInfo[1]->connectedNode = tID1;
            road->nodes[nIdx]->legInfo[1]->connectedNodeOutDirect = 0;
            road->nodes[nIdx]->legInfo[1]->connectingNode = tID1;
            road->nodes[nIdx]->legInfo[1]->connectingNodeInDirect = 0;
        }


        nodeConnectInfo[1] = QString("%1,0").arg( tID1 );

        float xt2 = xc + cos( angle3 ) * distToTN;
        float yt2 = yc + sin( angle3 ) * distToTN;

        int tID2 = road->CreateNode( -1, xt2, yt2, inLanes, outLanes );
        int tIdx2 = road->indexOfNode( tID2 );
        if( tIdx2 >= 0 ){
            road->nodes[tIdx2]->legInfo[0]->angle = angle1 * 57.3;
            road->nodes[tIdx2]->legInfo[0]->connectedNode = nID;
            road->nodes[tIdx2]->legInfo[0]->connectedNodeOutDirect = 3;
            road->nodes[tIdx2]->legInfo[0]->connectingNode = nID;
            road->nodes[tIdx2]->legInfo[0]->connectingNodeInDirect = 3;

            road->nodes[nIdx]->legInfo[3]->connectedNode = tID2;
            road->nodes[nIdx]->legInfo[3]->connectedNodeOutDirect = 0;
            road->nodes[nIdx]->legInfo[3]->connectingNode = tID2;
            road->nodes[nIdx]->legInfo[3]->connectingNodeInDirect = 0;
        }

        nodeConnectInfo[3] = QString("%1,0").arg( tID2 );

        // Create Lanes

        float LRrev = 1.0;
        if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
            LRrev = -1.0;
        }
        float nodeRadius = 6.0;

        for(int i=0;i<4;++i){

            QVector4D startPoint;
            QVector4D endPoint;

            if( i == 0 ){

                for(int j=0;j<d0StartLanes.size();++j){

                    startPoint.setX( road->lanes[d0StartLanes[j]]->shape.pos.first()->x() );
                    startPoint.setY( road->lanes[d0StartLanes[j]]->shape.pos.first()->y() );
                    startPoint.setZ( road->lanes[d0StartLanes[j]]->shape.pos.first()->z() );
                    startPoint.setW( atan2( road->lanes[d0StartLanes[j]]->shape.derivative.first()->y(), road->lanes[d0StartLanes[j]]->shape.derivative.first()->x()) );

                    if( j == 0 ){

                        float angle = angle1;
                        if( LRrev < 0.0 ){
                            angle = angle3;
                        }
                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 1;
                        if( LRrev < 0.0 ){
                            dir = 3;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }

                    if( j == d0StartLanes.size() - 1){

                        float angle = angle3;
                        if( LRrev < 0.0 ){
                            angle = angle1;
                        }
                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 3;
                        if( LRrev < 0.0 ){
                            dir = 1;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }
                }

            }
            else if( i == 2 ){

                for(int j=0;j<d2StartLanes.size();++j){

                    startPoint.setX( road->lanes[d2StartLanes[j]]->shape.pos.first()->x() );
                    startPoint.setY( road->lanes[d2StartLanes[j]]->shape.pos.first()->y() );
                    startPoint.setZ( road->lanes[d2StartLanes[j]]->shape.pos.first()->z() );
                    startPoint.setW( atan2( road->lanes[d2StartLanes[j]]->shape.derivative.first()->y(), road->lanes[d2StartLanes[j]]->shape.derivative.first()->x()) );

                    if( j == 0 ){

                        float angle = angle3;
                        if( LRrev < 0.0 ){
                            angle = angle1;
                        }
                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 3;
                        if( LRrev < 0.0 ){
                            dir = 1;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }

                    if( j == d2StartLanes.size() - 1){

                        float angle = angle1;
                        if( LRrev < 0.0 ){
                            angle = angle3;
                        }
                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 1;
                        if( LRrev < 0.0 ){
                            dir = 3;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }
                }
            }
            else{

                float offsetLane = 0.0;

                float angle = angle1;
                if( i == 3 ){
                    angle = angle3;
                }
                float c = cos( angle );
                float s = sin( angle );

                int nInLane = 1;
                for(int j=0;j<nInLane;++j){

                    startPoint.setX( xc + nodeRadius * c + (2.75 * j + 1.375 + offsetLane) * s * LRrev  );
                    startPoint.setY( yc + nodeRadius * s - (2.75 * j + 1.375 + offsetLane) * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW(  angle - 180.0 * 0.017452 );

                    if( j == 0 ){

                        int nOutLane = d0StartLanes.size();
                        int dir = 3;
                        if( i == 3 ){
                            nOutLane = d2StartLanes.size();
                            dir = 0;
                        }

                        for(int k=0;k<nOutLane;++k){

                            if( i == 1 ){
                                endPoint.setX( road->lanes[d0StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d0StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d0StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d0StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d0StartLanes[k]]->shape.derivative.last()->x()) );
                            }
                            else{
                                endPoint.setX( road->lanes[d2StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d2StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d2StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d2StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d2StartLanes[k]]->shape.derivative.last()->x()) );
                            }

                            int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                            road->SetNodeRelatedLane( nID, lId );
                        }

                    }

                    if( j == nInLane - 1 ){

                        int nOutLane = d2StartLanes.size();
                        int dir = 0;
                        if( i == 3 ){
                            nOutLane = d0StartLanes.size();
                            dir = 3;
                        }

                        for(int k=0;k<nOutLane;++k){

                            if( i == 1 ){
                                endPoint.setX( road->lanes[d2StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d2StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d2StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d2StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d2StartLanes[k]]->shape.derivative.last()->x()) );
                            }
                            else{
                                endPoint.setX( road->lanes[d0StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d0StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d0StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d0StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d0StartLanes[k]]->shape.derivative.last()->x()) );
                            }

                            int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                            road->SetNodeRelatedLane( nID, lId );
                        }
                    }

                    angle = angle3;
                    if( i == 3 ){
                        angle = angle1;
                    }
                    float c2 = cos( angle );
                    float s2 = sin( angle );

                    endPoint.setX( xc + nodeRadius * c2 - (2.75 * j + 1.375 + offsetLane) * s2 * LRrev  );
                    endPoint.setY( yc + nodeRadius * s2 + (2.75 * j + 1.375 + offsetLane) * c2 * LRrev );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle );

                    int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, (i+2)%4, true );
                    road->SetNodeRelatedLane( nID, lId );
                }
            }
        }

        for(int i=1;i<4;i+=2){

            int nInLane = 1;
            for(int k=0;k<nInLane;++k){

                QVector4D startPoint;
                QVector4D endPoint;

                float angle = angle1;
                if( i == 3 ){
                    angle = angle3;
                }

                float c = cos( angle );
                float s = sin( angle );

                float offsetLane2 = 0.0;

                // Lanes to terminal nodes
                startPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                startPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle );

                float L = distToTN - nodeRadius - 5.0;
                endPoint.setX( startPoint.x() + L * c );
                endPoint.setY( startPoint.y() + L * s );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle );

                int tID = tID1;
                if( i == 3 ){
                    tID = tID2;
                }
                int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, tID, 0, true );
                road->SetNodeRelatedLane( nID, lId );
                road->SetNodeRelatedLane( tID, lId );
            }


            int nOutLane = 1;
            int nLaneCreated = 0;
            for(int k=0;k<nOutLane;++k){

                QVector4D startPoint;
                QVector4D endPoint;

                float angle = angle1;
                if( i == 3 ){
                    angle = angle3;
                }
                float c = cos( angle );
                float s = sin( angle );

                float offsetLane = 0.0;


                // Lanes from terminal nodes
                endPoint.setX( xc + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                endPoint.setY( yc + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                endPoint.setZ( 0.0 );
                endPoint.setW(  angle - 180.0 * 0.017452 );

                float L = distToTN - nodeRadius - 5.0;
                startPoint.setX( endPoint.x() + L * c );
                startPoint.setY( endPoint.y() + L * s );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle - 180.0 * 0.017452 );

                int tID = tID1;
                if( i == 3 ){
                    tID = tID2;
                }
                int lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, nID, i, true );
                road->SetNodeRelatedLane( nID, lId );
                road->SetNodeRelatedLane( tID, lId );

                nLaneCreated++;
            }
        }
    }

    CheckRouteChangeByInsertNode( nID, edgeNodeInfo );

    road->CheckLaneConnectionFull();

    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();
    canvas->update();
}


void DataManipulator::CheckRouteChangeByInsertNode(int insertNodeID, QStringList edgeNodeInfo)
{
    int nIdx = road->indexOfNode( insertNodeID );
    if( nIdx < 0 ){
        return;
    }

    for(int i=0;i<edgeNodeInfo.size();++i){

        QStringList divStr = QString( edgeNodeInfo[i] ).split(",");
        int fromNode       = QString(divStr[0]).toInt();
        int fromNodeOutDir = QString(divStr[1]).toInt();
        int toNode         = QString(divStr[2]).toInt();
        int toNodeInDir    = QString(divStr[3]).toInt();

        int inDirNewNode = -1;
        int outDirNewNode = -1;
        for(int j=0;j<road->nodes[nIdx]->legInfo.size();++j){
            if( road->nodes[nIdx]->legInfo[j]->connectedNode == fromNode &&
                    road->nodes[nIdx]->legInfo[j]->connectedNodeOutDirect == fromNodeOutDir ){
                inDirNewNode = j;
            }
            if( road->nodes[nIdx]->legInfo[j]->connectingNode == toNode &&
                    road->nodes[nIdx]->legInfo[j]->connectingNodeInDirect == toNodeInDir ){
                outDirNewNode = j;
            }
        }

        if( fromNode >= 0 && fromNodeOutDir >= 0 &&
                toNode >= 0 && toNodeInDir >= 0 &&
                inDirNewNode >= 0 && outDirNewNode >= 0 ){

            for(int j=0;j<road->nodes.size();++j){
                if( road->nodes[j]->isOriginNode == false ){
                    continue;
                }
                if( road->nodes[j]->odData.size() == 0 ){
                    continue;
                }

                for(int k=0;k<road->nodes[j]->odData.size();++k){
                    for(int l=0;l<road->nodes[j]->odData[k]->route.size();++l){
                        bool matched = true;
                        while(matched){
                            matched = false;
                            for(int m=1;m<road->nodes[j]->odData[k]->route[l]->nodeList.size();++m){
                                if( road->nodes[j]->odData[k]->route[l]->nodeList[m-1]->node == fromNode &&
                                        road->nodes[j]->odData[k]->route[l]->nodeList[m-1]->outDir == fromNodeOutDir &&
                                        road->nodes[j]->odData[k]->route[l]->nodeList[m]->node == toNode &&
                                        road->nodes[j]->odData[k]->route[l]->nodeList[m]->inDir == toNodeInDir ){
                                   matched = true;
                                   struct RouteElem *re = new struct RouteElem;
                                   re->node = insertNodeID;
                                   re->inDir = inDirNewNode;
                                   re->outDir = outDirNewNode;
                                   road->nodes[j]->odData[k]->route[l]->nodeList.insert(m, re);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


void DataManipulator::InsertNode_4x2_noTS()
{

    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();
    canvas->update();
}


void DataManipulator::InsertNode_3Lx1_noTS()
{
    int nPoint = 0;
    float xc = 0.0;
    float yc = 0.0;

    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){

            xc += road->lanes[lidx]->shape.pos.first()->x();
            yc += road->lanes[lidx]->shape.pos.first()->y();
            nPoint++;

            xc += road->lanes[lidx]->shape.pos.last()->x();
            yc += road->lanes[lidx]->shape.pos.last()->y();
            nPoint++;
        }
    }
    if( nPoint == 0 ){
        return;
    }
    xc /= nPoint;
    yc /= nPoint;

    qDebug() << " xc = " << xc << " yc = " << yc;

    QStringList edgeNodeInfo;
    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){

            QString eni = QString("%1,%2,%3,%4")
                    .arg(road->lanes[lidx]->sWPInNode)
                    .arg(road->lanes[lidx]->sWPNodeDir)
                    .arg(road->lanes[lidx]->eWPInNode)
                    .arg(road->lanes[lidx]->eWPNodeDir);

            if( edgeNodeInfo.contains(eni) == false ){
                edgeNodeInfo.append( eni );
            }
        }
    }

    QStringList nodeConnectInfo;

    QList<int> d0StartLanes;

    float angle0 = 0.0;
    int nIn0 = 0;
    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){
            float rx = road->lanes[lidx]->shape.pos.first()->x() - xc;
            if( rx > 0.0 ){

                d0StartLanes.append( lidx );

                if( nIn0 == 0 ){
                    angle0 = atan2( -road->lanes[lidx]->shape.derivative.first()->y(), -road->lanes[lidx]->shape.derivative.first()->x() );

                    QString ncStr = QString("%1,%2").arg( road->lanes[lidx]->sWPInNode ).arg( road->lanes[lidx]->sWPNodeDir );
                    nodeConnectInfo.append( ncStr );
                }
                nIn0++;
            }
        }
    }
    if( nIn0 == 0 ){
        for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
            int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
            if( lidx >= 0 ){
                float rx = road->lanes[lidx]->shape.pos.last()->x() - xc;
                if( rx < 0.0 ){

                    d0StartLanes.append( lidx );

                    if( nIn0 == 0 ){
                        angle0 = atan2( road->lanes[lidx]->shape.derivative.last()->y(), road->lanes[lidx]->shape.derivative.last()->x() );

                        QString ncStr = QString("%1,%2").arg( road->lanes[lidx]->eWPInNode ).arg( road->lanes[lidx]->eWPNodeDir );
                        nodeConnectInfo.append( ncStr );
                    }
                    nIn0++;
                }
            }
        }
    }

    nodeConnectInfo.append( QString("-1,0") );

    QList<int> d2StartLanes;

    float angle2 = 0.0;
    int nIn2 = 0;
    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){
            float rx = road->lanes[lidx]->shape.pos.first()->x() - xc;
            if( rx < 0.0 ){

                d2StartLanes.append( lidx );

                if( nIn2 == 0 ){
                    angle2 = atan2( -road->lanes[lidx]->shape.derivative.first()->y(), -road->lanes[lidx]->shape.derivative.first()->x() );

                    QString ncStr = QString("%1,%2").arg( road->lanes[lidx]->sWPInNode ).arg( road->lanes[lidx]->sWPNodeDir );
                    nodeConnectInfo.append( ncStr );
                }
                nIn2++;
            }
        }
    }
    if( nIn2 == 0 ){
        for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
            int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
            if( lidx >= 0 ){
                float rx = road->lanes[lidx]->shape.pos.last()->x() - xc;
                if( rx > 0.0 ){

                    d2StartLanes.append( lidx );

                    if( nIn2 == 0 ){
                        angle2 = atan2( road->lanes[lidx]->shape.derivative.last()->y(), road->lanes[lidx]->shape.derivative.last()->x() );

                        QString ncStr = QString("%1,%2").arg( road->lanes[lidx]->eWPInNode ).arg( road->lanes[lidx]->eWPNodeDir );
                        nodeConnectInfo.append( ncStr );
                    }
                    nIn2++;
                }
            }
        }
    }

    if( cos(angle0) * cos(angle2) + sin(angle0) * sin(angle2) > 0.0 ){
        angle2 = angle0 + 3.141592;
        if( angle2 > 3.141592 ){
            angle2 -= 6.28272;
        }
        else if( angle2 < -3.141592 ){
            angle2 += 6.28272;
        }
    }

    qDebug() << " angle0 = " << angle0 * 57.3;
    qDebug() << " angle2 = " << angle2 * 57.3;

    float angle1 = angle0 + 1.5708;
    if( angle1 > 3.141592 ){
        angle1 -= 6.28272;
    }
    else if( angle1 < -3.141592 ){
        angle1 += 6.28272;
    }


    qDebug() << " angle1 = " << angle1 * 57.3;
    qDebug() << " nodeConnectInfo = " << nodeConnectInfo;

    QList<int> inLanes;
    QList<int> outLanes;

    inLanes << nIn0 << 1 << nIn2;
    outLanes << nIn2 << 1 << nIn0;

    int nID = road->CreateNode( -1, xc, yc, inLanes, outLanes );

    int nIdx = road->indexOfNode( nID );
    if( nIdx >= 0 ){

        road->nodes[nIdx]->legInfo[0]->angle = angle0 * 57.3;
        road->nodes[nIdx]->legInfo[1]->angle = angle1 * 57.3;
        road->nodes[nIdx]->legInfo[2]->angle = angle2 * 57.3;


        QStringList divStr0 = QString(nodeConnectInfo[0]).split(",");
        road->nodes[nIdx]->legInfo[0]->connectedNode = QString(divStr0[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[0]->connectedNodeOutDirect = QString(divStr0[1]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[0]->connectingNode = QString(divStr0[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[0]->connectingNodeInDirect = QString(divStr0[1]).trimmed().toInt();

        int cN0idx = road->indexOfNode( road->nodes[nIdx]->legInfo[0]->connectedNode );
        if( cN0idx >= 0 ){
            int dir = road->nodes[nIdx]->legInfo[0]->connectedNodeOutDirect;
            road->nodes[cN0idx]->legInfo[dir]->connectedNode = nID;
            road->nodes[cN0idx]->legInfo[dir]->connectedNodeOutDirect = 0;
            road->nodes[cN0idx]->legInfo[dir]->connectingNode = nID;
            road->nodes[cN0idx]->legInfo[dir]->connectingNodeInDirect = 0;

            for(int i=0;i<d0StartLanes.size();++i){
                int rlIdx = road->nodes[cN0idx]->relatedLanes.indexOf( road->lanes[d0StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN0idx]->relatedLanes.removeAt( rlIdx );
                }
            }

            for(int i=0;i<d2StartLanes.size();++i){
                int rlIdx = road->nodes[cN0idx]->relatedLanes.indexOf( road->lanes[d2StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN0idx]->relatedLanes.removeAt( rlIdx );
                }
            }
        }


        QStringList divStr2 = QString(nodeConnectInfo[2]).split(",");
        road->nodes[nIdx]->legInfo[2]->connectedNode = QString(divStr2[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[2]->connectedNodeOutDirect = QString(divStr2[1]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[2]->connectingNode = QString(divStr2[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[2]->connectingNodeInDirect = QString(divStr2[1]).trimmed().toInt();

        int cN2idx = road->indexOfNode( road->nodes[nIdx]->legInfo[2]->connectedNode );
        if( cN2idx >= 0 ){
            int dir = road->nodes[nIdx]->legInfo[2]->connectedNodeOutDirect;
            road->nodes[cN2idx]->legInfo[dir]->connectedNode = nID;
            road->nodes[cN2idx]->legInfo[dir]->connectedNodeOutDirect = 2;
            road->nodes[cN2idx]->legInfo[dir]->connectingNode = nID;
            road->nodes[cN2idx]->legInfo[dir]->connectingNodeInDirect = 2;

            for(int i=0;i<d0StartLanes.size();++i){
                int rlIdx = road->nodes[cN2idx]->relatedLanes.indexOf( road->lanes[d0StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN2idx]->relatedLanes.removeAt( rlIdx );
                }
            }

            for(int i=0;i<d2StartLanes.size();++i){
                int rlIdx = road->nodes[cN2idx]->relatedLanes.indexOf( road->lanes[d2StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN2idx]->relatedLanes.removeAt( rlIdx );
                }
            }
        }

        for(int i=0;i<d0StartLanes.size();++i){
            int lidx = d0StartLanes[i];
            road->lanes[lidx]->sWPInNode = nID;
            road->lanes[lidx]->sWPNodeDir = 0;
            road->lanes[lidx]->sWPBoundary = true;
            road->lanes[lidx]->eWPInNode = nID;
            road->lanes[lidx]->eWPNodeDir = 2;
            road->lanes[lidx]->eWPBoundary = true;

            road->nodes[nIdx]->relatedLanes.append( road->lanes[lidx]->id );

            depthCount = 0;
            ChangeRelatedLanesForPreviousLanes(lidx, nID, 0, road->nodes[nIdx]->legInfo[2]->connectedNode );

            depthCount = 0;
            ChangeRelatedLanesForNextLanes(lidx, nID, 2, road->nodes[nIdx]->legInfo[0]->connectedNode );
        }


        for(int i=0;i<d2StartLanes.size();++i){
            int lidx = d2StartLanes[i];
            road->lanes[lidx]->sWPInNode = nID;
            road->lanes[lidx]->sWPNodeDir = 2;
            road->lanes[lidx]->sWPBoundary = true;
            road->lanes[lidx]->eWPInNode = nID;
            road->lanes[lidx]->eWPNodeDir = 0;
            road->lanes[lidx]->eWPBoundary = true;

            road->nodes[nIdx]->relatedLanes.append( road->lanes[lidx]->id );

            depthCount = 0;
            ChangeRelatedLanesForPreviousLanes(lidx, nID, 2, road->nodes[nIdx]->legInfo[0]->connectedNode );

            depthCount = 0;
            ChangeRelatedLanesForNextLanes(lidx, nID, 0, road->nodes[nIdx]->legInfo[2]->connectedNode );
        }


        // Create Terminal Nodes
        float distToTN = 50.0;

        inLanes.clear();
        outLanes.clear();

        inLanes << 1;
        outLanes << 1;

        float xt1 = xc + cos( angle1 ) * distToTN;
        float yt1 = yc + sin( angle1 ) * distToTN;

        int tID1 = road->CreateNode( -1, xt1, yt1, inLanes, outLanes );
        int tIdx1 = road->indexOfNode( tID1 );
        if( tIdx1 >= 0 ){
            road->nodes[tIdx1]->legInfo[0]->angle = angle1 * 57.3 - 180.0;
            if( road->nodes[tIdx1]->legInfo[0]->angle > 180.0 ){
                road->nodes[tIdx1]->legInfo[0]->angle -= 360.0;
            }
            else if( road->nodes[tIdx1]->legInfo[0]->angle < -180.0 ){
                road->nodes[tIdx1]->legInfo[0]->angle += 360.0;
            }
            road->nodes[tIdx1]->legInfo[0]->connectedNode = nID;
            road->nodes[tIdx1]->legInfo[0]->connectedNodeOutDirect = 1;
            road->nodes[tIdx1]->legInfo[0]->connectingNode = nID;
            road->nodes[tIdx1]->legInfo[0]->connectingNodeInDirect = 1;

            road->nodes[nIdx]->legInfo[1]->connectedNode = tID1;
            road->nodes[nIdx]->legInfo[1]->connectedNodeOutDirect = 0;
            road->nodes[nIdx]->legInfo[1]->connectingNode = tID1;
            road->nodes[nIdx]->legInfo[1]->connectingNodeInDirect = 0;
        }

        nodeConnectInfo[1] = QString("%1,0").arg( tID1 );


        // Create Lanes

        float LRrev = 1.0;
        if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
            LRrev = -1.0;
        }
        float nodeRadius = 6.0;

        for(int i=0;i<3;++i){

            QVector4D startPoint;
            QVector4D endPoint;

            if( i == 0 ){

                for(int j=0;j<d0StartLanes.size();++j){

                    startPoint.setX( road->lanes[d0StartLanes[j]]->shape.pos.first()->x() );
                    startPoint.setY( road->lanes[d0StartLanes[j]]->shape.pos.first()->y() );
                    startPoint.setZ( road->lanes[d0StartLanes[j]]->shape.pos.first()->z() );
                    startPoint.setW( atan2( road->lanes[d0StartLanes[j]]->shape.derivative.first()->y(), road->lanes[d0StartLanes[j]]->shape.derivative.first()->x()) );

                    if( j == 0 && LRrev > 0.0 ){

                        float angle = angle1;

                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 1;
                        if( LRrev < 0.0 ){
                            dir = 3;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }

                    if( j == d0StartLanes.size() - 1 && LRrev < 0.0 ){

                        float angle = angle1;
                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 3;
                        if( LRrev < 0.0 ){
                            dir = 1;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }
                }

            }
            else if( i == 2 ){

                for(int j=0;j<d2StartLanes.size();++j){

                    startPoint.setX( road->lanes[d2StartLanes[j]]->shape.pos.first()->x() );
                    startPoint.setY( road->lanes[d2StartLanes[j]]->shape.pos.first()->y() );
                    startPoint.setZ( road->lanes[d2StartLanes[j]]->shape.pos.first()->z() );
                    startPoint.setW( atan2( road->lanes[d2StartLanes[j]]->shape.derivative.first()->y(), road->lanes[d2StartLanes[j]]->shape.derivative.first()->x()) );

                    if( j == 0 && LRrev < 0.0 ){

                        float angle = angle1;
                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 3;
                        if( LRrev < 0.0 ){
                            dir = 1;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }

                    if( j == d2StartLanes.size() - 1 && LRrev > 0.0 ){

                        float angle = angle1;
                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 1;
                        if( LRrev < 0.0 ){
                            dir = 3;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }
                }
            }
            else{

                float offsetLane = 0.0;

                float angle = angle1;
                float c = cos( angle );
                float s = sin( angle );

                int nInLane = 1;
                for(int j=0;j<nInLane;++j){

                    startPoint.setX( xc + nodeRadius * c + (2.75 * j + 1.375 + offsetLane) * s * LRrev  );
                    startPoint.setY( yc + nodeRadius * s - (2.75 * j + 1.375 + offsetLane) * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW(  angle - 180.0 * 0.017452 );

                    if( j == 0 ){

                        int nOutLane = d0StartLanes.size();
                        int dir = 3;
                        if( i == 3 ){
                            nOutLane = d2StartLanes.size();
                            dir = 0;
                        }

                        for(int k=0;k<nOutLane;++k){

                            if( i == 1 ){
                                endPoint.setX( road->lanes[d0StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d0StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d0StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d0StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d0StartLanes[k]]->shape.derivative.last()->x()) );
                            }
                            else{
                                endPoint.setX( road->lanes[d2StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d2StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d2StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d2StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d2StartLanes[k]]->shape.derivative.last()->x()) );
                            }

                            int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                            road->SetNodeRelatedLane( nID, lId );
                        }

                    }

                    if( j == nInLane - 1 ){

                        int nOutLane = d2StartLanes.size();
                        int dir = 0;
                        if( i == 3 ){
                            nOutLane = d0StartLanes.size();
                            dir = 3;
                        }

                        for(int k=0;k<nOutLane;++k){

                            if( i == 1 ){
                                endPoint.setX( road->lanes[d2StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d2StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d2StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d2StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d2StartLanes[k]]->shape.derivative.last()->x()) );
                            }
                            else{
                                endPoint.setX( road->lanes[d0StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d0StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d0StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d0StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d0StartLanes[k]]->shape.derivative.last()->x()) );
                            }

                            int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                            road->SetNodeRelatedLane( nID, lId );
                        }
                    }
                }
            }
        }

        {
            int i = 1;

            int nInLane = 1;
            for(int k=0;k<nInLane;++k){

                QVector4D startPoint;
                QVector4D endPoint;

                float angle = angle1;
                float c = cos( angle );
                float s = sin( angle );

                float offsetLane2 = 0.0;

                // Lanes to terminal nodes
                startPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                startPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle );

                float L = distToTN - nodeRadius - 5.0;
                endPoint.setX( startPoint.x() + L * c );
                endPoint.setY( startPoint.y() + L * s );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle );

                int tID = tID1;
                int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, tID, 0, true );
                road->SetNodeRelatedLane( nID, lId );
                road->SetNodeRelatedLane( tID, lId );
            }


            int nOutLane = 1;
            int nLaneCreated = 0;
            for(int k=0;k<nOutLane;++k){

                QVector4D startPoint;
                QVector4D endPoint;

                float angle = angle1;
                float c = cos( angle );
                float s = sin( angle );

                float offsetLane = 0.0;

                // Lanes from terminal nodes
                endPoint.setX( xc + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                endPoint.setY( yc + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                endPoint.setZ( 0.0 );
                endPoint.setW(  angle - 180.0 * 0.017452 );

                float L = distToTN - nodeRadius - 5.0;
                startPoint.setX( endPoint.x() + L * c );
                startPoint.setY( endPoint.y() + L * s );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle - 180.0 * 0.017452 );

                int tID = tID1;
                int lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, nID, i, true );
                road->SetNodeRelatedLane( nID, lId );
                road->SetNodeRelatedLane( tID, lId );

                nLaneCreated++;
            }
        }
    }

    CheckRouteChangeByInsertNode( nID, edgeNodeInfo );

    road->CheckLaneConnectionFull();

    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();
    canvas->update();
}


void DataManipulator::InsertNode_3Lx2_noTS()
{

    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();
    canvas->update();
}


void DataManipulator::InsertNode_3Rx1_noTS()
{
    int nPoint = 0;
    float xc = 0.0;
    float yc = 0.0;

    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){

            xc += road->lanes[lidx]->shape.pos.first()->x();
            yc += road->lanes[lidx]->shape.pos.first()->y();
            nPoint++;

            xc += road->lanes[lidx]->shape.pos.last()->x();
            yc += road->lanes[lidx]->shape.pos.last()->y();
            nPoint++;
        }
    }
    if( nPoint == 0 ){
        return;
    }
    xc /= nPoint;
    yc /= nPoint;

    qDebug() << " xc = " << xc << " yc = " << yc;

    QStringList edgeNodeInfo;
    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){

            QString eni = QString("%1,%2,%3,%4")
                    .arg(road->lanes[lidx]->sWPInNode)
                    .arg(road->lanes[lidx]->sWPNodeDir)
                    .arg(road->lanes[lidx]->eWPInNode)
                    .arg(road->lanes[lidx]->eWPNodeDir);

            if( edgeNodeInfo.contains(eni) == false ){
                edgeNodeInfo.append( eni );
            }
        }
    }

    QStringList nodeConnectInfo;

    QList<int> d0StartLanes;

    float angle0 = 0.0;
    int nIn0 = 0;
    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){
            float rx = road->lanes[lidx]->shape.pos.first()->x() - xc;
            if( rx < 0.0 ){

                d0StartLanes.append( lidx );

                if( nIn0 == 0 ){
                    angle0 = atan2( -road->lanes[lidx]->shape.derivative.first()->y(), -road->lanes[lidx]->shape.derivative.first()->x() );

                    QString ncStr = QString("%1,%2").arg( road->lanes[lidx]->sWPInNode ).arg( road->lanes[lidx]->sWPNodeDir );
                    nodeConnectInfo.append( ncStr );
                }
                nIn0++;
            }
        }
    }
    if( nIn0 == 0 ){
        for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
            int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
            if( lidx >= 0 ){
                float rx = road->lanes[lidx]->shape.pos.last()->x() - xc;
                if( rx < 0.0 ){

                    d0StartLanes.append( lidx );

                    if( nIn0 == 0 ){
                        angle0 = atan2( road->lanes[lidx]->shape.derivative.last()->y(), road->lanes[lidx]->shape.derivative.last()->x() );

                        QString ncStr = QString("%1,%2").arg( road->lanes[lidx]->eWPInNode ).arg( road->lanes[lidx]->eWPNodeDir );
                        nodeConnectInfo.append( ncStr );
                    }
                    nIn0++;
                }
            }
        }
    }

    nodeConnectInfo.append( QString("-1,0") );

    QList<int> d2StartLanes;

    float angle2 = 0.0;
    int nIn2 = 0;
    for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
        int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
        if( lidx >= 0 ){
            float rx = road->lanes[lidx]->shape.pos.first()->x() - xc;
            if( rx > 0.0 ){

                d2StartLanes.append( lidx );

                if( nIn2 == 0 ){
                    angle2 = atan2( -road->lanes[lidx]->shape.derivative.first()->y(), -road->lanes[lidx]->shape.derivative.first()->x() );

                    QString ncStr = QString("%1,%2").arg( road->lanes[lidx]->sWPInNode ).arg( road->lanes[lidx]->sWPNodeDir );
                    nodeConnectInfo.append( ncStr );
                }
                nIn2++;
            }
        }
    }
    if( nIn2 == 0 ){
        for(int i=0;i<canvas->selectedObj.selObjID.size();++i){
            int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
            if( lidx >= 0 ){
                float rx = road->lanes[lidx]->shape.pos.last()->x() - xc;
                if( rx > 0.0 ){

                    d2StartLanes.append( lidx );

                    if( nIn2 == 0 ){
                        angle2 = atan2( road->lanes[lidx]->shape.derivative.last()->y(), road->lanes[lidx]->shape.derivative.last()->x() );

                        QString ncStr = QString("%1,%2").arg( road->lanes[lidx]->eWPInNode ).arg( road->lanes[lidx]->eWPNodeDir );
                        nodeConnectInfo.append( ncStr );
                    }
                    nIn2++;
                }
            }
        }
    }

    if( cos(angle0) * cos(angle2) + sin(angle0) * sin(angle2) > 0.0 ){
        angle2 = angle0 + 3.141592;
        if( angle2 > 3.141592 ){
            angle2 -= 6.28272;
        }
        else if( angle2 < -3.141592 ){
            angle2 += 6.28272;
        }
    }

    qDebug() << " angle0 = " << angle0 * 57.3;
    qDebug() << " angle2 = " << angle2 * 57.3;

    float angle1 = angle0 + 1.5708;
    if( angle1 > 3.141592 ){
        angle1 -= 6.28272;
    }
    else if( angle1 < -3.141592 ){
        angle1 += 6.28272;
    }


    qDebug() << " angle1 = " << angle1 * 57.3;
    qDebug() << " nodeConnectInfo = " << nodeConnectInfo;

    QList<int> inLanes;
    QList<int> outLanes;

    inLanes << nIn0 << 1 << nIn2;
    outLanes << nIn2 << 1 << nIn0;

    int nID = road->CreateNode( -1, xc, yc, inLanes, outLanes );

    int nIdx = road->indexOfNode( nID );
    if( nIdx >= 0 ){

        road->nodes[nIdx]->legInfo[0]->angle = angle0 * 57.3;
        road->nodes[nIdx]->legInfo[1]->angle = angle1 * 57.3;
        road->nodes[nIdx]->legInfo[2]->angle = angle2 * 57.3;


        QStringList divStr0 = QString(nodeConnectInfo[0]).split(",");
        road->nodes[nIdx]->legInfo[0]->connectedNode = QString(divStr0[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[0]->connectedNodeOutDirect = QString(divStr0[1]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[0]->connectingNode = QString(divStr0[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[0]->connectingNodeInDirect = QString(divStr0[1]).trimmed().toInt();

        int cN0idx = road->indexOfNode( road->nodes[nIdx]->legInfo[0]->connectedNode );
        if( cN0idx >= 0 ){
            int dir = road->nodes[nIdx]->legInfo[0]->connectedNodeOutDirect;
            road->nodes[cN0idx]->legInfo[dir]->connectedNode = nID;
            road->nodes[cN0idx]->legInfo[dir]->connectedNodeOutDirect = 0;
            road->nodes[cN0idx]->legInfo[dir]->connectingNode = nID;
            road->nodes[cN0idx]->legInfo[dir]->connectingNodeInDirect = 0;

            for(int i=0;i<d0StartLanes.size();++i){
                int rlIdx = road->nodes[cN0idx]->relatedLanes.indexOf( road->lanes[d0StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN0idx]->relatedLanes.removeAt( rlIdx );
                }
            }

            for(int i=0;i<d2StartLanes.size();++i){
                int rlIdx = road->nodes[cN0idx]->relatedLanes.indexOf( road->lanes[d2StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN0idx]->relatedLanes.removeAt( rlIdx );
                }
            }
        }


        QStringList divStr2 = QString(nodeConnectInfo[2]).split(",");
        road->nodes[nIdx]->legInfo[2]->connectedNode = QString(divStr2[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[2]->connectedNodeOutDirect = QString(divStr2[1]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[2]->connectingNode = QString(divStr2[0]).trimmed().toInt();
        road->nodes[nIdx]->legInfo[2]->connectingNodeInDirect = QString(divStr2[1]).trimmed().toInt();

        int cN2idx = road->indexOfNode( road->nodes[nIdx]->legInfo[2]->connectedNode );
        if( cN2idx >= 0 ){
            int dir = road->nodes[nIdx]->legInfo[2]->connectedNodeOutDirect;
            road->nodes[cN2idx]->legInfo[dir]->connectedNode = nID;
            road->nodes[cN2idx]->legInfo[dir]->connectedNodeOutDirect = 2;
            road->nodes[cN2idx]->legInfo[dir]->connectingNode = nID;
            road->nodes[cN2idx]->legInfo[dir]->connectingNodeInDirect = 2;

            for(int i=0;i<d0StartLanes.size();++i){
                int rlIdx = road->nodes[cN2idx]->relatedLanes.indexOf( road->lanes[d0StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN2idx]->relatedLanes.removeAt( rlIdx );
                }
            }

            for(int i=0;i<d2StartLanes.size();++i){
                int rlIdx = road->nodes[cN2idx]->relatedLanes.indexOf( road->lanes[d2StartLanes[i]]->id );
                if( rlIdx >= 0 ){
                    road->nodes[cN2idx]->relatedLanes.removeAt( rlIdx );
                }
            }
        }

        for(int i=0;i<d0StartLanes.size();++i){
            int lidx = d0StartLanes[i];
            road->lanes[lidx]->sWPInNode = nID;
            road->lanes[lidx]->sWPNodeDir = 0;
            road->lanes[lidx]->sWPBoundary = true;
            road->lanes[lidx]->eWPInNode = nID;
            road->lanes[lidx]->eWPNodeDir = 2;
            road->lanes[lidx]->eWPBoundary = true;

            road->nodes[nIdx]->relatedLanes.append( road->lanes[lidx]->id );

            depthCount = 0;
            ChangeRelatedLanesForPreviousLanes(lidx, nID, 0, road->nodes[nIdx]->legInfo[2]->connectedNode );

            depthCount = 0;
            ChangeRelatedLanesForNextLanes(lidx, nID, 2, road->nodes[nIdx]->legInfo[0]->connectedNode );
        }


        for(int i=0;i<d2StartLanes.size();++i){
            int lidx = d2StartLanes[i];
            road->lanes[lidx]->sWPInNode = nID;
            road->lanes[lidx]->sWPNodeDir = 2;
            road->lanes[lidx]->sWPBoundary = true;
            road->lanes[lidx]->eWPInNode = nID;
            road->lanes[lidx]->eWPNodeDir = 0;
            road->lanes[lidx]->eWPBoundary = true;

            road->nodes[nIdx]->relatedLanes.append( road->lanes[lidx]->id );

            depthCount = 0;
            ChangeRelatedLanesForPreviousLanes(lidx, nID, 2, road->nodes[nIdx]->legInfo[0]->connectedNode );

            depthCount = 0;
            ChangeRelatedLanesForNextLanes(lidx, nID, 0, road->nodes[nIdx]->legInfo[2]->connectedNode );
        }


        // Create Terminal Nodes
        float distToTN = 50.0;

        inLanes.clear();
        outLanes.clear();

        inLanes << 1;
        outLanes << 1;

        float xt1 = xc + cos( angle1 ) * distToTN;
        float yt1 = yc + sin( angle1 ) * distToTN;

        int tID1 = road->CreateNode( -1, xt1, yt1, inLanes, outLanes );
        int tIdx1 = road->indexOfNode( tID1 );
        if( tIdx1 >= 0 ){
            road->nodes[tIdx1]->legInfo[0]->angle = angle1 * 57.3 - 180.0;
            if( road->nodes[tIdx1]->legInfo[0]->angle > 180.0 ){
                road->nodes[tIdx1]->legInfo[0]->angle -= 360.0;
            }
            else if( road->nodes[tIdx1]->legInfo[0]->angle < -180.0 ){
                road->nodes[tIdx1]->legInfo[0]->angle += 360.0;
            }
            road->nodes[tIdx1]->legInfo[0]->connectedNode = nID;
            road->nodes[tIdx1]->legInfo[0]->connectedNodeOutDirect = 1;
            road->nodes[tIdx1]->legInfo[0]->connectingNode = nID;
            road->nodes[tIdx1]->legInfo[0]->connectingNodeInDirect = 1;

            road->nodes[nIdx]->legInfo[1]->connectedNode = tID1;
            road->nodes[nIdx]->legInfo[1]->connectedNodeOutDirect = 0;
            road->nodes[nIdx]->legInfo[1]->connectingNode = tID1;
            road->nodes[nIdx]->legInfo[1]->connectingNodeInDirect = 0;
        }

        nodeConnectInfo[1] = QString("%1,0").arg( tID1 );


        // Create Lanes

        float LRrev = 1.0;
        if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
            LRrev = -1.0;
        }
        float nodeRadius = 6.0;

        for(int i=0;i<3;++i){

            QVector4D startPoint;
            QVector4D endPoint;

            if( i == 0 ){

                for(int j=0;j<d0StartLanes.size();++j){

                    startPoint.setX( road->lanes[d0StartLanes[j]]->shape.pos.first()->x() );
                    startPoint.setY( road->lanes[d0StartLanes[j]]->shape.pos.first()->y() );
                    startPoint.setZ( road->lanes[d0StartLanes[j]]->shape.pos.first()->z() );
                    startPoint.setW( atan2( road->lanes[d0StartLanes[j]]->shape.derivative.first()->y(), road->lanes[d0StartLanes[j]]->shape.derivative.first()->x()) );

                    if( j == 0 && LRrev > 0.0 ){

                        float angle = angle1;

                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 1;
                        if( LRrev < 0.0 ){
                            dir = 3;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }

                    if( j == d0StartLanes.size() - 1 && LRrev < 0.0 ){

                        float angle = angle1;
                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 3;
                        if( LRrev < 0.0 ){
                            dir = 1;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }
                }

            }
            else if( i == 2 ){

                for(int j=0;j<d2StartLanes.size();++j){

                    startPoint.setX( road->lanes[d2StartLanes[j]]->shape.pos.first()->x() );
                    startPoint.setY( road->lanes[d2StartLanes[j]]->shape.pos.first()->y() );
                    startPoint.setZ( road->lanes[d2StartLanes[j]]->shape.pos.first()->z() );
                    startPoint.setW( atan2( road->lanes[d2StartLanes[j]]->shape.derivative.first()->y(), road->lanes[d2StartLanes[j]]->shape.derivative.first()->x()) );

                    if( j == 0 && LRrev < 0.0 ){

                        float angle = angle1;
                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 3;
                        if( LRrev < 0.0 ){
                            dir = 1;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }

                    if( j == d2StartLanes.size() - 1 && LRrev > 0.0 ){

                        float angle = angle1;
                        float c = cos( angle );
                        float s = sin( angle );

                        int k = 0;
                        float offsetLane2 = 0.0;
                        endPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                        endPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int dir = 1;
                        if( LRrev < 0.0 ){
                            dir = 3;
                        }
                        int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                        road->SetNodeRelatedLane( nID, lId );
                    }
                }
            }
            else{

                float offsetLane = 0.0;

                float angle = angle1;
                float c = cos( angle );
                float s = sin( angle );

                int nInLane = 1;
                for(int j=0;j<nInLane;++j){

                    startPoint.setX( xc + nodeRadius * c + (2.75 * j + 1.375 + offsetLane) * s * LRrev  );
                    startPoint.setY( yc + nodeRadius * s - (2.75 * j + 1.375 + offsetLane) * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW(  angle - 180.0 * 0.017452 );

                    if( j == 0 ){

                        int nOutLane = d0StartLanes.size();
                        int dir = 3;
                        if( i == 3 ){
                            nOutLane = d2StartLanes.size();
                            dir = 0;
                        }

                        for(int k=0;k<nOutLane;++k){

                            if( i == 1 ){
                                endPoint.setX( road->lanes[d0StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d0StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d0StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d0StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d0StartLanes[k]]->shape.derivative.last()->x()) );
                            }
                            else{
                                endPoint.setX( road->lanes[d2StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d2StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d2StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d2StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d2StartLanes[k]]->shape.derivative.last()->x()) );
                            }

                            int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                            road->SetNodeRelatedLane( nID, lId );
                        }

                    }

                    if( j == nInLane - 1 ){

                        int nOutLane = d2StartLanes.size();
                        int dir = 0;
                        if( i == 3 ){
                            nOutLane = d0StartLanes.size();
                            dir = 3;
                        }

                        for(int k=0;k<nOutLane;++k){

                            if( i == 1 ){
                                endPoint.setX( road->lanes[d2StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d2StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d2StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d2StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d2StartLanes[k]]->shape.derivative.last()->x()) );
                            }
                            else{
                                endPoint.setX( road->lanes[d0StartLanes[k]]->shape.pos.last()->x() );
                                endPoint.setY( road->lanes[d0StartLanes[k]]->shape.pos.last()->y() );
                                endPoint.setZ( road->lanes[d0StartLanes[k]]->shape.pos.last()->z() );
                                endPoint.setW( atan2( road->lanes[d0StartLanes[k]]->shape.derivative.last()->y(), road->lanes[d0StartLanes[k]]->shape.derivative.last()->x()) );
                            }

                            int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, nID, dir, true );
                            road->SetNodeRelatedLane( nID, lId );
                        }
                    }
                }
            }
        }

        {
            int i = 1;

            int nInLane = 1;
            for(int k=0;k<nInLane;++k){

                QVector4D startPoint;
                QVector4D endPoint;

                float angle = angle1;
                float c = cos( angle );
                float s = sin( angle );

                float offsetLane2 = 0.0;

                // Lanes to terminal nodes
                startPoint.setX( xc + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                startPoint.setY( yc + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle );

                float L = distToTN - nodeRadius - 5.0;
                endPoint.setX( startPoint.x() + L * c );
                endPoint.setY( startPoint.y() + L * s );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle );

                int tID = tID1;
                int lId = road->CreateLane( -1, startPoint, nID, i, true, endPoint, tID, 0, true );
                road->SetNodeRelatedLane( nID, lId );
                road->SetNodeRelatedLane( tID, lId );
            }


            int nOutLane = 1;
            int nLaneCreated = 0;
            for(int k=0;k<nOutLane;++k){

                QVector4D startPoint;
                QVector4D endPoint;

                float angle = angle1;
                float c = cos( angle );
                float s = sin( angle );

                float offsetLane = 0.0;

                // Lanes from terminal nodes
                endPoint.setX( xc + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                endPoint.setY( yc + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                endPoint.setZ( 0.0 );
                endPoint.setW(  angle - 180.0 * 0.017452 );

                float L = distToTN - nodeRadius - 5.0;
                startPoint.setX( endPoint.x() + L * c );
                startPoint.setY( endPoint.y() + L * s );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle - 180.0 * 0.017452 );

                int tID = tID1;
                int lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, nID, i, true );
                road->SetNodeRelatedLane( nID, lId );
                road->SetNodeRelatedLane( tID, lId );

                nLaneCreated++;
            }
        }
    }

    CheckRouteChangeByInsertNode( nID, edgeNodeInfo );

    road->CheckLaneConnectionFull();

    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();
    canvas->update();
}


void DataManipulator::InsertNode_3Rx2_noTS()
{

    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();
    canvas->update();
}



