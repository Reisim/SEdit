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


#include "objectproperty.h"


void RoadObjectProperty::ChangeLaneInfo(int id)
{
    if( !laneInfo ){
        return;
    }

    laneInfo->clear();

    if( !road ){
        return;
    }

    int lIdx = road->indexOfLane( id );
    if( lIdx < 0 ){
        return;
    }

    QString infoStr = QString();

    infoStr += QString("\n");
    infoStr += QString("[Edge Info]\n");
    infoStr += QString("  Start: Related Node=%1, Leg=%2, Boundray=%3\n").arg( road->lanes[lIdx]->sWPInNode ).arg( road->lanes[lIdx]->sWPNodeDir ).arg( road->lanes[lIdx]->sWPBoundary );
    infoStr += QString("  End:   Related Node=%1, Leg=%2, Boundray=%3\n").arg( road->lanes[lIdx]->eWPInNode ).arg( road->lanes[lIdx]->eWPNodeDir ).arg( road->lanes[lIdx]->eWPBoundary );
    infoStr += QString("\n");
    infoStr += QString("[Node Info]\n");
    infoStr += QString("  Connected Node : Node %1\n").arg( road->lanes[lIdx]->connectedNode );
    infoStr += QString("      Out Direct : Leg %1\n").arg( road->lanes[lIdx]->connectedNodeOutDirect );
    infoStr += QString("      In Direct  : Leg %1\n").arg( road->lanes[lIdx]->connectedNodeInDirect );
    infoStr += QString("  Departure Node : Node %1\n").arg( road->lanes[lIdx]->departureNode );
    infoStr += QString("      Out Direct : Leg %1\n").arg( road->lanes[lIdx]->departureNodeOutDirect );
    infoStr += QString("\n");
    infoStr += QString("[Next Lanes]\n");
    for(int i=0;i<road->lanes[lIdx]->nextLanes.size();++i){
        infoStr += QString("  Lane %1\n" ).arg( road->lanes[lIdx]->nextLanes[i] );
    }
    infoStr += QString("\n");
    infoStr += QString("[Previous Lanes]\n");
    for(int i=0;i<road->lanes[lIdx]->previousLanes.size();++i){
        infoStr += QString("  Lane %1\n" ).arg( road->lanes[lIdx]->previousLanes[i] );
    }
    infoStr += QString("\n");
    infoStr += QString("[Cross Lanes]\n");
    for(int i=0;i<road->lanes[lIdx]->crossPoints.size();++i){
        infoStr += QString("  Lane %1:  (%2, %3)\n" ).arg( road->lanes[lIdx]->crossPoints[i]->crossLaneID )
                .arg( road->lanes[lIdx]->crossPoints[i]->pos.x() ).arg( road->lanes[lIdx]->crossPoints[i]->pos.y() );
        infoStr += QString("    Dist from Edge = %1\n").arg( road->lanes[lIdx]->crossPoints[i]->distanceFromLaneStartPoint );
    }
    infoStr += QString("\n");
    infoStr += QString("[Stop Point]\n");
    for(int i=0;i<road->lanes[lIdx]->stopPoints.size();++i){
        infoStr += QString("  StopLine %1:  (%2, %3)\n" ).arg( road->lanes[lIdx]->stopPoints[i]->stoplineID )
                .arg( road->lanes[lIdx]->stopPoints[i]->pos.x() ).arg( road->lanes[lIdx]->stopPoints[i]->pos.y() );
        infoStr += QString("    Dist from Edge = %1\n").arg( road->lanes[lIdx]->stopPoints[i]->distanceFromLaneStartPoint );
    }
    infoStr += QString("\n");
    infoStr += QString("[Shape]\n");
    infoStr += QString("  Length: %1\n").arg( road->lanes[lIdx]->shape.pathLength);

    float maxCurv = 0.0;
    for(int i=0;i<road->lanes[lIdx]->shape.curvature.size();++i){
        if( fabs(road->lanes[lIdx]->shape.curvature[i]) > maxCurv ){
            maxCurv = road->lanes[lIdx]->shape.curvature[i];
        }
    }
    infoStr += QString("  Max Curvature: %1\n").arg( maxCurv );


    disconnect( laneSpeed, SIGNAL(valueChanged(int)), this, SLOT(SpeedLimitChanged(int)) );

    laneSpeed->setValue( road->lanes[lIdx]->speedInfo );

    connect( laneSpeed, SIGNAL(valueChanged(int)), this, SLOT(SpeedLimitChanged(int)) );

    qDebug() << "speed limit = " << road->lanes[lIdx]->speedInfo;

    laneInfo->setText( infoStr );
    laneInfo->setAlignment( Qt::AlignTop );
}


void RoadObjectProperty::SpeedLimitChanged(int val)
{
    int laneID = laneIDSB->value();
    int lIdx = road->indexOfLane( laneID );
    if( lIdx < 0 ){
        return;
    }

    road->lanes[lIdx]->speedInfo = val;
}

