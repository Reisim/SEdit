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


    laneInfo->setText( infoStr );
    laneInfo->setAlignment( Qt::AlignTop );
    laneInfo->setFixedSize( laneInfo->sizeHint() );
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


void RoadObjectProperty::ActualSpeedChanged(int val)
{
    int laneID = laneIDSB->value();
    int lIdx = road->indexOfLane( laneID );
    if( lIdx < 0 ){
        return;
    }

    road->lanes[lIdx]->actualSpeed = val;
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
}

void RoadObjectProperty::CheckRelatedNode()
{
    qDebug() << "[RoadObjectProperty::CheckRelatedNode]";

    int laneID = laneIDSB->value();
    road->CheckLaneRelatedNode( laneID );
}


