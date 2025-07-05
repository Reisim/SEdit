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
#include <QDebug>
#include <QDialog>


void RoadObjectProperty::ChangeRoadBoundaryInfo(int roundBoundaryID)
{
    ChangeRoadBoundaryInfo(roundBoundaryID, 0);
}


void RoadObjectProperty::ChangeRoadBoundaryInfo(int id,int sect)
{
    if( !roadBoundaryInfo ){
        return;
    }

    if( !road ){
        return;
    }

    int idx = road->indexOfRoadBoundary( id );
    if( idx < 0 ){
        return;
    }

    if( sect < 0 || sect >= road->roadBoundary[idx]->height.size() ){
        return;
    }

    roadBoundaryRoadSide->setCurrentIndex( road->roadBoundary[idx]->RoadSide );
    roadBoundaryHeight->setValue( road->roadBoundary[idx]->height[sect] );

    QString infoStr = QString();

    infoStr += QString("\n");
    for(int i=0;i<road->roadBoundary[idx]->pos.size()-1;++i){
        infoStr += QString("[%1]  Pos(%2,%3,%4) angle=%5 L=%6 H=%7\n")
                .arg(i)
                .arg( road->roadBoundary[idx]->pos[i]->x() )
                .arg( road->roadBoundary[idx]->pos[i]->y() )
                .arg( road->roadBoundary[idx]->pos[i]->z() )
                .arg( road->roadBoundary[idx]->angles[i] * 57.3 )
                .arg( road->roadBoundary[idx]->length[i] )
                .arg( road->roadBoundary[idx]->height[i] );
    }
    int i = road->roadBoundary[idx]->pos.size() - 1;
    infoStr += QString("[%1]  Pos(%2,%3,%4)\n")
            .arg(i)
            .arg( road->roadBoundary[idx]->pos[i]->x() )
            .arg( road->roadBoundary[idx]->pos[i]->y() )
            .arg( road->roadBoundary[idx]->pos[i]->z() );

    infoStr += QString("RoadSide : ");
    if( road->roadBoundary[idx]->RoadSide == 0 ){
        infoStr += QString(" RIGHT\n");
    }
    else if( road->roadBoundary[idx]->RoadSide == 1 ){
        infoStr += QString(" LEFT\n");
    }
    else{
        infoStr += QString(" ???\n");
    }

    roadBoundaryInfo->setText( infoStr );
    roadBoundaryInfo->setAlignment( Qt::AlignTop );
    roadBoundaryInfo->setFixedSize( soInfo->sizeHint() );
}


void RoadObjectProperty::RoadBoundaryApplyClicked()
{
    qDebug() << "[RoadObjectProperty::RoadBoundaryApplyClicked]";

    int roadBoundaryID = roadBoundaryIDSB->value();
    int rbIdx = road->indexOfRoadBoundary( roadBoundaryID );
    if( rbIdx < 0 ){
        return;
    }

    int roadBoundaryPointIndex = roadBoundarySectionSB->value();
    if( roadBoundaryPointIndex < 0 || roadBoundaryPointIndex >= road->roadBoundary[rbIdx]->pos.size()-1 ){
        return;
    }

    road->roadBoundary[rbIdx]->RoadSide = roadBoundaryRoadSide->currentIndex();

    float height = roadBoundaryHeight->value();
    if( rbHeightAll->isChecked() == true ){
        for(int i=0;i<road->roadBoundary[rbIdx]->height.size();++i){
            road->roadBoundary[rbIdx]->height[i] = height;
        }
    }
    else{
        road->roadBoundary[rbIdx]->height[roadBoundaryPointIndex] = height;
    }

    ChangeRoadBoundaryInfo( roadBoundaryID );
    UpdateGraphic();
}
