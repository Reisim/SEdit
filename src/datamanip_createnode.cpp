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


int DataManipulator::CreateNode_4x2x1()
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
        inlanes << 2 << 1 << 2 << 1;

        QList<int> outlanes;
        outlanes << 2 << 1 << 2 << 1;

        cID = CreateNode(x,y,4,inlanes,outlanes);

        for(int i=0;i<4;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        QVector4D startPoint;
        QVector4D endPoint;


        int nLane = 1;
        int nLane2 = 1;
        float offset = 0.0;
        float offset2 = 0.0;

        for(int i=0;i<4;++i){

            if( i == 0 || i == 2 ){
                nLane = 2;
                nLane2 = 1;
                offset = 3.0;
                offset2 = 3.0;
            }
            else{
                nLane = 1;
                nLane2 = 2;
                offset = 3.0;
                offset2 = 3.0;
            }

            for(int j=0;j<nLane;++j){

                float angle = i * 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + (6.0 + offset) * c + (1.5 + 3.0 * j) * s * LRrev  );
                startPoint.setY( y + (6.0 + offset) * s - (1.5 + 3.0 * j) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW(  angle - 180.0 * 0.017452 );

                float L = 12.0 + offset * 2.0;
                endPoint.setX( startPoint.x() - L * c );
                endPoint.setY( startPoint.y() - L * s );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle - 180.0 * 0.017452 );

                int lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+2) % 4, cID, true );
                road->SetNodeRelatedLane( cID, lId );

                if( j == 0 ){
                    float angle2 = angle + 90.0 * 0.017452;
                    c = cos( angle2 );
                    s = sin( angle2 );
                    for(int k=0;k<nLane2;++k){
                        endPoint.setX( x + (6.0 + offset2) * c - (1.5 + 3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0 + offset2) * s + (1.5 + 3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle2 );

                        lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+1) % 4, cID, true );
                        road->SetNodeRelatedLane( cID, lId );
                    }
                }


                if( j == nLane - 1 ){
                    float angle3 = angle - 90.0 * 0.017452;
                    c = cos( angle3 );
                    s = sin( angle3 );
                    for(int k=0;k<nLane2;++k){
                        endPoint.setX( x + (6.0 + offset2) * c - (1.5 + 3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0 + offset2) * s + (1.5 + 3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle3 );

                        lId = road->CreateLane( -1, startPoint,cID, i, cID, true, endPoint, cID , (i+3) % 4, cID, true );
                        road->SetNodeRelatedLane( cID, lId );
                    }
                }

            }
        }
    }


    {
        for(int i=0;i<4;++i){

            QList<int> inlanes;
            if( i == 0 || i == 2 ){
                inlanes << 2;
            }
            else{
                inlanes << 1;
            }

            QList<int> outlanes;
            if( i == 0 || i == 2 ){
                outlanes << 2;
            }
            else{
                outlanes << 1;
            }


            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  80.0; rot = 180.0; break;
            case 1: dy =  50.0; rot = -90.0; break;
            case 2: dx = -80.0; rot = 0.0;   break;
            case 3: dy = -50.0; rot = 90.0;  break;
            }
            int tID = CreateNode( x + dx, y + dy, 1,inlanes,outlanes);
            road->RotateNodeLeg( tID, 0, rot);

            road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("InNodeOutDirect") );

            {
                int nLane = 1;
                if( i == 0 || i == 2 ){
                    nLane = 2;
                }
                float offset = 3.0;

                for(int k=0;k<nLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    startPoint.setX( x + (6.0 + offset) * c - (1.5 + 3.0 * k) * s * LRrev );
                    startPoint.setY( y + (6.0 + offset) * s + (1.5 + 3.0 * k) * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle );

                    float L = 35.0;
                    if( i == 0 || i == 2 ){
                        L += 30.0;
                    }

                    endPoint.setX( startPoint.x() + L * c );
                    endPoint.setY( startPoint.y() + L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle );

                    int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                    endPoint.setX( x + (6.0 + offset) * c + (1.5 + 3.0 * k) * s * LRrev );
                    endPoint.setY( y + (6.0 + offset) * s - (1.5 + 3.0 * k) * c * LRrev );
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
    }

    road->CheckLaneConnection();

    if( canvas ){
        canvas->update();
    }

    return cID;
}


int DataManipulator::CreateNode_4x2x2()
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
        inlanes << 2 << 2 << 2 << 2;

        QList<int> outlanes;
        outlanes << 2 << 2 << 2 << 2;

        cID = CreateNode(x,y,4,inlanes,outlanes);

        for(int i=0;i<4;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        QVector4D startPoint;
        QVector4D endPoint;


        int nLane = 2;
        int nLane2 = 2;
        float offset = 3.0;
        float offset2 = 3.0;

        for(int i=0;i<4;++i){

            for(int j=0;j<nLane;++j){

                float angle = i * 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + (6.0 + offset) * c + (1.5 + 3.0 * j) * s * LRrev  );
                startPoint.setY( y + (6.0 + offset) * s - (1.5 + 3.0 * j) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW(  angle - 180.0 * 0.017452 );

                float L = 12.0 + offset * 2.0;
                endPoint.setX( startPoint.x() - L * c );
                endPoint.setY( startPoint.y() - L * s );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle - 180.0 * 0.017452 );

                int lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+2) % 4, cID, true );
                road->SetNodeRelatedLane( cID, lId );

                if( j == 0 ){
                    float angle2 = angle + 90.0 * 0.017452;
                    c = cos( angle2 );
                    s = sin( angle2 );
                    for(int k=0;k<nLane2;++k){
                        endPoint.setX( x + (6.0 + offset2) * c - (1.5 + 3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0 + offset2) * s + (1.5 + 3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle2 );

                        lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+1) % 4, cID, true );
                        road->SetNodeRelatedLane( cID, lId );
                    }
                }


                if( j == nLane - 1 ){
                    float angle3 = angle - 90.0 * 0.017452;
                    c = cos( angle3 );
                    s = sin( angle3 );
                    for(int k=0;k<nLane2;++k){
                        endPoint.setX( x + (6.0 + offset2) * c - (1.5 + 3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0 + offset2) * s + (1.5 + 3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle3 );

                        lId = road->CreateLane( -1, startPoint,cID, i, cID, true, endPoint, cID , (i+3) % 4, cID, true );
                        road->SetNodeRelatedLane( cID, lId );
                    }
                }

            }
        }
    }


    {
        for(int i=0;i<4;++i){

            QList<int> inlanes;
            inlanes << 2;

            QList<int> outlanes;
            outlanes << 2;

            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  80.0; rot = 180.0; break;
            case 1: dy =  80.0; rot = -90.0; break;
            case 2: dx = -80.0; rot = 0.0;   break;
            case 3: dy = -80.0; rot = 90.0;  break;
            }
            int tID = CreateNode( x + dx, y + dy, 1,inlanes,outlanes);
            road->RotateNodeLeg( tID, 0, rot);

            road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("InNodeOutDirect") );

            {
                int nLane = 2;
                float offset = 3.0;

                for(int k=0;k<nLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    startPoint.setX( x + (6.0 + offset) * c - (1.5 + 3.0 * k) * s * LRrev );
                    startPoint.setY( y + (6.0 + offset) * s + (1.5 + 3.0 * k) * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle );

                    float L = 65.0;

                    endPoint.setX( startPoint.x() + L * c );
                    endPoint.setY( startPoint.y() + L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle );

                    int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                    endPoint.setX( x + (6.0 + offset) * c + (1.5 + 3.0 * k) * s * LRrev );
                    endPoint.setY( y + (6.0 + offset) * s - (1.5 + 3.0 * k) * c * LRrev );
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
    }

    road->CheckLaneConnection();

    if( canvas ){
        canvas->update();
    }

    return cID;
}


int DataManipulator::CreateNode_4x1x1_r()
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
        inlanes << 2 << 2 << 2 << 2;

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


        int nLane = 2;
        int nLane2 = 1;
        float offset = 3.0;
        float offset2 = 3.0;

        for(int i=0;i<4;++i){

            for(int j=0;j<nLane;++j){

                float angle = i * 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + (6.0 + offset) * c + (3.0 * j) * s * LRrev  );
                startPoint.setY( y + (6.0 + offset) * s - (3.0 * j) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW(  angle - 180.0 * 0.017452 );


                if( j > 0 ){
                    float L = 12.0 + offset * 2.0;
                    endPoint.setX( startPoint.x() - L * c );
                    endPoint.setY( startPoint.y() - L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle - 180.0 * 0.017452 );

                    int lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+2) % 4, cID, true );
                    road->SetNodeRelatedLane( cID, lId );
                }

                if( j == 0 ){
                    float angle2 = angle + 90.0 * 0.017452;
                    c = cos( angle2 );
                    s = sin( angle2 );
                    for(int k=1;k<=nLane2;++k){
                        endPoint.setX( x + (6.0 + offset2) * c - (3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0 + offset2) * s + (3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle2 );

                        int lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+1) % 4, cID, true );
                        road->SetNodeRelatedLane( cID, lId );
                    }
                }

                if( j == nLane - 1 ){
                    float angle3 = angle - 90.0 * 0.017452;
                    c = cos( angle3 );
                    s = sin( angle3 );
                    for(int k=1;k<=nLane2;++k){
                        endPoint.setX( x + (6.0 + offset2) * c - (3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0 + offset2) * s + (3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle3 );

                        int lId = road->CreateLane( -1, startPoint,cID, i, cID, true, endPoint, cID , (i+3) % 4, cID, true );
                        road->SetNodeRelatedLane( cID, lId );
                    }
                }

            }
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
            case 0: dx =  80.0; rot = 180.0; break;
            case 1: dy =  80.0; rot = -90.0; break;
            case 2: dx = -80.0; rot = 0.0;   break;
            case 3: dy = -80.0; rot = 90.0;  break;
            }
            int tID = CreateNode( x + dx, y + dy, 1,inlanes,outlanes);
            road->RotateNodeLeg( tID, 0, rot);

            road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("InNodeOutDirect") );

            float offset2 = 3.0;
            {
                QVector4D startPoint;
                QVector4D endPoint;

                float angle = i * 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + (6.0+offset2) * c - 3.0 * s * LRrev );
                startPoint.setY( y + (6.0+offset2) * s + 3.0 * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle );

                float L = 65.0;
                endPoint.setX( startPoint.x() + L * c );
                endPoint.setY( startPoint.y() + L * s );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle );

                int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                endPoint.setX( x + (6.0+offset2) * c + 3.0 * s * LRrev );
                endPoint.setY( y + (6.0+offset2) * s - 3.0 * c * LRrev );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle - 180.0 * 0.017452 );

                L = 30.0;
                startPoint.setX( endPoint.x() + L * c );
                startPoint.setY( endPoint.y() + L * s );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle - 180.0 * 0.017452 );

                int lId2 = road->CreateLane( -1, startPoint, -1, -1, tID, false, endPoint, cID, i, cID, true );


                endPoint.setX( x + (6.0+offset2) * c + 0.0 * s * LRrev );
                endPoint.setY( y + (6.0+offset2) * s - 0.0 * c * LRrev );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle - 180.0 * 0.017452 );

                int lId3 = road->CreateLane( -1, startPoint, -1, -1, tID, false, endPoint, cID, i, cID, true );

                endPoint = startPoint;

                L = 35.0;
                startPoint.setX( endPoint.x() + L * c );
                startPoint.setY( endPoint.y() + L * s );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle - 180.0 * 0.017452 );

                int lId4 = road->CreateLane( -1, startPoint, tID, 0, tID, true, endPoint, -1, -1, cID, false );


                road->SetNodeRelatedLane( cID, lId1 );
                road->SetNodeRelatedLane( cID, lId2 );
                road->SetNodeRelatedLane( cID, lId3 );
                road->SetNodeRelatedLane( cID, lId4 );

                road->SetNodeRelatedLane( tID, lId1 );
                road->SetNodeRelatedLane( tID, lId2 );
                road->SetNodeRelatedLane( tID, lId3 );
                road->SetNodeRelatedLane( tID, lId4 );
            }
        }
    }

    road->CheckLaneConnection();

    if( canvas ){
        canvas->update();
    }

    return  cID;
}


int DataManipulator::CreateNode_4x2x1_r()
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
        inlanes << 3 << 2 << 3 << 2;

        QList<int> outlanes;
        outlanes << 2 << 1 << 2 << 1;

        cID = CreateNode(x,y,4,inlanes,outlanes);

        for(int i=0;i<4;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        QVector4D startPoint;
        QVector4D endPoint;


        int nLane = 1;
        int nLane2 = 1;
        float offset = 3.0;
        float offset2 = 3.0;

        for(int i=0;i<4;++i){

            if( i == 0 || i == 2 ){
                nLane  = 3;
                nLane2 = 1;
                offset  = 3.0;
                offset2 = 6.0;
            }
            else{
                nLane  = 2;
                nLane2 = 2;
                offset  = 6.0;
                offset2 = 3.0;
            }

            for(int j=0;j<nLane;++j){

                float angle = i * 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + (6.0 + offset) * c + (3.0 * j) * s * LRrev  );
                startPoint.setY( y + (6.0 + offset) * s - (3.0 * j) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW(  angle - 180.0 * 0.017452 );


                if( j > 0 ){
                    float L = 12.0 + offset * 2.0;
                    endPoint.setX( startPoint.x() - L * c );
                    endPoint.setY( startPoint.y() - L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle - 180.0 * 0.017452 );

                    int lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+2) % 4, cID, true );
                    road->SetNodeRelatedLane( cID, lId );
                }

                if( j == 0 ){
                    float angle2 = angle + 90.0 * 0.017452;
                    c = cos( angle2 );
                    s = sin( angle2 );
                    for(int k=1;k<=nLane2;++k){
                        endPoint.setX( x + (6.0 + offset2) * c - (3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0 + offset2) * s + (3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle2 );

                        int lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+1) % 4, cID, true );
                        road->SetNodeRelatedLane( cID, lId );
                    }
                }

                if( j == nLane - 1 ){
                    float angle3 = angle - 90.0 * 0.017452;
                    c = cos( angle3 );
                    s = sin( angle3 );
                    for(int k=1;k<=nLane2;++k){
                        endPoint.setX( x + (6.0 + offset2) * c - (3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0 + offset2) * s + (3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle3 );

                        int lId = road->CreateLane( -1, startPoint,cID, i, cID, true, endPoint, cID , (i+3) % 4, cID, true );
                        road->SetNodeRelatedLane( cID, lId );
                    }
                }

            }
        }
    }

    {

        for(int i=0;i<4;++i){

            QList<int> inlanes;
            if( i == 0 || i == 2 ){
                inlanes << 2;
            }
            else{
                inlanes << 1;
            }

            QList<int> outlanes;
            if( i == 0 || i == 2 ){
                outlanes << 2;
            }
            else{
                outlanes << 1;
            }

            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  80.0; rot = 180.0; break;
            case 1: dy =  80.0; rot = -90.0; break;
            case 2: dx = -80.0; rot = 0.0;   break;
            case 3: dy = -80.0; rot = 90.0;  break;
            }
            int tID = CreateNode( x + dx, y + dy, 1,inlanes,outlanes);
            road->RotateNodeLeg( tID, 0, rot);

            road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("InNodeOutDirect") );

            float offset2 = 3.0;
            {
                int nLane = 1;
                if( i == 0 || i == 2 ){
                    nLane = 2;
                    offset2 = 3.0;
                }
                else{
                    offset2 = 6.0;
                }

                for(int k=1;k<=nLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    startPoint.setX( x + (6.0+offset2) * c - (3.0 * k) * s * LRrev );
                    startPoint.setY( y + (6.0+offset2) * s + (3.0 * k) * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle );

                    float L = 65.0;
                    endPoint.setX( startPoint.x() + L * c );
                    endPoint.setY( startPoint.y() + L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle );

                    int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                    if( k == 1 ){
                        endPoint.setX( x + (6.0+offset2) * c + 3.0 * s * LRrev );
                        endPoint.setY( y + (6.0+offset2) * s - 3.0 * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle - 180.0 * 0.017452 );

                        L = 30.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        int lId2 = road->CreateLane( -1, startPoint, -1, -1, tID, false, endPoint, cID, i, cID, true );


                        endPoint.setX( x + (6.0+offset2) * c + 0.0 * s * LRrev );
                        endPoint.setY( y + (6.0+offset2) * s - 0.0 * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle - 180.0 * 0.017452 );

                        int lId3 = road->CreateLane( -1, startPoint, -1, -1, tID, false, endPoint, cID, i, cID, true );

                        endPoint = startPoint;

                        L = 35.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        int lId4 = road->CreateLane( -1, startPoint, tID, 0, tID, true, endPoint, -1, -1, cID, false );


                        road->SetNodeRelatedLane( cID, lId1 );
                        road->SetNodeRelatedLane( cID, lId2 );
                        road->SetNodeRelatedLane( cID, lId3 );
                        road->SetNodeRelatedLane( cID, lId4 );

                        road->SetNodeRelatedLane( tID, lId1 );
                        road->SetNodeRelatedLane( tID, lId2 );
                        road->SetNodeRelatedLane( tID, lId3 );
                        road->SetNodeRelatedLane( tID, lId4 );
                    }
                    else{

                        endPoint.setX( x + (6.0+offset2) * c + (3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0+offset2) * s - (3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle - 180.0 * 0.017452 );

                        L = 65.0;
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
        }
    }

    road->CheckLaneConnection();

    if( canvas ){
        canvas->update();
    }

    return  cID;
}


int DataManipulator::CreateNode_4x2x2_r()
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
        inlanes << 3 << 3 << 3 << 3;

        QList<int> outlanes;
        outlanes << 2 << 2 << 2 << 2;

        cID = CreateNode(x,y,4,inlanes,outlanes);

        for(int i=0;i<4;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        QVector4D startPoint;
        QVector4D endPoint;

        int nLane = 3;
        int nLane2 = 2;
        float offset = 3.0;
        float offset2 = 3.0;

        for(int i=0;i<4;++i){

            if( i == 0 || i == 2 ){
                nLane  = 3;
                nLane2 = 2;
                offset  = 3.0;
                offset2 = 3.0;
            }
            else{
                nLane  = 3;
                nLane2 = 2;
                offset  = 3.0;
                offset2 = 3.0;
            }

            for(int j=0;j<nLane;++j){

                float angle = i * 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + (6.0 + offset) * c + (3.0 * j) * s * LRrev  );
                startPoint.setY( y + (6.0 + offset) * s - (3.0 * j) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW(  angle - 180.0 * 0.017452 );


                if( j > 0 ){
                    float L = 12.0 + offset * 2.0;
                    endPoint.setX( startPoint.x() - L * c );
                    endPoint.setY( startPoint.y() - L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle - 180.0 * 0.017452 );

                    int lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+2) % 4, cID, true );
                    road->SetNodeRelatedLane( cID, lId );
                }

                if( j == 0 ){
                    float angle2 = angle + 90.0 * 0.017452;
                    c = cos( angle2 );
                    s = sin( angle2 );
                    for(int k=1;k<=nLane2;++k){
                        endPoint.setX( x + (6.0 + offset2) * c - (3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0 + offset2) * s + (3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle2 );

                        int lId = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, cID, (i+1) % 4, cID, true );
                        road->SetNodeRelatedLane( cID, lId );
                    }
                }

                if( j == nLane - 1 ){
                    float angle3 = angle - 90.0 * 0.017452;
                    c = cos( angle3 );
                    s = sin( angle3 );
                    for(int k=1;k<=nLane2;++k){
                        endPoint.setX( x + (6.0 + offset2) * c - (3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0 + offset2) * s + (3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle3 );

                        int lId = road->CreateLane( -1, startPoint,cID, i, cID, true, endPoint, cID , (i+3) % 4, cID, true );
                        road->SetNodeRelatedLane( cID, lId );
                    }
                }

            }
        }
    }

    {

        for(int i=0;i<4;++i){

            QList<int> inlanes;
            if( i == 0 || i == 2 ){
                inlanes << 2;
            }
            else{
                inlanes << 2;
            }

            QList<int> outlanes;
            if( i == 0 || i == 2 ){
                outlanes << 2;
            }
            else{
                outlanes << 2;
            }

            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  100.0; rot = 180.0; break;
            case 1: dy =  100.0; rot = -90.0; break;
            case 2: dx = -100.0; rot = 0.0;   break;
            case 3: dy = -100.0; rot = 90.0;  break;
            }
            int tID = CreateNode( x + dx, y + dy, 1,inlanes,outlanes);
            road->RotateNodeLeg( tID, 0, rot);

            road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("InNodeOutDirect") );

            float offset2 = 3.0;
            {
                int nLane = 2;
                if( i == 0 || i == 2 ){
                    nLane = 2;
                    offset2 = 3.0;
                }
                else{
                    nLane = 2;
                    offset2 = 3.0;
                }

                for(int k=1;k<=nLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    startPoint.setX( x + (6.0+offset2) * c - (3.0 * k) * s * LRrev );
                    startPoint.setY( y + (6.0+offset2) * s + (3.0 * k) * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle );

                    float L = 85.0;
                    endPoint.setX( startPoint.x() + L * c );
                    endPoint.setY( startPoint.y() + L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle );

                    int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                    if( k == 1 ){
                        endPoint.setX( x + (6.0+offset2) * c + 3.0 * s * LRrev );
                        endPoint.setY( y + (6.0+offset2) * s - 3.0 * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle - 180.0 * 0.017452 );

                        L = 50.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        int lId2 = road->CreateLane( -1, startPoint, -1, -1, tID, false, endPoint, cID, i, cID, true );


                        endPoint.setX( x + (6.0+offset2) * c + 0.0 * s * LRrev );
                        endPoint.setY( y + (6.0+offset2) * s - 0.0 * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle - 180.0 * 0.017452 );

                        int lId3 = road->CreateLane( -1, startPoint, -1, -1, tID, false, endPoint, cID, i, cID, true );

                        endPoint = startPoint;

                        L = 35.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        int lId4 = road->CreateLane( -1, startPoint, tID, 0, tID, true, endPoint, -1, -1, cID, false );


                        road->SetNodeRelatedLane( cID, lId1 );
                        road->SetNodeRelatedLane( cID, lId2 );
                        road->SetNodeRelatedLane( cID, lId3 );
                        road->SetNodeRelatedLane( cID, lId4 );

                        road->SetNodeRelatedLane( tID, lId1 );
                        road->SetNodeRelatedLane( tID, lId2 );
                        road->SetNodeRelatedLane( tID, lId3 );
                        road->SetNodeRelatedLane( tID, lId4 );
                    }
                    else{

                        endPoint.setX( x + (6.0+offset2) * c + (3.0 * k) * s * LRrev );
                        endPoint.setY( y + (6.0+offset2) * s - (3.0 * k) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle - 180.0 * 0.017452 );

                        L = 85.0;
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
        }
    }

    road->CheckLaneConnection();

    if( canvas ){
        canvas->update();
    }

    return  cID;
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


    if( road->updateCPEveryOperation == true ){
        road->CheckLaneCrossPoints();
    }


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


    if( road->updateCPEveryOperation == true ){
        road->CheckLaneCrossPoints();
    }


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


    if( road->updateCPEveryOperation == true ){
        road->CheckLaneCrossPoints();
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x2x1_TS()
{
    int cNodeID = CreateNode_4x2x1();
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


    if( road->updateCPEveryOperation == true ){
        road->CheckLaneCrossPoints();
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x2x2_TS()
{
    int cNodeID = CreateNode_4x2x2();
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


    if( road->updateCPEveryOperation == true ){
        road->CheckLaneCrossPoints();
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x1x1_r_TS()
{
    int cNodeID = CreateNode_4x1x1_r();
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


    if( road->updateCPEveryOperation == true ){
        road->CheckLaneCrossPoints();
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x2x1_r_TS()
{
    int cNodeID = CreateNode_4x2x1_r();
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


    if( road->updateCPEveryOperation == true ){
        road->CheckLaneCrossPoints();
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x2x2_r_TS()
{
    int cNodeID = CreateNode_4x2x2_r();
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


    if( road->updateCPEveryOperation == true ){
        road->CheckLaneCrossPoints();
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_2L_exist()
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
        inlanes << 0 << 0 << 2;

        QList<int> outlanes;
        outlanes << 2 << 1 << 0;

        cID = CreateNode(x,y,3,inlanes,outlanes);

        for(int i=0;i<3;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        QVector4D startPoint;
        QVector4D endPoint;

        int nLane = 2;
        for(int i=0;i<nLane;++i){

            float angle = 180.0 * 0.017452;
            float c = cos( angle );
            float s = sin( angle );

            startPoint.setX( x + 30.0 * c + (3.5 * i) * s * LRrev  );
            startPoint.setY( y + 30.0 * s - (3.5 * i) * c * LRrev );
            startPoint.setZ( 0.0 );
            startPoint.setW(  angle - 180.0 * 0.017452 );

            float L = 60.0;
            endPoint.setX( startPoint.x() - L * c );
            endPoint.setY( startPoint.y() - L * s );
            endPoint.setZ( 0.0 );
            endPoint.setW( angle - 180.0 * 0.017452 );

            int lId = road->CreateLane( -1, startPoint, cID, 2, cID, true, endPoint, cID, 0, cID, true );
            road->SetNodeRelatedLane( cID, lId );

            if( i == nLane - 1 ){
                // Create Exist Lane

                float angle = 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                endPoint.setX( x + 30.0 * c - 3.5 / 2.0 * s * LRrev );
                endPoint.setY( y + 30.0 * s + 3.5 / 2.0 * c * LRrev );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle );

                lId = road->CreateLane( -1, startPoint, cID, 2, cID, true, endPoint, cID, 1, cID, true );
                road->SetNodeRelatedLane( cID, lId );
            }
        }
    }

    {

        for(int i=0;i<3;++i){

            QList<int> inlanes;
            QList<int> outlanes;

            if( i == 0 ){
                inlanes << 2;
                outlanes << 0;
            }
            else if( i == 1 ){
                inlanes << 1;
                outlanes << 0;
            }
            else if( i == 2 ){
                inlanes << 0;
                outlanes << 2;
            }

            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  200.0; rot = 180.0; break;
            case 1: dy =  200.0; rot = -90.0; break;
            case 2: dx = -200.0; rot = 0.0;   break;
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

                if( i == 0 ){

                    int nLane = 2;
                    for(int j=0;j<nLane;++j){

                        float angle = i * 90.0 * 0.017452;
                        float c = cos( angle );
                        float s = sin( angle );

                        startPoint.setX( x + 30.0 * c - (3.5 * j) * s * LRrev );
                        startPoint.setY( y + 30.0 * s + (3.5 * j) * c * LRrev );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle );

                        float L = 160;
                        endPoint.setX( startPoint.x() + L * c );
                        endPoint.setY( startPoint.y() + L * s );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                        road->SetNodeRelatedLane( cID, lId1 );
                        road->SetNodeRelatedLane( tID, lId1 );
                    }

                }
                else if( i == 1 ){

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    startPoint.setX( x + 30.0 * c - 3.5 / 2.0 * s * LRrev  );
                    startPoint.setY( y + 30.0 * s + 3.5 / 2.0 * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle );

                    float L = 160.0;
                    endPoint.setX( startPoint.x() + L * c );
                    endPoint.setY( startPoint.y() + L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle );

                    int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                    road->SetNodeRelatedLane( cID, lId1 );
                    road->SetNodeRelatedLane( tID, lId1 );
                }
                else if( i == 2 ){

                    int nLane = 2;
                    for(int j=0;j<nLane;++j){

                        float angle = i * 90.0 * 0.017452;
                        float c = cos( angle );
                        float s = sin( angle );

                        endPoint.setX( x + 30.0 * c + (3.5 * j) * s * LRrev );
                        endPoint.setY( y + 30.0 * s - (3.5 * j) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle - 180.0 * 0.017452 );

                        float L = 160.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        int lId2 = road->CreateLane( -1, startPoint, tID, 0, tID, true, endPoint, cID, i, cID, true );

                        road->SetNodeRelatedLane( cID, lId2 );
                        road->SetNodeRelatedLane( tID, lId2 );
                    }
                }
            }
        }
    }

    road->CheckLaneConnection();

    if( canvas ){
        canvas->update();
    }

    return cID;
}


int DataManipulator::CreateNode_2L_merge()
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
        inlanes << 0 << 1 << 2;

        QList<int> outlanes;
        outlanes << 2 << 0 << 0;

        cID = CreateNode(x,y,3,inlanes,outlanes);

        for(int i=0;i<3;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        QVector4D startPoint;
        QVector4D endPoint;

        int nLane = 2;
        for(int i=0;i<nLane;++i){

            float angle = 180.0 * 0.017452;
            float c = cos( angle );
            float s = sin( angle );

            startPoint.setX( x + 30.0 * c + (3.5 * i) * s * LRrev  );
            startPoint.setY( y + 30.0 * s - (3.5 * i) * c * LRrev );
            startPoint.setZ( 0.0 );
            startPoint.setW(  angle - 180.0 * 0.017452 );

            float L = 60.0;
            endPoint.setX( startPoint.x() - L * c );
            endPoint.setY( startPoint.y() - L * s );
            endPoint.setZ( 0.0 );
            endPoint.setW( angle - 180.0 * 0.017452 );

            int lId = road->CreateLane( -1, startPoint, cID, 2, cID, true, endPoint, cID, 0, cID, true );
            road->SetNodeRelatedLane( cID, lId );

            if( i == nLane - 1 ){
                // Create Exist Lane

                float angle = 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + 30.0 * c + 3.5 / 2.0 * s * LRrev );
                startPoint.setY( y + 30.0 * s - 3.5 / 2.0 * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle - 180.0 * 0.017452 );

                lId = road->CreateLane( -1, startPoint, cID, 1, cID, true, endPoint, cID, 0, cID, true );
                road->SetNodeRelatedLane( cID, lId );
            }
        }
    }

    {

        for(int i=0;i<3;++i){

            QList<int> inlanes;
            QList<int> outlanes;

            if( i == 0 ){
                inlanes << 2;
                outlanes << 0;
            }
            else if( i == 1 ){
                inlanes << 0;
                outlanes << 1;
            }
            else if( i == 2 ){
                inlanes << 0;
                outlanes << 2;
            }

            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  200.0; rot = 180.0; break;
            case 1: dy =  200.0; rot = -90.0; break;
            case 2: dx = -200.0; rot = 0.0;   break;
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

                if( i == 0 ){

                    int nLane = 2;
                    for(int j=0;j<nLane;++j){

                        float angle = i * 90.0 * 0.017452;
                        float c = cos( angle );
                        float s = sin( angle );

                        startPoint.setX( x + 30.0 * c - (3.5 * j) * s * LRrev );
                        startPoint.setY( y + 30.0 * s + (3.5 * j) * c * LRrev );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle );

                        float L = 160;
                        endPoint.setX( startPoint.x() + L * c );
                        endPoint.setY( startPoint.y() + L * s );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                        road->SetNodeRelatedLane( cID, lId1 );
                        road->SetNodeRelatedLane( tID, lId1 );
                    }

                }
                else if( i == 1 ){

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    endPoint.setX( x + 30.0 * c + 3.5 / 2.0 * s * LRrev  );
                    endPoint.setY( y + 30.0 * s - 3.5 / 2.0 * c * LRrev );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle - 180.0 * 0.017452 );

                    float L = 160.0;
                    startPoint.setX( endPoint.x() + L * c );
                    startPoint.setY( endPoint.y() + L * s );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle - 180.0 * 0.017452 );

                    int lId2 = road->CreateLane( -1, startPoint, tID, 0, tID, true, endPoint, cID, i, cID, true );

                    road->SetNodeRelatedLane( cID, lId2 );
                    road->SetNodeRelatedLane( tID, lId2 );
                }
                else if( i == 2 ){

                    int nLane = 2;
                    for(int j=0;j<nLane;++j){

                        float angle = i * 90.0 * 0.017452;
                        float c = cos( angle );
                        float s = sin( angle );

                        endPoint.setX( x + 30.0 * c + (3.5 * j) * s * LRrev );
                        endPoint.setY( y + 30.0 * s - (3.5 * j) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle - 180.0 * 0.017452 );

                        float L = 160.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        int lId2 = road->CreateLane( -1, startPoint, tID, 0, tID, true, endPoint, cID, i, cID, true );

                        road->SetNodeRelatedLane( cID, lId2 );
                        road->SetNodeRelatedLane( tID, lId2 );
                    }
                }
            }
        }
    }

    road->CheckLaneConnection();

    if( canvas ){
        canvas->update();
    }

    return cID;
}


int DataManipulator::CreateNode_3L_exist()
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
        inlanes << 0 << 0 << 3;

        QList<int> outlanes;
        outlanes << 3 << 1 << 0;

        cID = CreateNode(x,y,3,inlanes,outlanes);

        for(int i=0;i<3;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        QVector4D startPoint;
        QVector4D endPoint;

        int nLane = 3;
        for(int i=0;i<nLane;++i){

            float angle = 180.0 * 0.017452;
            float c = cos( angle );
            float s = sin( angle );

            startPoint.setX( x + 30.0 * c + (3.5 * i) * s * LRrev  );
            startPoint.setY( y + 30.0 * s - (3.5 * i) * c * LRrev );
            startPoint.setZ( 0.0 );
            startPoint.setW(  angle - 180.0 * 0.017452 );

            float L = 60.0;
            endPoint.setX( startPoint.x() - L * c );
            endPoint.setY( startPoint.y() - L * s );
            endPoint.setZ( 0.0 );
            endPoint.setW( angle - 180.0 * 0.017452 );

            int lId = road->CreateLane( -1, startPoint, cID, 2, cID, true, endPoint, cID, 0, cID, true );
            road->SetNodeRelatedLane( cID, lId );

            if( i == nLane - 1 ){
                // Create Exist Lane

                float angle = 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                endPoint.setX( x + 30.0 * c - 3.5 / 2.0 * s * LRrev );
                endPoint.setY( y + 30.0 * s + 3.5 / 2.0 * c * LRrev );
                endPoint.setZ( 0.0 );
                endPoint.setW( angle );

                lId = road->CreateLane( -1, startPoint, cID, 2, cID, true, endPoint, cID, 1, cID, true );
                road->SetNodeRelatedLane( cID, lId );
            }
        }
    }

    {

        for(int i=0;i<3;++i){

            QList<int> inlanes;
            QList<int> outlanes;

            if( i == 0 ){
                inlanes << 3;
                outlanes << 0;
            }
            else if( i == 1 ){
                inlanes << 1;
                outlanes << 0;
            }
            else if( i == 2 ){
                inlanes << 0;
                outlanes << 3;
            }

            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  200.0; rot = 180.0; break;
            case 1: dy =  200.0; rot = -90.0; break;
            case 2: dx = -200.0; rot = 0.0;   break;
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

                if( i == 0 ){

                    int nLane = 3;
                    for(int j=0;j<nLane;++j){

                        float angle = i * 90.0 * 0.017452;
                        float c = cos( angle );
                        float s = sin( angle );

                        startPoint.setX( x + 30.0 * c - (3.5 * j) * s * LRrev );
                        startPoint.setY( y + 30.0 * s + (3.5 * j) * c * LRrev );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle );

                        float L = 160;
                        endPoint.setX( startPoint.x() + L * c );
                        endPoint.setY( startPoint.y() + L * s );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                        road->SetNodeRelatedLane( cID, lId1 );
                        road->SetNodeRelatedLane( tID, lId1 );
                    }

                }
                else if( i == 1 ){

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    startPoint.setX( x + 30.0 * c - 3.5 / 2.0 * s * LRrev  );
                    startPoint.setY( y + 30.0 * s + 3.5 / 2.0 * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle );

                    float L = 160.0;
                    endPoint.setX( startPoint.x() + L * c );
                    endPoint.setY( startPoint.y() + L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle );

                    int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                    road->SetNodeRelatedLane( cID, lId1 );
                    road->SetNodeRelatedLane( tID, lId1 );
                }
                else if( i == 2 ){

                    int nLane = 3;
                    for(int j=0;j<nLane;++j){

                        float angle = i * 90.0 * 0.017452;
                        float c = cos( angle );
                        float s = sin( angle );

                        endPoint.setX( x + 30.0 * c + (3.5 * j) * s * LRrev );
                        endPoint.setY( y + 30.0 * s - (3.5 * j) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle - 180.0 * 0.017452 );

                        float L = 160.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        int lId2 = road->CreateLane( -1, startPoint, tID, 0, tID, true, endPoint, cID, i, cID, true );

                        road->SetNodeRelatedLane( cID, lId2 );
                        road->SetNodeRelatedLane( tID, lId2 );
                    }
                }
            }
        }
    }

    road->CheckLaneConnection();

    if( canvas ){
        canvas->update();
    }

    return cID;
}


int DataManipulator::CreateNode_3L_merge()
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
        inlanes << 0 << 1 << 3;

        QList<int> outlanes;
        outlanes << 3 << 0 << 0;

        cID = CreateNode(x,y,3,inlanes,outlanes);

        for(int i=0;i<3;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        QVector4D startPoint;
        QVector4D endPoint;

        int nLane = 3;
        for(int i=0;i<nLane;++i){

            float angle = 180.0 * 0.017452;
            float c = cos( angle );
            float s = sin( angle );

            startPoint.setX( x + 30.0 * c + (3.5 * i) * s * LRrev  );
            startPoint.setY( y + 30.0 * s - (3.5 * i) * c * LRrev );
            startPoint.setZ( 0.0 );
            startPoint.setW(  angle - 180.0 * 0.017452 );

            float L = 60.0;
            endPoint.setX( startPoint.x() - L * c );
            endPoint.setY( startPoint.y() - L * s );
            endPoint.setZ( 0.0 );
            endPoint.setW( angle - 180.0 * 0.017452 );

            int lId = road->CreateLane( -1, startPoint, cID, 2, cID, true, endPoint, cID, 0, cID, true );
            road->SetNodeRelatedLane( cID, lId );

            if( i == nLane - 1 ){
                // Create Exist Lane

                float angle = 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + 30.0 * c + 3.5 / 2.0 * s * LRrev );
                startPoint.setY( y + 30.0 * s - 3.5 / 2.0 * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle - 180.0 * 0.017452 );

                lId = road->CreateLane( -1, startPoint, cID, 1, cID, true, endPoint, cID, 0, cID, true );
                road->SetNodeRelatedLane( cID, lId );
            }
        }
    }

    {

        for(int i=0;i<3;++i){

            QList<int> inlanes;
            QList<int> outlanes;

            if( i == 0 ){
                inlanes << 3;
                outlanes << 0;
            }
            else if( i == 1 ){
                inlanes << 0;
                outlanes << 1;
            }
            else if( i == 2 ){
                inlanes << 0;
                outlanes << 3;
            }

            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  200.0; rot = 180.0; break;
            case 1: dy =  200.0; rot = -90.0; break;
            case 2: dx = -200.0; rot = 0.0;   break;
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

                if( i == 0 ){

                    int nLane = 3;
                    for(int j=0;j<nLane;++j){

                        float angle = i * 90.0 * 0.017452;
                        float c = cos( angle );
                        float s = sin( angle );

                        startPoint.setX( x + 30.0 * c - (3.5 * j) * s * LRrev );
                        startPoint.setY( y + 30.0 * s + (3.5 * j) * c * LRrev );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle );

                        float L = 160;
                        endPoint.setX( startPoint.x() + L * c );
                        endPoint.setY( startPoint.y() + L * s );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle );

                        int lId1 = road->CreateLane( -1, startPoint, cID, i, cID, true, endPoint, tID, 0, tID, true );

                        road->SetNodeRelatedLane( cID, lId1 );
                        road->SetNodeRelatedLane( tID, lId1 );
                    }

                }
                else if( i == 1 ){

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    endPoint.setX( x + 30.0 * c + 3.5 / 2.0 * s * LRrev  );
                    endPoint.setY( y + 30.0 * s - 3.5 / 2.0 * c * LRrev );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle - 180.0 * 0.017452 );

                    float L = 160.0;
                    startPoint.setX( endPoint.x() + L * c );
                    startPoint.setY( endPoint.y() + L * s );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle - 180.0 * 0.017452 );

                    int lId2 = road->CreateLane( -1, startPoint, tID, 0, tID, true, endPoint, cID, i, cID, true );

                    road->SetNodeRelatedLane( cID, lId2 );
                    road->SetNodeRelatedLane( tID, lId2 );
                }
                else if( i == 2 ){

                    int nLane = 3;
                    for(int j=0;j<nLane;++j){

                        float angle = i * 90.0 * 0.017452;
                        float c = cos( angle );
                        float s = sin( angle );

                        endPoint.setX( x + 30.0 * c + (3.5 * j) * s * LRrev );
                        endPoint.setY( y + 30.0 * s - (3.5 * j) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle - 180.0 * 0.017452 );

                        float L = 160.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        int lId2 = road->CreateLane( -1, startPoint, tID, 0, tID, true, endPoint, cID, i, cID, true );

                        road->SetNodeRelatedLane( cID, lId2 );
                        road->SetNodeRelatedLane( tID, lId2 );
                    }
                }
            }
        }
    }

    road->CheckLaneConnection();

    if( canvas ){
        canvas->update();
    }

    return cID;
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


