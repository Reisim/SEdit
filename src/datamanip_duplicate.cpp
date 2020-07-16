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
#include <qDebug>

#include <QMap>

void DataManipulator::DuplicateNodes()
{
    qDebug() << "[DataManipulator::DuplicateNodes]";

    if( canvas->selectedObj.selObjKind.size() == 0 ){
        qDebug() << "selObjKind.size() = 0";
        return;
    }


    float createAtX = 0.0;
    float createAtY = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), createAtX, createAtY );
    }


    int maxNodeID = -1;
    for(int j=0;j<road->nodes.size();++j){
        if( maxNodeID < road->nodes[j]->id ){
            maxNodeID = road->nodes[j]->id;
        }
    }
    maxNodeID++;

    int maxLaneID = -1;
    for(int j=0;j<road->lanes.size();++j){
        if( maxLaneID < road->lanes[j]->id ){
            maxLaneID = road->lanes[j]->id;
        }
    }
    maxLaneID++;

    int maxTSID = -1;
    for(int j=0;j<road->nodes.size();++j){
        for(int k=0;k<road->nodes[j]->trafficSignals.size();++k){
            if( maxTSID < road->nodes[j]->trafficSignals[k]->id ){
                maxTSID = road->nodes[j]->trafficSignals[k]->id;
            }
        }
    }
    maxTSID++;

    int maxSLID = -1;
    for(int j=0;j<road->nodes.size();++j){
        for(int k=0;k<road->nodes[j]->stopLines.size();++k){
            if( maxSLID < road->nodes[j]->stopLines[k]->id ){
                maxSLID = road->nodes[j]->stopLines[k]->id;
            }
        }
    }
    maxSLID++;


    float centerX = 0.0;
    float centerY = 0.0;
    int nNode = 0;
    QList<int> correspondIDs;
    QMap<int,int> laneCorrensponding;
    QList<int> lanesListToBeDup;

    for(int i=0;i<canvas->selectedObj.selObjKind.size();++i){

        if( canvas->selectedObj.selObjKind[i] != canvas->SEL_NODE ){
            correspondIDs.append( -1 );
            continue;
        }

        int ndIdx = road->indexOfNode( canvas->selectedObj.selObjID[i] );
        if( ndIdx < 0 ){
            correspondIDs.append( -1 );
            continue;
        }

        correspondIDs.append( maxNodeID );
        maxNodeID++;

        centerX += road->nodes[ndIdx]->pos.x();
        centerY += road->nodes[ndIdx]->pos.y();
        nNode++;

        for(int j=0;j<road->nodes[ndIdx]->relatedLanes.size();++j){
            if( lanesListToBeDup.indexOf( road->nodes[ndIdx]->relatedLanes[j] ) < 0 ){
                laneCorrensponding.insert( road->nodes[ndIdx]->relatedLanes[j], maxLaneID );
                maxLaneID++;

                lanesListToBeDup.append( road->nodes[ndIdx]->relatedLanes[j] );
            }
        }
    }

    qDebug() << "laneCorrensponding.size = " << laneCorrensponding.size();
    qDebug() << "lanesListToBeDup.size = " << lanesListToBeDup.size();

    if( nNode == 0 ){
        return;
    }

    centerX /= nNode;
    centerY /= nNode;


    // Check all connected node is selected
    bool selectionValid = true;
    for(int i=0;i<canvas->selectedObj.selObjKind.size();++i){
        if( canvas->selectedObj.selObjKind[i] != canvas->SEL_NODE ){
            continue;
        }

        int ndIdx = road->indexOfNode( canvas->selectedObj.selObjID[i] );
        if( ndIdx < 0 ){
            continue;
        }

        int nLeg = road->nodes[ndIdx]->legInfo.size();
        for(int j=0;j<nLeg;++j){
            bool found = false;
            for(int k=0;k<canvas->selectedObj.selObjKind.size();++k){
                if( canvas->selectedObj.selObjKind[k] != canvas->SEL_NODE ){
                    continue;
                }
                if( canvas->selectedObj.selObjID[k] == road->nodes[ndIdx]->legInfo[j]->connectedNode ){
                    found = true;
                    break;
                }
            }
            if( found == false ){
                selectionValid = false;
                break;
            }

            found = true;
            for(int k=0;k<canvas->selectedObj.selObjKind.size();++k){
                if( canvas->selectedObj.selObjKind[k] != canvas->SEL_NODE ){
                    continue;
                }
                if( canvas->selectedObj.selObjID[k] == road->nodes[ndIdx]->legInfo[j]->connectingNode ){
                    found = true;
                    break;
                }
            }
            if( found == false ){
                selectionValid = false;
                break;
            }
        }
    }
    if( selectionValid == false ){
        qDebug() << "All the necesarry nodes to duplicate node-network is not collected in the selection.";
        return;
    }

    QList<int> newNodeList;

    for(int i=0;i<canvas->selectedObj.selObjKind.size();++i){

        if( canvas->selectedObj.selObjKind[i] != canvas->SEL_NODE ){
            continue;
        }

        int ndIdx = road->indexOfNode( canvas->selectedObj.selObjID[i] );
        if( ndIdx < 0 ){
            continue;
        }


        int assignID = correspondIDs[i];
        float xNd = road->nodes[ndIdx]->pos.x() - centerX + createAtX;
        float yNd = road->nodes[ndIdx]->pos.y() - centerY + createAtY;
        int nLeg = road->nodes[ndIdx]->legInfo.size();

        QList<int> inlanes;
        QList<int> outlanes;
        for(int j=0;j<nLeg;++j){
            inlanes.append( road->nodes[ndIdx]->legInfo[j]->nLaneIn );
            outlanes.append( road->nodes[ndIdx]->legInfo[j]->nLaneOut );
        }

        road->CreateNode( assignID, xNd, yNd, inlanes, outlanes );
        newNodeList.append( assignID );

        int crIdx = road->indexOfNode( assignID );

        road->nodes[crIdx]->hasTS = road->nodes[ndIdx]->hasTS;
        road->nodes[crIdx]->isDestinationNode = road->nodes[ndIdx]->isDestinationNode;
        road->nodes[crIdx]->isOriginNode = road->nodes[ndIdx]->isOriginNode;
        road->nodes[crIdx]->nLeg = road->nodes[ndIdx]->nLeg;
        for(int j=0;j<nLeg;++j){
            road->nodes[crIdx]->legInfo[j]->legID = road->nodes[ndIdx]->legInfo[j]->legID;
            road->nodes[crIdx]->legInfo[j]->angle = road->nodes[ndIdx]->legInfo[j]->angle;
            road->nodes[crIdx]->legInfo[j]->connectedNodeOutDirect = road->nodes[ndIdx]->legInfo[j]->connectedNodeOutDirect;
            road->nodes[crIdx]->legInfo[j]->connectingNodeInDirect = road->nodes[ndIdx]->legInfo[j]->connectingNodeInDirect;

            for(int k=0;k<canvas->selectedObj.selObjKind.size();++k){
                if( canvas->selectedObj.selObjKind[k] != canvas->SEL_NODE ){
                    continue;
                }
                if( canvas->selectedObj.selObjID[k] == road->nodes[ndIdx]->legInfo[j]->connectedNode ){
                    road->nodes[crIdx]->legInfo[j]->connectedNode = correspondIDs[k];
                }
                if( canvas->selectedObj.selObjID[k] == road->nodes[ndIdx]->legInfo[j]->connectingNode ){
                    road->nodes[crIdx]->legInfo[j]->connectingNode = correspondIDs[k];
                }
            }
        }


        for(int j=0;j<road->nodes[ndIdx]->trafficSignals.size();++j){

            int tsID = road->CreateTrafficSignal(-1,
                                                 assignID,
                                                 road->nodes[ndIdx]->trafficSignals[j]->controlNodeDirection,
                                                 road->nodes[ndIdx]->trafficSignals[j]->type );

            int tsIdx = road->indexOfTS( tsID, assignID );
            if( tsIdx >= 0 ){

                road->nodes[crIdx]->trafficSignals[tsIdx]->controlCrossWalk = road->nodes[ndIdx]->trafficSignals[j]->controlCrossWalk;
                road->nodes[crIdx]->trafficSignals[tsIdx]->controlNodeLane = road->nodes[ndIdx]->trafficSignals[j]->controlNodeLane;
                road->nodes[crIdx]->trafficSignals[tsIdx]->facingDirect = road->nodes[ndIdx]->trafficSignals[j]->facingDirect;

                road->nodes[crIdx]->trafficSignals[tsIdx]->pos.setX( road->nodes[ndIdx]->trafficSignals[j]->pos.x() - centerX + createAtX );
                road->nodes[crIdx]->trafficSignals[tsIdx]->pos.setY( road->nodes[ndIdx]->trafficSignals[j]->pos.y() - centerY + createAtY );

                road->nodes[crIdx]->trafficSignals[tsIdx]->startOffset = road->nodes[ndIdx]->trafficSignals[j]->startOffset;
                road->nodes[crIdx]->trafficSignals[tsIdx]->sigPattern.clear();
                for(int k=0;k<road->nodes[ndIdx]->trafficSignals[j]->sigPattern.size();++k){

                    struct SignalPatternData *spd = new struct SignalPatternData;

                    spd->signal = road->nodes[ndIdx]->trafficSignals[j]->sigPattern[k]->signal;
                    spd->duration = road->nodes[ndIdx]->trafficSignals[j]->sigPattern[k]->duration;

                    road->nodes[crIdx]->trafficSignals[tsIdx]->sigPattern.append( spd );
                }
            }
        }

        for(int j=0;j<road->nodes[ndIdx]->stopLines.size();++j){

            int cId = road->CreateStopLine(-1,
                                           assignID,
                                           road->nodes[ndIdx]->stopLines[j]->relatedNodeDir,
                                           road->nodes[ndIdx]->stopLines[j]->stopLineType );

            int slIdx = road->indexOfSL( cId, assignID );
            if( slIdx >= 0 ){
                road->nodes[crIdx]->stopLines[slIdx]->leftEdge.setX( road->nodes[ndIdx]->stopLines[j]->leftEdge.x() - centerX + createAtX );
                road->nodes[crIdx]->stopLines[slIdx]->leftEdge.setY( road->nodes[ndIdx]->stopLines[j]->leftEdge.y() - centerY + createAtY );
                road->nodes[crIdx]->stopLines[slIdx]->rightEdge.setX( road->nodes[ndIdx]->stopLines[j]->rightEdge.x() - centerX + createAtX );
                road->nodes[crIdx]->stopLines[slIdx]->rightEdge.setY( road->nodes[ndIdx]->stopLines[j]->rightEdge.y() - centerY + createAtY );
            }
        }
    }

    QMapIterator<int,int> i(laneCorrensponding);
    while( i.hasNext() ){
        i.next();

        int origLaneID = i.key();
        int lIdx = road->indexOfLane( origLaneID );
        if( lIdx < 0 ){
            qDebug() << "Invalid Lane ID: " << origLaneID;
            continue;
        }

        QVector4D startPoint;
        QVector4D endPoint;

        startPoint.setX( road->lanes[lIdx]->shape.pos.first()->x() - centerX + createAtX  );
        startPoint.setY( road->lanes[lIdx]->shape.pos.first()->y() - centerY + createAtY );
        startPoint.setZ( road->lanes[lIdx]->shape.pos.first()->z() );
        startPoint.setW( atan2( road->lanes[lIdx]->shape.derivative.first()->y(), road->lanes[lIdx]->shape.derivative.first()->x() ) );

        endPoint.setX( road->lanes[lIdx]->shape.pos.last()->x() - centerX + createAtX );
        endPoint.setY( road->lanes[lIdx]->shape.pos.last()->y() - centerY + createAtY );
        endPoint.setZ( road->lanes[lIdx]->shape.pos.last()->z() );
        endPoint.setW( atan2( road->lanes[lIdx]->shape.derivative.last()->y(), road->lanes[lIdx]->shape.derivative.last()->x() ) );

        int assignID = i.value();

        int sWPNode = road->lanes[lIdx]->sWPInNode;
        int eWPNode = road->lanes[lIdx]->eWPInNode;

        for(int k=0;k<canvas->selectedObj.selObjKind.size();++k){
            if( canvas->selectedObj.selObjKind[k] != canvas->SEL_NODE ){
                continue;
            }
            if( canvas->selectedObj.selObjID[k] == road->lanes[lIdx]->sWPInNode ){
                sWPNode = correspondIDs[k];
            }
            if( canvas->selectedObj.selObjID[k] == road->lanes[lIdx]->eWPInNode ){
                eWPNode = correspondIDs[k];
            }
        }

        int sWPDir = road->lanes[lIdx]->sWPNodeDir;
        int eWPDir = road->lanes[lIdx]->eWPNodeDir;

        bool sWPBoundary = road->lanes[lIdx]->sWPBoundary;
        bool eWPBoundary = road->lanes[lIdx]->eWPBoundary;

        qDebug() << "Create Lane : " << assignID;

        int lId = road->CreateLane( assignID,
                                    startPoint, sWPNode, sWPDir, sWPBoundary,
                                    endPoint, eWPNode, eWPDir, eWPBoundary );
        road->SetNodeRelatedLane( sWPNode, lId );
        if( sWPNode != eWPNode ){
            road->SetNodeRelatedLane( eWPNode, lId );
        }

        int clIdx = road->indexOfLane(  assignID );

        road->lanes[clIdx]->actualSpeed = road->lanes[lIdx]->actualSpeed;
        road->lanes[clIdx]->automaticDrivingEnabled = road->lanes[lIdx]->automaticDrivingEnabled;

        for(int k=0;k<canvas->selectedObj.selObjKind.size();++k){
            if( canvas->selectedObj.selObjKind[k] != canvas->SEL_NODE ){
                continue;
            }
            if( canvas->selectedObj.selObjID[k] == road->lanes[lIdx]->connectedNode ){
                road->lanes[clIdx]->connectedNode = correspondIDs[k];
            }
            if( canvas->selectedObj.selObjID[k] == road->lanes[lIdx]->departureNode ){
                road->lanes[clIdx]->departureNode = correspondIDs[k];
            }
        }

        road->lanes[clIdx]->connectedNodeInDirect = road->lanes[lIdx]->connectedNodeInDirect;
        road->lanes[clIdx]->connectedNodeOutDirect = road->lanes[lIdx]->connectedNodeOutDirect;
        road->lanes[clIdx]->departureNodeOutDirect = road->lanes[lIdx]->departureNodeOutDirect;

        road->lanes[clIdx]->driverErrorProb = road->lanes[lIdx]->driverErrorProb;
        road->lanes[clIdx]->hasStopLine = road->lanes[lIdx]->hasStopLine;
        road->lanes[clIdx]->insideIntersection = road->lanes[lIdx]->insideIntersection;
        road->lanes[clIdx]->laneNum = road->lanes[lIdx]->laneNum;
        road->lanes[clIdx]->laneWidth = road->lanes[lIdx]->laneWidth;
        road->lanes[clIdx]->speedInfo = road->lanes[lIdx]->speedInfo;
    }


    // Check Lane Connection
    road->CheckLaneConnectionFull();

    // Calculate Stop Point Data
    road->CheckAllStopLineCrossLane();

    // Check Cross Points
    road->CheckLaneCrossPoints();

    // Create WP Data
    road->CreateWPData();

    // Check Route Data
    road->CheckRouteInOutDirection();

    // Set Lane List
    road->SetAllLaneLists();

    // Set Turn Direction Info
    {
        QList<int> nodeList;
        for(int i=0;i<road->nodes.size();++i){
            nodeList.append( i );
        }
        road->SetTurnDirectionInfo(nodeList);
    }

    // Find PedestSignal
    road->FindPedestSignalFroCrossWalk();


    // Set Route Lane List
    road->SetAllRouteLaneList();


    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();
    for(int i=0;i<newNodeList.size();++i){
        canvas->selectedObj.selObjKind.append( canvas->SEL_NODE );
        canvas->selectedObj.selObjID.append( newNodeList[i] );
    }


    if( canvas ){
        canvas->update();
    }
}



