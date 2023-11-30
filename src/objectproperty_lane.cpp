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
    infoStr += QString("  End:   Related Node=%1, Leg=%2, Boundray=%3\n").arg( road->lanes[lIdx]->eWPInNode ).arg( road->lanes[lIdx]->eWPNodeDir ).arg( road->lanes[lIdx]->eWPBoundary );
    infoStr += QString("  Start: Related Node=%1, Leg=%2, Boundray=%3\n").arg( road->lanes[lIdx]->sWPInNode ).arg( road->lanes[lIdx]->sWPNodeDir ).arg( road->lanes[lIdx]->sWPBoundary );
    infoStr += QString("  endWP = %2, StartWP = %1\n").arg( road->lanes[lIdx]->startWPID ).arg( road->lanes[lIdx]->endWPID );
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
    infoStr += QString("[Pedest Cross Lanes]\n");
    for(int i=0;i<road->lanes[lIdx]->pedestCrossPoints.size();++i){
        infoStr += QString("  Pedest-Lane %1 - Section %2:\n     (%3, %4)\n" )
                .arg( road->lanes[lIdx]->pedestCrossPoints[i]->crossLaneID )
                .arg( road->lanes[lIdx]->pedestCrossPoints[i]->crossSectIndex)
                .arg( road->lanes[lIdx]->pedestCrossPoints[i]->pos.x() )
                .arg( road->lanes[lIdx]->pedestCrossPoints[i]->pos.y() );
        infoStr += QString("    Dist from Edge = %1\n").arg( road->lanes[lIdx]->pedestCrossPoints[i]->distanceFromLaneStartPoint );
    }
    infoStr += QString("\n");
    infoStr += QString("[Shape]\n");
    infoStr += QString("  Length: %1 [m]\n").arg( road->lanes[lIdx]->shape.pathLength);

    infoStr += QString("  Curvature:\n");

    float maxCurv = 0.0;
    float meanCurv = 0.0;
    for(int i=0;i<road->lanes[lIdx]->shape.curvature.size();++i){

        infoStr += QString("  [%1] %2\n").arg(i).arg( road->lanes[lIdx]->shape.curvature[i] );

        if( fabs(road->lanes[lIdx]->shape.curvature[i]) > maxCurv ){
            maxCurv = road->lanes[lIdx]->shape.curvature[i];
        }

        meanCurv += road->lanes[lIdx]->shape.curvature[i];
    }
    meanCurv /= road->lanes[lIdx]->shape.curvature.size();
    infoStr += QString("  Mean Curvature: %1 [1/m]\n").arg( meanCurv );
    infoStr += QString("  Max Curvature: %1 [1/m]\n").arg( maxCurv );
    if( fabs(maxCurv) > 0.0010 ){
        infoStr += QString("  Max Radius: %1 [m]\n").arg( 1.0/fabs(maxCurv) );
    }

    disconnect( laneSpeed, SIGNAL(valueChanged(int)), this, SLOT(SpeedLimitChanged(int)) );
    laneSpeed->setValue( road->lanes[lIdx]->speedInfo );
    connect( laneSpeed, SIGNAL(valueChanged(int)), this, SLOT(SpeedLimitChanged(int)) );
    //qDebug() << "speed limit = " << road->lanes[lIdx]->speedInfo;


    disconnect( laneActualSpeed, SIGNAL(valueChanged(int)), this, SLOT(ActualSpeedChanged(int)) );
    laneActualSpeed->setValue( road->lanes[lIdx]->actualSpeed );
    connect( laneActualSpeed, SIGNAL(valueChanged(int)), this, SLOT(ActualSpeedChanged(int)) );
    //qDebug() << "actual speed = " << road->lanes[lIdx]->actualSpeed;


    disconnect( laneAutomaticDrivingEnabled, SIGNAL(toggled(bool)), this, SLOT(AutomaticDrivingEnableFlagChanged(bool)) );
    laneAutomaticDrivingEnabled->setChecked( road->lanes[lIdx]->automaticDrivingEnabled );
    connect( laneAutomaticDrivingEnabled, SIGNAL(toggled(bool)), this, SLOT(AutomaticDrivingEnableFlagChanged(bool)) );
    //qDebug() << "automaticDrivingEnabled = " << road->lanes[lIdx]->automaticDrivingEnabled;


    disconnect( laneDriverErrorProb, SIGNAL(valueChanged(double)), this, SLOT(DriverErrorProbChanged(double)) );
    laneDriverErrorProb->setValue( road->lanes[lIdx]->driverErrorProb );
    connect( laneDriverErrorProb, SIGNAL(valueChanged(double)), this, SLOT(DriverErrorProbChanged(double)) );
    //qDebug() << "driverErrorProb = " << road->lanes[lIdx]->driverErrorProb;

    disconnect( laneWidth, SIGNAL(valueChanged(double)), this, SLOT(LaneWidthChanged(double)) );
    laneWidth->setValue( road->lanes[lIdx]->laneWidth );
    connect( laneWidth, SIGNAL(valueChanged(double)), this, SLOT(LaneWidthChanged(double)) );


    laneInfo->setText( infoStr );
    laneInfo->setAlignment( Qt::AlignTop );
    laneInfo->setFixedSize( laneInfo->sizeHint() );

    laneStartX->setValue( road->lanes[lIdx]->shape.pos.first()->x() );
    laneStartY->setValue( road->lanes[lIdx]->shape.pos.first()->y() );
    laneStartZ->setValue( road->lanes[lIdx]->shape.pos.first()->z() );
    float dirs = atan2( road->lanes[lIdx]->shape.derivative.first()->y(), road->lanes[lIdx]->shape.derivative.first()->x() )  * 57.3;
    laneStartDir->setValue( dirs );

    laneEndX->setValue( road->lanes[lIdx]->shape.pos.last()->x() );
    laneEndY->setValue( road->lanes[lIdx]->shape.pos.last()->y() );
    laneEndZ->setValue( road->lanes[lIdx]->shape.pos.last()->z() );
    float dire = atan2( road->lanes[lIdx]->shape.derivative.last()->y(), road->lanes[lIdx]->shape.derivative.last()->x() ) * 57.3;
    laneEndDir->setValue( dire );

    if( cbChangeSelectionBySpinbox->isChecked() == true ){
        emit ChangeSelectionRequest(2,id);
    }
}


void RoadObjectProperty::SpeedLimitChanged(int val)
{
    int laneID = laneIDSB->value();
    int lIdx = road->indexOfLane( laneID );
    if( lIdx < 0 ){
        return;
    }

    road->lanes[lIdx]->speedInfo = val;

    emit UpdateGraphic();
}


void RoadObjectProperty::ActualSpeedChanged(int val)
{
    int laneID = laneIDSB->value();
    int lIdx = road->indexOfLane( laneID );
    if( lIdx < 0 ){
        return;
    }

    road->lanes[lIdx]->actualSpeed = val;

    emit UpdateGraphic();
}


void RoadObjectProperty::AutomaticDrivingEnableFlagChanged(bool b)
{
    int laneID = laneIDSB->value();
    int lIdx = road->indexOfLane( laneID );
    if( lIdx < 0 ){
        return;
    }

    road->lanes[lIdx]->automaticDrivingEnabled = b;
}


void RoadObjectProperty::LaneWidthChanged(double w)
{
    int laneID = laneIDSB->value();
    int lIdx = road->indexOfLane( laneID );
    if( lIdx < 0 ){
        return;
    }

    road->lanes[lIdx]->laneWidth = w;

    emit UpdateGraphic();
}


void RoadObjectProperty::DriverErrorProbChanged(double v)
{
    int laneID = laneIDSB->value();
    int lIdx = road->indexOfLane( laneID );
    if( lIdx < 0 ){
        return;
    }

    road->lanes[lIdx]->driverErrorProb = v;
}


void RoadObjectProperty::EditLaneData()
{
    qDebug() << "[RoadObjectProperty::EditLaneData]";

    QDialog *dialog = new QDialog();

    QGridLayout *gLay = new QGridLayout();
    gLay->addWidget( new QLabel("eWPInNode")   , 0, 0 );
    gLay->addWidget( new QLabel("eWPNodeDir")  , 1, 0 );
    gLay->addWidget( new QLabel("eWPBoundary") , 2, 0 );
    gLay->addWidget( new QLabel("sWPInNode")   , 3, 0 );
    gLay->addWidget( new QLabel("sWPNodeDir")  , 4, 0 );
    gLay->addWidget( new QLabel("sWPBoundary") , 5, 0 );

    QSpinBox *eWPInNodeSB = new QSpinBox();
    eWPInNodeSB->setMinimum(0);
    eWPInNodeSB->setMaximum(60000);

    QSpinBox *eWPNodeDirSB = new QSpinBox();
    eWPNodeDirSB->setMinimum(0);
    eWPNodeDirSB->setMaximum(10);

    QCheckBox *eWPBoundaryCB = new QCheckBox("Boundary");

    QSpinBox *sWPInNodeSB = new QSpinBox();
    sWPInNodeSB->setMinimum(0);
    sWPInNodeSB->setMaximum(60000);

    QSpinBox *sWPNodeDirSB = new QSpinBox();
    sWPNodeDirSB->setMinimum(0);
    sWPNodeDirSB->setMaximum(10);

    QCheckBox *sWPBoundaryCB = new QCheckBox("Boundary");

    gLay->addWidget( eWPInNodeSB   , 0, 1 );
    gLay->addWidget( eWPNodeDirSB  , 1, 1 );
    gLay->addWidget( eWPBoundaryCB , 2, 1 );
    gLay->addWidget( sWPInNodeSB   , 3, 1 );
    gLay->addWidget( sWPNodeDirSB  , 4, 1 );
    gLay->addWidget( sWPBoundaryCB , 5, 1 );


    QPushButton *okBtn = new QPushButton("Accept");
    okBtn->setIcon(QIcon(":/images/accept.png"));
    QPushButton *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setIcon(QIcon(":/images/delete.png"));

    connect( okBtn, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect( cancelBtn, SIGNAL(clicked()), dialog, SLOT(reject()));
    connect( okBtn, SIGNAL(clicked()), dialog, SLOT(close()));
    connect( cancelBtn, SIGNAL(clicked()), dialog, SLOT(close()));

    QHBoxLayout *btnLay = new QHBoxLayout();
    btnLay->addStretch(1);
    btnLay->addWidget( okBtn );
    btnLay->addSpacing(50);
    btnLay->addWidget( cancelBtn );
    btnLay->addStretch(1);

    QVBoxLayout *mLay = new QVBoxLayout();
    mLay->addLayout( gLay );
    mLay->addLayout( btnLay );

    dialog->setLayout( mLay );
    dialog->setFixedSize( dialog->sizeHint() );

    int laneID = laneIDSB->value();
    int lIdx = road->indexOfLane( laneID );
    if( lIdx >= 0 ){

        eWPInNodeSB->setValue( road->lanes[lIdx]->eWPInNode );
        eWPNodeDirSB->setValue( road->lanes[lIdx]->eWPNodeDir );
        if( road->lanes[lIdx]->eWPBoundary == true ){
            eWPBoundaryCB->setChecked( true );
        }
        else{
            eWPBoundaryCB->setChecked( false );
        }

        sWPInNodeSB->setValue( road->lanes[lIdx]->sWPInNode );
        sWPNodeDirSB->setValue( road->lanes[lIdx]->sWPNodeDir );
        if( road->lanes[lIdx]->sWPBoundary == true ){
            sWPBoundaryCB->setChecked( true );
        }
        else{
            sWPBoundaryCB->setChecked( false );
        }
    }


    dialog->exec();

    if( dialog->result() == QDialog::Accepted ){

        if( lIdx >= 0 ){

            road->lanes[lIdx]->eWPInNode   = eWPInNodeSB->value();
            road->lanes[lIdx]->eWPNodeDir  = eWPNodeDirSB->value();
            road->lanes[lIdx]->eWPBoundary = eWPBoundaryCB->isChecked();

            road->lanes[lIdx]->sWPInNode   = sWPInNodeSB->value();
            road->lanes[lIdx]->sWPNodeDir  = sWPNodeDirSB->value();
            road->lanes[lIdx]->sWPBoundary = sWPBoundaryCB->isChecked();

            ChangeLaneInfo( laneID );
        }
    }

    emit UpdateGraphic();
}

void RoadObjectProperty::CheckRelatedNode()
{
    qDebug() << "[RoadObjectProperty::CheckRelatedNode]";

    int laneID = laneIDSB->value();
    road->CheckLaneRelatedNode( laneID );

    emit UpdateGraphic();
}


void RoadObjectProperty::LaneEdgePosChanged()
{
    qDebug() << "[RoadObjectProperty::LaneEdgePosChanged]";

    float xe = laneEndX->value();
    float ye = laneEndY->value();
    float ze = laneEndZ->value();

    float dxe = cos( laneEndDir->value() * 0.017452 );
    float dye = sin( laneEndDir->value() * 0.017452 );


    float xs = laneStartX->value();
    float ys = laneStartY->value();
    float zs = laneStartZ->value();

    float dxs = cos( laneStartDir->value() * 0.017452 );
    float dys = sin( laneStartDir->value() * 0.017452 );


    int laneID = laneIDSB->value();
    int lidx = road->indexOfLane( laneID );

    for(int i=0;i<road->lanes[lidx]->nextLanes.size();++i){

        int nidx = road->indexOfLane( road->lanes[lidx]->nextLanes[i] );

        road->lanes[nidx]->shape.pos.first()->setX( xe );
        road->lanes[nidx]->shape.pos.first()->setY( ye );
        road->lanes[nidx]->shape.pos.first()->setZ( ze );

        road->lanes[nidx]->shape.derivative.first()->setX( dxe );
        road->lanes[nidx]->shape.derivative.first()->setY( dye );

        road->CalculateShape( &(road->lanes[nidx]->shape) );
    }

    for(int i=0;i<road->lanes[lidx]->previousLanes.size();++i){

        int pidx = road->indexOfLane( road->lanes[lidx]->previousLanes[i] );

        road->lanes[pidx]->shape.pos.last()->setX( xs );
        road->lanes[pidx]->shape.pos.last()->setY( ys );
        road->lanes[pidx]->shape.pos.last()->setZ( zs );

        road->lanes[pidx]->shape.derivative.last()->setX( dxs );
        road->lanes[pidx]->shape.derivative.last()->setY( dys );

        road->CalculateShape( &(road->lanes[pidx]->shape) );
    }

    road->lanes[lidx]->shape.pos.first()->setX( xs );
    road->lanes[lidx]->shape.pos.first()->setY( ys );
    road->lanes[lidx]->shape.pos.first()->setZ( zs );

    road->lanes[lidx]->shape.derivative.first()->setX( dxs );
    road->lanes[lidx]->shape.derivative.first()->setY( dys );

    road->lanes[lidx]->shape.pos.last()->setX( xe );
    road->lanes[lidx]->shape.pos.last()->setY( ye );
    road->lanes[lidx]->shape.pos.last()->setZ( ze );

    road->lanes[lidx]->shape.derivative.last()->setX( dxe );
    road->lanes[lidx]->shape.derivative.last()->setY( dye );

    road->CalculateShape( &(road->lanes[lidx]->shape) );

    if( road->updateCPEveryOperation == true ){
        road->CheckLaneCrossPoints();
        road->CheckAllStopLineCrossLane();
    }

    if( road->updateWPDataEveryOperation == true ){
        road->CreateWPData();
    }

    emit UpdateGraphic();

    ChangeLaneInfo( laneID );
}


void RoadObjectProperty::GetLaneHeightFromUE()
{
    int laneID = laneIDSB->value();
    int lIdx = road->indexOfLane( laneID );
    if( lIdx < 0 ){
        return;
    }

    QUdpSocket sock;
    QUdpSocket rsock;

    rsock.bind( QHostAddress::Any , 58000 );

    float xe = laneEndX->value();
    float ye = laneEndY->value();
    float ze = laneEndZ->value();

    float xs = laneStartX->value();
    float ys = laneStartY->value();
    float zs = laneStartZ->value();

    for(int i=0;i<2;++i){

        char sendData[100];
        sendData[0] = 'G';
        sendData[1] = 'H';
        sendData[2] = 'R';

        if( i == 0 ){
            float x = xs;
            float y = ys;
            float z = zs;

            memcpy( &(sendData[3]) , &x, sizeof(float) );
            memcpy( &(sendData[7]) , &y, sizeof(float) );
            memcpy( &(sendData[11]), &z, sizeof(float) );
        }
        else if( i == 1 ){
            float x = xe;
            float y = ye;
            float z = ze;

            memcpy( &(sendData[3]) , &x, sizeof(float) );
            memcpy( &(sendData[7]) , &y, sizeof(float) );
            memcpy( &(sendData[11]), &z, sizeof(float) );
        }


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

                if( i == 0 ){
                    laneStartZ->setValue( zue * 0.01 );
                }
                else{
                    laneEndZ->setValue( zue * 0.01 );
                }
                break;
            }
        }
    }

    LaneEdgePosChanged();
}
