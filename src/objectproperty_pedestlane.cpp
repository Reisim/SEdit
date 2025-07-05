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
        pedestMarginToRoad->setValue( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->marginToRoadForRunOut );

        if( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->runOutDirect == 1 ){
            pedestRunOutDirect->setCurrentIndex( 0 );
        }
        else if( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->runOutDirect == -1 ){
            pedestRunOutDirect->setCurrentIndex( 1 );
        }

        cbCanWaitTaxi->setChecked( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->canWaitTaxi );
        pedestLaneTaxiTakeProbability->setValue( road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->taxiTakeProbability );

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

    int tHeight = 500;
    if( pedestLaneTrafficVolume->sizeHint().height() + 100 < tHeight ){
        tHeight = pedestLaneTrafficVolume->sizeHint().height() + 100;
    }
    pedestLaneTrafficVolume->setFixedHeight( tHeight );

    if( cbChangeSelectionBySpinbox->isChecked() == true ){
        emit ChangeSelectionRequest(7,pedestLaneID);
    }
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

        road->FindPedestSignalFroCrossWalk();
    }
    else{
        road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->isCrossWalk = false;
    }

    road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->width = pedestLaneWidth->value();
    road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->runOutProb = pedestRunOutProb->value();
    road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->marginToRoadForRunOut = pedestMarginToRoad->value();

    if( pedestRunOutDirect->currentIndex() == 0 ){
        road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->runOutDirect = 1;
    }
    else if( pedestRunOutDirect->currentIndex() == 1 ){
        road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->runOutDirect = -1;
    }

    if( cbCanWaitTaxi->checkState() == Qt::Checked ){
        road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->canWaitTaxi = true;
        road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->taxiTakeProbability = pedestLaneTaxiTakeProbability->value();
    }
    else{
        road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->canWaitTaxi = false;
        road->pedestLanes[plIdx]->shape[pedestLanePointIndex]->taxiTakeProbability = 0.0;
    }

    int nPedKind = setDlg->GetPedestrianKindNum();
    if( road->pedestLanes[plIdx]->trafficVolume.size() < nPedKind ){
        int nAdd = nPedKind - road->pedestLanes[plIdx]->trafficVolume.size();
        for(int i=0;i<nAdd;++i){
            road->pedestLanes[plIdx]->trafficVolume.append(0);
        }
    }
    for(int i=0;i<nPedKind;++i){
        road->pedestLanes[plIdx]->trafficVolume[i] = pedestLaneTrafficVolume->item(i,1)->text().trimmed().toInt();
    }

    UpdateGraphic();
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
        if(road->pedestLanes[plIdx]->trafficVolume.size() > i ){
            itemVolume->setText( QString("%1").arg( road->pedestLanes[plIdx]->trafficVolume[i] ) );
        }
        else{
            itemVolume->setText( 0 );
        }
        pedestLaneTrafficVolume->setItem(i,1,itemVolume);
    }
}


void RoadObjectProperty::GetPedestLaneHeightFromUE()
{
    int pedestLaneID = pedestLaneIDSB->value();
    int plIdx = road->indexOfPedestLane( pedestLaneID );
    if( plIdx < 0 ){
        return;
    }

    QUdpSocket sock;
    QUdpSocket rsock;

    rsock.bind( QHostAddress::Any , 58000 );

    for(int i=0;i<road->pedestLanes[plIdx]->shape.size();++i){

        char sendData[100];
        sendData[0] = 'G';
        sendData[1] = 'H';
        sendData[2] = 'R';

        float x = road->pedestLanes[plIdx]->shape[i]->pos.x();
        float y = road->pedestLanes[plIdx]->shape[i]->pos.y();
        float z = road->pedestLanes[plIdx]->shape[i]->pos.z();

        memcpy( &(sendData[3]) , &x, sizeof(float) );
        memcpy( &(sendData[7]) , &y, sizeof(float) );
        memcpy( &(sendData[11]), &z, sizeof(float) );

        //qDebug() << "[GetHeightDataFromUE]";
        //qDebug() << "  send data : x = " << x << " y = " << y << " z = " << z;

        sock.writeDatagram( sendData, 15, QHostAddress("192.168.1.102"), 56000 );

        char recvData[10];
        while(1){
            int ret = rsock.readDatagram(recvData,10);
            if( ret >= 8 ){
                int n = 0;
                memcpy(&n, &(recvData[0]), sizeof(int) );

                float zue = 0.0;
                memcpy(&zue, &(recvData[4]), sizeof(float) );

                road->pedestLanes[plIdx]->shape[i]->pos.setZ(zue * 0.01);

                //qDebug() << "n = " << n << " zue = " << zue;
                break;
            }
        }
    }
}

