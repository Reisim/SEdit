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
    QSize infoAreaSizeLane = QSize(300,800);
    QSize infoAreaSizeTS = QSize(500,350);
    QSize infoAreaSizeSL = QSize(300,300);

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

    laneInfo = new QLabel();
    laneInfo->setMinimumSize( infoAreaSizeLane );

    QGridLayout *laneGrid = new QGridLayout();
    laneGrid->addWidget( new QLabel("ID:"), 0, 0 );
    laneGrid->addWidget( laneIDSB, 0, 1 );
    laneGrid->addWidget( new QLabel("Speed Limit:"), 1, 0 );
    laneGrid->addWidget( laneSpeed, 1, 1 );
    laneGrid->addWidget( laneInfo, 2, 1 );
    laneGrid->setColumnStretch(3,1);

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
    slGrid->setColumnStretch(3,1);

    stopLinePage->setLayout( slGrid );


    connect(nodeIDSB,SIGNAL(valueChanged(int)),this,SLOT(ChangeNodeInfo(int)));
    connect(laneIDSB,SIGNAL(valueChanged(int)),this,SLOT(ChangeLaneInfo(int)));
    connect(tsIDSB,  SIGNAL(valueChanged(int)),this,SLOT(ChangeTrafficSignalInfo(int)));
    connect(slIDSB,  SIGNAL(valueChanged(int)),this,SLOT(ChangeStopLineInfo(int)));


    tab->addTab( nodePage, QString("Node") );
    tab->addTab( lanePage, QString("Lane") );
    tab->addTab( trafficSignalPage, QString("Traffic Signal") );
    tab->addTab( stopLinePage, QString("Stop Line") );

    //connect( tab, SIGNAL(currentChanged(int)), this, SLOT(ChangeVisibilityODRoute(int)) );

    QVBoxLayout *mainLayout = new QVBoxLayout();

    mainLayout->addWidget( tab );

    setLayout( mainLayout );

    setWindowTitle("Road Object Property");
}


void RoadObjectProperty::ChangeTabPage(int page)
{
    if( page < 0 || page >= 4 ){
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


