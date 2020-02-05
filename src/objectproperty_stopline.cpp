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


void RoadObjectProperty::ChangeStopLineInfo(int id)
{
    if( !slInfo ){
        return;
    }

    slInfo->clear();

    if( !road ){
        return;
    }

    int nodeID = road->indexOfSL( id, -1 );
    if( nodeID < 0){
        return;
    }
    int ndIdx  = road->indexOfNode( nodeID );
    int slIdx  = road->indexOfSL( id, nodeID );
    if( slIdx < 0 ){
        return;
    }

    QString infoStr = QString();

    infoStr += QString("\n");
    infoStr += QString("[Node Info]\n");
    infoStr += QString("  Related Node : %1\n").arg( road->nodes[ndIdx]->stopLines[slIdx]->relatedNode );
    infoStr += QString("  Related Leg  : %1\n").arg( road->nodes[ndIdx]->stopLines[slIdx]->relatedNodeDir );
    infoStr += QString("\n");
    infoStr += QString("[Cross Lanes]\n");
    for(int i=0;i<road->nodes[ndIdx]->stopLines[slIdx]->crossLanes.size();++i){
        infoStr += QString("  Lane: %1\n").arg( road->nodes[ndIdx]->stopLines[slIdx]->crossLanes[i] );
    }
    infoStr += QString("\n");

    slInfo->setText( infoStr );
    slInfo->setAlignment( Qt::AlignTop );

}
