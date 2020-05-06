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

#include <QDialog>
#include <QRadioButton>
#include <QSizePolicy>


int DataManipulator::CreateNode_4(float x,float y,QList<int> inlanes, QList<int> outlanes, QList<bool> createTurnLane,QStringList turnRestriction)
{
    if( inlanes.size() != 4 || outlanes.size() != 4 || createTurnLane.size() != 4 ){
        qDebug() << "[DataManipulator::CreateNode_4] size of onalens or outlanes is not 4.";
        return -1;
    }


    float LRrev = 1.0;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        LRrev = -1.0;
    }


    int maxLaneNum = 0;
    for(int i=0;i<inlanes.size();++i){
        if( maxLaneNum < inlanes[i] ){
            maxLaneNum = inlanes[i];
        }
    }
    for(int i=0;i<outlanes.size();++i){
        if( maxLaneNum < outlanes[i] ){
            maxLaneNum = outlanes[i];
        }
    }


    float nodeRadius = 6.0 + (maxLaneNum -1) * 4.0;

    qDebug() << "[DataManipulator::CreateNode_4] maxLaneNum = " << maxLaneNum;
    qDebug() << "[DataManipulator::CreateNode_4] nodeRadius = " << nodeRadius;



    //
    // Create center node
    int cID = -1;
    {
        cID = CreateNode(x,y,4,inlanes,outlanes);

        // Set Node Connection Info
        for(int i=0;i<4;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        // Create Lanes inside node
        QVector4D startPoint;
        QVector4D endPoint;

        for(int i=0;i<4;++i){

            float offsetLane = 0.0;
            if( createTurnLane[i] == true ){
                offsetLane = -1.375;
            }

            int nInLane = inlanes[i];
            int nStraightLane = 0;
            for(int j=0;j<nInLane;++j){

                float angle = i * 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );



                startPoint.setX( x + nodeRadius * c + (2.75 * j + 1.375 + offsetLane) * s * LRrev  );
                startPoint.setY( y + nodeRadius * s - (2.75 * j + 1.375 + offsetLane) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW(  angle - 180.0 * 0.017452 );


                // Right-turn lanes ( Left-turn lanes for RIGHT-HAND Traffic)
                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 4 && QString(turnRestriction[i]).contains( QString("%1").arg((i+1*(int)LRrev)%4) ) == true ) ){
                    if( j == 0 ){

                        float angle2 = angle + 90.0 * 0.017452 * LRrev;
                        float c2 = cos( angle2 );
                        float s2 = sin( angle2 );

                        float offsetLane2 = 0.0;
                        if( createTurnLane[(i+1*(int)LRrev)%4] == true ){
                            offsetLane2 = 1.375;
                        }

                        int nOutLane = outlanes[(i+1*(int)LRrev)%4];
                        for(int k=0;k<nOutLane;++k){
                            endPoint.setX( x + nodeRadius * c2 - (2.75 * k + 1.375 + offsetLane2) * s2 * LRrev );
                            endPoint.setY( y + nodeRadius * s2 + (2.75 * k + 1.375 + offsetLane2) * c2 * LRrev );
                            endPoint.setZ( 0.0 );
                            endPoint.setW( angle2 );

                            int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, cID, (i+1*(int)LRrev)%4, true );
                            road->SetNodeRelatedLane( cID, lId );
                        }

                        if(createTurnLane[i] == true){
                            continue;
                        }
                    }
                }

                // Straight Lanes
                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 4 && QString(turnRestriction[i]).contains( QString("%1").arg((i+2)%4) ) == true ) ){

                    int nOutLane = outlanes[(i+2)%4];
                    if( nStraightLane < nOutLane ){

                        float angle2 = angle + 180.0 * 0.017452;
                        float c2 = cos( angle2 );
                        float s2 = sin( angle2 );

                        float offsetLane2 = 0.0;
                        if( createTurnLane[(i+2) % 4] == true ){
                            offsetLane2 = 1.375;
                        }

                        endPoint.setX( x + nodeRadius * c2 - (2.75 * nStraightLane + 1.375 + offsetLane2) * s2 * LRrev  );
                        endPoint.setY( y + nodeRadius * s2 + (2.75 * nStraightLane + 1.375 + offsetLane2) * c2 * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW( angle2 );

                        int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, cID, (i+2) % 4, true );
                        road->SetNodeRelatedLane( cID, lId );

                        nStraightLane++;
                    }
                }


                // Left-turn lanes ( Right-turn lanes for RIGHT-HAND Traffic)
                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 4 && QString(turnRestriction[i]).contains( QString("%1").arg((i-1*(int)LRrev+4)%4) ) == true ) ){
                    if( j == nInLane - 1 ){

                        float angle2 = angle - 90.0 * 0.017452 * LRrev;
                        float c2 = cos( angle2 );
                        float s2 = sin( angle2 );

                        float offsetLane2 = 0.0;
                        if( createTurnLane[(i-1*(int)LRrev+4)%4] == true ){
                            offsetLane2 = 1.375;
                        }

                        int nOutLane = outlanes[(i-1*(int)LRrev+4)%4];
                        for(int k=0;k<nOutLane;++k){
                            endPoint.setX( x + nodeRadius * c2 - (2.75 * k + 1.375 + offsetLane2) * s2 * LRrev );
                            endPoint.setY( y + nodeRadius * s2 + (2.75 * k + 1.375 + offsetLane2) * c2 * LRrev );
                            endPoint.setZ( 0.0 );
                            endPoint.setW( angle2 );

                            int lId = road->CreateLane( -1, startPoint,cID, i, true, endPoint, cID , (i-1*(int)LRrev+4)%4, true );
                            road->SetNodeRelatedLane( cID, lId );
                        }
                    }
                }
            }
        }
    }

    //
    // Create Terminal nodes around center node
    {
        float distToTN = 50.0 + (maxLaneNum -1) * 50.0;
        float turnLaneLength = 20.0 + (maxLaneNum -2) * 15.0;
        float shiftLength = 15.0 + (maxLaneNum -2) * 3.0;

        for(int i=0;i<4;++i){

            QList<int> inlanesTN;
            inlanesTN << outlanes[i];

            int numTrunLane = 0;
            if( createTurnLane[i] == true ){
                numTrunLane = 1;
            }

            QList<int> outlanesTN;
            outlanesTN << (inlanes[i] - numTrunLane);


            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  distToTN; rot = 180.0; break;
            case 1: dy =  distToTN; rot = -90.0; break;
            case 2: dx = -distToTN; rot = 0.0;   break;
            case 3: dy = -distToTN; rot = 90.0;  break;
            }

            int tID = CreateNode( x + dx, y + dy, 1, inlanesTN, outlanesTN);
            road->RotateNodeLeg( tID, 0, rot);

            // Set Node Connection Info
            road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("InNodeOutDirect") );


            // Create Lanes
            {
                int nInLane = inlanesTN[0];
                for(int k=0;k<nInLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    float offsetLane2 = 0.0;
                    if( createTurnLane[i] == true ){
                        offsetLane2 = 1.375;
                    }

                    // Lanes to terminal nodes
                    startPoint.setX( x + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                    startPoint.setY( y + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle );

                    float L = distToTN - nodeRadius - 5.0;
                    endPoint.setX( startPoint.x() + L * c );
                    endPoint.setY( startPoint.y() + L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle );

                    int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, tID, 0, true );
                    road->SetNodeRelatedLane( cID, lId );
                    road->SetNodeRelatedLane( tID, lId );
                }


                int nOutLane = outlanesTN[0];
                int nLaneCreated = 0;
                for(int k=0;k<nOutLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    float offsetLane = 0.0;
                    if( createTurnLane[i] == true ){
                        offsetLane = -1.375;
                    }

                    // Lanes from terminal nodes

                    if( createTurnLane[i] == true && k == 0 ){

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle - 180.0 * 0.017452 );

                        float L = turnLaneLength;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        QVector4D EPShift = startPoint;

                        int lId = road->CreateLane( -1, startPoint, tID, 0, false, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle - 180.0 * 0.017452 );

                        L = turnLaneLength + shiftLength;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        lId = road->CreateLane( -1, startPoint, tID, 0, false, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        QVector4D SPShift = startPoint;


                        endPoint = startPoint;

                        L = distToTN - nodeRadius - 5.0 - (turnLaneLength + shiftLength);
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, false );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );


                        // Shift Lane to turn lane
                        lId = road->CreateLane( -1, SPShift, tID, 0, false, EPShift, cID, i, false );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;
                    }
                    else{

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle - 180.0 * 0.017452 );

                        float L = distToTN - nodeRadius - 5.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        int lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;
                    }
                }
            }
        }
    }

    road->CheckLaneConnectionOfNode( cID );

    if( canvas ){
        canvas->update();
    }

    return  cID;
}


int DataManipulator::CreateNode_3(float x,float y,QList<int> inlanes, QList<int> outlanes, QList<bool> createTurnLane, QStringList turnRestriction)
{
    if( inlanes.size() != 3 || outlanes.size() != 3 || createTurnLane.size() != 3 ){
        qDebug() << "[DataManipulator::CreateNode_3] size of onalens or outlanes is not 3.";
        return -1;
    }


    float LRrev = 1.0;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        LRrev = -1.0;
    }


    int maxLaneNum = 0;
    for(int i=0;i<inlanes.size();++i){
        if( maxLaneNum < inlanes[i] ){
            maxLaneNum = inlanes[i];
        }
    }
    for(int i=0;i<outlanes.size();++i){
        if( maxLaneNum < outlanes[i] ){
            maxLaneNum = outlanes[i];
        }
    }


    float nodeRadius = 6.0 + (maxLaneNum -1) * 4.0;

    qDebug() << "[DataManipulator::CreateNode_3] maxLaneNum = " << maxLaneNum;
    qDebug() << "[DataManipulator::CreateNode_3] nodeRadius = " << nodeRadius;


    //
    // Create center node
    int cID = -1;
    {
        cID = CreateNode(x,y,3,inlanes,outlanes);

        // Set Node Connection Info
        for(int i=0;i<3;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        // Create Lanes inside node
        QVector4D startPoint;
        QVector4D endPoint;

        for(int i=0;i<3;++i){

            float offsetLane = 0.0;
            if( createTurnLane[i] == true ){
                offsetLane = -1.375;
            }

            int nInLane = inlanes[i];
            int nStraightLane = 0;
            for(int j=0;j<nInLane;++j){

                float angle = i * 90.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + nodeRadius * c + (2.75 * j + 1.375 + offsetLane) * s * LRrev  );
                startPoint.setY( y + nodeRadius * s - (2.75 * j + 1.375 + offsetLane) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW( angle - 180.0 * 0.017452 );


                // Right-turn lanes ( Left-turn lanes for RIGHT-HAND Traffic)
                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 3 && QString(turnRestriction[i]).contains( QString("%1").arg(i+1*(int)LRrev) ) == true ) ){
                    if( (LRrev > 0.0 && i == 0 && j == 0) || (LRrev < 0.0 && i == 2 && j == 0) || (i == 1 && j == 0) ){

                        float angle2 = angle + 90.0 * 0.017452 * LRrev;
                        float c2 = cos( angle2 );
                        float s2 = sin( angle2 );

                        float offsetLane2 = 0.0;
                        if( createTurnLane[(i+1*(int)LRrev)] == true ){
                            offsetLane2 = 1.375;
                        }

                        int nOutLane = outlanes[(i+1*(int)LRrev)];
                        for(int k=0;k<nOutLane;++k){
                            endPoint.setX( x + nodeRadius * c2 - (2.75 * k + 1.375 + offsetLane2) * s2 * LRrev );
                            endPoint.setY( y + nodeRadius * s2 + (2.75 * k + 1.375 + offsetLane2) * c2 * LRrev );
                            endPoint.setZ( 0.0 );
                            endPoint.setW( angle2 );

                            int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, cID, (i+1*(int)LRrev), true );
                            road->SetNodeRelatedLane( cID, lId );
                        }

                        if(createTurnLane[i] == true){
                            continue;
                        }
                    }
                }


                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 3 && QString(turnRestriction[i]).contains( QString("%1").arg((i+2)%4) ) == true ) ){
                    if( i != 1 ){

                        // Straight Lanes
                        int nOutLane = outlanes[(i+2)%4];
                        if( nStraightLane < nOutLane ){

                            float angle2 = angle + 180.0 * 0.017452;
                            float c2 = cos( angle2 );
                            float s2 = sin( angle2 );

                            float offsetLane2 = 0.0;
                            if( createTurnLane[(i+2) % 4] == true ){
                                offsetLane2 = 1.375;
                            }

                            endPoint.setX( x + nodeRadius * c2 - (2.75 * nStraightLane + 1.375 + offsetLane2) * s2 * LRrev  );
                            endPoint.setY( y + nodeRadius * s2 + (2.75 * nStraightLane + 1.375 + offsetLane2) * c2 * LRrev );
                            endPoint.setZ( 0.0 );
                            endPoint.setW( angle2 );

                            int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, cID, (i+2) % 4, true );
                            road->SetNodeRelatedLane( cID, lId );

                            nStraightLane++;
                        }
                    }
                }


                // Left-turn lanes ( Right-turn lanes for RIGHT-HAND Traffic)
                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 3 && QString(turnRestriction[i]).contains( QString("%1").arg(i-1*(int)LRrev) ) == true ) ){
                    if( (LRrev > 0.0 && i != 0 && j == nInLane - 1) || (LRrev < 0.0 && i != 2 && j == nInLane - 1 ) ){

                        float angle2 = angle - 90.0 * 0.017452 * LRrev;
                        float c2 = cos( angle2 );
                        float s2 = sin( angle2 );

                        float offsetLane2 = 0.0;
                        if( createTurnLane[(i-1*(int)LRrev)] == true ){
                            offsetLane2 = 1.375;
                        }

                        int nOutLane = outlanes[(i-1*(int)LRrev)];
                        for(int k=0;k<nOutLane;++k){
                            endPoint.setX( x + nodeRadius * c2 - (2.75 * k + 1.375 + offsetLane2) * s2 * LRrev );
                            endPoint.setY( y + nodeRadius * s2 + (2.75 * k + 1.375 + offsetLane2) * c2 * LRrev );
                            endPoint.setZ( 0.0 );
                            endPoint.setW( angle2 );

                            int lId = road->CreateLane( -1, startPoint,cID, i, true, endPoint, cID , (i-1*(int)LRrev), true );
                            road->SetNodeRelatedLane( cID, lId );
                        }
                    }
                }
            }
        }
    }

    //
    // Create Terminal nodes around center node
    {
        float distToTN = 50.0 + (maxLaneNum -1) * 50.0;
        float turnLaneLength = 20.0 + (maxLaneNum -2) * 15.0;
        float shiftLength = 15.0 + (maxLaneNum -2) * 3.0;

        for(int i=0;i<3;++i){

            QList<int> inlanesTN;
            inlanesTN << outlanes[i];

            int numTrunLane = 0;
            if( createTurnLane[i] == true ){
                numTrunLane = 1;
            }

            QList<int> outlanesTN;
            outlanesTN << (inlanes[i] - numTrunLane);


            float dx = 0.0;
            float dy = 0.0;
            float rot = 0.0;
            switch(i){
            case 0: dx =  distToTN; rot = 180.0; break;
            case 1: dy =  distToTN; rot = -90.0; break;
            case 2: dx = -distToTN; rot = 0.0;   break;
            case 3: dy = -distToTN; rot = 90.0;  break;
            }

            int tID = CreateNode( x + dx, y + dy, 1, inlanesTN, outlanesTN);
            road->RotateNodeLeg( tID, 0, rot);

            // Set Node Connection Info
            road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("InNodeOutDirect") );


            // Create Lanes
            {
                int nInLane = inlanesTN[0];
                for(int k=0;k<nInLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    float offsetLane2 = 0.0;
                    if( createTurnLane[i] == true ){
                        offsetLane2 = 1.375;
                    }

                    // Lanes to terminal nodes
                    startPoint.setX( x + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                    startPoint.setY( y + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle );

                    float L = distToTN - nodeRadius - 5.0;
                    endPoint.setX( startPoint.x() + L * c );
                    endPoint.setY( startPoint.y() + L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle );

                    int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, tID, 0, true );
                    road->SetNodeRelatedLane( cID, lId );
                    road->SetNodeRelatedLane( tID, lId );
                }


                int nOutLane = outlanesTN[0];
                int nLaneCreated = 0;
                for(int k=0;k<nOutLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 90.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    float offsetLane = 0.0;
                    if( createTurnLane[i] == true ){
                        offsetLane = -1.375;
                    }

                    // Lanes from terminal nodes

                    if( createTurnLane[i] == true && k == 0 && ( (LRrev > 0.0 && i != 2) || (LRrev < 0.0 && i != 0) )){

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle - 180.0 * 0.017452 );

                        float L = turnLaneLength;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        QVector4D EPShift = startPoint;

                        int lId = road->CreateLane( -1, startPoint, tID, 0, false, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle - 180.0 * 0.017452 );

                        L = turnLaneLength + shiftLength;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        lId = road->CreateLane( -1, startPoint, tID, 0, false, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        QVector4D SPShift = startPoint;


                        endPoint = startPoint;

                        L = distToTN - nodeRadius - 5.0 - (turnLaneLength + shiftLength);
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, false );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );


                        // Shift Lane to turn lane
                        lId = road->CreateLane( -1, SPShift, tID, 0, false, EPShift, cID, i, false );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;
                    }
                    else{

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle - 180.0 * 0.017452 );

                        float L = distToTN - nodeRadius - 5.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle - 180.0 * 0.017452 );

                        int lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;
                    }
                }
            }
        }
    }

    road->CheckLaneConnectionOfNode( cID );

    if( canvas ){
        canvas->update();
    }

    return  cID;
}


int DataManipulator::CreateNode_5(float x,float y,QList<int> inlanes, QList<int> outlanes, QList<bool> createTurnLane,QStringList turnRestriction)
{
    if( inlanes.size() != 5 || outlanes.size() != 5 || createTurnLane.size() != 5 ){
        qDebug() << "[DataManipulator::CreateNode_5] size of onalens or outlanes is not 5.";
        return -1;
    }


    float LRrev = 1.0;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        LRrev = -1.0;
    }


    int maxLaneNum = 0;
    for(int i=0;i<inlanes.size();++i){
        if( maxLaneNum < inlanes[i] ){
            maxLaneNum = inlanes[i];
        }
    }
    for(int i=0;i<outlanes.size();++i){
        if( maxLaneNum < outlanes[i] ){
            maxLaneNum = outlanes[i];
        }
    }


    float nodeRadius = 6.0 + (maxLaneNum -1) * 4.0;

    qDebug() << "[DataManipulator::CreateNode_5] maxLaneNum = " << maxLaneNum;
    qDebug() << "[DataManipulator::CreateNode_5] nodeRadius = " << nodeRadius;



    //
    // Create center node
    int cID = -1;
    {
        cID = CreateNode(x,y,5,inlanes,outlanes);

        // Set Node Connection Info
        for(int i=0;i<5;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        // Create Lanes inside node
        QVector4D startPoint;
        QVector4D endPoint;

        for(int i=0;i<5;++i){

            float offsetLane = 0.0;
            if( createTurnLane[i] == true ){
                offsetLane = -1.375;
            }

            int nInLane = inlanes[i];
            int nStraightLane = 0;
            for(int j=0;j<nInLane;++j){

                float angle = i * 360.0 / 5.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + nodeRadius * c + (2.75 * j + 1.375 + offsetLane) * s * LRrev  );
                startPoint.setY( y + nodeRadius * s - (2.75 * j + 1.375 + offsetLane) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW(  angle - 180.0 * 0.017452 );

                QList<int> processedDir;

                // Right-turn lanes ( Left-turn lanes for RIGHT-HAND Traffic)
                int dir = (i+1*(int)LRrev)%5;
                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 5 && QString(turnRestriction[i]).contains( QString("%1").arg(dir) ) == true ) ){
                    if( j == 0 ){

                        float angle2 = angle + 360.0 / 5.0 * 0.017452 * LRrev;
                        float c2 = cos( angle2 );
                        float s2 = sin( angle2 );

                        float offsetLane2 = 0.0;
                        if( createTurnLane[dir] == true ){
                            offsetLane2 = 1.375;
                        }

                        int nOutLane = outlanes[dir];
                        for(int k=0;k<nOutLane;++k){
                            endPoint.setX( x + nodeRadius * c2 - (2.75 * k + 1.375 + offsetLane2) * s2 * LRrev );
                            endPoint.setY( y + nodeRadius * s2 + (2.75 * k + 1.375 + offsetLane2) * c2 * LRrev );
                            endPoint.setZ( 0.0 );
                            endPoint.setW( angle2 );

                            int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, cID, dir, true );
                            road->SetNodeRelatedLane( cID, lId );
                        }

                        if(createTurnLane[i] == true){
                            continue;
                        }
                    }
                }

                processedDir.append( dir );


                // Left-turn lanes ( Right-turn lanes for RIGHT-HAND Traffic)
                dir = (i-1*(int)LRrev+5)%5;
                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 5 && QString(turnRestriction[i]).contains( QString("%1").arg(dir) ) == true ) ){
                    if( j == nInLane - 1 ){

                        float angle2 = angle - 360.0 / 5.0 * 0.017452 * LRrev;
                        float c2 = cos( angle2 );
                        float s2 = sin( angle2 );

                        float offsetLane2 = 0.0;
                        if( createTurnLane[dir] == true ){
                            offsetLane2 = 1.375;
                        }

                        int nOutLane = outlanes[dir];
                        for(int k=0;k<nOutLane;++k){
                            endPoint.setX( x + nodeRadius * c2 - (2.75 * k + 1.375 + offsetLane2) * s2 * LRrev );
                            endPoint.setY( y + nodeRadius * s2 + (2.75 * k + 1.375 + offsetLane2) * c2 * LRrev );
                            endPoint.setZ( 0.0 );
                            endPoint.setW( angle2 );

                            int lId = road->CreateLane( -1, startPoint,cID, i, true, endPoint, cID , dir, true );
                            road->SetNodeRelatedLane( cID, lId );
                        }
                    }
                }

                processedDir.append( dir );

                // Straight Lanes
                for( dir = 0; dir < 5; dir ++ ){

                    if( processedDir.contains( dir ) == true ){
                        continue;
                    }
                    if( dir == i ){
                        continue;
                    }

                    if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 5 && QString(turnRestriction[i]).contains( QString("%1").arg(dir) ) == true ) ){

                        int nOutLane = outlanes[dir];
                        if( nStraightLane < nOutLane ){

                            float angle2 = 360.0 / 5.0 * dir;
                            if( angle2 > 180.0 ){
                                angle2 -= 360.0;
                            }
                            angle2 *= 0.017452;

                            float c2 = cos( angle2 );
                            float s2 = sin( angle2 );

                            float offsetLane2 = 0.0;
                            if( createTurnLane[dir] == true ){
                                offsetLane2 = 1.375;
                            }

                            endPoint.setX( x + nodeRadius * c2 - (2.75 * nStraightLane + 1.375 + offsetLane2) * s2 * LRrev  );
                            endPoint.setY( y + nodeRadius * s2 + (2.75 * nStraightLane + 1.375 + offsetLane2) * c2 * LRrev );
                            endPoint.setZ( 0.0 );
                            endPoint.setW( angle2 );

                            int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, cID, dir, true );
                            road->SetNodeRelatedLane( cID, lId );

                        }
                    }
                }

                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 5 && QString(turnRestriction[i]).contains( QString("%1").arg(dir) ) == true ) ){
                    nStraightLane++;
                }
            }
        }
    }

    //
    // Create Terminal nodes around center node
    {
        float distToTN = 50.0 + (maxLaneNum -1) * 50.0;
        float turnLaneLength = 20.0 + (maxLaneNum -2) * 15.0;
        float shiftLength = 15.0 + (maxLaneNum -2) * 3.0;

        for(int i=0;i<5;++i){

            QList<int> inlanesTN;
            inlanesTN << outlanes[i];

            int numTrunLane = 0;
            if( createTurnLane[i] == true ){
                numTrunLane = 1;
            }

            QList<int> outlanesTN;
            outlanesTN << (inlanes[i] - numTrunLane);



            float dx = 0.0;
            float dy = 0.0;
            float rot = 360.0 / 5 * i;
            if( rot > 180.0 ){
                rot -= 360.0;
            }
            dx = distToTN * cos( rot * 0.017452 );
            dy = distToTN * sin( rot * 0.017452 );
            rot += 180.0;

            int tID = CreateNode( x + dx, y + dy, 1, inlanesTN, outlanesTN);
            road->RotateNodeLeg( tID, 0, rot);

            // Set Node Connection Info
            road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("InNodeOutDirect") );


            // Create Lanes
            {
                int nInLane = inlanesTN[0];
                for(int k=0;k<nInLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 360.0 / 5.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    float offsetLane2 = 0.0;
                    if( createTurnLane[i] == true ){
                        offsetLane2 = 1.375;
                    }

                    // Lanes to terminal nodes
                    startPoint.setX( x + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                    startPoint.setY( y + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle );

                    float L = distToTN - nodeRadius - 5.0;
                    endPoint.setX( startPoint.x() + L * c );
                    endPoint.setY( startPoint.y() + L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle );

                    int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, tID, 0, true );
                    road->SetNodeRelatedLane( cID, lId );
                    road->SetNodeRelatedLane( tID, lId );
                }


                int nOutLane = outlanesTN[0];
                int nLaneCreated = 0;
                for(int k=0;k<nOutLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 360.0 / 5.0;
                    if( angle > 180.0 ){
                        angle -= 360.0;
                    }
                    float angle2 = angle -180.0;
                    if( angle2 > -180.0 ){
                        angle2 += 360.0;
                    }
                    angle *= 0.017452;
                    angle2 *= 0.017452;

                    float c = cos( angle );
                    float s = sin( angle );

                    float offsetLane = 0.0;
                    if( createTurnLane[i] == true ){
                        offsetLane = -1.375;
                    }

                    // Lanes from terminal nodes

                    if( createTurnLane[i] == true && k == 0 ){

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle2 );

                        float L = turnLaneLength;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle2 );

                        QVector4D EPShift = startPoint;

                        int lId = road->CreateLane( -1, startPoint, tID, 0, false, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle2 );

                        L = turnLaneLength + shiftLength;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle2 );

                        lId = road->CreateLane( -1, startPoint, tID, 0, false, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        QVector4D SPShift = startPoint;


                        endPoint = startPoint;

                        L = distToTN - nodeRadius - 5.0 - (turnLaneLength + shiftLength);
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle2 );

                        lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, false );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );


                        // Shift Lane to turn lane
                        lId = road->CreateLane( -1, SPShift, tID, 0, false, EPShift, cID, i, false );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;
                    }
                    else{

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle2 );

                        float L = distToTN - nodeRadius - 5.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle2 );

                        int lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;
                    }
                }
            }
        }
    }

    road->CheckLaneConnectionOfNode( cID );

    if( canvas ){
        canvas->update();
    }

    return  cID;
}


int DataManipulator::CreateNode_6(float x,float y,QList<int> inlanes, QList<int> outlanes, QList<bool> createTurnLane,QStringList turnRestriction)
{
    if( inlanes.size() != 6 || outlanes.size() != 6 || createTurnLane.size() != 6 ){
        qDebug() << "[DataManipulator::CreateNode_6] size of onalens or outlanes is not 6.";
        return -1;
    }


    float LRrev = 1.0;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        LRrev = -1.0;
    }


    int maxLaneNum = 0;
    for(int i=0;i<inlanes.size();++i){
        if( maxLaneNum < inlanes[i] ){
            maxLaneNum = inlanes[i];
        }
    }
    for(int i=0;i<outlanes.size();++i){
        if( maxLaneNum < outlanes[i] ){
            maxLaneNum = outlanes[i];
        }
    }


    float nodeRadius = 6.0 + (maxLaneNum -1) * 4.0;

    qDebug() << "[DataManipulator::CreateNode_6] maxLaneNum = " << maxLaneNum;
    qDebug() << "[DataManipulator::CreateNode_6] nodeRadius = " << nodeRadius;



    //
    // Create center node
    int cID = -1;
    {
        cID = CreateNode(x,y,6,inlanes,outlanes);

        // Set Node Connection Info
        for(int i=0;i<6;++i){
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("OutNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( cID, i, cID+i+1, QString("InNode") );
            road->SetNodeConnectInfo( cID, i, 0      , QString("InNodeOutDirect") );
        }

        // Create Lanes inside node
        QVector4D startPoint;
        QVector4D endPoint;

        for(int i=0;i<6;++i){

            float offsetLane = 0.0;
            if( createTurnLane[i] == true ){
                offsetLane = -1.375;
            }

            int nInLane = inlanes[i];
            int nStraightLane = 0;
            for(int j=0;j<nInLane;++j){

                float angle = i * 360.0 / 6.0 * 0.017452;
                float c = cos( angle );
                float s = sin( angle );

                startPoint.setX( x + nodeRadius * c + (2.75 * j + 1.375 + offsetLane) * s * LRrev  );
                startPoint.setY( y + nodeRadius * s - (2.75 * j + 1.375 + offsetLane) * c * LRrev );
                startPoint.setZ( 0.0 );
                startPoint.setW(  angle - 180.0 * 0.017452 );

                QList<int> processedDir;

                // Right-turn lanes ( Left-turn lanes for RIGHT-HAND Traffic)
                int dir = (i+1*(int)LRrev)%6;
                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 6 && QString(turnRestriction[i]).contains( QString("%1").arg(dir) ) == true ) ){
                    if( j == 0 ){

                        float angle2 = angle + 360.0 / 6.0 * 0.017452 * LRrev;
                        float c2 = cos( angle2 );
                        float s2 = sin( angle2 );

                        float offsetLane2 = 0.0;
                        if( createTurnLane[dir] == true ){
                            offsetLane2 = 1.375;
                        }

                        int nOutLane = outlanes[dir];
                        for(int k=0;k<nOutLane;++k){
                            endPoint.setX( x + nodeRadius * c2 - (2.75 * k + 1.375 + offsetLane2) * s2 * LRrev );
                            endPoint.setY( y + nodeRadius * s2 + (2.75 * k + 1.375 + offsetLane2) * c2 * LRrev );
                            endPoint.setZ( 0.0 );
                            endPoint.setW( angle2 );

                            int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, cID, dir, true );
                            road->SetNodeRelatedLane( cID, lId );
                        }

                        if(createTurnLane[i] == true){
                            continue;
                        }
                    }
                }

                processedDir.append( dir );


                // Left-turn lanes ( Right-turn lanes for RIGHT-HAND Traffic)
                dir = (i-1*(int)LRrev+6)%6;
                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 6 && QString(turnRestriction[i]).contains( QString("%1").arg(dir) ) == true ) ){
                    if( j == nInLane - 1 ){

                        float angle2 = angle - 360.0 / 6.0 * 0.017452 * LRrev;
                        float c2 = cos( angle2 );
                        float s2 = sin( angle2 );

                        float offsetLane2 = 0.0;
                        if( createTurnLane[dir] == true ){
                            offsetLane2 = 1.375;
                        }

                        int nOutLane = outlanes[dir];
                        for(int k=0;k<nOutLane;++k){
                            endPoint.setX( x + nodeRadius * c2 - (2.75 * k + 1.375 + offsetLane2) * s2 * LRrev );
                            endPoint.setY( y + nodeRadius * s2 + (2.75 * k + 1.375 + offsetLane2) * c2 * LRrev );
                            endPoint.setZ( 0.0 );
                            endPoint.setW( angle2 );

                            int lId = road->CreateLane( -1, startPoint,cID, i, true, endPoint, cID , dir, true );
                            road->SetNodeRelatedLane( cID, lId );
                        }
                    }
                }

                processedDir.append( dir );

                // Straight Lanes
                for( dir = 0; dir < 6; dir ++ ){

                    if( processedDir.contains( dir ) == true ){
                        continue;
                    }
                    if( dir == i ){
                        continue;
                    }

                    if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 6 && QString(turnRestriction[i]).contains( QString("%1").arg(dir) ) == true ) ){

                        int nOutLane = outlanes[dir];
                        if( nStraightLane < nOutLane ){

                            float angle2 = 360.0 / 6.0 * dir;
                            if( angle2 > 180.0 ){
                                angle2 -= 360.0;
                            }
                            angle2 *= 0.017452;

                            float c2 = cos( angle2 );
                            float s2 = sin( angle2 );

                            float offsetLane2 = 0.0;
                            if( createTurnLane[dir] == true ){
                                offsetLane2 = 1.375;
                            }

                            endPoint.setX( x + nodeRadius * c2 - (2.75 * nStraightLane + 1.375 + offsetLane2) * s2 * LRrev  );
                            endPoint.setY( y + nodeRadius * s2 + (2.75 * nStraightLane + 1.375 + offsetLane2) * c2 * LRrev );
                            endPoint.setZ( 0.0 );
                            endPoint.setW( angle2 );

                            int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, cID, dir, true );
                            road->SetNodeRelatedLane( cID, lId );

                        }
                    }
                }

                if( turnRestriction.isEmpty() == true || ( turnRestriction.size() == 6 && QString(turnRestriction[i]).contains( QString("%1").arg(dir) ) == true ) ){
                    nStraightLane++;
                }
            }
        }
    }

    //
    // Create Terminal nodes around center node
    {
        float distToTN = 50.0 + (maxLaneNum -1) * 50.0;
        float turnLaneLength = 20.0 + (maxLaneNum -2) * 15.0;
        float shiftLength = 15.0 + (maxLaneNum -2) * 3.0;

        for(int i=0;i<6;++i){

            QList<int> inlanesTN;
            inlanesTN << outlanes[i];

            int numTrunLane = 0;
            if( createTurnLane[i] == true ){
                numTrunLane = 1;
            }

            QList<int> outlanesTN;
            outlanesTN << (inlanes[i] - numTrunLane);


            float dx = 0.0;
            float dy = 0.0;
            float rot = 360.0 / 6 * i;
            if( rot > 180.0 ){
                rot -= 360.0;
            }
            dx = distToTN * cos( rot * 0.017452 );
            dy = distToTN * sin( rot * 0.017452 );
            rot += 180.0;

            int tID = CreateNode( x + dx, y + dy, 1, inlanesTN, outlanesTN);
            road->RotateNodeLeg( tID, 0, rot);

            // Set Node Connection Info
            road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("OutNodeInDirect") );
            road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
            road->SetNodeConnectInfo( tID, 0, i  , QString("InNodeOutDirect") );


            // Create Lanes
            {
                int nInLane = inlanesTN[0];
                for(int k=0;k<nInLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 360.0 / 6.0 * 0.017452;
                    float c = cos( angle );
                    float s = sin( angle );

                    float offsetLane2 = 0.0;
                    if( createTurnLane[i] == true ){
                        offsetLane2 = 1.375;
                    }

                    // Lanes to terminal nodes
                    startPoint.setX( x + nodeRadius * c - (2.75 * k + 1.375 + offsetLane2) * s * LRrev  );
                    startPoint.setY( y + nodeRadius * s + (2.75 * k + 1.375 + offsetLane2) * c * LRrev );
                    startPoint.setZ( 0.0 );
                    startPoint.setW( angle );

                    float L = distToTN - nodeRadius - 5.0;
                    endPoint.setX( startPoint.x() + L * c );
                    endPoint.setY( startPoint.y() + L * s );
                    endPoint.setZ( 0.0 );
                    endPoint.setW( angle );

                    int lId = road->CreateLane( -1, startPoint, cID, i, true, endPoint, tID, 0, true );
                    road->SetNodeRelatedLane( cID, lId );
                    road->SetNodeRelatedLane( tID, lId );
                }


                int nOutLane = outlanesTN[0];
                int nLaneCreated = 0;
                for(int k=0;k<nOutLane;++k){

                    QVector4D startPoint;
                    QVector4D endPoint;

                    float angle = i * 360.0 / 6.0;
                    if( angle > 180.0 ){
                        angle -= 360.0;
                    }
                    float angle2 = angle -180.0;
                    if( angle2 > -180.0 ){
                        angle2 += 360.0;
                    }
                    angle *= 0.017452;
                    angle2 *= 0.017452;

                    float c = cos( angle );
                    float s = sin( angle );

                    float offsetLane = 0.0;
                    if( createTurnLane[i] == true ){
                        offsetLane = -1.375;
                    }

                    // Lanes from terminal nodes

                    if( createTurnLane[i] == true && k == 0 ){

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle2 );

                        float L = turnLaneLength;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle2 );

                        QVector4D EPShift = startPoint;

                        int lId = road->CreateLane( -1, startPoint, tID, 0, false, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle2 );

                        L = turnLaneLength + shiftLength;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle2 );

                        lId = road->CreateLane( -1, startPoint, tID, 0, false, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        QVector4D SPShift = startPoint;


                        endPoint = startPoint;

                        L = distToTN - nodeRadius - 5.0 - (turnLaneLength + shiftLength);
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle2 );

                        lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, false );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );


                        // Shift Lane to turn lane
                        lId = road->CreateLane( -1, SPShift, tID, 0, false, EPShift, cID, i, false );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;
                    }
                    else{

                        endPoint.setX( x + nodeRadius * c + (2.75 * nLaneCreated + 1.375 + offsetLane) * s * LRrev  );
                        endPoint.setY( y + nodeRadius * s - (2.75 * nLaneCreated + 1.375 + offsetLane) * c * LRrev );
                        endPoint.setZ( 0.0 );
                        endPoint.setW(  angle2 );

                        float L = distToTN - nodeRadius - 5.0;
                        startPoint.setX( endPoint.x() + L * c );
                        startPoint.setY( endPoint.y() + L * s );
                        startPoint.setZ( 0.0 );
                        startPoint.setW( angle2 );

                        int lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, true );
                        road->SetNodeRelatedLane( cID, lId );
                        road->SetNodeRelatedLane( tID, lId );

                        nLaneCreated++;
                    }
                }
            }
        }
    }

    road->CheckLaneConnectionOfNode( cID );

    if( canvas ){
        canvas->update();
    }

    return  cID;
}


int DataManipulator::CreateStraight(float x,float y,int nLane)
{
    float LRrev = 1.0;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        LRrev = -1.0;
    }

    QList<int> inlanes;
    inlanes << nLane;

    QList<int> outlanes;
    outlanes << nLane;

    float lengthCourse = 200.0;

    int cID = CreateNode( x, y, 1, inlanes, outlanes);

    int tID = CreateNode( x + lengthCourse, y, 1, inlanes, outlanes);
    road->RotateNodeLeg( tID, 0, 180.0);


    // Set Node Connection Info
    road->SetNodeConnectInfo( cID, 0, tID, QString("OutNode") );
    road->SetNodeConnectInfo( cID, 0, 0  , QString("OutNodeInDirect") );
    road->SetNodeConnectInfo( cID, 0, tID, QString("InNode") );
    road->SetNodeConnectInfo( cID, 0, 0  , QString("InNodeOutDirect") );

    road->SetNodeConnectInfo( tID, 0, cID, QString("OutNode") );
    road->SetNodeConnectInfo( tID, 0, 0  , QString("OutNodeInDirect") );
    road->SetNodeConnectInfo( tID, 0, cID, QString("InNode") );
    road->SetNodeConnectInfo( tID, 0, 0  , QString("InNodeOutDirect") );


    float nodeRadius = 5.0;

    for(int k=0;k<nLane;++k){

        QVector4D startPoint;
        QVector4D endPoint;

        float angle = 0.0;
        float c = 1.0;
        float s = 0.0;


        startPoint.setX( x + nodeRadius * c - (2.75 * k + 1.375 ) * s * LRrev  );
        startPoint.setY( y + nodeRadius * s + (2.75 * k + 1.375 ) * c * LRrev );
        startPoint.setZ( 0.0 );
        startPoint.setW( angle );

        float L = lengthCourse - nodeRadius * 2.0;
        endPoint.setX( startPoint.x() + L * c );
        endPoint.setY( startPoint.y() + L * s );
        endPoint.setZ( 0.0 );
        endPoint.setW( angle );

        int lId = road->CreateLane( -1, startPoint, cID, 0, true, endPoint, tID, 0, true );
        road->SetNodeRelatedLane( cID, lId );
        road->SetNodeRelatedLane( tID, lId );
    }


    for(int k=0;k<nLane;++k){

        QVector4D startPoint;
        QVector4D endPoint;

        float c = -1.0;
        float s = 0.0;

        endPoint.setX( x - nodeRadius * c - (2.75 * k + 1.375 ) * s * LRrev  );
        endPoint.setY( y - nodeRadius * s + (2.75 * k + 1.375 ) * c * LRrev );
        endPoint.setZ( 0.0 );
        endPoint.setW( 180.0 * 0.017452 );

        float L = lengthCourse - nodeRadius * 2.0;
        startPoint.setX( endPoint.x() - L * c );
        startPoint.setY( endPoint.y() - L * s );
        startPoint.setZ( 0.0 );
        startPoint.setW( 180.0 * 0.017452 );

        int lId = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID , 0, true );
        road->SetNodeRelatedLane( cID, lId );
        road->SetNodeRelatedLane( tID, lId );
    }

    return cID;
}


int DataManipulator::CreateNode_4x1x1()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    QList<int> inlanes;
    inlanes << 1 << 1 << 1 << 1;

    QList<int> outlanes;
    outlanes << 1 << 1 << 1 << 1;

    QList<bool> createTurnLanes;
    createTurnLanes << false << false << false << false;

    return CreateNode_4( x, y, inlanes, outlanes, createTurnLanes );
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


    QList<int> inlanes;
    inlanes << 2 << 1 << 2 << 1;

    QList<int> outlanes;
    outlanes << 2 << 1 << 2 << 1;

    QList<bool> createTurnLanes;
    createTurnLanes << false << false << false << false;

    return CreateNode_4( x, y, inlanes, outlanes, createTurnLanes );
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

    QList<int> inlanes;
    inlanes << 2 << 2 << 2 << 2;

    QList<int> outlanes;
    outlanes << 2 << 2 << 2 << 2;

    QList<bool> createTurnLanes;
    createTurnLanes << false << false << false << false;

    return CreateNode_4( x, y, inlanes, outlanes, createTurnLanes );
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

    QList<int> inlanes;
    inlanes << 2 << 2 << 2 << 2;

    QList<int> outlanes;
    outlanes << 1 << 1 << 1 << 1;

    QList<bool> createTurnLanes;
    createTurnLanes << true << true << true << true;

    return CreateNode_4( x, y, inlanes, outlanes, createTurnLanes );
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

    QList<int> inlanes;
    inlanes << 3 << 2 << 3 << 2;

    QList<int> outlanes;
    outlanes << 2 << 1 << 2 << 1;

    QList<bool> createTurnLane;
    createTurnLane << true << true << true << true;

    return CreateNode_4( x, y, inlanes, outlanes, createTurnLane);
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

    QList<int> inlanes;
    inlanes << 3 << 3 << 3 << 3;

    QList<int> outlanes;
    outlanes << 2 << 2 << 2 << 2;

    QList<bool> createTurnLane;
    createTurnLane << true << true << true << true;

    return CreateNode_4( x, y, inlanes, outlanes, createTurnLane);
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


    QList<int> inlanes;
    inlanes << 1 << 1 << 1;

    QList<int> outlanes;
    outlanes << 1 << 1 << 1;

    QList<bool> createTurnLanes;
    createTurnLanes << false << false << false;

    return CreateNode_3( x, y, inlanes, outlanes, createTurnLanes );
}


int DataManipulator::CreateNode_3x2x1()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }


    QList<int> inlanes;
    inlanes << 2 << 1 << 2;

    QList<int> outlanes;
    outlanes << 2 << 1 << 2;

    QList<bool> createTurnLanes;
    createTurnLanes << false << false << false;

    return CreateNode_3( x, y, inlanes, outlanes, createTurnLanes );
}


int DataManipulator::CreateNode_3x2x1_rm()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }


    QList<int> inlanes;
    QList<bool> createTurnLanes;

    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        inlanes << 2 << 1 << 3;
        createTurnLanes << false << false << true;
    }
    else{
        inlanes << 3 << 1 << 2;
        createTurnLanes << true << false << false;
    }

    QList<int> outlanes;
    outlanes << 2 << 1 << 2;

    return CreateNode_3( x, y, inlanes, outlanes, createTurnLanes );
}


int DataManipulator::CreateNode_3x1x1_tr()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    QList<int> inlanes;
    inlanes << 1 << 1 << 1;

    QList<int> outlanes;
    outlanes << 1 << 1 << 1;

    QList<bool> createTurnLanes;
    createTurnLanes << false << false << false;

    QStringList turnRestirct;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        turnRestirct << "12" << "2" << "0";
    }
    else{
        turnRestirct << "2" << "0" << "01";
    }

    return CreateNode_3( x, y, inlanes, outlanes, createTurnLanes, turnRestirct );
}


int DataManipulator::CreateNode_3x2x1_tr()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    QList<int> inlanes;
    inlanes << 2 << 1 << 2;

    QList<int> outlanes;
    outlanes << 2 << 1 << 2;

    QList<bool> createTurnLanes;
    createTurnLanes << false << false << false;

    QStringList turnRestirct;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        turnRestirct << "12" << "2" << "0";
    }
    else{
        turnRestirct << "2" << "0" << "01";
    }

    return CreateNode_3( x, y, inlanes, outlanes, createTurnLanes, turnRestirct );
}


int DataManipulator::CreateNode_3x3x1_tr()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    QList<int> inlanes;
    inlanes << 3 << 1 << 3;

    QList<int> outlanes;
    outlanes << 3 << 1 << 3;

    QList<bool> createTurnLanes;
    createTurnLanes << false << false << false;

    QStringList turnRestirct;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        turnRestirct << "12" << "2" << "0";
    }
    else{
        turnRestirct << "2" << "0" << "01";
    }

    return CreateNode_3( x, y, inlanes, outlanes, createTurnLanes, turnRestirct );
}


int DataManipulator::CreateNode_3x1x1_r()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    QList<int> inlanes;
    QList<bool> createTurnLanes;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        inlanes << 1 << 2 << 2;
        createTurnLanes << false << true << true;
    }
    else{
        inlanes << 2 << 2 << 1;
        createTurnLanes << true << true << false;
    }

    QList<int> outlanes;
    outlanes << 1 << 1 << 1;

    return CreateNode_3( x, y, inlanes, outlanes, createTurnLanes );
}


int DataManipulator::CreateNode_3x2x1_r()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    QList<int> inlanes;
    QList<bool> createTurnLanes;
    if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
        inlanes << 2 << 2 << 3;
        createTurnLanes << false << true << true;
    }
    else{
        inlanes << 3 << 2 << 2;
        createTurnLanes << true << true << false;
    }

    QList<int> outlanes;
    outlanes << 2 << 1 << 2;

    return CreateNode_3( x, y, inlanes, outlanes, createTurnLanes );
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

    road->CheckLaneConnection();
    road->CreateWPData();

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x2x1_noTS()
{
    int cNodeID = CreateNode_4x2x1();
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

    road->CheckLaneConnection();
    road->CreateWPData();

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
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

    road->CheckLaneConnection();
    road->CreateWPData();

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_3x2x1_noTS()
{
    int cNodeID = CreateNode_3x2x1();
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

    road->CheckLaneConnection();
    road->CreateWPData();

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_3x1x1_tr_noTS()
{
    int cNodeID = CreateNode_3x1x1_tr();
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

    road->CheckLaneConnection();
    road->CreateWPData();

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_3x2x1_tr_noTS()
{
    int cNodeID = CreateNode_3x2x1_tr();
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

    road->CheckLaneConnection();
    road->CreateWPData();

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_3x3x1_tr_noTS()
{
    int cNodeID = CreateNode_3x3x1_tr();
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

    road->CheckLaneConnection();
    road->CreateWPData();

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x1x1_TS()
{
    int cNodeID = CreateNode_4x1x1();

    road->CheckLaneConnection();
    road->CreateWPData();

    int ndIdx = road->indexOfNode( cNodeID );
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){
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

            // StopLine for TS
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
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
    }

    road->nodes[ndIdx]->hasTS = true;

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }

    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_3x1x1_TS()
{
    int cNodeID = CreateNode_3x1x1();

    road->CheckLaneConnection();
    road->CreateWPData();

    int ndIdx = road->indexOfNode( cNodeID );
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){
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

            // StopLine for TS
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
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
    }

    road->nodes[ndIdx]->hasTS = true;

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }

    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_3x2x1_TS()
{
    int cNodeID = CreateNode_3x2x1();

    road->CheckLaneConnection();
    road->CreateWPData();

    int ndIdx = road->indexOfNode( cNodeID );
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){

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

            // StopLine for TS
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
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
    }

    road->nodes[ndIdx]->hasTS = true;

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }

    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_3x2x1_rm_TS()
{
    int cNodeID = CreateNode_3x2x1_rm();

    road->CheckLaneConnection();
    road->CreateWPData();

    int ndIdx = road->indexOfNode( cNodeID );
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){

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

            // StopLine for TS
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
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
    }

    road->nodes[ndIdx]->hasTS = true;

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }

    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x2x1_TS()
{
    int cNodeID = CreateNode_4x2x1();

    road->CheckLaneConnection();
    road->CreateWPData();

    int ndIdx = road->indexOfNode( cNodeID );
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){
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

            // StopLine for TS
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
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
    }

    road->nodes[ndIdx]->hasTS = true;

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }

    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x2x2_TS()
{
    int cNodeID = CreateNode_4x2x2();

    road->CheckLaneConnection();
    road->CreateWPData();

    int ndIdx = road->indexOfNode( cNodeID );
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){

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

            // StopLine for TS
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
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
    }

    road->nodes[ndIdx]->hasTS = true;

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }

    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x1x1_r_TS()
{
    int cNodeID = CreateNode_4x1x1_r();

    road->CheckLaneConnection();
    road->CreateWPData();

    int ndIdx = road->indexOfNode( cNodeID );
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){
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

            // StopLine for TS
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
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
    }

    road->nodes[ndIdx]->hasTS = true;

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_3x1x1_r_TS()
{
    int cNodeID = CreateNode_3x1x1_r();

    road->CheckLaneConnection();
    road->CreateWPData();

    int ndIdx = road->indexOfNode( cNodeID );
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){
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
            // StopLine for TS
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
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
    }

    road->nodes[ndIdx]->hasTS = true;

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_3x2x1_r_TS()
{
    int cNodeID = CreateNode_3x2x1_r();

    road->CheckLaneConnection();
    road->CreateWPData();

    int ndIdx = road->indexOfNode( cNodeID );
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){
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

            // StopLine for TS
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
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
    }

    road->nodes[ndIdx]->hasTS = true;

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x2x1_r_TS()
{
    int cNodeID = CreateNode_4x2x1_r();

    road->CheckLaneConnection();
    road->CreateWPData();

    int ndIdx = road->indexOfNode( cNodeID );
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){
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

            // StopLine for TS
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
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
    }

    road->nodes[ndIdx]->hasTS = true;

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
    }


    if( canvas ){
        canvas->update();
    }

    return cNodeID;
}


int DataManipulator::CreateNode_4x2x2_r_TS()
{
    int cNodeID = CreateNode_4x2x2_r();

    road->CheckLaneConnection();
    road->CreateWPData();

    int ndIdx = road->indexOfNode( cNodeID );
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

        if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){
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

            // StopLine for TS
            int sl = CreateStopLine( cNodeID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
            if( sl >= 0 ){
                road->CheckStopLineCrossLanes( sl );
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
    }

    road->nodes[ndIdx]->hasTS = true;

    if( road->updateCPEveryOperation == true ){
        QList<int> nodeList;
        nodeList.append( ndIdx );
        road->CheckLaneCrossPointsInsideNode( nodeList );
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

            int lId = road->CreateLane( -1, startPoint, cID, 2, true, endPoint, cID, 0, true );
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

                lId = road->CreateLane( -1, startPoint, cID, 2, true, endPoint, cID, 1, true );
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

                        int lId1 = road->CreateLane( -1, startPoint, cID, i, true, endPoint, tID, 0, true );

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

                    int lId1 = road->CreateLane( -1, startPoint, cID, i, true, endPoint, tID, 0, true );

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

                        int lId2 = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, true );

                        road->SetNodeRelatedLane( cID, lId2 );
                        road->SetNodeRelatedLane( tID, lId2 );
                    }
                }
            }
        }
    }

    road->CheckLaneConnectionOfNode( cID );

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

            int lId = road->CreateLane( -1, startPoint, cID, 2, true, endPoint, cID, 0, true );
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

                lId = road->CreateLane( -1, startPoint, cID, 1, true, endPoint, cID, 0, true );
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

                        int lId1 = road->CreateLane( -1, startPoint, cID, i, true, endPoint, tID, 0, true );

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

                    int lId2 = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, true );

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

                        int lId2 = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, true );

                        road->SetNodeRelatedLane( cID, lId2 );
                        road->SetNodeRelatedLane( tID, lId2 );
                    }
                }
            }
        }
    }

    road->CheckLaneConnectionOfNode( cID );

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

            int lId = road->CreateLane( -1, startPoint, cID, 2, true, endPoint, cID, 0, true );
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

                lId = road->CreateLane( -1, startPoint, cID, 2, true, endPoint, cID, 1, true );
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

                        int lId1 = road->CreateLane( -1, startPoint, cID, i, true, endPoint, tID, 0, true );

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

                    int lId1 = road->CreateLane( -1, startPoint, cID, i, true, endPoint, tID, 0, true );

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

                        int lId2 = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, true );

                        road->SetNodeRelatedLane( cID, lId2 );
                        road->SetNodeRelatedLane( tID, lId2 );
                    }
                }
            }
        }
    }

    road->CheckLaneConnectionOfNode( cID );

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

            int lId = road->CreateLane( -1, startPoint, cID, 2, true, endPoint, cID, 0, true );
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

                lId = road->CreateLane( -1, startPoint, cID, 1, true, endPoint, cID, 0, true );
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

                        int lId1 = road->CreateLane( -1, startPoint, cID, i, true, endPoint, tID, 0, true );

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

                    int lId2 = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, true );

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

                        int lId2 = road->CreateLane( -1, startPoint, tID, 0, true, endPoint, cID, i, true );

                        road->SetNodeRelatedLane( cID, lId2 );
                        road->SetNodeRelatedLane( tID, lId2 );
                    }
                }
            }
        }
    }

    road->CheckLaneConnectionOfNode( cID );

    if( canvas ){
        canvas->update();
    }

    return cID;
}


int DataManipulator::CreateNode_straight_1()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    int cID = CreateStraight(x,y,1);

    road->CheckLaneConnectionOfNode( cID );

    if( canvas ){
        canvas->update();
    }

    return cID;
}


int DataManipulator::CreateNode_straight_2()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    int cID = CreateStraight(x,y,2);

    road->CheckLaneConnectionOfNode( cID );

    if( canvas ){
        canvas->update();
    }

    return cID;
}


int DataManipulator::CreateNode_straight_3()
{
    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    int cID = CreateStraight(x,y,3);

    road->CheckLaneConnectionOfNode( cID );

    if( canvas ){
        canvas->update();
    }

    return cID;
}


void DataManipulator::CreateNode_Dialog()
{
    qDebug() << "[DataManipulator::CreateNode_Dialog]";

    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

    QDialog *dialog = new QDialog();

    QRadioButton *is4Leg = new QRadioButton("4-Leg");
    is4Leg->setChecked(true);
    QRadioButton *is3Leg = new QRadioButton("3-Leg");
    QRadioButton *is5Leg = new QRadioButton("5-Leg");
    QRadioButton *is6Leg = new QRadioButton("6-Leg");

    QLineEdit *inLaneStr = new QLineEdit();
    inLaneStr->setMinimumWidth(300);
    inLaneStr->setSizePolicy( QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding) );
    QLineEdit *outLaneStr = new QLineEdit();
    outLaneStr->setMinimumWidth(300);
    outLaneStr->setSizePolicy( QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding) );
    QLineEdit *turnLaneStr = new QLineEdit();
    turnLaneStr->setMinimumWidth(300);
    turnLaneStr->setSizePolicy( QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding) );
    QLineEdit *turnRestrictStr = new QLineEdit();
    turnRestrictStr->setMinimumWidth(300);
    turnRestrictStr->setSizePolicy( QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding) );

    QCheckBox *isTSIntersect = new QCheckBox("Signalized Intersection");


    QPushButton *acceptBtn = new QPushButton("Accept");
    acceptBtn->setIcon( QIcon(":/images/accept.png") );
    acceptBtn->setDefault(true);
    connect( acceptBtn, SIGNAL(clicked()), dialog, SLOT(accept()) );

    QPushButton *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setIcon( QIcon(":/images/delete.png") );
    connect( cancelBtn, SIGNAL(clicked()), dialog, SLOT(reject()) );


    QHBoxLayout *radioLay = new QHBoxLayout();
    radioLay->addWidget( new QLabel("Type:") );
    radioLay->addWidget( is4Leg );
    radioLay->addWidget( is3Leg );
    radioLay->addWidget( is5Leg );
    radioLay->addWidget( is6Leg );
    radioLay->addStretch( 1 );


    QGridLayout *gLay = new QGridLayout();
    gLay->setColumnMinimumWidth(1,300);
    gLay->addWidget( new QLabel("In-Lane Numbers:"), 0, 0, Qt::AlignLeft );
    gLay->addWidget( new QLabel("Out-Lane Numbers:"), 1, 0, Qt::AlignLeft );
    gLay->addWidget( new QLabel("Turn Lanes(0:no, 1:yes):"), 2, 0, Qt::AlignLeft );
    gLay->addWidget( new QLabel("Turn Restriction:"), 3, 0, Qt::AlignLeft );
    gLay->addWidget( new QLabel("(-1:no, LegID:Allowed Dir.)"), 4, 0, Qt::AlignLeft );

    gLay->addWidget( inLaneStr, 0, 1, Qt::AlignLeft );
    gLay->addWidget( outLaneStr, 1, 1, Qt::AlignLeft );
    gLay->addWidget( turnLaneStr, 2, 1, Qt::AlignLeft );
    gLay->addWidget( turnRestrictStr, 3, 1, Qt::AlignLeft );
    gLay->addWidget( isTSIntersect, 5, 1, Qt::AlignLeft );


    QHBoxLayout *btnLay = new QHBoxLayout();
    btnLay->addStretch(1);
    btnLay->addWidget( acceptBtn );
    btnLay->addSpacing( 50 );
    btnLay->addWidget( cancelBtn );
    btnLay->addStretch(1);


    QVBoxLayout *mainLay = new QVBoxLayout();
    mainLay->addLayout( radioLay );
    mainLay->addLayout( gLay );
    mainLay->addLayout( btnLay );

    dialog->setLayout( mainLay );

    dialog->setFixedSize( dialog->sizeHint() );

    int ret = dialog->exec();
    qDebug() << "ret = " << ret;

    // If Accepted,
    if( ret == 1 ){

        // Check
        if( inLaneStr->text().isEmpty() || outLaneStr->text().isEmpty() ){
            qDebug() << "No data suppiled.";
            return;
        }

        if( is4Leg->isChecked() == true ){
            qDebug() << "Create 4-Leg Intersection";

            QStringList inLaneDiv = inLaneStr->text().split(",");
            if( inLaneDiv.size() != 4 ){
                qDebug() << "Size of In-Lane Number data is not 4.";
                return;
            }

            QStringList outLaneDiv = outLaneStr->text().split(",");
            if( outLaneDiv.size() != 4 ){
                qDebug() << "Size of Out-Lane Number data is not 4.";
                return;
            }

            QStringList turnLaneDiv = turnLaneStr->text().split(",");
            if( turnLaneDiv.size() != 4 ){
                qDebug() << "Size of Turn-Lane Flag data is not 4. Reset Flag data";
                turnLaneDiv = QStringList();
            }

            QStringList turnRestrict = turnRestrictStr->text().split(",");
            if( turnRestrict.size() != 4 ){
                qDebug() << "Size of Turn-Restriction data is not 4. Reset Restriction data";
                turnRestrict = QStringList();
            }

            QList<int> inlanes;
            for(int i=0;i<4;++i){
                inlanes << QString(inLaneDiv[i]).trimmed().toInt();
            }

            QList<int> outlanes;
            for(int i=0;i<4;++i){
                outlanes << QString(outLaneDiv[i]).trimmed().toInt();
            }

            QList<bool> turnlanes;
            if( turnLaneDiv.size() == 0 ){
                for(int i=0;i<4;++i){
                    turnlanes << false;
                }
            }
            else{
                for(int i=0;i<4;++i){
                    turnlanes << ( QString(turnLaneDiv[i]).trimmed().toInt() == 1 ? true : false);
                }
            }

            int cID = CreateNode_4( x, y, inlanes, outlanes, turnlanes, turnRestrict );
            int ndIdx = road->indexOfNode( cID );

            road->CheckLaneConnection();
            road->CreateWPData();

            if( isTSIntersect->isChecked() == true ){

                road->nodes[ndIdx]->hasTS = true;

                for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

                    if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){
                        // TS for Vehicle
                        int vtsID = CreateTrafficSignal( cID, road->nodes[ndIdx]->legInfo[i]->legID , 0 );
                        int vtsIdx = road->indexOfTS( vtsID, cID );
                        if( vtsIdx >= 0 ){
                            if(i % 2 == 0 ){
                                road->nodes[ndIdx]->trafficSignals[vtsIdx]->startOffset = 0;
                            }
                            else{
                                road->nodes[ndIdx]->trafficSignals[vtsIdx]->startOffset = 60;
                            }
                        }

                        // StopLine for TS
                        int sl = CreateStopLine( cID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
                        if( sl >= 0 ){
                            road->CheckStopLineCrossLanes( sl );
                        }
                    }

                    // TS for Pedestrian
                    int ptsId = CreateTrafficSignal( cID, road->nodes[ndIdx]->legInfo[i]->legID , 1 );
                    int ptsIdx = road->indexOfTS( ptsId, cID );
                    if( ptsIdx >= 0 ){
                        if(i % 2 == 1 ){
                            road->nodes[ndIdx]->trafficSignals[ptsIdx]->startOffset = 0;
                        }
                        else{
                            road->nodes[ndIdx]->trafficSignals[ptsIdx]->startOffset = 60;
                        }
                    }
                }
            }

            if( road->updateCPEveryOperation == true ){
                QList<int> nodeList;
                nodeList.append( ndIdx );
                road->CheckLaneCrossPointsInsideNode( nodeList );
            }
        }
        else if( is3Leg->isChecked() == true ){
            qDebug() << "Create 3-Leg Intersection";

            QStringList inLaneDiv = inLaneStr->text().split(",");
            if( inLaneDiv.size() != 3 ){
                qDebug() << "Size of In-Lane Number data is not 3.";
                return;
            }

            QStringList outLaneDiv = outLaneStr->text().split(",");
            if( outLaneDiv.size() != 3 ){
                qDebug() << "Size of Out-Lane Number data is not 3.";
                return;
            }

            QStringList turnLaneDiv = turnLaneStr->text().split(",");
            if( turnLaneDiv.size() != 3 ){
                qDebug() << "Size of Turn-Lane Flag data is not 3. Reset Flag data";
                turnLaneDiv = QStringList();
            }

            QStringList turnRestrict = turnRestrictStr->text().split(",");
            if( turnRestrict.size() != 3 ){
                qDebug() << "Size of Turn-Restriction data is not 3. Reset Restriction data";
                turnRestrict = QStringList();
            }

            QList<int> inlanes;
            for(int i=0;i<3;++i){
                inlanes << QString(inLaneDiv[i]).trimmed().toInt();
            }

            QList<int> outlanes;
            for(int i=0;i<3;++i){
                outlanes << QString(outLaneDiv[i]).trimmed().toInt();
            }

            QList<bool> turnlanes;
            if( turnLaneDiv.size() == 0 ){
                for(int i=0;i<3;++i){
                    turnlanes << false;
                }
            }
            else{
                for(int i=0;i<3;++i){
                    turnlanes << ( QString(turnLaneDiv[i]).trimmed().toInt() == 1 ? true : false);
                }
            }

            int cID = CreateNode_3( x, y, inlanes, outlanes, turnlanes, turnRestrict );
            int ndIdx = road->indexOfNode( cID );

            road->CheckLaneConnection();
            road->CreateWPData();

            if( isTSIntersect->isChecked() == true ){

                road->nodes[ndIdx]->hasTS = true;

                for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

                    if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){

                        // TS for Vehicle
                        int vtsID = CreateTrafficSignal( cID, road->nodes[ndIdx]->legInfo[i]->legID , 0 );
                        int vtsIdx = road->indexOfTS( vtsID, cID );
                        if( vtsIdx >= 0 ){
                            if(i % 2 == 0 ){
                                road->nodes[ndIdx]->trafficSignals[vtsIdx]->startOffset = 0;
                            }
                            else{
                                road->nodes[ndIdx]->trafficSignals[vtsIdx]->startOffset = 60;
                            }
                        }

                        // StopLine for TS
                        int sl = CreateStopLine( cID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
                        if( sl >= 0 ){
                            road->CheckStopLineCrossLanes( sl );
                        }
                    }

                    // TS for Pedestrian
                    int ptsId = CreateTrafficSignal( cID, road->nodes[ndIdx]->legInfo[i]->legID , 1 );
                    int ptsIdx = road->indexOfTS( ptsId, cID );
                    if( ptsIdx >= 0 ){
                        if(i % 2 == 1 ){
                            road->nodes[ndIdx]->trafficSignals[ptsIdx]->startOffset = 0;
                        }
                        else{
                            road->nodes[ndIdx]->trafficSignals[ptsIdx]->startOffset = 60;
                        }
                    }
                }
            }

            if( road->updateCPEveryOperation == true ){
                QList<int> nodeList;
                nodeList.append( ndIdx );
                road->CheckLaneCrossPointsInsideNode( nodeList );
            }
        }
        else if( is5Leg->isChecked() == true ){
            qDebug() << "Create 5-Leg Intersection";

            QStringList inLaneDiv = inLaneStr->text().split(",");
            if( inLaneDiv.size() != 5 ){
                qDebug() << "Size of In-Lane Number data is not 5.";
                return;
            }

            QStringList outLaneDiv = outLaneStr->text().split(",");
            if( outLaneDiv.size() != 5 ){
                qDebug() << "Size of Out-Lane Number data is not 5.";
                return;
            }

            QStringList turnLaneDiv = turnLaneStr->text().split(",");
            if( turnLaneDiv.size() != 5 ){
                qDebug() << "Size of Turn-Lane Flag data is not 5. Reset Flag data";
                turnLaneDiv = QStringList();
            }

            QStringList turnRestrict = turnRestrictStr->text().split(",");
            if( turnRestrict.size() != 5 ){
                qDebug() << "Size of Turn-Restriction data is not 5. Reset Restriction data";
                turnRestrict = QStringList();
            }

            QList<int> inlanes;
            for(int i=0;i<5;++i){
                inlanes << QString(inLaneDiv[i]).trimmed().toInt();
            }

            QList<int> outlanes;
            for(int i=0;i<5;++i){
                outlanes << QString(outLaneDiv[i]).trimmed().toInt();
            }

            QList<bool> turnlanes;
            if( turnLaneDiv.size() == 0 ){
                for(int i=0;i<5;++i){
                    turnlanes << false;
                }
            }
            else{
                for(int i=0;i<5;++i){
                    turnlanes << ( QString(turnLaneDiv[i]).trimmed().toInt() == 1 ? true : false);
                }
            }

            int cID = CreateNode_5( x, y, inlanes, outlanes, turnlanes, turnRestrict );
            int ndIdx = road->indexOfNode( cID );

            road->CheckLaneConnection();
            road->CreateWPData();

            if( isTSIntersect->isChecked() == true ){

                road->nodes[ndIdx]->hasTS = true;

                for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

                    if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){
                        // TS for Vehicle
                        int vtsID = CreateTrafficSignal( cID, road->nodes[ndIdx]->legInfo[i]->legID , 0 );
                        int vtsIdx = road->indexOfTS( vtsID, cID );
                        if( vtsIdx >= 0 ){
                            if(i % 2 == 0 ){
                                road->nodes[ndIdx]->trafficSignals[vtsIdx]->startOffset = 0;
                            }
                            else{
                                road->nodes[ndIdx]->trafficSignals[vtsIdx]->startOffset = 60;
                            }
                        }

                        // StopLine for TS
                        int sl = CreateStopLine( cID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
                        if( sl >= 0 ){
                            road->CheckStopLineCrossLanes( sl );
                        }
                    }

                    // TS for Pedestrian
                    int ptsId = CreateTrafficSignal( cID, road->nodes[ndIdx]->legInfo[i]->legID , 1 );
                    int ptsIdx = road->indexOfTS( ptsId, cID );
                    if( ptsIdx >= 0 ){
                        if(i % 2 == 1 ){
                            road->nodes[ndIdx]->trafficSignals[ptsIdx]->startOffset = 0;
                        }
                        else{
                            road->nodes[ndIdx]->trafficSignals[ptsIdx]->startOffset = 60;
                        }
                    }
                }
            }

            if( road->updateCPEveryOperation == true ){
                QList<int> nodeList;
                nodeList.append( ndIdx );
                road->CheckLaneCrossPointsInsideNode( nodeList );
            }
        }
        else if( is6Leg->isChecked() == true ){
            qDebug() << "Create 6-Leg Intersection";

            QStringList inLaneDiv = inLaneStr->text().split(",");
            if( inLaneDiv.size() != 6 ){
                qDebug() << "Size of In-Lane Number data is not 6.";
                return;
            }

            QStringList outLaneDiv = outLaneStr->text().split(",");
            if( outLaneDiv.size() != 6 ){
                qDebug() << "Size of Out-Lane Number data is not 6.";
                return;
            }

            QStringList turnLaneDiv = turnLaneStr->text().split(",");
            if( turnLaneDiv.size() != 6 ){
                qDebug() << "Size of Turn-Lane Flag data is not 6. Reset Flag data";
                turnLaneDiv = QStringList();
            }

            QStringList turnRestrict = turnRestrictStr->text().split(",");
            if( turnRestrict.size() != 6 ){
                qDebug() << "Size of Turn-Restriction data is not 6. Reset Restriction data";
                turnRestrict = QStringList();
            }

            QList<int> inlanes;
            for(int i=0;i<6;++i){
                inlanes << QString(inLaneDiv[i]).trimmed().toInt();
            }

            QList<int> outlanes;
            for(int i=0;i<6;++i){
                outlanes << QString(outLaneDiv[i]).trimmed().toInt();
            }

            QList<bool> turnlanes;
            if( turnLaneDiv.size() == 0 ){
                for(int i=0;i<6;++i){
                    turnlanes << false;
                }
            }
            else{
                for(int i=0;i<6;++i){
                    turnlanes << ( QString(turnLaneDiv[i]).trimmed().toInt() == 1 ? true : false);
                }
            }

            int cID = CreateNode_6( x, y, inlanes, outlanes, turnlanes, turnRestrict );
            int ndIdx = road->indexOfNode( cID );

            road->CheckLaneConnection();
            road->CreateWPData();

            if( isTSIntersect->isChecked() == true ){

                road->nodes[ndIdx]->hasTS = true;

                for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){

                    if( road->nodes[ndIdx]->legInfo[i]->inWPs.size() > 0 ){
                        // TS for Vehicle
                        int vtsID = CreateTrafficSignal( cID, road->nodes[ndIdx]->legInfo[i]->legID , 0 );
                        int vtsIdx = road->indexOfTS( vtsID, cID );
                        if( vtsIdx >= 0 ){
                            if(i % 2 == 0 ){
                                road->nodes[ndIdx]->trafficSignals[vtsIdx]->startOffset = 0;
                            }
                            else{
                                road->nodes[ndIdx]->trafficSignals[vtsIdx]->startOffset = 60;
                            }
                        }

                        // StopLine for TS
                        int sl = CreateStopLine( cID, road->nodes[ndIdx]->legInfo[i]->legID, _STOPLINE_KIND::STOPLINE_SIGNAL );
                        if( sl >= 0 ){
                            road->CheckStopLineCrossLanes( sl );
                        }
                    }

                    // TS for Pedestrian
                    int ptsId = CreateTrafficSignal( cID, road->nodes[ndIdx]->legInfo[i]->legID , 1 );
                    int ptsIdx = road->indexOfTS( ptsId, cID );
                    if( ptsIdx >= 0 ){
                        if(i % 2 == 1 ){
                            road->nodes[ndIdx]->trafficSignals[ptsIdx]->startOffset = 0;
                        }
                        else{
                            road->nodes[ndIdx]->trafficSignals[ptsIdx]->startOffset = 60;
                        }
                    }
                }
            }


            if( road->updateCPEveryOperation == true ){
                QList<int> nodeList;
                nodeList.append( ndIdx );
                road->CheckLaneCrossPointsInsideNode( nodeList );
            }
        }
    }

    if( canvas ){
        canvas->update();
    }
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


void DataManipulator::CreateTrafficSignalForVehicle()
{
    if( canvas->selectedObj.selObjKind.size() != 1 ){
        return;
    }

    if( canvas->selectedObj.selObjKind[0] != canvas->SEL_NODE ){
        return;
    }

    int nodeID = canvas->selectedObj.selObjID[0];
    int ndIdx = road->indexOfNode( nodeID );
    if( ndIdx < 0 ){
        return;
    }

    int dir = canvas->GetNumberKeyPressed() - 1;
    if( dir < 0 || dir >= road->nodes[ndIdx]->legInfo.size() ){
        return;
    }

    road->AddTrafficSignalToNode( nodeID, -1, 0, dir );

    canvas->update();
}


void DataManipulator::CreateTrafficSignalForPedestrian()
{
    if( canvas->selectedObj.selObjKind.size() != 1 ){
        return;
    }

    if( canvas->selectedObj.selObjKind[0] != canvas->SEL_NODE ){
        return;
    }

    int nodeID = canvas->selectedObj.selObjID[0];
    int ndIdx = road->indexOfNode( nodeID );
    if( ndIdx < 0 ){
        return;
    }

    int dir = canvas->GetNumberKeyPressed() - 1;
    if( dir < 0 || dir >= road->nodes[ndIdx]->legInfo.size() ){
        return;
    }

    road->AddTrafficSignalToNode( nodeID, -1, 1, dir );

    canvas->update();
}


void DataManipulator::CreateStopLineForInDir()
{
    if( canvas->selectedObj.selObjKind.size() != 1 ){
        return;
    }

    if( canvas->selectedObj.selObjKind[0] != canvas->SEL_NODE ){
        return;
    }

    int nodeID = canvas->selectedObj.selObjID[0];
    int ndIdx = road->indexOfNode( nodeID );
    if( ndIdx < 0 ){
        return;
    }

    int dir = canvas->GetNumberKeyPressed() - 1;
    if( dir < 0 || dir >= road->nodes[ndIdx]->legInfo.size() ){
        return;
    }

    road->AddStopLineToNode( nodeID, -1, dir );

    canvas->update();
}


