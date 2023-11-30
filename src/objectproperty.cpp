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

    QSize infoAreaSizeNode = QSize(500,700);
    QSize infoAreaSizeLane = QSize(500,600);
    QSize infoAreaSizeTS = QSize(500,350);
    QSize infoAreaSizeSL = QSize(500,300);
    QSize infoAreaSizePL = QSize(500,300);
    QSize infoAreaSizeSO = QSize(500,150);

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
    laneIDSB->setMaximum(50000);

    laneSpeed = new QSpinBox();
    laneSpeed->setMinimum(0);
    laneSpeed->setMaximum(300);
    laneSpeed->setValue(60);
    laneSpeed->setFixedWidth(120);
    laneSpeed->setSuffix( QString("[km/h]") );
    connect( laneSpeed, SIGNAL(valueChanged(int)), this, SLOT(SpeedLimitChanged(int)) );

    laneActualSpeed = new QSpinBox();
    laneActualSpeed->setMinimum(0);
    laneActualSpeed->setMaximum(300);
    laneActualSpeed->setValue(65);
    laneActualSpeed->setFixedWidth(120);
    laneActualSpeed->setSuffix( QString("[km/h]") );
    connect( laneActualSpeed, SIGNAL(valueChanged(int)), this, SLOT(ActualSpeedChanged(int)) );

    laneAutomaticDrivingEnabled = new QCheckBox("Automatic Driving Enabled");
    laneAutomaticDrivingEnabled->setChecked(false);
    connect( laneAutomaticDrivingEnabled, SIGNAL(toggled(bool)), this, SLOT(AutomaticDrivingEnableFlagChanged(bool)) );

    laneStartX = new QDoubleSpinBox();
    laneStartX->setMinimum( -1.0e10 );
    laneStartX->setMaximum(  1.0e10 );
    laneStartX->setValue( 0.0 );
    laneStartX->setDecimals( 2 );
    laneStartX->setFixedWidth( 120 );
    laneStartX->setSingleStep( 0.1 );

    laneStartY = new QDoubleSpinBox();
    laneStartY->setMinimum( -1.0e10 );
    laneStartY->setMaximum(  1.0e10 );
    laneStartY->setValue( 0.0 );
    laneStartY->setDecimals( 2 );
    laneStartY->setFixedWidth( 120 );
    laneStartY->setSingleStep( 0.1 );

    laneStartZ = new QDoubleSpinBox();
    laneStartZ->setMinimum( -1.0e10 );
    laneStartZ->setMaximum(  1.0e10 );
    laneStartZ->setValue( 0.0 );
    laneStartZ->setDecimals( 2 );
    laneStartZ->setFixedWidth( 120 );
    laneStartZ->setSingleStep( 0.1 );

    laneStartDir = new QDoubleSpinBox();
    laneStartDir->setMinimum( -180.0 );
    laneStartDir->setMaximum(  180.0 );
    laneStartDir->setValue( 0.0 );
    laneStartDir->setDecimals( 2 );
    laneStartDir->setFixedWidth( 120 );
    laneStartDir->setSingleStep( 0.1 );


    laneEndX = new QDoubleSpinBox();
    laneEndX->setMinimum( -1.0e10 );
    laneEndX->setMaximum(  1.0e10 );
    laneEndX->setValue( 0.0 );
    laneEndX->setDecimals( 2 );
    laneEndX->setFixedWidth( 120 );
    laneEndX->setSingleStep( 0.1 );

    laneEndY = new QDoubleSpinBox();
    laneEndY->setMinimum( -1.0e10 );
    laneEndY->setMaximum(  1.0e10 );
    laneEndY->setValue( 0.0 );
    laneEndY->setDecimals( 2 );
    laneEndY->setFixedWidth( 120 );
    laneEndY->setSingleStep( 0.1 );

    laneEndZ = new QDoubleSpinBox();
    laneEndZ->setMinimum( -1.0e10 );
    laneEndZ->setMaximum(  1.0e10 );
    laneEndZ->setValue( 0.0 );
    laneEndZ->setDecimals( 2 );
    laneEndZ->setFixedWidth( 120 );
    laneEndZ->setSingleStep( 0.1 );

    laneEndDir = new QDoubleSpinBox();
    laneEndDir->setMinimum( -180.0 );
    laneEndDir->setMaximum(  180.0 );
    laneEndDir->setValue( 0.0 );
    laneEndDir->setDecimals( 2 );
    laneEndDir->setFixedWidth( 120 );
    laneEndDir->setSingleStep( 0.1 );


    updateLaneShapeBtn = new QPushButton("Update Lane Shape");
    updateLaneShapeBtn->setIcon(QIcon(":/images/refresh.png"));
    updateLaneShapeBtn->setFixedSize( updateLaneShapeBtn->sizeHint() );
    connect( updateLaneShapeBtn, SIGNAL(clicked()), this, SLOT(LaneEdgePosChanged()) );

    getHeightFromUEBtn = new QPushButton("Get Height From UE");
    getHeightFromUEBtn->setIcon(QIcon(":/images/fit_lane.png"));
    getHeightFromUEBtn->setFixedSize( getHeightFromUEBtn->sizeHint() );
    connect( getHeightFromUEBtn, SIGNAL(clicked()), this, SLOT(GetLaneHeightFromUE()) );


    laneDriverErrorProb = new QDoubleSpinBox();
    laneDriverErrorProb->setMinimum(0.0);
    laneDriverErrorProb->setMaximum(1.0);
    laneDriverErrorProb->setValue(0.0);
    laneDriverErrorProb->setDecimals(4);
    laneDriverErrorProb->setFixedWidth(120);
    connect( laneDriverErrorProb, SIGNAL(valueChanged(double)), this, SLOT(DriverErrorProbChanged(double)) );

    laneWidth = new QDoubleSpinBox();
    laneWidth->setMinimum(2.5);
    laneWidth->setMaximum(20.0);
    laneWidth->setValue(3.0);
    laneWidth->setDecimals(2);
    laneWidth->setFixedWidth(120);
    connect( laneWidth, SIGNAL(valueChanged(double)), this, SLOT(LaneWidthChanged(double)) );

    editLaneData = new QPushButton("Edit");
    editLaneData->setIcon(QIcon(":/images/edit.png"));
    editLaneData->setFixedSize( editLaneData->sizeHint() );
    connect( editLaneData, SIGNAL(clicked()), this, SLOT(EditLaneData()) );

    checkRelatedNode = new QPushButton("Check Related Node");
    checkRelatedNode->setIcon(QIcon(":/images/refresh.png"));
    checkRelatedNode->setFixedSize( checkRelatedNode->sizeHint() );
    connect( checkRelatedNode, SIGNAL(clicked()), this, SLOT(CheckRelatedNode()) );

    QHBoxLayout *laneButtonsLayout = new QHBoxLayout();
    laneButtonsLayout->addWidget( editLaneData );
    laneButtonsLayout->addWidget( checkRelatedNode );
    laneButtonsLayout->addStretch( 1 );

    QHBoxLayout *laneEdgePosStartLayout = new QHBoxLayout();
    laneEdgePosStartLayout->addWidget( new QLabel("X ") );
    laneEdgePosStartLayout->addWidget( laneStartX );
    laneEdgePosStartLayout->addSpacing( 10 );
    laneEdgePosStartLayout->addWidget( new QLabel("Y ") );
    laneEdgePosStartLayout->addWidget( laneStartY );
    laneEdgePosStartLayout->addSpacing( 10 );
    laneEdgePosStartLayout->addWidget( new QLabel("Z ") );
    laneEdgePosStartLayout->addWidget( laneStartZ );
    laneEdgePosStartLayout->addSpacing( 10 );
    laneEdgePosStartLayout->addWidget( new QLabel("Dir ") );
    laneEdgePosStartLayout->addWidget( laneStartDir );
    laneEdgePosStartLayout->addStretch( 1 );

    QHBoxLayout *laneEdgePosEndLayout = new QHBoxLayout();
    laneEdgePosEndLayout->addWidget( new QLabel("X ") );
    laneEdgePosEndLayout->addWidget( laneEndX );
    laneEdgePosEndLayout->addSpacing( 10 );
    laneEdgePosEndLayout->addWidget( new QLabel("Y ") );
    laneEdgePosEndLayout->addWidget( laneEndY );
    laneEdgePosEndLayout->addSpacing( 10 );
    laneEdgePosEndLayout->addWidget( new QLabel("Z ") );
    laneEdgePosEndLayout->addWidget( laneEndZ );
    laneEdgePosEndLayout->addSpacing( 10 );
    laneEdgePosEndLayout->addWidget( new QLabel("Dir ") );
    laneEdgePosEndLayout->addWidget( laneEndDir );
    laneEdgePosEndLayout->addStretch( 1 );

    laneInfo = new QLabel("\nNot exist.\n");
    laneInfo->setAlignment( Qt::AlignTop );

    laneInfoScrollArea = new QScrollArea();
    laneInfoScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    laneInfoScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    laneInfoScrollArea->setWidget( laneInfo );
    laneInfoScrollArea->setMinimumSize( infoAreaSizeLane );

    QGridLayout *laneGrid = new QGridLayout();
    laneGrid->addWidget( new QLabel("ID:"), 0, 0 );
    laneGrid->addWidget( laneIDSB, 0, 1 );
    laneGrid->addWidget( new QLabel("Speed Limit:"), 1, 0 );
    laneGrid->addWidget( laneSpeed, 1, 1 );
    laneGrid->addWidget( new QLabel("Actual Speed:"), 2, 0 );
    laneGrid->addWidget( laneActualSpeed, 2, 1 );
    laneGrid->addWidget( new QLabel("Width:"), 3, 0 );
    laneGrid->addWidget( laneWidth, 3, 1 );
    laneGrid->addWidget( new QLabel("Start Point:"), 4, 0 );
    laneGrid->addLayout( laneEdgePosStartLayout, 4, 1 );
    laneGrid->addWidget( new QLabel("End Point:"), 5, 0 );
    laneGrid->addLayout( laneEdgePosEndLayout, 5, 1 );

    QHBoxLayout *tmpShapeBtnLay = new QHBoxLayout();
    tmpShapeBtnLay->addWidget( updateLaneShapeBtn );
    tmpShapeBtnLay->addWidget( getHeightFromUEBtn );
    tmpShapeBtnLay->addStretch(1);
    laneGrid->addLayout( tmpShapeBtnLay, 6, 1 );

    laneGrid->addWidget( new QLabel("ODD:"), 7, 0 );
    laneGrid->addWidget( laneAutomaticDrivingEnabled, 7, 1);
    laneGrid->addWidget( new QLabel("Driver Error:"), 8, 0 );
    laneGrid->addWidget( laneDriverErrorProb, 8, 1 );
    laneGrid->addLayout( laneButtonsLayout, 9, 1 );
    laneGrid->addWidget( laneInfoScrollArea, 10, 1 );
    laneGrid->setRowStretch(11,1);
    laneGrid->setColumnStretch(2,1);

    lanePage->setLayout( laneGrid );

    trafficSignalPage = new QWidget();
    tsIDSB = new QSpinBox();
    tsIDSB->setMinimum(0);
    tsIDSB->setFixedWidth(80);
    tsIDSB->setMaximum(10000);

    tsInfo = new QLabel("\nNot exist.\n");
    tsInfo->setAlignment( Qt::AlignTop );

    tsInfoScrollArea = new QScrollArea();
    tsInfoScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tsInfoScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tsInfoScrollArea->setWidget( tsInfo );
    tsInfoScrollArea->setMinimumSize( infoAreaSizeTS );

    tsStartOffset = new QSpinBox();
    tsStartOffset->setMinimum(0);
    tsStartOffset->setFixedWidth(120);
    tsStartOffset->setSuffix("[s]");

    tsChangeTimeBySensor = new QSpinBox();
    tsChangeTimeBySensor->setMinimum(0);
    tsChangeTimeBySensor->setFixedWidth(120);
    tsChangeTimeBySensor->setSuffix("[s]");

    tsIsSensorType = new QCheckBox();

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
    tsDisplayPattern->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

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

    copyDisplayPattern = new QPushButton("Copy Data From Other TS");
    copyDisplayPattern->setIcon(QIcon(":/images/back.png"));
    connect( copyDisplayPattern,SIGNAL(clicked()),this,SLOT(TSPatternCopyClicked()));

    QHBoxLayout *tsDPButtonLayout = new QHBoxLayout();
    tsDPButtonLayout->addWidget(addRowDisplayPattern);
    tsDPButtonLayout->addWidget(delRowDisplayPattern);
    tsDPButtonLayout->addStretch(1);

    QHBoxLayout *tsApplyButtonLayout = new QHBoxLayout();
    tsApplyButtonLayout->addStretch(1);
    tsApplyButtonLayout->addWidget(applyDisplayPattern);
    tsApplyButtonLayout->addWidget(copyDisplayPattern);
    tsApplyButtonLayout->addStretch(1);

    QGridLayout *tsGrid = new QGridLayout();
    tsGrid->addWidget( new QLabel("ID:"), 0, 0 );
    tsGrid->addWidget( tsIDSB, 0, 1 );
    tsGrid->addWidget( tsInfoScrollArea, 1, 1 );
    tsGrid->addWidget( new QLabel("Sensor-Type:"), 2, 0 );
    tsGrid->addWidget( tsIsSensorType, 2, 1 );
    tsGrid->addWidget( new QLabel("Change Time:"), 3, 0 );
    tsGrid->addWidget( tsChangeTimeBySensor, 3, 1 );
    tsGrid->addWidget( new QLabel("Start Offset:"), 4, 0 );
    tsGrid->addWidget( tsStartOffset, 4, 1 );
    tsGrid->addWidget( new QLabel("Display Pettern:"), 5, 0, 1, 2 , Qt::AlignLeft );
    tsGrid->addLayout( tsDPButtonLayout, 6, 1 );
    tsGrid->addWidget( tsDisplayPattern, 7, 1 );
    tsGrid->addLayout( tsApplyButtonLayout, 8, 1 );
    tsGrid->setRowStretch(9,1);

    trafficSignalPage->setLayout( tsGrid );

    stopLinePage = new QWidget();
    slIDSB = new QSpinBox();
    slIDSB->setMinimum(0);
    slIDSB->setFixedWidth(80);
    slIDSB->setMaximum(10000);

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

    pedestMarginToRoad = new QDoubleSpinBox();
    pedestMarginToRoad->setMinimum(0.0);
    pedestMarginToRoad->setMaximum(100.0);
    pedestMarginToRoad->setValue(0.0);
    pedestMarginToRoad->setDecimals(4);
    pedestMarginToRoad->setFixedWidth(120);

    pedestRunOutDirect = new QComboBox();
    QStringList runOutDirItem;
    runOutDirItem << "Left" << "Right";
    pedestRunOutDirect->addItems( runOutDirItem );
    pedestRunOutDirect->setFixedWidth(120);

    getPedestHeightFromUEBtn = new QPushButton("Get Height From UE");
    getPedestHeightFromUEBtn->setIcon(QIcon(":/images/fit_lane.png"));
    getPedestHeightFromUEBtn->setFixedSize( getPedestHeightFromUEBtn->sizeHint() );
    connect( getPedestHeightFromUEBtn,SIGNAL(clicked()),this,SLOT(GetPedestLaneHeightFromUE()));

    plGrid->addWidget( new QLabel("Width"), 3, 0 );
    plGrid->addWidget( pedestLaneWidth, 3, 1 );
    plGrid->addWidget( cbIsCrossWalk, 4, 1 );
    plGrid->addWidget( new QLabel("Run-out Prob."), 5, 0 );
    plGrid->addWidget( pedestRunOutProb, 5, 1 );
    plGrid->addWidget( new QLabel("Run-out Direct"), 6, 0 );
    plGrid->addWidget( pedestRunOutDirect, 6, 1 );
    plGrid->addWidget( new QLabel("Margine to Road"), 7, 0 );
    plGrid->addWidget( pedestMarginToRoad, 7, 1 );
    plGrid->addWidget( getPedestHeightFromUEBtn, 8, 1 );
    plGrid->addWidget( pedestLaneInfo, 9, 1 );
    plGrid->setRowStretch(10,1);
    plGrid->setColumnStretch(2,1);


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


    staticObjPage = new QWidget();
    soIDSB = new QSpinBox();
    soIDSB->setMinimum(0);
    soIDSB->setFixedWidth(80);
    soIDSB->setMaximum(10000);

    soInfo = new QLabel();
    soInfo->setMinimumSize( infoAreaSizeSO );

    editStaticObject = new QPushButton("Edit");
    editStaticObject->setIcon(QIcon(":/images/edit.png"));
    editStaticObject->setFixedSize( editStaticObject->sizeHint() );
    connect( editStaticObject, SIGNAL(clicked()), this, SLOT(EditStaticObjectData()) );

    QGridLayout *soGrid = new QGridLayout();
    soGrid->addWidget( new QLabel("ID:"), 0, 0 );
    soGrid->addWidget( soIDSB, 0, 1 );
    soGrid->addWidget( soInfo, 1, 1 );
    soGrid->addWidget( editStaticObject, 2, 1 );
    soGrid->setRowStretch(3,1);
    soGrid->setColumnStretch(2,1);

    staticObjPage->setLayout( soGrid );


    connect(nodeIDSB,SIGNAL(valueChanged(int)),this,SLOT(ChangeNodeInfo(int)));
    connect(laneIDSB,SIGNAL(valueChanged(int)),this,SLOT(ChangeLaneInfo(int)));
    connect(tsIDSB,  SIGNAL(valueChanged(int)),this,SLOT(ChangeTrafficSignalInfo(int)));
    connect(slIDSB,  SIGNAL(valueChanged(int)),this,SLOT(ChangeStopLineInfo(int)));
    connect(pedestLaneIDSB,  SIGNAL(valueChanged(int)),this,SLOT(ChangePedestLaneInfo(int)));
    connect(soIDSB,  SIGNAL(valueChanged(int)),this,SLOT(ChangeStaticObjInfo(int)));


    tab->addTab( nodePage, QString("Node") );
    tab->addTab( lanePage, QString("Lane") );
    tab->addTab( trafficSignalPage, QString("Traffic Signal") );
    tab->addTab( stopLinePage, QString("Stop Line") );
    tab->addTab( pedestLanePage, QString("Pedestrian Lane") );
    tab->addTab( staticObjPage, QString("Static Object") );


    QVBoxLayout *mainLayout = new QVBoxLayout();

    QHBoxLayout *subLayout = new QHBoxLayout();

    cbChangeSelectionBySpinbox = new QCheckBox("Change Selection by ID-spinbox value change");
    subLayout->addWidget( cbChangeSelectionBySpinbox );
    subLayout->addStretch(1);

    mainLayout->addLayout( subLayout );
    mainLayout->addWidget( tab );

    setLayout( mainLayout );

    setWindowTitle("Road Object Property");
}


void RoadObjectProperty::ChangeTabPage(int page)
{
    if( page < 0 || page >= 6 ){
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


