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



void RoadObjectProperty::ChangePedestLaneInfo(int pedestLaneID)
{
    ChangePedestLaneInfo(pedestLaneID, -1);
}

void RoadObjectProperty::ChangePedestLaneInfo(int pedestLaneID, int pedestLanePointIndex)
{

    if( !pedestLaneInfo ){
        return;
    }

    pedestLaneInfo->clear();

    if( !road ){
        return;
    }

    int plIdx = road->indexOfPedestLane( pedestLaneID );
    if( plIdx < 0 ){
        return;
    }

    QString infoStr = QString();

    infoStr += QString("\n");
    infoStr += QString("[Numer of Point] N = %1\n").arg( road->pedestLanes[plIdx]->shape.size() );
    infoStr += QString("\n");

    if( pedestLanePointIndex < 0 ){
        pedestLanePointIndex = 0;
    }

    if( pedestLanePointIndex >= 0 && pedestLanePointIndex < road->pedestLanes[plIdx]->shape.size() ){

        pedestLaneSectionSB->setValue( pedestLanePointIndex );

        pedestLaneWidth->setValue( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->width );
        cbIsCrossWalk->setChecked( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->isCrossWalk );
        pedestRunOutProb->setValue( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->runOutProb );
        if( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->runOutDirect == 1 ){
            pedestRunOutDirect->setCurrentIndex( 0 );
        }
        else if( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->runOutDirect == -1 ){
            pedestRunOutDirect->setCurrentIndex( 1 );
        }

        infoStr += QString("[Point Data]\n");
        infoStr += QString("   x = %1,  y = %2\n")
                .arg( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->pos.x() )
                .arg( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->pos.y() );
        infoStr += QString("   Length = %1\n").arg( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->distanceToNextPos );

        if( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->isCrossWalk == true ){
            infoStr += QString("\n[CrossWalk] control PedestTS ID = %1\n").arg( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->controlPedestSignalID );
        }

    }

    pedestLaneInfo->setText( infoStr );
    pedestLaneInfo->setAlignment( Qt::AlignTop );

    SetPedestLaneTrafficVolume( pedestLaneID );
}



void RoadObjectProperty::PedestLaneApplyClicked()
{
    qDebug() << "[RoadObjectProperty::PedestLaneApplyClicked]";

    int pedestLaneID = pedestLaneIDSB->value();
    int plIdx = road->indexOfPedestLane( pedestLaneID );
    if( plIdx < 0 ){
        return;
    }

    int pedestLanePointIndex = pedestLaneSectionSB->value();
    if( pedestLanePointIndex < 0 || pedestLanePointIndex >= road->pedestLanes[plIdx]->shape.size() ){
        return;
    }


    if( cbIsCrossWalk->checkState() == Qt::Checked ){
        road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->isCrossWalk = true;
    }
    else{
        road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->isCrossWalk = false;
    }

    road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->width = pedestLaneWidth->value();
    road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->runOutProb = pedestRunOutProb->value();

    if( pedestRunOutDirect->currentIndex() == 0 ){
        road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->runOutDirect = 1;
    }
    else if( pedestRunOutDirect->currentIndex() == 1 ){
        road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->runOutDirect = -1;
    }


    int nPedKind = setDlg->GetPedestrianKindNum();
    if( road->pedestLanes[plIdx]->trafficVolume.size() < nPedKind ){
        road->pedestLanes[plIdx]->trafficVolume.reserve( nPedKind );
    }
    for(int i=0;i<nPedKind;++i){
        road->pedestLanes[plIdx]->trafficVolume[i] = pedestLaneTrafficVolume->item(i,1)->text().trimmed().toInt();
    }
}


void RoadObjectProperty::SetPedestLaneTrafficVolume(int id)
{
    qDebug() << "[RoadObjectProperty::SetPedestLaneTrafficVolume] id = " << id;

    int plIdx = road->indexOfPedestLane( id );
    if( plIdx < 0 ){
        qDebug() << "cannot find index";
        return;
    }

    int nPedKind = setDlg->GetPedestrianKindNum();
//    qDebug() << "nPedKind = " << nPedKind;

    if( pedestLaneTrafficVolume->rowCount() < nPedKind ){
        for(int i=pedestLaneTrafficVolume->rowCount();i<nPedKind;++i){
            pedestLaneTrafficVolume->insertRow( i );
        }
    }

    pedestLaneTrafficVolume->clearContents();

    QStringList categStr    = setDlg->GetPedestianKindCategory();
    QStringList subCategStr = setDlg->GetPedestianKindSubcategory();

//    qDebug() << "categStr = " << categStr;
//    qDebug() << "subCategStr = " << subCategStr;

    for(int i=0;i<nPedKind;++i){

        QTableWidgetItem *itemLabel = new QTableWidgetItem();
        itemLabel->setText( QString(categStr[i]) + QString("-") + QString(subCategStr[i]) );
        pedestLaneTrafficVolume->setItem(i,0,itemLabel);


        QTableWidgetItem *itemVolume = new QTableWidgetItem();
        itemVolume->setText( QString("%1").arg( road->pedestLanes[plIdx]->trafficVolume[i] ) );
        pedestLaneTrafficVolume->setItem(i,1,itemVolume);
    }
}


