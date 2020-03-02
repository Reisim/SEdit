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


#include "roadinfo.h"
#include "QDebug"


int RoadInfo::CreateTrafficSignal(int assignId, int relatedNodeID, int relatedNodeDir, int TSType)
{
    int rndIdx = indexOfNode( relatedNodeID );
    if( rndIdx < 0 ){
        qDebug() << "[RoadInfo::CreateTrafficSignal] invalid relatedNodeID, no Node found.";
        return -1;
    }

    bool nDirCheck = false;
    float angle = 0.0;
    int nLane = 1;
    for(int i=0;i<nodes[rndIdx]->legInfo.size();++i){
        if( nodes[rndIdx]->legInfo[i]->legID == relatedNodeDir ){
            nDirCheck = true;
            angle = nodes[rndIdx]->legInfo[i]->angle * 0.017452;
            nLane = nodes[rndIdx]->legInfo[i]->nLaneIn;
            break;
        }
    }
    if( nDirCheck == false ){
        qDebug() << "[RoadInfo::CreateTrafficSignal] invalid relatedNodeDir, no Leg found.";
        return -1;
    }

    // Id check
    int cId = -1;
    if( assignId < 0 ){
        cId = 0;
        for(int i=0;i<nodes.size();++i){
            for(int j=0;j<nodes[i]->trafficSignals.size();++j){
                if( cId <= nodes[i]->trafficSignals[j]->id ){
                    cId = nodes[i]->trafficSignals[j]->id + 1;
                }
            }
        }
    }
    else{
        if( indexOfTS(assignId,-1) < 0 ){
            cId = assignId;
        }
        else{
            qDebug() << "[CreateTrafficSignal] assigned ID already exists.";
            return -1;
        }
    }

    struct TrafficSignalInfo *tsInfo = new TrafficSignalInfo;

    tsInfo->id = cId;
    tsInfo->type = TSType;

    tsInfo->relatedNode = relatedNodeID;
    tsInfo->controlNodeDirection = relatedNodeDir;
    tsInfo->controlNodeLane = -1;  // Control all lane as default
    tsInfo->controlCrossWalk = -1; // For Pedestrian Signal


    float cp = cos( angle );
    float sp = sin( angle );

    if( TSType == 0 ){  // for vehicles

        float shift = -2.0 - (nLane - 1) * 3.0;
        if( LeftOrRight == RIGHT_HAND_TRAFFIC ){
            shift = 2.0 + (nLane - 1) * 3.0;
        }
        float x_TS = nodes[rndIdx]->pos.x() + 5.0 * cp - shift * sp;
        float y_TS = nodes[rndIdx]->pos.y() + 5.0 * sp + shift * cp;

        tsInfo->pos.setX( x_TS );
        tsInfo->pos.setY( y_TS );
        tsInfo->pos.setZ( 5.0 );

        tsInfo->facingDirect = angle * 57.3;


        // Set Default Signal Pattern
        struct SignalPatternData *sd = new struct SignalPatternData;
        sd->signal = 1;
        sd->duration = 55;
        tsInfo->sigPattern.append( sd );

        sd = new struct SignalPatternData;
        sd->signal = 2;
        sd->duration = 3;
        tsInfo->sigPattern.append( sd );

        sd = new struct SignalPatternData;
        sd->signal = 4;
        sd->duration = 62;
        tsInfo->sigPattern.append( sd );
    }
    else if( TSType == 1 ){  // for pedestrian

        float shift = -3.0- (nLane - 1) * 3.0;
        if( LeftOrRight == RIGHT_HAND_TRAFFIC ){
            shift = 3.0 + (nLane - 1) * 3.0;
        }
        float x_TS = nodes[rndIdx]->pos.x() + 6.0 * cp - shift * sp;
        float y_TS = nodes[rndIdx]->pos.y() + 6.0 * sp + shift * cp;

        tsInfo->pos.setX( x_TS );
        tsInfo->pos.setY( y_TS );
        tsInfo->pos.setZ( 3.0 );

        tsInfo->facingDirect = angle * 57.3 + 90.0;


        // Set Default Signal Pattern
        struct SignalPatternData *sd = new struct SignalPatternData;
        sd->signal = 1;
        sd->duration = 45;
        tsInfo->sigPattern.append( sd );

        sd = new struct SignalPatternData;
        sd->signal = 2;
        sd->duration = 3;
        tsInfo->sigPattern.append( sd );

        sd = new struct SignalPatternData;
        sd->signal = 4;
        sd->duration = 72;
        tsInfo->sigPattern.append( sd );
    }

    nodes[rndIdx]->trafficSignals.append( tsInfo );

    return cId;
}


int RoadInfo::GetNearestTrafficSignal(QVector2D pos,float &dist)
{
    int ret = -1;
    float nearDist = 0.0;

    for(int i=0;i<nodes.size();++i){

        float dx = nodes[i]->pos.x() - pos.x();
        float dy = nodes[i]->pos.y() - pos.y();
        float D = dx * dx + dy * dy;
        if( D > 400.0 ){
            continue;
        }

        for(int j=0;j<nodes[i]->trafficSignals.size();++j){

            dx = nodes[i]->trafficSignals[j]->pos.x() - pos.x();
            dy = nodes[i]->trafficSignals[j]->pos.y() - pos.y();
            D = dx * dx + dy * dy;
            if( ret < 0 || nearDist > D ){
                ret = nodes[i]->trafficSignals[j]->id;
                nearDist = D;
            }
        }
    }

    qDebug() << "[GetNearestTrafficSignal] ret = " << ret << " nearDist = " << nearDist;

    dist = nearDist;

    return ret;
}


void RoadInfo::MoveTrafficSignal(int id, float moveX, float moveY)
{
    int rNd = indexOfTS(id,-1);
    if( rNd < 0 ){
        qDebug() << "[MoveTrafficSignal] cannot find node contain TS id = " << id;
        return;
    }
    int idx = indexOfTS(id,rNd);
    if( idx < 0 ){
        qDebug() << "[MoveTrafficSignal] cannot find index of TS id = " << id;
        return;
    }
    int ndIdx = indexOfNode(rNd);
    if( ndIdx < 0 ){
        qDebug() << "[MoveTrafficSignal] cannot find index of Node id = " << rNd;
        return;
    }

    nodes[ndIdx]->trafficSignals[idx]->pos.setX( nodes[ndIdx]->trafficSignals[idx]->pos.x() + moveX );
    nodes[ndIdx]->trafficSignals[idx]->pos.setY( nodes[ndIdx]->trafficSignals[idx]->pos.y() + moveY );
}


