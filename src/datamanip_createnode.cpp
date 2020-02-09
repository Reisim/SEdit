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


int DataManipulator::CreateNode_4x1x1()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    float LRrev = 1.0;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        LRrev = -1.0;
    }

    int cID = -1;
    {
        QList<int> inlanes;
        inlanes << 1 << 1 << 1 << 1;

        QList<int> outlanes;
        outlanes << 1 << 1 << 1 << 1;

        cID = CreateNode(x,y,4,inlanes,outlanes);

        for(int i=0;i<4;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        QVector4D startPoint;
        QVector4D endPoint;



        for(int i=0;i<4;++i){

            float angle = i * 90.0 * 0.017452;
            float c = cos( angle );
            float s = sin( angle );


            startPoint.setX( x + 6.0 * c + 1.5 * s * LRrev  );
            startPoint.setY( y + 6.0 * s - 1.5 * c * LRrev);
            startPoint.setZ( 0.0 );
            startPoint.setW(  angle - 180.0 * 0.017452 );

            float L = 12.0;
            endPoint.setX( startPoint.x() - L * c );
            endPoint.setY( startPoint.y() - L * s );
            endPoint.setZ( 0.0 );
            endPoint.setW( angle - 180.0 * 0.017452 );

            int lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+2) % 4, cID, true );
            road->SetNodeRelatedLane( cID, lId );

            float angle2 = angle + 90.0 * 0.017452;
            c = cos( angle2 );
            s = sin( angle2 );

            endPoint.setX( x + 6.0 * c - 1.5 * s * LRrev );
            endPoint.setY( y + 6.0 * s + 1.5 * c * LRrev );
            endPoint.setZ( 0.0 );
            endPoint.setW( angle2 );

            lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+1) % 4, cID, true );
            road->SetNodeRelatedLane( cID, lId );

            float angle3 = angle - 90.0 * 0.017452;
            c = cos( angle3 );
            s = sin( angle3 );

            endPoint.setX( x + 6.0 * c - 1.5 * s * LRrev );
            endPoint.setY( y + 6.0 * s + 1.5 * c * LRrev );
            endPoint.setZ( 0.0 );
            endPoint.setW( angle3 );

            lId = road->CreateLane( -1, startPoint,cID, i, cID, true, endPoint, cID , (i+3) % 4, cID, true );
            road->SetNodeRelatedLane( cID, lId );
        }

    }

    {
        QList<int> inlanes;
        inlanes << 1;

        QList<int> outlanes;
        outlanes << 1;

        for(int i=0;i<4;++i){

            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  50.0; rot = 180.0; break;
            case 1: dy =  50.0; rot = -90.0; break;
            case 2: dx = -50.0; rot = 0.0;   break;
            case 3: dy = -50.0; rot = 90.0;  break;
            }
            int tID = CreateNode( x + dx, y + dy, 1,inlanes,outlanes);
            road->RotateNodeLeg( tID, 0, rot);

            road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("InNodeOutDirect") );

            {
                QVector4D startPoint;
                QVector4D endPoint;

                float angle = i * 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + 6.0 * c - 1.5 * s * LRrev );
                startPoint.setY( y + 6.0 * s + 1.5 * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle );

                float L = 38.0;
                endPoint.setX( startPoint.x() + L * c );
                endPoint.setY( startPoint.y() + L * s );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle );

                int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                endPoint.setX( x + 6.0 * c + 1.5 * s * LRrev );
                endPoint.setY( y + 6.0 * s - 1.5 * c * LRrev );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle - 180.0 * 0.017452 );

                startPoint.setX( endPoint.x() + L * c );
                startPoint.setY( endPoint.y() + L * s );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle - 180.0 * 0.017452 );

                int lId2 = road->CreateLane( -1, startPoint, tID, 0, tID, true, endPoint, cID, i, cID, true );

                road->SetNodeRelatedLane( cID, lId1 );
                road->SetNodeRelatedLane( cID, lId2 );

                road->SetNodeRelatedLane( tID, lId1 );
                road->SetNodeRelatedLane( tID, lId2 );
            }
        }
    }

    road->CheckLaneConnection();

    if( canvas ){
        canvas->update();
    }

    return cID;
}

int DataManipulator::CreateNode_3x1x1()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    float LRrev = 1.0;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        LRrev = -1.0;
    }

    int cID = -1;
    {
        QList<int> inlanes;
        inlanes << 1 << 1 << 1;

        QList<int> outlanes;
        outlanes << 1 << 1 << 1;

        cID = CreateNode(x,y,3,inlanes,outlanes);

        for(int i=0;i<3;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        QVector4D startPoint;
        QVector4D endPoint;

        for(int i=0;i<3;++i){

            float angle = i * 90.0 * 0.017452;
            float c = cos( angle );
            float s = sin( angle );

            startPoint.setX( x + 6.0 * c + 1.5 * s * LRrev );
            startPoint.setY( y + 6.0 * s - 1.5 * c * LRrev );
            startPoint.setZ( 0.0 );
            startPoint.setW(  angle - 180.0 * 0.017452 );

            float L = 12.0;
            if( i != 1 ){
                endPoint.setX( startPoint.x() - L * c );
                endPoint.setY( startPoint.y() - L * s );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle - 180.0 * 0.017452 );

                int lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+2) % 4, cID, true );  // Straight
                road->SetNodeRelatedLane( cID, lId );
            }

            if( i != 2 ){
                float angle2 = angle + 90.0 * 0.017452;
                c = cos( angle2 );
                s = sin( angle2 );

                endPoint.setX( x + 6.0 * c - 1.5 * s * LRrev );
                endPoint.setY( y + 6.0 * s + 1.5 * c * LRrev );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle2 );

                int lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+1) % 4, cID, true );  // Right
                road->SetNodeRelatedLane( cID, lId );
            }

            if( i != 0 ){
                float angle3 = angle - 90.0 * 0.017452;
                c = cos( angle3 );
                s = sin( angle3 );

                endPoint.setX( x + 6.0 * c - 1.5 * s * LRrev );
                endPoint.setY( y + 6.0 * s + 1.5 * c * LRrev );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle3 );

                int lId = road->CreateLane( -1, startPoint,cID, i, cID, true, endPoint, cID , (i+3) % 4, cID, true );  // Left
                road->SetNodeRelatedLane( cID, lId );
            }
        }
    }

    {
        QList<int> inlanes;
        inlanes << 1;

        QList<int> outlanes;
        outlanes << 1;

        for(int i=0;i<3;++i){

            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  50.0; rot = 180.0; break;
            case 1: dy =  50.0; rot = -90.0; break;
            case 2: dx = -50.0; rot = 0.0;   break;
            }
            int tID = CreateNode( x + dx, y + dy, 1,inlanes,outlanes);
            road->RotateNodeLeg( tID, 0, rot);

            road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("InNodeOutDirect") );

            {
                QVector4D startPoint;
                QVector4D endPoint;

                float angle = i * 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + 6.0 * c - 1.5 * s * LRrev );
                startPoint.setY( y + 6.0 * s + 1.5 * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle );

                float L = 38.0;
                endPoint.setX( startPoint.x() + L * c );
                endPoint.setY( startPoint.y() + L * s );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle );

                int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                endPoint.setX( x + 6.0 * c + 1.5 * s * LRrev );
                endPoint.setY( y + 6.0 * s - 1.5 * c * LRrev );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle - 180.0 * 0.017452 );

                startPoint.setX( endPoint.x() + L * c );
                startPoint.setY( endPoint.y() + L * s );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle - 180.0 * 0.017452 );

                int lId2 = road->CreateLane( -1, startPoint, tID, 0, tID, true, endPoint, cID, i, cID, true );

                road->SetNodeRelatedLane( cID, lId1 );
                road->SetNodeRelatedLane( cID, lId2 );

                road->SetNodeRelatedLane( tID, lId1 );
                road->SetNodeRelatedLane( tID, lId2 );
            }
        }
    }

    road->CheckLaneConnection();

    if( canvas ){
        canvas->update();
    }

    return cID;
}



int DataManipulator::CreateNode_4x1x1_noTS()
{
    int cNodeID = CreateNode_4x1x1();
    int ndIdx = road->indexOfNode( cNodeID );

    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){


        if( (i % 2) == 1 ){
            // StopLine for Yield
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_TEMPSTOP );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
            }
        }

    }

    road->CheckLaneCrossPoints();

    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_3x1x1_noTS()
{
    int cNodeID = CreateNode_3x1x1();
    int ndIdx = road->indexOfNode( cNodeID );

    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){


        if( (i % 2) == 1 ){
            // StopLine for Yield
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_TEMPSTOP );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
            }
        }

    }

    road->CheckLaneCrossPoints();

    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x1x1_TS()
{
    int cNodeID = CreateNode_4x1x1();
    int ndIdx = road->indexOfNode( cNodeID );

    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        // TS for Vehicle
        int vtsID = CreateTrafficSignal( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID , 0 );
        int vtsIdx = road->indexOfTS( vtsID, cNodeID );
        if( vtsIdx >= 0 ){
            if(i % 2 == 0 ){
                road->nodes[ndIdx]->trafficSignals[vtsIdx]->startOffset = 0;
            }
            else{
                road->nodes[ndIdx]->trafficSignals[vtsIdx]->startOffset = 60;
            }
        }


        // TS for Pedestrian
        int ptsId = CreateTrafficSignal( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID , 1 );
        int ptsIdx = road->indexOfTS( ptsId, cNodeID );
        if( ptsIdx >= 0 ){
            if(i % 2 == 1 ){
                road->nodes[ndIdx]->trafficSignals[ptsIdx]->startOffset = 0;
            }
            else{
                road->nodes[ndIdx]->trafficSignals[ptsIdx]->startOffset = 60;
            }
        }

        // StopLine for TS
        int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
        if( sl >= 0 ){
            road->CheckStopLineCrossLanes( sl );
        }
    }

    road->CheckLaneCrossPoints();

    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode(float x,float y,int nLeg,QList<int> inlanes,QList<int> outlanes)
{
    qDebug() << "[DataManipulator::CreateNode] nLeg=" << nLeg;
    qDebug() << "    in-Lane = " << inlanes;
    qDebug() << "    out-Lane = " << outlanes;


    int cId = road->CreateNode(-1, x, y, inlanes, outlanes );
    if( cId >= 0 ){
        QString command = QString("CreateNode,%1,%2,%3,%4").arg(cId).arg(x).arg(y).arg(nLeg);
        for(int i=0;i<inlanes.size();++i){
            command += QString(",%1").arg(inlanes[i]);
        }
        for(int i=0;i<outlanes.size();++i){
            command += QString(",%1").arg(outlanes[i]);
        }
        operationHistory << command;

        qDebug() << " add operationHistory: " << command;
    }
    return cId;
}


int DataManipulator::CreateTrafficSignal(int nodeID,int nodeDir,int type)
{
    qDebug() << "[DataManipulator::CreateTrafficSignal] nodeID=" << nodeID << " nodeDir=" << nodeDir << " type=" << type;

    int cId = road->CreateTrafficSignal(-1, nodeID, nodeDir, type );
    if( cId >= 0 ){
        QString command = QString("CreateTrafficSignal,%1,%2,%3,%4").arg(cId).arg(nodeID).arg(nodeDir).arg(type);
        operationHistory << command;
        qDebug() << " add operationHistory: " << command;
    }
    return cId;
}


int DataManipulator::CreateStopLine(int nodeID,int nodeDir,int type)
{
    qDebug() << "[DataManipulator::CreateStopLine] nodeID=" << nodeID << " nodeDir=" << nodeDir << " type=" << type;

    int cId = road->CreateStopLine(-1, nodeID, nodeDir, type );
    if( cId >= 0 ){
        QString command = QString("CreateStopLine,%1,%2,%3,%4").arg(cId).arg(nodeID).arg(nodeDir).arg(type);
        operationHistory << command;
        qDebug() << " add operationHistory: " << command;
    }
    return cId;
}


