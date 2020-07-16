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


#include "displaycontrol.h"

DisplayControl::DisplayControl(QWidget *parent) : QWidget(parent)
{
    QGridLayout *grid = new QGridLayout();

    road = NULL;

    resetRotate = new QPushButton("Reset Rotate");

    moveTo = new QPushButton("Move");
    connect( moveTo,SIGNAL(clicked()),this,SLOT(moveToClicked()) );

    moveToX = new QLineEdit("0.0");
    moveToX->setFixedWidth(50);
    moveToXVal = new QDoubleValidator();
    moveToX->setValidator( moveToXVal );

    moveToY = new QLineEdit("0.0");
    moveToY->setFixedWidth(50);
    moveToYVal = new QDoubleValidator();
    moveToY->setValidator( moveToYVal );

    OrthogonalView = new QCheckBox("Orthogonal");

    int row = 0;

    grid->addWidget( new QLabel("View:"), 0, 0 );
    grid->addWidget( OrthogonalView, row++, 1 );
    grid->addWidget( moveTo, row++, 1 );
    grid->addWidget( new QLabel("X:"), row, 1, 1, 1, Qt::AlignRight );
    grid->addWidget( moveToX, row++, 2 );
    grid->addWidget( new QLabel("Y:"), row, 1, 1, 1, Qt::AlignRight  );
    grid->addWidget( moveToY, row++, 2 );
    grid->addWidget( resetRotate, row++, 1 );


    showNodes      = new QCheckBox("Nodes");
    showNodeLabels = new QCheckBox("Labels");

    showNodeLaneList = new QCheckBox("Lane Lists");
    showRelatedLanes = new QCheckBox("Related Lanes");
    showRouteLaneList = new QCheckBox("Route Lane Lists");

    prevLaneList = new QPushButton();
    prevLaneList->setIcon( QIcon(":/images/back.png") );
    prevLaneList->setFixedSize( prevLaneList->sizeHint() );

    nextLaneList = new QPushButton();
    nextLaneList->setIcon( QIcon(":/images/next.png") );
    nextLaneList->setFixedSize( nextLaneList->sizeHint() );

    showLanes      = new QCheckBox("Lanes");
    showLaneLabels = new QCheckBox("Labels");

    colorMapOfLaneSpeedLimit = new QCheckBox("Colored by Speed Limit");
    colorMapOfLaneActualSpeed = new QCheckBox("Colored by Actual Speed");

    laneWidth = new QSpinBox();
    laneWidth->setMinimum(1);
    laneWidth->setMaximum(10); 
    laneWidth->setFixedSize( laneWidth->sizeHint() );

    manualUpdateOfCP = new QCheckBox("Manual Update of CP");
    manualUpdateOfCP->setChecked( true );
    connect( manualUpdateOfCP,SIGNAL(toggled(bool)),this,SLOT(SetManualUpdateCPFlag(bool)) );

    manualUpdateOfWPData = new QCheckBox("Manual Update of WPData");
    manualUpdateOfWPData->setChecked( true );
    connect( manualUpdateOfWPData,SIGNAL(toggled(bool)),this,SLOT(SetManualUpdateWPDataFlag(bool)) );

    showTrafficSignals      = new QCheckBox("Traffic Signals");
    showTrafficSignalLabels = new QCheckBox("Label");

    showStopLines      = new QCheckBox("Stop Lines");
    showStopLineLabels = new QCheckBox("Label");

    showPedestLanes      = new QCheckBox("Pedestrian Lane");
    showPedestLaneLabels = new QCheckBox("Label");

    showLabels = new QCheckBox("Labels");

    showMapImage = new QCheckBox("Map Image");
    backMapImage = new QCheckBox("Move Back");


    grid->addWidget( new QLabel("Visilibity:"), row++, 0 );
    grid->addWidget( showNodes, row, 1 );
    grid->addWidget( showNodeLabels, row++, 2 );
    grid->addWidget( showNodeLaneList, row++, 2 );
    grid->addWidget( showRelatedLanes, row++, 2 );
    grid->addWidget( showRouteLaneList, row++, 2 );

    QHBoxLayout *LaneListHBox = new QHBoxLayout();
    LaneListHBox->addWidget( prevLaneList );
    LaneListHBox->addWidget( nextLaneList );
    LaneListHBox->addStretch(1);
    grid->addLayout(  LaneListHBox, row++, 2 );
    grid->addWidget( manualUpdateOfWPData, row++, 2 );

    grid->addWidget( showLanes, row, 1 );
    grid->addWidget( showLaneLabels, row++, 2 );
    grid->addWidget( new QLabel("Width:"), row, 1, 1, 1, Qt::AlignRight );
    grid->addWidget( laneWidth, row++, 2 );
    grid->addWidget( manualUpdateOfCP, row++, 2 );
    grid->addWidget( colorMapOfLaneSpeedLimit, row++, 2 );
    grid->addWidget( colorMapOfLaneActualSpeed, row++, 2 );

    grid->addWidget( showTrafficSignals, row, 1 );
    grid->addWidget( showTrafficSignalLabels, row++, 2 );
    grid->addWidget( showStopLines, row, 1 );
    grid->addWidget( showStopLineLabels, row++, 2 );
    grid->addWidget( showPedestLanes, row, 1 );
    grid->addWidget( showPedestLaneLabels, row++, 2 );

    grid->addWidget( showLabels, row++, 1 );
    grid->addWidget( showMapImage, row, 1 );
    grid->addWidget( backMapImage, row++, 2 );


    selectNode          = new QCheckBox("Node");
    selectLane          = new QCheckBox("Lane");
    selectTrafficSignal = new QCheckBox("Traffic Signal");
    selectStopLine      = new QCheckBox("Stop Line");
    selectPedestLane    = new QCheckBox("Pedestrian Lane");


    grid->addWidget( new QLabel("Selection:"), row++, 0 );
    grid->addWidget( selectNode, row++, 1 );
    grid->addWidget( selectLane, row++, 1 );
    grid->addWidget( selectTrafficSignal, row++, 1 );
    grid->addWidget( selectStopLine, row++, 1 );
    grid->addWidget( selectPedestLane, row++, 1 );


    InitSetting();


    QHBoxLayout *mainLayout = new QHBoxLayout();

    mainLayout->addLayout( grid );

    setLayout( mainLayout );

    setWindowTitle("Display&Edit Control");

    road = NULL;
}


void DisplayControl::InitSetting()
{
    showNodes->setChecked(true);
    showNodeLabels->setChecked(true);

    showNodeLaneList->setChecked(false);
    showRelatedLanes->setChecked(false);
    showRouteLaneList->setChecked(true);

    showLanes->setChecked(true);
    laneWidth->setValue(6);
    manualUpdateOfCP->setChecked(true);
    manualUpdateOfWPData->setChecked(true);
    showLaneLabels->setChecked(true);
    colorMapOfLaneSpeedLimit->setChecked(false);
    colorMapOfLaneActualSpeed->setChecked(false);

    showTrafficSignals->setChecked(true);
    showTrafficSignalLabels->setChecked(true);

    showStopLines->setChecked(true);
    showStopLineLabels->setChecked(true);

    showPedestLanes->setChecked(true);
    showPedestLaneLabels->setChecked(true);

    showLabels->setChecked(true);

    showMapImage->setChecked(true);
    backMapImage->setChecked(false);

    selectNode->setChecked(true);
    selectLane->setChecked(true);
    selectTrafficSignal->setChecked(true);
    selectStopLine->setChecked(true);
    selectPedestLane->setChecked(true);
}


void DisplayControl::moveToClicked()
{
    float x = moveToX->text().toFloat();
    float y = moveToY->text().toFloat();
    emit ViewMoveTo(x,y);
}


void DisplayControl::SetManualUpdateCPFlag(bool v)
{
    if( road != NULL ){
        if( v == true ){
            road->updateCPEveryOperation = false;
        }
        else{
            road->updateCPEveryOperation = true;
        }
    }
}

void DisplayControl::SetManualUpdateWPDataFlag(bool v)
{
    if( road != NULL ){
        if( v == true ){
            road->updateWPDataEveryOperation = false;
        }
        else{
            road->updateWPDataEveryOperation = true;
        }
    }
}

