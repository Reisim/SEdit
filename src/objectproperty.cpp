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

RoadObjectProperty::RoadObjectProperty(QWidget *parent) : QWidget(parent)
{
    road = NULL;

    QSize infoAreaSizeNode = QSize(500,800);
    QSize infoAreaSizeLane = QSize(500,800);
    QSize infoAreaSizeTS = QSize(500,350);
    QSize infoAreaSizeSL = QSize(500,300);
    QSize infoAreaSizePL = QSize(500,300);

    tab = new QTabWidget();

    nodePage = new QWidget();

    nodeIDSB = new QSpinBox();
    nodeIDSB->setMinimum(0);
    nodeIDSB->setFixedWidth(80);
    nodeIDSB->setMaximum(10000);

    nodeInfo = new QLabel("\nNot exist.\n");
    nodeInfo->setAlignment( Qt::AlignTop );

    nodeInfoScrollArea = new QScrollArea();
    nodeInfoScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    nodeInfoScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    nodeInfoScrollArea->setWidget( nodeInfo );
    nodeInfoScrollArea->setMinimumSize( infoAreaSizeNode );

    cbONode = new QCheckBox("Origin Node");
    cbDNode = new QCheckBox("Destination Node");

    connect( cbONode, SIGNAL(toggled(bool)), this, SLOT(CBOriginNodeChanged(bool)) );
    connect( cbDNode, SIGNAL(toggled(bool)), this, SLOT(CBDestinationNodeChanged(bool)) );

    QGridLayout *nodeGrid = new QGridLayout();
    nodeGrid->addWidget( new QLabel("ID:"), 0, 0 );
    nodeGrid->addWidget( nodeIDSB, 0, 1 );
    nodeGrid->addWidget( cbONode, 1, 1 );
    nodeGrid->addWidget( cbDNode, 2, 1 );
    nodeGrid->addWidget( nodeInfoScrollArea, 3, 1 );
    nodeGrid->setRowStretch(4,1);
    nodeGrid->setColumnStretch(2,1);

    nodePage->setLayout( nodeGrid );


    lanePage = new QWidget();

    laneIDSB = new QSpinBox();
    laneIDSB->setMinimum(0);
    laneIDSB->setFixedWidth(80);
    laneIDSB->setMaximum(10000);

    laneSpeed = new QSpinBox();
    laneSpeed->setMinimum(5);
    laneSpeed->setMaximum(300);
    laneSpeed->setValue(60);
    laneSpeed->setFixedWidth(120);
    laneSpeed->setSuffix( QString("[km/h]") );
    connect( laneSpeed, SIGNAL(valueChanged(int)), this, SLOT(SpeedLimitChanged(int)) );

    laneActualSpeed = new QSpinBox();
    laneActualSpeed->setMinimum(5);
    laneActualSpeed->setMaximum(300);
    laneActualSpeed->setValue(65);
    laneActualSpeed->setFixedWidth(120);
    laneActualSpeed->setSuffix( QString("[km/h]") );
    connect( laneActualSpeed, SIGNAL(valueChanged(int)), this, SLOT(ActualSpeedChanged(int)) );

    laneAutomaticDrivingEnabled = new QCheckBox("Automatic Driving Enabled");
    laneAutomaticDrivingEnabled->setChecked(false);
    connect( laneAutomaticDrivingEnabled, SIGNAL(toggled(bool)), this, SLOT(AutomaticDrivingEnableFlagChanged(bool)) );

    laneDriverErrorProb = new QDoubleSpinBox();
    laneDriverErrorProb->setMinimum(0.0);
    laneDriverErrorProb->setMaximum(1.0);
    laneDriverErrorProb->setValue(0.0);
    laneDriverErrorProb->setDecimals(4);
    laneDriverErrorProb->setFixedWidth(120);
    connect( laneDriverErrorProb, SIGNAL(valueChanged(double)), this, SLOT(DriverErrorProbChanged(double)) );


    laneInfo = new QLabel();
    laneInfo->setMinimumSize( infoAreaSizeLane );

    QGridLayout *laneGrid = new QGridLayout();
    laneGrid->addWidget( new QLabel("ID:"), 0, 0 );
    laneGrid->addWidget( laneIDSB, 0, 1 );
    laneGrid->addWidget( new QLabel("Speed Limit:"), 1, 0 );
    laneGrid->addWidget( laneSpeed, 1, 1 );
    laneGrid->addWidget( new QLabel("Actual Speed:"), 2, 0 );
    laneGrid->addWidget( laneActualSpeed, 2, 1 );
    laneGrid->addWidget( new QLabel("ODD:"), 3, 0 );
    laneGrid->addWidget( laneAutomaticDrivingEnabled, 3, 1 );
    laneGrid->addWidget( new QLabel("Driver Error:"), 4, 0 );
    laneGrid->addWidget( laneDriverErrorProb, 4, 1 );
    laneGrid->addWidget( laneInfo, 5, 1 );
    laneGrid->setColumnStretch(4,1);

    lanePage->setLayout( laneGrid );

    trafficSignalPage = new QWidget();
    tsIDSB = new QSpinBox();
    tsIDSB->setMinimum(0);
    tsIDSB->setFixedWidth(80);
    tsInfo = new QLabel();
    tsInfo->setFixedSize( infoAreaSizeTS );

    tsStartOffset = new QSpinBox();
    tsStartOffset->setMinimum(0);
    tsStartOffset->setFixedWidth(120);
    tsStartOffset->setSuffix("[s]");


    tsDisplayPattern = new QTableWidget;
    tsDisplayPattern->setColumnCount(7);

    QStringList tableLabels;
    tableLabels << "G";
    tableLabels << "A/Y";
    tableLabels << "R";
    tableLabels << "<";
    tableLabels << "^";
    tableLabels << ">";
    tableLabels << "Duration[s]";

    for(int i=0;i<6;++i){
        tsDisplayPattern->setColumnWidth(i,10);
    }

    tsDisplayPattern->setHorizontalHeaderLabels( tableLabels );

    SetDefaultTSPattern();

    addRowDisplayPattern = new QPushButton("Add Row");
    addRowDisplayPattern->setIcon(QIcon(":/images/Add.png"));
    connect( addRowDisplayPattern,SIGNAL(clicked()),this,SLOT(TSPatternAddRowClicked()));

    delRowDisplayPattern = new QPushButton("Remove Row");
    delRowDisplayPattern->setIcon(QIcon(":/images/Remove.png"));
    connect( delRowDisplayPattern,SIGNAL(clicked()),this,SLOT(TSPatternDelRowClicked()));

    applyDisplayPattern = new QPushButton("Apply Data");
    applyDisplayPattern->setIcon(QIcon(":/images/accept.png"));
    connect( applyDisplayPattern,SIGNAL(clicked()),this,SLOT(TSPatternApplyClicked()));

    QHBoxLayout *tsDPButtonLayout = new QHBoxLayout();
    tsDPButtonLayout->addWidget(addRowDisplayPattern);
    tsDPButtonLayout->addWidget(delRowDisplayPattern);
    tsDPButtonLayout->addWidget(applyDisplayPattern);
    tsDPButtonLayout->addStretch(1);

    QGridLayout *tsGrid = new QGridLayout();
    tsGrid->addWidget( new QLabel("ID:"), 0, 0 );
    tsGrid->addWidget( tsIDSB, 0, 1 );
    tsGrid->addWidget( tsInfo, 1, 1 );
    tsGrid->addWidget( new QLabel("Start Offset:"), 2, 0 );
    tsGrid->addWidget( tsStartOffset, 2, 1 );
    tsGrid->addWidget( new QLabel("Display Pettern:"), 3, 0, 1, 2 , Qt::AlignLeft );
    tsGrid->addLayout( tsDPButtonLayout, 4, 1 );
    tsGrid->addWidget( tsDisplayPattern, 5, 1 );
    tsGrid->setColumnStretch(4,1);

    trafficSignalPage->setLayout( tsGrid );

    stopLinePage = new QWidget();
    slIDSB = new QSpinBox();
    slIDSB->setMinimum(0);
    slIDSB->setFixedWidth(80);
    slInfo = new QLabel();
    slInfo->setMinimumSize( infoAreaSizeSL );

    QGridLayout *slGrid = new QGridLayout();
    slGrid->addWidget( new QLabel("ID:"), 0, 0 );
    slGrid->addWidget( slIDSB, 0, 1 );
    slGrid->addWidget( slInfo, 1, 1 );
    slGrid->setColumnStretch(2,1);

    stopLinePage->setLayout( slGrid );


    pedestLanePage = new QWidget();

    pedestLaneIDSB = new QSpinBox();
    pedestLaneIDSB->setMinimum(0);
    pedestLaneIDSB->setFixedWidth(80);
    pedestLaneInfo = new QLabel();

    pedestLaneSectionSB = new QSpinBox();
    pedestLaneSectionSB->setMinimum(0);
    pedestLaneSectionSB->setFixedWidth(80);

    pedestLaneInfo->setMinimumSize( infoAreaSizePL );

    QGridLayout *plGrid = new QGridLayout();
    plGrid->addWidget( new QLabel("ID:"), 0, 0 );
    plGrid->addWidget( pedestLaneIDSB, 0, 1 );

    pedestLaneTrafficVolume = new QTableWidget();
    pedestLaneTrafficVolume->setColumnCount(2);

    tableLabels.clear();
    tableLabels << "Pedestrian Kind";
    tableLabels << "Volume[person/h]";

    pedestLaneTrafficVolume->setHorizontalHeaderLabels( tableLabels );

    plGrid->addWidget( pedestLaneTrafficVolume, 1, 1 );

    plGrid->addWidget( new QLabel("Section:"), 2, 0 );
    plGrid->addWidget( pedestLaneSectionSB, 2, 1 );

    cbIsCrossWalk = new QCheckBox("Cross Walk");

    pedestLaneWidth = new QDoubleSpinBox();
    pedestLaneWidth->setMinimum(0.0);
    pedestLaneWidth->setMaximum(10.0);
    pedestLaneWidth->setValue(2.0);
    pedestLaneWidth->setDecimals(4);
    pedestLaneWidth->setFixedWidth(120);
    pedestLaneWidth->setSuffix("[m]");

    pedestRunOutProb = new QDoubleSpinBox();
    pedestRunOutProb->setMinimum(0.0);
    pedestRunOutProb->setMaximum(1.0);
    pedestRunOutProb->setValue(0.0);
    pedestRunOutProb->setDecimals(4);
    pedestRunOutProb->setFixedWidth(120);

    pedestRunOutDirect = new QComboBox();
    QStringList runOutDirItem;
    runOutDirItem << "Left" << "Right";
    pedestRunOutDirect->addItems( runOutDirItem );
    pedestRunOutDirect->setFixedWidth(120);

    plGrid->addWidget( new QLabel("Width"), 3, 0 );
    plGrid->addWidget( pedestLaneWidth, 3, 1 );
    plGrid->addWidget( cbIsCrossWalk, 4, 1 );
    plGrid->addWidget( new QLabel("Run-out Prob."), 5, 0 );
    plGrid->addWidget( pedestRunOutProb, 5, 1 );
    plGrid->addWidget( new QLabel("Run-out Direct"), 6, 0 );
    plGrid->addWidget( pedestRunOutDirect, 6, 1 );
    plGrid->addWidget( pedestLaneInfo, 7, 1 );

    plGrid->setColumnStretch(8,1);

    applyPedestLaneDataChange = new QPushButton("Apply");
    applyPedestLaneDataChange->setIcon(QIcon(":/images/accept.png"));
    connect( applyPedestLaneDataChange,SIGNAL(clicked()),this,SLOT(PedestLaneApplyClicked()));

    QHBoxLayout* apldcLay = new QHBoxLayout();
    apldcLay->addStretch(1);
    apldcLay->addWidget( applyPedestLaneDataChange );
    apldcLay->addStretch(1);

    QVBoxLayout *plMainLay = new QVBoxLayout();
    plMainLay->addLayout( plGrid );
    plMainLay->addLayout( apldcLay );

    pedestLanePage->setLayout( plMainLay );


    connect(nodeIDSB,SIGNAL(valueChanged(int)),this,SLOT(ChangeNodeInfo(int)));
    connect(laneIDSB,SIGNAL(valueChanged(int)),this,SLOT(ChangeLaneInfo(int)));
    connect(tsIDSB,  SIGNAL(valueChanged(int)),this,SLOT(ChangeTrafficSignalInfo(int)));
    connect(slIDSB,  SIGNAL(valueChanged(int)),this,SLOT(ChangeStopLineInfo(int)));
    connect(pedestLaneIDSB,  SIGNAL(valueChanged(int)),this,SLOT(ChangePedestLaneInfo(int)));


    tab->addTab( nodePage, QString("Node") );
    tab->addTab( lanePage, QString("Lane") );
    tab->addTab( trafficSignalPage, QString("Traffic Signal") );
    tab->addTab( stopLinePage, QString("Stop Line") );
    tab->addTab( pedestLanePage, QString("Pedestrian Lane") );


    QVBoxLayout *mainLayout = new QVBoxLayout();

    mainLayout->addWidget( tab );

    setLayout( mainLayout );

    setWindowTitle("Road Object Property");
}


void RoadObjectProperty::ChangeTabPage(int page)
{
    if( page < 0 || page >= 5 ){
        return;
    }

    tab->setCurrentIndex( page );
}


void RoadObjectProperty::ChangeVisibilityODRoute(int page)
{
    if( page == 0 ){
        emit ShowODRouteEdit();
    }
    else{
        emit HideODRouteEdit();
    }
}


