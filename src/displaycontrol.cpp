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


    showNodes = new QCheckBox("Nodes");
    showNodes->setChecked(true);

    showNodeLabels = new QCheckBox("Labels");
    showNodeLabels->setChecked(true);

    showNodeLaneList = new QCheckBox("Lane Lists");
    showNodeLaneList->setChecked(false);

    showRelatedLanes = new QCheckBox("Related Lanes");
    showRelatedLanes->setChecked(false);

    prevLaneList = new QPushButton();
    prevLaneList->setIcon( QIcon(":/images/back.png") );
    prevLaneList->setFixedSize( prevLaneList->sizeHint() );

    nextLaneList = new QPushButton();
    nextLaneList->setIcon( QIcon(":/images/next.png") );
    nextLaneList->setFixedSize( nextLaneList->sizeHint() );

    showLanes = new QCheckBox("Lanes");
    showLanes->setChecked(true);

    showLaneLabels = new QCheckBox("Labels");
    showLaneLabels->setChecked(true);

    laneWidth = new QSpinBox();
    laneWidth->setMinimum(1);
    laneWidth->setMaximum(10);
    laneWidth->setValue(6);
    laneWidth->setFixedSize( laneWidth->sizeHint() );

    showTrafficSignals = new QCheckBox("Traffic Signals");
    showTrafficSignals->setChecked(true);

    showTrafficSignalLabels = new QCheckBox("Label");
    showTrafficSignalLabels->setChecked(true);

    showStopLines = new QCheckBox("Stop Lines");
    showStopLines->setChecked(true);

    showStopLineLabels = new QCheckBox("Label");
    showStopLineLabels->setChecked(true);

    showPedestLanes = new QCheckBox("Pedestrian Lane");
    showPedestLanes->setChecked(true);

    showPedestLaneLabels = new QCheckBox("Label");
    showPedestLaneLabels->setChecked(true);

    showLabels = new QCheckBox("Labels");
    showLabels->setChecked(true);

    showMapImage = new QCheckBox("Map Image");
    showMapImage->setChecked(true);

    backMapImage = new QCheckBox("Move Back");
    backMapImage->setChecked(false);

    grid->addWidget( new QLabel("Visilibity:"), row++, 0 );
    grid->addWidget( showNodes, row, 1 );
    grid->addWidget( showNodeLabels, row++, 2 );
    grid->addWidget( showNodeLaneList, row++, 2 );
    grid->addWidget( showRelatedLanes, row++, 2 );

    QHBoxLayout *LaneListHBox = new QHBoxLayout();
    LaneListHBox->addWidget( prevLaneList );
    LaneListHBox->addWidget( nextLaneList );
    LaneListHBox->addStretch(1);
    grid->addLayout(  LaneListHBox, row++, 2 );

    grid->addWidget( showLanes, row, 1 );
    grid->addWidget( showLaneLabels, row++, 2 );
    grid->addWidget( new QLabel("Width:"), row, 1, 1, 1, Qt::AlignRight );
    grid->addWidget( laneWidth, row++, 2 );
    grid->addWidget( showTrafficSignals, row, 1 );
    grid->addWidget( showTrafficSignalLabels, row++, 2 );
    grid->addWidget( showStopLines, row, 1 );
    grid->addWidget( showStopLineLabels, row++, 2 );
    grid->addWidget( showPedestLanes, row, 1 );
    grid->addWidget( showPedestLaneLabels, row++, 2 );

    grid->addWidget( showLabels, row++, 1 );
    grid->addWidget( showMapImage, row, 1 );
    grid->addWidget( backMapImage, row++, 2 );


    selectNode = new QCheckBox("Node");
    selectNode->setChecked(true);

    selectLane = new QCheckBox("Lane");
    selectLane->setChecked(true);

    selectTrafficSignal = new QCheckBox("Traffic Signal");
    selectTrafficSignal->setChecked(true);

    selectStopLine = new QCheckBox("Stop Line");
    selectStopLine->setChecked(true);

    selectPedestLane = new QCheckBox("Pedestrian Lane");
    selectPedestLane->setChecked(true);

    grid->addWidget( new QLabel("Selection:"), row++, 0 );
    grid->addWidget( selectNode, row++, 1 );
    grid->addWidget( selectLane, row++, 1 );
    grid->addWidget( selectTrafficSignal, row++, 1 );
    grid->addWidget( selectStopLine, row++, 1 );
    grid->addWidget( selectPedestLane, row++, 1 );


    QHBoxLayout *mainLayout = new QHBoxLayout();

    mainLayout->addLayout( grid );

    setLayout( mainLayout );

    setWindowTitle("Display&Edit Control");
}


void DisplayControl::moveToClicked()
{
    float x = moveToX->text().toFloat();
    float y = moveToY->text().toFloat();
    emit ViewMoveTo(x,y);
}

