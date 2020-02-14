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


void RoadObjectProperty::ChangeNodeInfo(int id)
{
    qDebug() << "[ChangeNodeInfo] id = " << id;

    if( !nodeInfo ){
        return;
    }

    nodeInfo->clear();

    disconnect( cbONode, SIGNAL(toggled(bool)), this, SLOT(CBOriginNodeChanged(bool)) );
    disconnect( cbDNode, SIGNAL(toggled(bool)), this, SLOT(CBDestinationNodeChanged(bool)) );

    cbONode->setChecked( false );
    cbDNode->setChecked( false );

    connect( cbONode, SIGNAL(toggled(bool)), this, SLOT(CBOriginNodeChanged(bool)) );
    connect( cbDNode, SIGNAL(toggled(bool)), this, SLOT(CBDestinationNodeChanged(bool)) );

    if( !road ){
        return;
    }

    int ndIdx = road->indexOfNode( id );
    if( ndIdx < 0 ){

        QString infoStr = QString("\nNot exist.\n");
        nodeInfo->setText( infoStr );
        nodeInfo->setAlignment( Qt::AlignTop );
        nodeInfo->update();

        emit HideODRouteEdit();
        return;
    }

    QString infoStr = QString();

    infoStr += QString("\n");
    infoStr += QString("Center Position: (%1,%2)\n").arg( road->nodes[ndIdx]->pos.x(), 0, 'g', 5 ).arg( road->nodes[ndIdx]->pos.y(), 0, 'g', 5 );
    infoStr += QString("\n");
    for(int i=0;i<road->nodes[ndIdx]->legInfo.size();++i){
        infoStr += QString("[Leg %1]\n").arg( road->nodes[ndIdx]->legInfo[i]->legID );
        infoStr += QString("  Connected from   : Node %1 ,Leg %2\n").arg( road->nodes[ndIdx]->legInfo[i]->connectedNode ).arg( road->nodes[ndIdx]->legInfo[i]->connectedNodeOutDirect );
        infoStr += QString("  Connecting to    : Node %1 ,Leg %2\n").arg( road->nodes[ndIdx]->legInfo[i]->connectingNode ).arg( road->nodes[ndIdx]->legInfo[i]->connectingNodeInDirect );
        infoStr += QString("  Number WPin  : %1\n").arg( road->nodes[ndIdx]->legInfo[i]->inWPs.size() );
        infoStr += QString("  Number WPout : %1\n").arg( road->nodes[ndIdx]->legInfo[i]->outWPs.size() );
        infoStr += QString("  Oncoming LegID   : %1\n").arg( road->nodes[ndIdx]->legInfo[i]->oncomingLegID);
        infoStr += QString("  Left-Turn LegID  : ");
        for(int j=0;j<road->nodes[ndIdx]->legInfo[i]->leftTurnLegID.size();++j){
            infoStr += QString(" %1").arg( road->nodes[ndIdx]->legInfo[i]->leftTurnLegID[j] );
        }
        infoStr += QString("\n");
        infoStr += QString("  Right-Turn LegID : ");
        for(int j=0;j<road->nodes[ndIdx]->legInfo[i]->rightTurnLegID.size();++j){
            infoStr += QString(" %1").arg( road->nodes[ndIdx]->legInfo[i]->rightTurnLegID[j] );
        }
        infoStr += QString("\n");
    }
    infoStr += QString("\n");

    infoStr += QString("Lane List:\n");
    for(int i=0;i<road->nodes[ndIdx]->nLeg;++i){
        for(int j=0;j<road->nodes[ndIdx]->nLeg;++j){

            infoStr += QString("  Out Leg: %1  In Leg: %2\n").arg( road->nodes[ndIdx]->legInfo[i]->legID)
                    .arg( road->nodes[ndIdx]->legInfo[j]->legID );

            for(int k=0;k<road->nodes[ndIdx]->laneList.size();++k){
                if( road->nodes[ndIdx]->legInfo[i]->legID == road->nodes[ndIdx]->laneList[k]->relatedNodeOutDirection &&
                        road->nodes[ndIdx]->legInfo[j]->legID == road->nodes[ndIdx]->laneList[k]->relatedNodeInDirection  ){

                    for(int l=0;l<road->nodes[ndIdx]->laneList[k]->lanes.size();++l){

                        infoStr += QString("    ");
                        for(int m=0;m<road->nodes[ndIdx]->laneList[k]->lanes[l].size();m++){
                            infoStr += QString(" %1").arg(road->nodes[ndIdx]->laneList[k]->lanes[l][m] );
                            if( m < road->nodes[ndIdx]->laneList[k]->lanes[l].size() - 1 ){
                                infoStr += QString(" <-");
                            }
                            else{
                                infoStr += QString("\n");
                            }
                        }
                    }
                }

            }
        }
    }


    nodeInfo->setText( infoStr );
    nodeInfo->setAlignment( Qt::AlignTop );
    nodeInfo->setFixedSize( nodeInfo->sizeHint() );


    qDebug() << "isOriginNode = " << road->nodes[ndIdx]->isOriginNode
             << " isDestinationNode = " << road->nodes[ndIdx]->isDestinationNode;

    disconnect( cbONode, SIGNAL(toggled(bool)), this, SLOT(CBOriginNodeChanged(bool)) );
    disconnect( cbDNode, SIGNAL(toggled(bool)), this, SLOT(CBDestinationNodeChanged(bool)) );

    cbONode->setChecked( road->nodes[ndIdx]->isOriginNode );
    cbDNode->setChecked( road->nodes[ndIdx]->isDestinationNode );

    connect( cbONode, SIGNAL(toggled(bool)), this, SLOT(CBOriginNodeChanged(bool)) );
    connect( cbDNode, SIGNAL(toggled(bool)), this, SLOT(CBDestinationNodeChanged(bool)) );

    if( road->nodes[ndIdx]->isOriginNode == true ){
        qDebug() << "emit OriginNodeChanged";
        emit OriginNodeChanged( road->nodes[ndIdx]->id, true );
        emit ShowODRouteEdit();
    }
    else{
        emit HideODRouteEdit();
    }

    emit ResetLaneListIndex();
}


void RoadObjectProperty::CBOriginNodeChanged(bool val)
{
    int nodeID = nodeIDSB->value();
    int ndIdx = road->indexOfNode( nodeID );
    if( ndIdx < 0 ){
        return;
    }

    road->nodes[ndIdx]->isOriginNode = val;
}


void RoadObjectProperty::CBDestinationNodeChanged(bool val)
{
    int nodeID = nodeIDSB->value();
    int ndIdx = road->indexOfNode( nodeID );
    if( ndIdx < 0 ){
        return;
    }

    road->nodes[ndIdx]->isDestinationNode = val;

    emit DestinationNodeChanged( nodeID, false );
}

