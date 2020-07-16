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


#include "scenarioeditor.h"
#include <QDebug>
#include <math.h>


#define TABLE_TIME_WIDTH 100
#define TABLE_VAL_WIDTH 125
#define TABLE_MIN_WIDTH 255


ScenarioEditor::ScenarioEditor(SettingDialog *s, RoadInfo *r,QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Scenario Editor");

    currentScenarioFile = QString();


    setDlg = s;
    road   = r;

    //----- Graph Dialog
    speedProfileGraph = new GraphDialog();
    speedProfileGraph->Initialize(0);
    speedProfileGraph->setFixedSize( speedProfileGraph->sizeHint() );
    speedProfileGraph->setWindowTitle("Speed Profile");
    speedProfileGraph->hide();
    connect( speedProfileGraph, SIGNAL(requestDataAndUpdate(int)), this, SLOT(SetDataToGraph(int)) );

    accelDecelGraph = new GraphDialog();
    accelDecelGraph->Initialize(1);
    accelDecelGraph->setFixedSize( accelDecelGraph->sizeHint() );
    accelDecelGraph->setWindowTitle("Accel/Brake Override");
    accelDecelGraph->hide();
    connect( accelDecelGraph, SIGNAL(requestDataAndUpdate(int)), this, SLOT(SetDataToGraph(int)) );

    steerGraph = new GraphDialog();
    steerGraph->Initialize(2);
    steerGraph->setFixedSize( accelDecelGraph->sizeHint() );
    steerGraph->setWindowTitle("Steering Override");
    steerGraph->hide();
    connect( steerGraph, SIGNAL(requestDataAndUpdate(int)), this, SLOT(SetDataToGraph(int)) );

    pedestSpeedProfileGraph = new GraphDialog();
    pedestSpeedProfileGraph->Initialize(3);
    pedestSpeedProfileGraph->setFixedSize( pedestSpeedProfileGraph->sizeHint() );
    pedestSpeedProfileGraph->setWindowTitle("Speed Profile");
    pedestSpeedProfileGraph->hide();
    connect( pedestSpeedProfileGraph, SIGNAL(requestDataAndUpdate(int)), this, SLOT(SetDataToGraph(int)) );


    //----- Toolbar
    toolbar = new QToolBar;
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->addWidget( toolbar );
    toolbarLayout->addStretch( 1 );

    QAction* newAct = new QAction( tr("&New"), this );
    newAct->setIcon(QIcon(":/images/new.png"));
    newAct->setStatusTip( tr("Clear All Data nad Create a Scenario Data") );
    connect( newAct, SIGNAL(triggered()), this, SLOT(NewData()));
    toolbar->addAction( newAct );

    QAction* openAct = new QAction( tr("&Open"), this );
    openAct->setIcon(QIcon(":/images/open.png"));
    connect( openAct, SIGNAL(triggered()), this, SLOT(LoadData()));
    toolbar->addAction( openAct );

    QAction* saveAct = new QAction( tr("&Save"), this );
    saveAct->setIcon(QIcon(":/images/save.png"));
    connect( saveAct, SIGNAL(triggered()), this, SLOT(SaveData()));
    toolbar->addAction( saveAct );

    QAction* saveAsAct = new QAction( tr("&SaveAs"), this );
    saveAsAct->setIcon(QIcon(":/images/saveas.png"));
    connect( saveAsAct, SIGNAL(triggered()), this, SLOT(SaveAsData()));
    toolbar->addAction( saveAsAct );


    //----- Add/Del buttons
    QPushButton *addSSBtn = new QPushButton("Add System");
    addSSBtn->setIcon( QIcon(":/images/Bell.png") );
    addSSBtn->setFixedSize( addSSBtn->sizeHint() );
    connect( addSSBtn, SIGNAL(clicked(bool)), this, SLOT(AddScenarioSystem()) );

    QPushButton *addSVBtn = new QPushButton("Add Vehicle");
    addSVBtn->setIcon( QIcon(":/images/vehicle.png") );
    addSVBtn->setFixedSize( addSVBtn->sizeHint() );
    connect( addSVBtn, SIGNAL(clicked(bool)), this, SLOT(AddScenarioVehicle()) );

    QPushButton *addSPBtn = new QPushButton("Add Pedestrian");
    addSPBtn->setIcon( QIcon(":/images/pedestrian.png") );
    addSPBtn->setFixedSize( addSPBtn->sizeHint() );
    connect( addSPBtn, SIGNAL(clicked(bool)), this, SLOT(AddScenarioPedestrian()) );

    QPushButton *delSBtn = new QPushButton("Delete");
    delSBtn->setIcon( QIcon(":/images/delete.png") );
    delSBtn->setFixedSize( delSBtn->sizeHint() );
    connect( delSBtn, SIGNAL(clicked(bool)), this, SLOT(DelScenarioClicked()) );

    confirmToDelete = new QCheckBox("Need confirm to delete");
    confirmToDelete->setChecked( true );

    QPushButton *simBtn = new QPushButton("Simulate");
    simBtn->setIcon( QIcon(":/images/setting.png") );
    simBtn->setFixedSize( simBtn->sizeHint() );


    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget( addSSBtn );
    btnLayout->addWidget( addSVBtn );
    btnLayout->addWidget( addSPBtn );
    btnLayout->addWidget( delSBtn );
    btnLayout->addWidget( confirmToDelete );
    btnLayout->addStretch( 1 );
    btnLayout->addWidget( simBtn );


    //----- Tab Widget
    tabW = new QTabWidget();

    QWidget *sysTab = new QWidget();

    QVBoxLayout *sysTabLayout = new QVBoxLayout();

    sysList = new QListWidget();
    connect( sysList, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );

    sysTabLayout->addWidget( sysList );
    sysTabLayout->addStretch( 1 );

    sysTab->setLayout( sysTabLayout );

    QWidget *vehicleTab = new QWidget();

    QVBoxLayout *vTabLayout = new QVBoxLayout();

    vehicleList = new QListWidget();
    connect( vehicleList, SIGNAL(currentRowChanged(int)), this, SLOT(UpdateScenarioVehicleSlotList()) );

    vehicleSlot = new QListWidget();
    connect( vehicleSlot, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );

    vehicleAddSlot = new QPushButton("");
    vehicleAddSlot->setIcon( QIcon(":/images/Add.png") );
    vehicleAddSlot->setFixedSize( vehicleAddSlot->sizeHint() );
    connect( vehicleAddSlot, SIGNAL(clicked(bool)), this, SLOT( AddScenarioVehicleSlot()) );

    vehicleDelSlot = new QPushButton("");
    vehicleDelSlot->setIcon( QIcon(":/images/delete.png") );
    vehicleDelSlot->setFixedSize( vehicleDelSlot->sizeHint() );
    connect( vehicleDelSlot, SIGNAL(clicked(bool)), this, SLOT( DelScenarioVehicleSlot()) );

    QGridLayout *vehicleGrid = new QGridLayout();
    vehicleGrid->addWidget( new QLabel("Object:"), 1, 0 );
    vehicleGrid->addWidget( vehicleList, 2, 0 );
    vehicleGrid->addWidget( new QLabel("Slots:"), 1, 1 );
    vehicleGrid->addWidget( vehicleSlot, 2, 1 );

    QHBoxLayout *vehicleSlotBtnLayout= new QHBoxLayout();
    vehicleSlotBtnLayout->addWidget( vehicleAddSlot );
    vehicleSlotBtnLayout->addWidget( vehicleDelSlot );
    vehicleSlotBtnLayout->addStretch( 1 );

    vehicleGrid->addLayout( vehicleSlotBtnLayout, 0, 1 );

    vTabLayout->addLayout( vehicleGrid );
    vTabLayout->addStretch( 1 );

    vehicleTab->setLayout( vTabLayout );

    QWidget *pedestTab = new QWidget();

    QVBoxLayout *pTabLayout = new QVBoxLayout();

    pedestList = new QListWidget();
    connect( pedestList, SIGNAL(currentRowChanged(int)), this, SLOT(UpdateScenarioPedestSlotList()) );

    pedestSlot = new QListWidget();
    connect( pedestSlot, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );

    pedestAddSlot = new QPushButton("");
    pedestAddSlot->setIcon( QIcon(":/images/Add.png") );
    pedestAddSlot->setFixedSize( pedestAddSlot->sizeHint() );
    connect( pedestAddSlot, SIGNAL(clicked(bool)), this, SLOT( AddScenarioPedestSlot()) );

    pedestDelSlot = new QPushButton("");
    pedestDelSlot->setIcon( QIcon(":/images/delete.png") );
    pedestDelSlot->setFixedSize( pedestDelSlot->sizeHint() );
    connect( pedestDelSlot, SIGNAL(clicked(bool)), this, SLOT( DelScenarioPedestSlot()) );

    QGridLayout *pedestGrid = new QGridLayout();
    pedestGrid->addWidget( new QLabel("Object:"), 1, 0 );
    pedestGrid->addWidget( pedestList, 2, 0 );
    pedestGrid->addWidget( new QLabel("Slots:"), 1, 1 );
    pedestGrid->addWidget( pedestSlot, 2, 1 );

    QHBoxLayout *pedestSlotBtnLayout= new QHBoxLayout();
    pedestSlotBtnLayout->addWidget( pedestAddSlot );
    pedestSlotBtnLayout->addWidget( pedestDelSlot );
    pedestSlotBtnLayout->addStretch( 1 );

    pedestGrid->addLayout( pedestSlotBtnLayout, 0, 1 );

    pTabLayout->addLayout( pedestGrid );
    pTabLayout->addStretch( 1 );

    pedestTab->setLayout( pTabLayout );

    tabW->addTab( sysTab, QString("System") );
    tabW->addTab( vehicleTab, QString("Vehicle") );
    tabW->addTab( pedestTab, QString("Pedestrian") );

    connect( tabW, SIGNAL(currentChanged(int)), this, SLOT(ChangeActDataContents(int)) );


    //----- Conditions
    condDataGB = new QGroupBox("Trigger Condtions");

    AND_OR = new QComboBox();
    QStringList AND_OR_Items;
    AND_OR_Items << "AND" << "OR";
    AND_OR->addItems(AND_OR_Items);
    AND_OR->setFixedSize( AND_OR->sizeHint() );


    QGridLayout *cndDataGrid = new QGridLayout();

    int row = 0;
    cndDataGrid->addWidget( AND_OR, row, 0 );
    row++;

    //----- No trigger
    noTrigger = new QCheckBox("No Trigger");

    cndDataGrid->addWidget( noTrigger, row, 0 );
    row++;


    //----- Time trigger
    timeTrigger = new QCheckBox("Time Trigger");

    ttMinSB = new QSpinBox();
    ttMinSB->setMinimum(0);
    ttMinSB->setMaximum(59);
    ttMinSB->setValue(0);
    ttMinSB->setSuffix("[min]");

    ttSecSB = new QDoubleSpinBox();
    ttSecSB->setMinimum(0.0);
    ttSecSB->setMaximum(59.99);
    ttSecSB->setSingleStep(0.01);
    ttSecSB->setValue(0.0);
    ttSecSB->setSuffix("[sec]");

    QHBoxLayout *ttSBLayout = new QHBoxLayout();
    ttSBLayout->addWidget( ttMinSB );
    ttSBLayout->addWidget( ttSecSB );
    ttSBLayout->addStretch( 1 );

    ttRelative = new QRadioButton("Time from appear");
    ttAbsolute = new QRadioButton("Absolute Time");
    ttAbsolute->setChecked( true );

    ttRadioGroup = new QButtonGroup();
    ttRadioGroup->addButton( ttAbsolute );
    ttRadioGroup->addButton( ttRelative );

    QHBoxLayout *ttRBLayout = new QHBoxLayout();
    ttRBLayout->addWidget(ttAbsolute);
    ttRBLayout->addWidget(ttRelative);
    ttRBLayout->addStretch( 1 );


    cndDataGrid->addWidget( timeTrigger, row, 0 );
    cndDataGrid->addLayout( ttSBLayout, row, 1 );
    row++;

    cndDataGrid->addLayout( ttRBLayout, row, 1 );
    row++;


    //----- Position trigger
    positionTrigger = new QCheckBox("Position Trigger");

    ptX = new QDoubleSpinBox();
    ptX->setSingleStep(0.05);
    ptX->setValue(0.0);
    ptX->setMinimum( -1.0e6 );
    ptX->setMaximum( 1.0e6 );
    ptX->setPrefix("X:");
    ptX->setSuffix("[m]");

    ptY = new QDoubleSpinBox();
    ptY->setSingleStep(0.05);
    ptY->setValue(0.0);
    ptY->setMinimum( -1.0e6 );
    ptY->setMaximum( 1.0e6 );
    ptY->setPrefix("Y:");
    ptY->setSuffix("[m]");

    ptPsi = new QDoubleSpinBox();
    ptPsi->setSingleStep(0.05);
    ptPsi->setValue(0.0);
    ptPsi->setMinimum( -180.0 );
    ptPsi->setMaximum(  180.0 );
    ptPsi->setPrefix("Pass Direction:");
    ptPsi->setSuffix("[deg]");

    ptWidth = new QDoubleSpinBox();
    ptWidth->setSingleStep(0.05);
    ptWidth->setValue(10.0);
    ptWidth->setMinimum(   1.0 );
    ptWidth->setMaximum( 100.0 );
    ptWidth->setPrefix("Width:");
    ptWidth->setSuffix("[m]");

    ptTargetID = new QSpinBox();
    ptTargetID->setMinimum( -1 );
    ptTargetID->setMaximum( 100000 );
    ptTargetID->setValue( -1 );

    ptSelPosition = new QPushButton();
    ptSelPosition->setIcon( QIcon(":images/Flag_blue.png") );
    ptSelPosition->setFixedSize( ptSelPosition->sizeHint() );
    ptSelPosition->setObjectName("PositionTrigger");
    connect( ptSelPosition, SIGNAL(clicked(bool)), this, SLOT(SetPickMode()));

    QHBoxLayout *ptPosLayout = new QHBoxLayout();
    ptPosLayout->addWidget( ptX );
    ptPosLayout->addWidget( ptY );
    ptPosLayout->addWidget( ptSelPosition );
    ptPosLayout->addStretch( 1 );

    QHBoxLayout *ptAngleLayout = new QHBoxLayout();
    ptAngleLayout->addWidget( ptPsi );
    ptAngleLayout->addStretch( 1 );

    QHBoxLayout *ptWidthLayout = new QHBoxLayout();
    ptWidthLayout->addWidget( ptWidth );
    ptWidthLayout->addStretch( 1 );

    QHBoxLayout *ptTargetLayout = new QHBoxLayout();
    ptTargetLayout->addWidget( new QLabel("Target Object ID:") );
    ptTargetLayout->addWidget( ptTargetID );
    ptTargetLayout->addStretch( 1 );

    cndDataGrid->addWidget( positionTrigger, row, 0 );
    cndDataGrid->addLayout( ptPosLayout, row, 1 );
    row++;
    cndDataGrid->addLayout( ptAngleLayout, row, 1 );
    row++;
    cndDataGrid->addLayout( ptWidthLayout, row, 1 );
    row++;
    cndDataGrid->addLayout( ptTargetLayout, row, 1 );
    row++;


    //----- Velocity trigger
    velocityTrigger = new QCheckBox("Velocity Trigger");

    vtSpeed = new QDoubleSpinBox();
    vtSpeed->setSingleStep(0.1);
    vtSpeed->setValue(0.0);
    vtSpeed->setMinimum( 0.0 );
    vtSpeed->setMaximum( 200.0 );
    vtSpeed->setSuffix("[km/h]");

    vtSlower = new QRadioButton("Slower");
    vtHigher = new QRadioButton("Faster");
    vtHigher->setChecked( true );

    vtRadioGroup = new QButtonGroup();
    vtRadioGroup->addButton( vtHigher );
    vtRadioGroup->addButton( vtSlower );

    QHBoxLayout *vtSpeedLayout = new QHBoxLayout();
    vtSpeedLayout->addWidget(vtSpeed);
    vtSpeedLayout->addWidget(vtHigher);
    vtSpeedLayout->addWidget(vtSlower);
    vtSpeedLayout->addStretch( 1 );

    vtTargetID = new QSpinBox();
    vtTargetID->setMinimum( -1 );
    vtTargetID->setMaximum( 100000 );
    vtTargetID->setValue( -1 );

    QHBoxLayout *vtTargetLayout = new QHBoxLayout();
    vtTargetLayout->addWidget( new QLabel("Target Object ID:") );
    vtTargetLayout->addWidget( vtTargetID );
    vtTargetLayout->addStretch( 1 );

    cndDataGrid->addWidget( velocityTrigger, row, 0 );
    cndDataGrid->addLayout( vtSpeedLayout, row, 1 );
    row++;

    cndDataGrid->addLayout( vtTargetLayout, row, 1 );
    row++;


    //----- TTC trigger
    TTCTrigger = new QCheckBox("TTC Trigger");

    ttcX = new QDoubleSpinBox();
    ttcX->setSingleStep(0.05);
    ttcX->setValue(0.0);
    ttcX->setMinimum( -1.0e6 );
    ttcX->setMaximum( 1.0e6 );
    ttcX->setPrefix("X:");
    ttcX->setSuffix("[m]");

    ttcY = new QDoubleSpinBox();
    ttcY->setSingleStep(0.05);
    ttcY->setValue(0.0);
    ttcY->setMinimum( -1.0e6 );
    ttcY->setMaximum( 1.0e6 );
    ttcY->setPrefix("Y:");
    ttcY->setSuffix("[m]");

    ttcSelPosition = new QPushButton();
    ttcSelPosition->setIcon( QIcon(":images/Flag_blue.png") );
    ttcSelPosition->setFixedSize( ttcSelPosition->sizeHint() );
    ttcSelPosition->setObjectName("TTCTrigger");
    connect( ttcSelPosition, SIGNAL(clicked(bool)),this,SLOT(SetPickMode()) );

    ttcTargetID = new QSpinBox();
    ttcTargetID->setMinimum( -1 );
    ttcTargetID->setMaximum( 100000 );
    ttcTargetID->setValue( -1 );

    ttcCalObjectID = new QSpinBox();
    ttcCalObjectID->setMinimum( -1 );
    ttcCalObjectID->setMaximum( 100000 );
    ttcCalObjectID->setValue( -1 );
    ttcCalObjectID->setFixedWidth( ttcCalObjectID->sizeHint().width() );

    ttcVal = new QDoubleSpinBox();
    ttcVal->setSingleStep(0.01);
    ttcVal->setValue(0.0);
    ttcVal->setMinimum( 0.0 );
    ttcVal->setMaximum( 1000 );
    ttcVal->setPrefix("TTC:");
    ttcVal->setSuffix("[sec]");

    ttcObject = new QRadioButton("Calculate TTC to Object");
    ttcPoint = new QRadioButton("Calculate TTC to Point");
    ttcPoint->setChecked( true );

    ttcRadioGroup = new QButtonGroup();
    ttcRadioGroup->addButton( ttcObject );
    ttcRadioGroup->addButton( ttcPoint );

    QHBoxLayout *ttcValLayout = new QHBoxLayout();
    ttcValLayout->addWidget(ttcVal);
    ttcValLayout->addStretch( 1 );

    QHBoxLayout *ttcTargetLayout = new QHBoxLayout();
    ttcTargetLayout->addWidget( new QLabel("Target Object ID:") );
    ttcTargetLayout->addWidget( ttcTargetID );
    ttcTargetLayout->addStretch( 1 );

    QHBoxLayout *ttcPosLayout = new QHBoxLayout();
    ttcPosLayout->addWidget( ttcX );
    ttcPosLayout->addWidget( ttcY );
    ttcPosLayout->addWidget( ttcSelPosition );
    ttcPosLayout->addStretch( 1 );

    cndDataGrid->addWidget( TTCTrigger, row, 0 );
    cndDataGrid->addLayout( ttcValLayout, row, 1 );
    row++;

    cndDataGrid->addLayout( ttcTargetLayout, row, 1 );
    row++;

    cndDataGrid->addWidget( ttcPoint, row, 1 );
    row++;

    cndDataGrid->addLayout( ttcPosLayout, row, 1 );
    row++;

    cndDataGrid->addWidget( ttcObject, row, 1 );
    row++;

    cndDataGrid->addWidget( ttcCalObjectID, row, 1 );
    row++;

    // FuncExtender Trigger
    FETrigger = new QCheckBox("FuncExtend Trigger");

    cndDataGrid->addWidget( FETrigger, row, 0 );
    row++;

    // External Trigger
    externalTrigger = new QCheckBox("External Trigger");

    etKeys = new QComboBox();
    QStringList etKeyStrs;
    etKeyStrs << "F1" << "F2" << "F3" << "F4" << "F5" << "F6";
    etKeyStrs << "F7" << "F8" << "F9" << "F10" << "F11" << "F12";
    etKeyStrs << "Num0" << "Num1" << "Num2" << "Num3" << "Num4" << "Num5";
    etKeyStrs << "Num6" << "Num7" << "Num8" << "Num9";
    etKeys->addItems( etKeyStrs );
    etKeys->setCurrentIndex( 0 );

    etKeys->setFixedWidth( etKeys->sizeHint().width() );

    cndDataGrid->addWidget( externalTrigger, row, 0 );
    cndDataGrid->addWidget( etKeys, row, 1 );
    row++;

    condDataGB->setLayout( cndDataGrid );


    //----- Actions
    actDataGB = new QGroupBox("Actions");

    int maxHeight = 0;
    int maxWidth  = 0;

    //----- System actions
    QGridLayout *sysActDataGrid = new QGridLayout();

    sysActSelCB = new QComboBox();
    QStringList saSelCBStrs;
    saSelCBStrs << "Teleportation" << "Change Traffic Signal" << "Change Speed Info" << "Send UDP Data";
    sysActSelCB->addItems( saSelCBStrs );
    sysActSelCB->setFixedSize( sysActSelCB->sizeHint() );
    connect( sysActSelCB, SIGNAL(currentIndexChanged(int)),this,SLOT(ChangeSystemActSeletion(int)));

    sysActDataGrid->addWidget( new QLabel("Actions"), 0 , 0 );
    sysActDataGrid->addWidget( sysActSelCB, 0 , 1 );

    //--------
    saTeleportWidget = new QWidget();

    saMoveToX = new QDoubleSpinBox();
    saMoveToX->setMinimum( -1.0e6 );
    saMoveToX->setMaximum(  1.0e6 );
    saMoveToX->setValue( 0 );
    saMoveToX->setSingleStep( 0.01 );
    saMoveToX->setPrefix("X:");
    saMoveToX->setSuffix("[m]");

    saMoveToY = new QDoubleSpinBox();
    saMoveToY->setMinimum( -1.0e6 );
    saMoveToY->setMaximum(  1.0e6 );
    saMoveToY->setValue( 0 );
    saMoveToY->setSingleStep( 0.01 );
    saMoveToY->setPrefix("X:");
    saMoveToY->setSuffix("[m]");

    saMoveToPsi = new QDoubleSpinBox();
    saMoveToPsi->setMinimum( -180.0 );
    saMoveToPsi->setMaximum( 180.0 );
    saMoveToPsi->setValue( 0 );
    saMoveToPsi->setSingleStep( 0.01 );
    saMoveToPsi->setPrefix("Orientation:");
    saMoveToPsi->setSuffix("[deg]");

    saMoveToNearLaneID = new QSpinBox();
    saMoveToNearLaneID->setMinimum( -1 );
    saMoveToNearLaneID->setMaximum( 100000 );
    saMoveToNearLaneID->setValue( -1 );
    saMoveToNearLaneID->setPrefix("Nearest Lane ID: ");

    saMoveToTargetID = new QSpinBox();
    saMoveToTargetID->setMinimum( 0 );
    saMoveToTargetID->setMaximum( 100000 );
    saMoveToTargetID->setValue( 0 );
    saMoveToTargetID->setPrefix("Target Object ID: ");

    saMoveToPick = new QPushButton();
    saMoveToPick->setIcon( QIcon(":images/Flag_blue.png") );
    saMoveToPick->setFixedSize( saMoveToPick->sizeHint() );
    saMoveToPick->setObjectName("Teleport");
    connect( saMoveToPick, SIGNAL(clicked(bool)), this, SLOT(SetPickMode()) );

    saMoveToSetToNearestLane = new QPushButton();
    saMoveToSetToNearestLane->setIcon( QIcon(":images/fit_lane.png") );
    saMoveToSetToNearestLane->setFixedSize( saMoveToSetToNearestLane->sizeHint() );
    saMoveToSetToNearestLane->setObjectName("Teleport");
    connect( saMoveToSetToNearestLane, SIGNAL(clicked(bool)), this, SLOT(SetToNearestLane()) );

    saMoveToWithSurroundingVehicles = new QCheckBox("Move Surrounding Objects together");
    saMoveToOnlyOnce                = new QCheckBox("Teleport only once");
    saMoveToClearLateralOffset      = new QCheckBox("Clear Lateral Offset");

    QGridLayout *saTeleportGrid = new QGridLayout();
    row = 0;
    saTeleportGrid->addWidget( saMoveToX, row , 0 );
    saTeleportGrid->addWidget( saMoveToY, row , 1 );
    saTeleportGrid->addWidget( saMoveToPick, row , 2 );
    saTeleportGrid->addWidget( saMoveToSetToNearestLane, row , 3 );
    row++;

    saTeleportGrid->addWidget( saMoveToPsi, row , 0, 1, 2 );
    row++;

    saTeleportGrid->addWidget( saMoveToNearLaneID, row , 0, 1, 2 );
    row++;

    saTeleportGrid->addWidget( saMoveToTargetID, row , 0, 1, 2 );
    row++;

    saTeleportGrid->addWidget( saMoveToWithSurroundingVehicles, row , 0, 1, 2 );
    row++;

    saTeleportGrid->addWidget( saMoveToOnlyOnce, row , 0, 1, 2 );
    row++;

    saTeleportGrid->addWidget( saMoveToClearLateralOffset, row , 0, 1, 2 );
    row++;

    saTeleportGrid->setRowStretch( row, 1 );

    saTeleportWidget->setLayout( saTeleportGrid );

    if( maxHeight < saTeleportWidget->sizeHint().height() ){
        maxHeight = saTeleportWidget->sizeHint().height();
    }
    if( maxWidth < saTeleportWidget->sizeHint().width() ){
        maxWidth = saTeleportWidget->sizeHint().width();
    }


    //--------
    saChangeTSWidget = new QWidget();
    saChangeTSWidget->hide();

    saChangeTSTargetTSID = new QSpinBox();
    saChangeTSTargetTSID->setMinimum( 0 );
    saChangeTSTargetTSID->setMaximum( 100000 );
    saChangeTSTargetTSID->setValue( 0 );
    saChangeTSTargetTSID->setPrefix("Target TS ID: ");

    saChangeTSChangeToIndex = new QSpinBox();
    saChangeTSChangeToIndex->setMinimum( 0 );
    saChangeTSChangeToIndex->setMaximum( 100000 );
    saChangeTSChangeToIndex->setValue( 0 );
    saChangeTSChangeToIndex->setPrefix("Signal Display Index: ");

    saChangeTSPickTS = new QPushButton();
    saChangeTSPickTS->setIcon( QIcon(":images/Flag_blue.png") );
    saChangeTSPickTS->setFixedSize( saChangeTSPickTS->sizeHint() );
    saChangeTSPickTS->setObjectName("pickTS");
    connect( saChangeTSPickTS, SIGNAL(clicked(bool)), this, SLOT(SetPickMode()) );

    saChangeTSSystemDown = new QCheckBox("Display OFF");
    saChangeTSApplyAll   = new QCheckBox("Apply to All Signals");


    QGridLayout *saChangeTSGrid = new QGridLayout();
    row = 0;
    saChangeTSGrid->addWidget( saChangeTSTargetTSID, row , 0 );
    saChangeTSGrid->addWidget( saChangeTSPickTS,     row , 1 );
    row++;

    saChangeTSGrid->addWidget( saChangeTSChangeToIndex,     row , 0 );
    row++;

    saChangeTSGrid->addWidget( saChangeTSSystemDown,     row , 0 );
    row++;

    saChangeTSGrid->addWidget( saChangeTSApplyAll,     row , 0 );
    row++;

    saChangeTSGrid->setRowStretch( row, 1 );


    saChangeTSWidget->setLayout( saChangeTSGrid );

    if( maxHeight < saChangeTSWidget->sizeHint().height() ){
        maxHeight = saChangeTSWidget->sizeHint().height();
    }
    if( maxWidth < saChangeTSWidget->sizeHint().width() ){
        maxWidth = saChangeTSWidget->sizeHint().width();
    }


    //--------
    saChangeSpeedInfoWidget = new QWidget();
    saChangeSpeedInfoWidget->hide();

    saChangeSpeedInfoSpeedLimit = new QCheckBox("Change Speed Limit");
    saChangeSpeedInfoActualSpeed = new QCheckBox("Change Actual Speed");

    saChangeSpeedInfoSpeedVal = new QDoubleSpinBox();
    saChangeSpeedInfoSpeedVal->setMinimum( 0.0 );
    saChangeSpeedInfoSpeedVal->setMaximum( 250.0 );
    saChangeSpeedInfoSpeedVal->setValue( 0 );
    saChangeSpeedInfoSpeedVal->setSingleStep( 0.01 );
    saChangeSpeedInfoSpeedVal->setPrefix("Change to :");
    saChangeSpeedInfoSpeedVal->setSuffix("[km/h]");

    saChangeSpeedInfoTargetLanes = new QLineEdit();
    QRegExp laneListRegex ("^?([0-9]+)(,[0-9]+)*$");
    QRegExpValidator *laneListValidator = new QRegExpValidator(laneListRegex, this);
    saChangeSpeedInfoTargetLanes->setValidator( laneListValidator );

    saChangeSpeedInfoPickLane = new QPushButton();
    saChangeSpeedInfoPickLane->setIcon( QIcon(":images/Flag_blue.png") );
    saChangeSpeedInfoPickLane->setFixedSize( saChangeSpeedInfoPickLane->sizeHint() );
    saChangeSpeedInfoPickLane->setObjectName("pickLane");
    connect( saChangeSpeedInfoPickLane, SIGNAL(clicked(bool)), this, SLOT(SetPickMode()) );

    QGridLayout *saChangeSpeedInfoGrid = new QGridLayout();
    row = 0;

    saChangeSpeedInfoGrid->addWidget( saChangeSpeedInfoSpeedLimit, row , 0 );
    row++;

    saChangeSpeedInfoGrid->addWidget( saChangeSpeedInfoActualSpeed, row , 0 );
    row++;

    saChangeSpeedInfoGrid->addWidget( saChangeSpeedInfoSpeedVal, row , 0 );
    row++;

    saChangeSpeedInfoGrid->addWidget( saChangeSpeedInfoTargetLanes, row , 0 );
    saChangeSpeedInfoGrid->addWidget( saChangeSpeedInfoPickLane,     row , 1 );
    row++;

    saChangeSpeedInfoGrid->setRowStretch( row, 1 );

    saChangeSpeedInfoWidget->setLayout( saChangeSpeedInfoGrid );

    if( maxHeight < saChangeSpeedInfoWidget->sizeHint().height() ){
        maxHeight = saChangeSpeedInfoWidget->sizeHint().height();
    }
    if( maxWidth < saChangeSpeedInfoWidget->sizeHint().width() ){
        maxWidth = saChangeSpeedInfoWidget->sizeHint().width();
    }


    //--------
    saSendUDPWidget = new QWidget();
    saSendUDPWidget->hide();

    saUDPIPAddr = new QLineEdit();

    QString saIPRange = "(([ 0]+)|([ 0]*[0-9] *)|([0-9][0-9] )|([ 0][0-9][0-9])|(1[0-9][0-9])|([2][0-4][0-9])|(25[0-5]))";
    QRegExp saIPRegex ("^" + saIPRange
                     + "\\." + saIPRange
                     + "\\." + saIPRange
                     + "\\." + saIPRange + "$");
    QRegExpValidator *saIPValidator = new QRegExpValidator(saIPRegex, this);
    saUDPIPAddr->setValidator( saIPValidator );
    saUDPIPAddr->setInputMask("000.000.000.000");

    saUDPPortSB = new QSpinBox();
    saUDPPortSB->setMinimum( 0 );
    saUDPPortSB->setMaximum( 100000 );
    saUDPPortSB->setValue( 3000 );

    saUDPSendData = new QLineEdit();

    saRepeatSend = new QCheckBox("Repeat sedning the data");

    QGridLayout *saSendUDPGrid = new QGridLayout();
    saSendUDPGrid->addWidget( new QLabel("IP-Address"), 0 , 0 );
    saSendUDPGrid->addWidget( new QLabel("Port"), 1 , 0 );
    saSendUDPGrid->addWidget( new QLabel("IP-Address"), 0 , 0 );
    saSendUDPGrid->addWidget( saUDPIPAddr, 0 , 1 );
    saSendUDPGrid->addWidget( new QLabel("Port"), 1 , 0 );
    saSendUDPGrid->addWidget( saUDPPortSB, 1 , 1 );
    saSendUDPGrid->addWidget( new QLabel("Data"), 2 , 0 );
    saSendUDPGrid->addWidget( saUDPSendData, 2 , 1 );
    saSendUDPGrid->addWidget( saRepeatSend, 3 , 1 );

    saSendUDPGrid->setRowStretch( 4, 1 );

    saSendUDPWidget->setLayout( saSendUDPGrid );

    if( maxHeight < saSendUDPWidget->sizeHint().height() ){
        maxHeight = saSendUDPWidget->sizeHint().height();
    }
    if( maxWidth < saSendUDPWidget->sizeHint().width() ){
        maxWidth = saSendUDPWidget->sizeHint().width();
    }


    //--------
    sysActDataGrid->addWidget( saTeleportWidget,            1 , 1 );
    sysActDataGrid->addWidget( saChangeTSWidget,        2 , 1 );
    sysActDataGrid->addWidget( saChangeSpeedInfoWidget, 3 , 1 );
    sysActDataGrid->addWidget( saSendUDPWidget,         4 , 1 );
    sysActDataGrid->setColumnStretch(2,1);



    //----- Vehicle actions
    QGridLayout *vehicleActDataGrid = new QGridLayout();

    vehicleActSelCB = new QComboBox();
    QStringList vaSelCBStrs;
    vaSelCBStrs << "Appear" << "Control" << "Send UDP Data" << "Disappear";
    vehicleActSelCB->addItems( vaSelCBStrs );
    vehicleActSelCB->setFixedSize( vehicleActSelCB->sizeHint() );
    connect( vehicleActSelCB, SIGNAL(currentIndexChanged(int)),this,SLOT(ChangeVehicleActSeletion(int)));

    vehicleActDataGrid->addWidget( new QLabel("Actions"), 0 , 0 );
    vehicleActDataGrid->addWidget( vehicleActSelCB, 0 , 1 );

    //--------
    vaAppearWidget = new QWidget();

    vaAppearX = new QDoubleSpinBox();
    vaAppearX->setMinimum( -1.0e6 );
    vaAppearX->setMaximum(  1.0e6 );
    vaAppearX->setValue( 0 );
    vaAppearX->setSingleStep( 0.01 );
    vaAppearX->setPrefix("X:");
    vaAppearX->setSuffix("[m]");

    vaAppearY = new QDoubleSpinBox();
    vaAppearY->setMinimum( -1.0e6 );
    vaAppearY->setMaximum(  1.0e6 );
    vaAppearY->setValue( 0 );
    vaAppearY->setSingleStep( 0.01 );
    vaAppearY->setPrefix("Y:");
    vaAppearY->setSuffix("[m]");

    vaAppearPsi = new QDoubleSpinBox();
    vaAppearPsi->setMinimum( -180.0 );
    vaAppearPsi->setMaximum( 180.0 );
    vaAppearPsi->setValue( 0 );
    vaAppearPsi->setSingleStep( 0.01 );
    vaAppearPsi->setPrefix("Orientation:");
    vaAppearPsi->setSuffix("[deg]");

    vaAppearSpeed = new QDoubleSpinBox();
    vaAppearSpeed->setMinimum( 0.0 );
    vaAppearSpeed->setMaximum( 250.0 );
    vaAppearSpeed->setValue( 0 );
    vaAppearSpeed->setSingleStep( 0.01 );
    vaAppearSpeed->setPrefix("Speed:");
    vaAppearSpeed->setSuffix("[km/h]");

    vaAppearNearLaneID = new QSpinBox();
    vaAppearNearLaneID->setMinimum( -1 );
    vaAppearNearLaneID->setMaximum( 50000 );
    vaAppearNearLaneID->setValue( -1 );
    vaAppearNearLaneID->setPrefix( "Nearest Lane ID : " );

    vaVehicleModelID = new QSpinBox();
    vaVehicleModelID->setMinimum( 0 );
    vaVehicleModelID->setMaximum( setDlg->GetVehicleKindNum() - 1 );
    vaVehicleModelID->setValue( 0 );
    vaVehicleModelID->setPrefix( "Vehicle Model ID : " );

    vaAppearPick = new QPushButton();
    vaAppearPick->setIcon( QIcon(":images/Flag_blue.png") );
    vaAppearPick->setFixedSize( vaAppearPick->sizeHint() );
    vaAppearPick->setObjectName("VehicleAppear");
    connect( vaAppearPick, SIGNAL(clicked(bool)), this, SLOT(SetPickMode()) );

    vaAppearSetToNearestLane = new QPushButton();
    vaAppearSetToNearestLane->setIcon( QIcon(":images/fit_lane.png") );
    vaAppearSetToNearestLane->setFixedSize( vaAppearSetToNearestLane->sizeHint() );
    vaAppearSetToNearestLane->setObjectName("VehicleAppear");
    connect( vaAppearSetToNearestLane, SIGNAL(clicked(bool)), this, SLOT(SetToNearestLane()) );

    vaAppearAllowRepeat = new QCheckBox("Appear again");

    vaAppearNodeRoute = new QRadioButton("Node Route");
    vaAppearNodeRoute->setChecked( true );

    vaAppearSetNodeRoute = new QPushButton("Set");
    vaAppearSetNodeRoute->setIcon( QIcon(":images/setting.png") );
    vaAppearSetNodeRoute->setFixedSize( vaAppearSetNodeRoute->sizeHint() );
    vaAppearSetNodeRoute->setObjectName("VehicleSetNodeRoute");
    connect( vaAppearSetNodeRoute, SIGNAL(clicked(bool)), this, SLOT(SetPickMode()) );

    vaAppearClearNodeRoute = new QPushButton("Clear");
    vaAppearClearNodeRoute->setIcon( QIcon(":images/delete.png") );
    vaAppearClearNodeRoute->setFixedSize( vaAppearClearNodeRoute->sizeHint() );
    vaAppearClearNodeRoute->setObjectName("VehicleClearNodeRoute");
    connect( vaAppearClearNodeRoute, SIGNAL(clicked(bool)), this, SLOT(ClearRouteData()) );

    vaAppearPathRoute = new QRadioButton("Path Route");

    vaAppearSetPathRoute = new QPushButton("Set");
    vaAppearSetPathRoute->setIcon( QIcon(":images/setting.png") );
    vaAppearSetPathRoute->setFixedSize( vaAppearSetPathRoute->sizeHint() );
    vaAppearSetPathRoute->setObjectName("VehicleSetPathRoute");
    connect( vaAppearSetPathRoute, SIGNAL(clicked(bool)), this, SLOT(SetPickMode()) );

    vaAppearClearPathRoute = new QPushButton("Clear");
    vaAppearClearPathRoute->setIcon( QIcon(":images/delete.png") );
    vaAppearClearPathRoute->setFixedSize( vaAppearClearPathRoute->sizeHint() );
    vaAppearClearPathRoute->setObjectName("VehicleClearPathRoute");
    connect( vaAppearClearPathRoute, SIGNAL(clicked(bool)), this, SLOT(ClearRouteData()) );

    vaAppearRadioGroup = new QButtonGroup();
    vaAppearRadioGroup->addButton( vaAppearNodeRoute );
    vaAppearRadioGroup->addButton( vaAppearPathRoute );

    vaAppearRouteInfo = new QLabel("Route Info:");

    QGridLayout *vaAppearGrid = new QGridLayout();
    row = 0;
    vaAppearGrid->addWidget( vaAppearX, row , 0 );
    vaAppearGrid->addWidget( vaAppearY, row , 1 );
    vaAppearGrid->addWidget( vaAppearPick, row , 2 );
    vaAppearGrid->addWidget( vaAppearSetToNearestLane, row , 3 );
    row++;

    vaAppearGrid->addWidget( vaAppearNearLaneID, row , 0, 1, 2 );
    row++;

    vaAppearGrid->addWidget( vaAppearPsi, row , 0, 1, 2 );
    row++;

    vaAppearGrid->addWidget( vaAppearSpeed, row , 0, 1, 2 );
    row++;

    vaAppearGrid->addWidget( vaVehicleModelID, row , 0, 1, 2 );
    row++;

    vaAppearGrid->addWidget( vaAppearNodeRoute, row , 0, 1, 2 );
    row++;

    QHBoxLayout *vaAppearNodeRouteManipLayout = new QHBoxLayout();
    vaAppearNodeRouteManipLayout->addWidget( vaAppearSetNodeRoute );
    vaAppearNodeRouteManipLayout->addWidget( vaAppearClearNodeRoute );
    vaAppearNodeRouteManipLayout->addStretch( 1 );

    vaAppearGrid->addLayout( vaAppearNodeRouteManipLayout, row , 0, 1, 2 );
    row++;

    vaAppearGrid->addWidget( vaAppearPathRoute, row , 0, 1, 2 );
    row++;

    QHBoxLayout *vaAppearPathRouteManipLayout = new QHBoxLayout();
    vaAppearPathRouteManipLayout->addWidget( vaAppearSetPathRoute );
    vaAppearPathRouteManipLayout->addWidget( vaAppearClearPathRoute );
    vaAppearPathRouteManipLayout->addStretch( 1 );

    vaAppearGrid->addLayout( vaAppearPathRouteManipLayout, row , 0, 1, 2 );
    row++;

    vaAppearGrid->addWidget( vaAppearRouteInfo, row , 0, 1, 2 );
    row++;

    vaAppearGrid->addWidget( vaAppearAllowRepeat, row , 0, 1, 2 );
    row++;

    vaAppearGrid->setRowStretch( row, 1 );

    vaAppearWidget->setLayout( vaAppearGrid );

    if( maxHeight < vaAppearWidget->sizeHint().height() ){
        maxHeight = vaAppearWidget->sizeHint().height();
    }
    if( maxWidth < vaAppearWidget->sizeHint().width() ){
        maxWidth = vaAppearWidget->sizeHint().width();
    }

    //--------
    vaControlWidget = new QWidget();
    vaControlWidget->hide();

    vaCtrlChangeControlMode = new QCheckBox("Change Control Mode");

    vaCtrlSelControlMode = new QComboBox();
    QStringList ctrlMode;
    ctrlMode<< "Agent Logic" << "Headway Distance Control" << "Speed Profile Control" << "Stop Control";
    vaCtrlSelControlMode->addItems( ctrlMode );

    vaCtrlTargetID = new QSpinBox();
    vaCtrlTargetID->setMinimum( -1 );
    vaCtrlTargetID->setMaximum( 10000 );
    vaCtrlTargetID->setValue( -1 );
    vaCtrlTargetID->setPrefix("Target Object ID: ");

    vaCtrlTargetSpeed = new QDoubleSpinBox();
    vaCtrlTargetSpeed->setMinimum( 0.0 );
    vaCtrlTargetSpeed->setMaximum( 200.0 );
    vaCtrlTargetSpeed->setValue( 60.0 );
    vaCtrlTargetSpeed->setPrefix("Target Speed: ");
    vaCtrlTargetSpeed->setSuffix("[km/h]");

    vaCtrlTargetHeadwayTime = new QDoubleSpinBox();
    vaCtrlTargetHeadwayTime->setMinimum( -1.0 );
    vaCtrlTargetHeadwayTime->setMaximum( 200.0 );
    vaCtrlTargetHeadwayTime->setValue( 1.0 );
    vaCtrlTargetHeadwayTime->setPrefix("Headway Time: ");
    vaCtrlTargetHeadwayTime->setSuffix("[s]");

    vaCtrlTargetHeadwayDistance = new QDoubleSpinBox();
    vaCtrlTargetHeadwayDistance->setMinimum( -1.0 );
    vaCtrlTargetHeadwayDistance->setMaximum( 200.0 );
    vaCtrlTargetHeadwayDistance->setValue( 5.0 );
    vaCtrlTargetHeadwayDistance->setPrefix("Headway Distance: ");
    vaCtrlTargetHeadwayDistance->setSuffix("[m]");

    vaCtrlStopX = new QDoubleSpinBox();
    vaCtrlStopX->setMinimum( -1.0e6 );
    vaCtrlStopX->setMaximum(  1.0e6 );
    vaCtrlStopX->setValue( 0.0 );
    vaCtrlStopX->setPrefix("X: ");
    vaCtrlStopX->setSuffix("[m]");

    vaCtrlStopY = new QDoubleSpinBox();
    vaCtrlStopY->setMinimum( -1.0e6 );
    vaCtrlStopY->setMaximum(  1.0e6 );
    vaCtrlStopY->setValue( 0.0 );
    vaCtrlStopY->setPrefix("Y: ");
    vaCtrlStopY->setSuffix("[m]");

    vaCtrlSelPosition = new QPushButton();
    vaCtrlSelPosition->setIcon( QIcon(":images/Flag_blue.png") );
    vaCtrlSelPosition->setFixedSize( vaCtrlSelPosition->sizeHint() );
    vaCtrlSelPosition->setObjectName("VehicleStopPoint");
    connect( vaCtrlSelPosition,SIGNAL(clicked(bool)), this, SLOT(SetPickMode()) );

    vaCtrlSpeedProfile = new QTableWidget();
    vaCtrlSpeedProfile->setColumnCount(2);

    QStringList tableLabels;
    tableLabels << "Time[s]";
    tableLabels << "Speed[km/h]";

    vaCtrlSpeedProfile->setHorizontalHeaderLabels( tableLabels );
    vaCtrlSpeedProfile->setColumnWidth(0,TABLE_TIME_WIDTH);
    vaCtrlSpeedProfile->setColumnWidth(1,TABLE_VAL_WIDTH);

    vaCtrlSpeedProfile->setMinimumWidth( TABLE_MIN_WIDTH );

    addRowSpeedProfileTable = new QPushButton("Add");
    addRowSpeedProfileTable->setIcon( QIcon(":images/Add.png") );
    addRowSpeedProfileTable->setFixedSize( addRowSpeedProfileTable->sizeHint() );
    addRowSpeedProfileTable->setObjectName("SpeedProfile");
    connect( addRowSpeedProfileTable, SIGNAL(clicked()), this, SLOT(AddRowTable()) );

    delRowSpeedProfileTable = new QPushButton("Del");
    delRowSpeedProfileTable->setIcon( QIcon(":images/delete.png") );
    delRowSpeedProfileTable->setFixedSize( delRowSpeedProfileTable->sizeHint() );
    delRowSpeedProfileTable->setObjectName("SpeedProfile");
    connect( delRowSpeedProfileTable, SIGNAL(clicked()), this, SLOT(DelRowTable()) );

    showSpeedProfileTableAsGraph = new QPushButton("Graph");
    showSpeedProfileTableAsGraph->setIcon( QIcon(":images/Chart_bar.png") );
    showSpeedProfileTableAsGraph->setFixedSize( showSpeedProfileTableAsGraph->sizeHint() );
    connect( showSpeedProfileTableAsGraph, SIGNAL(clicked()), speedProfileGraph, SLOT(show()) );

    vaCtrlSteer = new QCheckBox("Steering Input");

    vaCtrlSteerTable = new QTableWidget();
    vaCtrlSteerTable->setColumnCount(2);

    tableLabels.clear();
    tableLabels << "Time[s]";
    tableLabels << "Steer[deg]";

    vaCtrlSteerTable->setHorizontalHeaderLabels( tableLabels );
    vaCtrlSteerTable->setColumnWidth(0,TABLE_TIME_WIDTH);
    vaCtrlSteerTable->setColumnWidth(1,TABLE_VAL_WIDTH);

    vaCtrlSteerTable->setMinimumWidth( TABLE_MIN_WIDTH );

    addRowSteerTable = new QPushButton("Add");
    addRowSteerTable->setIcon( QIcon(":images/Add.png") );
    addRowSteerTable->setFixedSize( addRowSteerTable->sizeHint() );
    addRowSteerTable->setObjectName("Steer");
    connect( addRowSteerTable, SIGNAL(clicked()), this, SLOT(AddRowTable()) );

    delRowSteerTable = new QPushButton("Del");
    delRowSteerTable->setIcon( QIcon(":images/delete.png") );
    delRowSteerTable->setFixedSize( delRowSteerTable->sizeHint() );
    delRowSteerTable->setObjectName("Steer");
    connect( delRowSteerTable, SIGNAL(clicked()), this, SLOT(DelRowTable()) );

    showSteerTableAsGraph = new QPushButton("Graph");
    showSteerTableAsGraph->setIcon( QIcon(":images/Chart_bar.png") );
    showSteerTableAsGraph->setFixedSize( showSteerTableAsGraph->sizeHint() );
    connect( showSteerTableAsGraph, SIGNAL(clicked()), steerGraph, SLOT(show()) );


    vaCtrlAccelDecel = new QCheckBox("Accel/Brake Input");

    vaCtrlAccelDecelTable = new QTableWidget();
    vaCtrlAccelDecelTable->setColumnCount(2);

    tableLabels.clear();
    tableLabels << "Time[s]";
    tableLabels << "Accel[G]";

    vaCtrlAccelDecelTable->setHorizontalHeaderLabels( tableLabels );
    vaCtrlAccelDecelTable->setColumnWidth(0,TABLE_TIME_WIDTH);
    vaCtrlAccelDecelTable->setColumnWidth(1,TABLE_VAL_WIDTH);

    vaCtrlAccelDecelTable->setMinimumWidth( TABLE_MIN_WIDTH );

    addRowAccelDecelTable = new QPushButton("Add");
    addRowAccelDecelTable->setIcon( QIcon(":images/Add.png") );
    addRowAccelDecelTable->setFixedSize( addRowAccelDecelTable->sizeHint() );
    addRowAccelDecelTable->setObjectName("AccelDecel");
    connect( addRowAccelDecelTable, SIGNAL(clicked()), this, SLOT(AddRowTable()) );

    delRowAccelDecelTable = new QPushButton("Del");
    delRowAccelDecelTable->setIcon( QIcon(":images/delete.png") );
    delRowAccelDecelTable->setFixedSize( delRowAccelDecelTable->sizeHint() );
    delRowAccelDecelTable->setObjectName("AccelDecel");
    connect( delRowAccelDecelTable, SIGNAL(clicked()), this, SLOT(DelRowTable()) );

    showAccelDecelTableAsGraph = new QPushButton("Graph");
    showAccelDecelTableAsGraph->setIcon( QIcon(":images/Chart_bar.png") );
    showAccelDecelTableAsGraph->setFixedSize( showAccelDecelTableAsGraph->sizeHint() );
    connect( showAccelDecelTableAsGraph, SIGNAL(clicked()), accelDecelGraph, SLOT(show()) );


    vaCtrlAllowRepeat = new QCheckBox("Apply again");


    QGridLayout *vaControlGrid = new QGridLayout();

    row = 0;
    vaControlGrid->addWidget( vaCtrlChangeControlMode, row , 0 );
    vaControlGrid->addWidget( vaCtrlSelControlMode, row , 1 );
    row++;

    vaControlGrid->addWidget( vaCtrlTargetID, row , 1 );
    row++;
    vaControlGrid->addWidget( vaCtrlTargetSpeed, row , 1 );
    row++;
    vaControlGrid->addWidget( vaCtrlTargetHeadwayTime, row , 1 );
    row++;
    vaControlGrid->addWidget( vaCtrlTargetHeadwayDistance, row , 1 );
    row++;

    QHBoxLayout *vaCtrlSpeedProfileAllLayout = new QHBoxLayout();

    QVBoxLayout *vaCtrlSpeedProfileBtnsLayout = new QVBoxLayout();
    vaCtrlSpeedProfileBtnsLayout->addWidget( addRowSpeedProfileTable );
    vaCtrlSpeedProfileBtnsLayout->addWidget( delRowSpeedProfileTable );
    vaCtrlSpeedProfileBtnsLayout->addWidget( showSpeedProfileTableAsGraph );
    vaCtrlSpeedProfileBtnsLayout->addStretch( 1 );

    vaCtrlSpeedProfileAllLayout->addWidget( vaCtrlSpeedProfile );
    vaCtrlSpeedProfileAllLayout->addLayout( vaCtrlSpeedProfileBtnsLayout );

    vaControlGrid->addLayout( vaCtrlSpeedProfileAllLayout, row , 1 );
    row++;

    QHBoxLayout *vaCtrlStopLayout = new QHBoxLayout();
    vaCtrlStopLayout->addWidget( vaCtrlStopX );
    vaCtrlStopLayout->addWidget( vaCtrlStopY );
    vaCtrlStopLayout->addWidget( vaCtrlSelPosition );
    vaCtrlStopLayout->addStretch( 1 );

    vaControlGrid->addLayout( vaCtrlStopLayout, row , 1 );
    row++;

    vaControlGrid->addWidget( vaCtrlAccelDecel, row , 0, Qt::AlignTop );

    QHBoxLayout *vaCtrlAccelDecelAllLayout = new QHBoxLayout();

    QVBoxLayout *vaCtrlAccelDecelBtnsLayout = new QVBoxLayout();
    vaCtrlAccelDecelBtnsLayout->addWidget( addRowAccelDecelTable );
    vaCtrlAccelDecelBtnsLayout->addWidget( delRowAccelDecelTable );
    vaCtrlAccelDecelBtnsLayout->addWidget( showAccelDecelTableAsGraph );
    vaCtrlAccelDecelBtnsLayout->addStretch( 1 );

    vaCtrlAccelDecelAllLayout->addWidget( vaCtrlAccelDecelTable );
    vaCtrlAccelDecelAllLayout->addLayout( vaCtrlAccelDecelBtnsLayout );

    vaControlGrid->addLayout( vaCtrlAccelDecelAllLayout, row , 1 );
    row++;

    vaControlGrid->addWidget( vaCtrlSteer, row , 0, Qt::AlignTop );

    QHBoxLayout *vaCtrlSteerAllLayout = new QHBoxLayout();

    QVBoxLayout *vaCtrlSteerBtnsLayout = new QVBoxLayout();
    vaCtrlSteerBtnsLayout->addWidget( addRowSteerTable );
    vaCtrlSteerBtnsLayout->addWidget( delRowSteerTable );
    vaCtrlSteerBtnsLayout->addWidget( showSteerTableAsGraph );
    vaCtrlSteerBtnsLayout->addStretch( 1 );

    vaCtrlSteerAllLayout->addWidget( vaCtrlSteerTable );
    vaCtrlSteerAllLayout->addLayout( vaCtrlSteerBtnsLayout );

    vaControlGrid->addLayout( vaCtrlSteerAllLayout, row , 1 );
    row++;

    vaControlGrid->addWidget( vaCtrlAllowRepeat, row, 0 );
    row++;

    vaControlGrid->setRowStretch( row, 1 );

    vaControlWidget->setLayout( vaControlGrid );

    if( maxHeight < vaControlWidget->sizeHint().height() ){
        maxHeight = vaControlWidget->sizeHint().height();
    }
    if( maxWidth < vaControlWidget->sizeHint().width() ){
        maxWidth = vaControlWidget->sizeHint().width();
    }

    //--------
    vaUDPWidget = new QWidget();
    vaUDPWidget->hide();

    vaUDPIPAddr = new QLineEdit();

    QString vaIPrange = "(([ 0]+)|([ 0]*[0-9] *)|([0-9][0-9] )|([ 0][0-9][0-9])|(1[0-9][0-9])|([2][0-4][0-9])|(25[0-5]))";
    QRegExp vaIPRegex ("^" + vaIPrange
                     + "\\." + vaIPrange
                     + "\\." + vaIPrange
                     + "\\." + vaIPrange + "$");
    QRegExpValidator *vaIPValidator = new QRegExpValidator(vaIPRegex, this);
    vaUDPIPAddr->setValidator( vaIPValidator );
    vaUDPIPAddr->setInputMask("000.000.000.000");

    vaUDPPortSB = new QSpinBox();
    vaUDPPortSB->setMinimum( 0 );
    vaUDPPortSB->setMaximum( 100000 );
    vaUDPPortSB->setValue( 3000 );

    vaUDPSendData = new QLineEdit();

    vaRepeatSend = new QCheckBox("Repeat sedning the data");

    QGridLayout *vaUDPGrid = new QGridLayout();
    vaUDPGrid->addWidget( new QLabel("IP-Address"), 0 , 0 );
    vaUDPGrid->addWidget( new QLabel("Port"), 1 , 0 );
    vaUDPGrid->addWidget( new QLabel("IP-Address"), 0 , 0 );
    vaUDPGrid->addWidget( vaUDPIPAddr, 0 , 1 );
    vaUDPGrid->addWidget( new QLabel("Port"), 1 , 0 );
    vaUDPGrid->addWidget( vaUDPPortSB, 1 , 1 );
    vaUDPGrid->addWidget( new QLabel("Data"), 2 , 0 );
    vaUDPGrid->addWidget( vaUDPSendData, 2 , 1 );
    vaUDPGrid->addWidget( vaRepeatSend, 3 , 1 );

    vaUDPGrid->setRowStretch( 4, 1 );

    vaUDPWidget->setLayout( vaUDPGrid );

    vaDisappearWidget = new QWidget();
    vaDisappearWidget->hide();

    vaDisappearAppearSoon = new QCheckBox("Apply if appeared again");

    QGridLayout *vaDisappearGrid = new QGridLayout();
    row = 0;

    vaDisappearGrid->addWidget( vaDisappearAppearSoon, row , 0 );
    row++;

    vaDisappearGrid->setRowStretch( row, 1 );

    vaDisappearWidget->setLayout( vaDisappearGrid );

    if( maxHeight < vaDisappearWidget->sizeHint().height() ){
        maxHeight = vaDisappearWidget->sizeHint().height();
    }
    if( maxWidth < vaDisappearWidget->sizeHint().width() ){
        maxWidth = vaDisappearWidget->sizeHint().width();
    }


    //--------
    vehicleActDataGrid->addWidget( vaAppearWidget,    1 , 1 );
    vehicleActDataGrid->addWidget( vaControlWidget,   2 , 1 );
    vehicleActDataGrid->addWidget( vaUDPWidget,       3 , 1 );
    vehicleActDataGrid->addWidget( vaDisappearWidget, 4 , 1 );
    vehicleActDataGrid->setColumnStretch(2,1);

    if( maxWidth < vehicleActDataGrid->sizeHint().width() ){
        maxWidth = vehicleActDataGrid->sizeHint().width();
    }


    //----- Pedest actions
    QGridLayout *pedestActDataGrid = new QGridLayout();

    pedestActSelCB = new QComboBox();
    QStringList paSelCBStrs;
    paSelCBStrs << "Appear" << "Control" << "Send UDP Data" << "Disappear";
    pedestActSelCB->addItems( paSelCBStrs );
    pedestActSelCB->setFixedSize( pedestActSelCB->sizeHint() );
    connect( pedestActSelCB, SIGNAL(currentIndexChanged(int)),this,SLOT(ChangePedestActSeletion(int)));

    pedestActDataGrid->addWidget( new QLabel("Actions"), 0 , 0 );
    pedestActDataGrid->addWidget( pedestActSelCB, 0 , 1 );


    //--------
    paAppearWidget = new QWidget();

    paAppearX = new QDoubleSpinBox();
    paAppearX->setMinimum( -1.0e6 );
    paAppearX->setMaximum(  1.0e6 );
    paAppearX->setValue( 0 );
    paAppearX->setSingleStep( 0.01 );
    paAppearX->setPrefix("X:");
    paAppearX->setSuffix("[m]");

    paAppearY = new QDoubleSpinBox();
    paAppearY->setMinimum( -1.0e6 );
    paAppearY->setMaximum(  1.0e6 );
    paAppearY->setValue( 0 );
    paAppearY->setSingleStep( 0.01 );
    paAppearY->setPrefix("Y:");
    paAppearY->setSuffix("[m]");

    paAppearPsi = new QDoubleSpinBox();
    paAppearPsi->setMinimum( -180.0 );
    paAppearPsi->setMaximum( 180.0 );
    paAppearPsi->setValue( 0 );
    paAppearPsi->setSingleStep( 0.01 );
    paAppearPsi->setPrefix("Orientation:");
    paAppearPsi->setSuffix("[deg]");

    paAppearNearLaneID = new QSpinBox();
    paAppearNearLaneID->setMinimum( -1 );
    paAppearNearLaneID->setMaximum( 50000 );
    paAppearNearLaneID->setValue( -1 );
    paAppearNearLaneID->setPrefix( "Nearest Lane ID : " );

    paPedestModelID = new QSpinBox();
    paPedestModelID->setMinimum( 0 );
    paPedestModelID->setMaximum( setDlg->GetPedestrianKindNum() - 1 );
    paPedestModelID->setValue( 0 );
    paPedestModelID->setPrefix( "Pedestrian Model ID : " );

    paAppearSpeed = new QDoubleSpinBox();
    paAppearSpeed->setMinimum( 0.0 );
    paAppearSpeed->setMaximum( 100 );
    paAppearSpeed->setValue( 0 );
    paAppearSpeed->setSingleStep( 0.01 );
    paAppearSpeed->setPrefix("Speed:");
    paAppearSpeed->setSuffix("[m/s]");

    paAppearPick = new QPushButton();
    paAppearPick->setIcon( QIcon(":images/Flag_blue.png") );
    paAppearPick->setFixedSize( paAppearPick->sizeHint() );
    paAppearPick->setObjectName("PedestrianAppear");
    connect( paAppearPick, SIGNAL(clicked(bool)), this, SLOT(SetPickMode()) );

    paAppearAllowRepeat = new QCheckBox("Appear again");

    paAppearSetPathRoute = new QPushButton("Set");
    paAppearSetPathRoute->setIcon( QIcon(":images/setting.png") );
    paAppearSetPathRoute->setFixedSize( paAppearSetPathRoute->sizeHint() );
    paAppearSetPathRoute->setObjectName("PedestrianSetPathRoute");
    connect( paAppearSetPathRoute, SIGNAL(clicked(bool)), this, SLOT(SetPickMode()) );

    paAppearClearPathRoute = new QPushButton("Clear");
    paAppearClearPathRoute->setIcon( QIcon(":images/delete.png") );
    paAppearClearPathRoute->setFixedSize( paAppearClearPathRoute->sizeHint() );
    paAppearClearPathRoute->setObjectName("PedestRouteClear");
    connect( paAppearClearPathRoute, SIGNAL(clicked(bool)), this, SLOT(ClearRouteData()) );

    paAppearRouteInfo = new QLabel("Route Info:");

    QGridLayout *paAppearGrid = new QGridLayout();
    row = 0;
    paAppearGrid->addWidget( paAppearX, row , 0 );
    paAppearGrid->addWidget( paAppearY, row , 1 );
    paAppearGrid->addWidget( paAppearPick, row , 2 );
    row++;

    paAppearGrid->addWidget( paAppearNearLaneID, row , 0, 1, 2 );
    row++;

    paAppearGrid->addWidget( paAppearPsi, row , 0, 1, 2 );
    row++;

    paAppearGrid->addWidget( paAppearSpeed, row , 0, 1, 2 );
    row++;

    paAppearGrid->addWidget( paPedestModelID, row , 0, 1, 2 );
    row++;


    QHBoxLayout *paAppearPathRouteManipLayout = new QHBoxLayout();
    paAppearPathRouteManipLayout->addWidget( paAppearSetPathRoute );
    paAppearPathRouteManipLayout->addWidget( paAppearClearPathRoute );
    paAppearPathRouteManipLayout->addStretch( 1 );

    paAppearGrid->addLayout( paAppearPathRouteManipLayout, row , 0, 1, 2 );
    row++;

    paAppearGrid->addWidget( paAppearRouteInfo, row , 0, 1, 2 );
    row++;

    paAppearGrid->addWidget( paAppearAllowRepeat, row , 0, 1, 2 );
    row++;

    paAppearGrid->setRowStretch( row, 1 );

    paAppearWidget->setLayout( paAppearGrid );

    if( maxHeight < paAppearWidget->sizeHint().height() ){
        maxHeight = paAppearWidget->sizeHint().height();
    }
    if( maxWidth < paAppearWidget->sizeHint().width() ){
        maxWidth = paAppearWidget->sizeHint().width();
    }


    //--------
    paControlWidget = new QWidget();
    paControlWidget->hide();

    paCtrlSelControlMode = new QComboBox();
    ctrlMode.clear();
    ctrlMode<< "Agent Logic" << "Constant Speed" << "Speed Profile Control" << "Run Out into Street";
    paCtrlSelControlMode->addItems( ctrlMode );

    paCtrlTargetSpeed = new QDoubleSpinBox();
    paCtrlTargetSpeed->setMinimum( 0.0 );
    paCtrlTargetSpeed->setMaximum( 20.0 );
    paCtrlTargetSpeed->setValue( 1.0 );
    paCtrlTargetSpeed->setPrefix("Target Speed: ");
    paCtrlTargetSpeed->setSuffix("[m/s]");

    addRowPedestSpeedProfileTable = new QPushButton("Add");
    addRowPedestSpeedProfileTable->setIcon( QIcon(":images/Add.png") );
    addRowPedestSpeedProfileTable->setFixedSize( addRowPedestSpeedProfileTable->sizeHint() );
    addRowPedestSpeedProfileTable->setObjectName("PedestSpeedProfile");
    connect( addRowPedestSpeedProfileTable, SIGNAL(clicked()), this, SLOT(AddRowTable()) );

    delRowPedestSpeedProfileTable = new QPushButton("Del");
    delRowPedestSpeedProfileTable->setIcon( QIcon(":images/delete.png") );
    delRowPedestSpeedProfileTable->setFixedSize( delRowPedestSpeedProfileTable->sizeHint() );
    delRowPedestSpeedProfileTable->setObjectName("PedestSpeedProfile");
    connect( delRowPedestSpeedProfileTable, SIGNAL(clicked()), this, SLOT(DelRowTable()) );

    showPedestSpeedProfileTableAsGraph = new QPushButton("Graph");
    showPedestSpeedProfileTableAsGraph->setIcon( QIcon(":images/Chart_bar.png") );
    showPedestSpeedProfileTableAsGraph->setFixedSize( showPedestSpeedProfileTableAsGraph->sizeHint() );
    connect( showPedestSpeedProfileTableAsGraph, SIGNAL(clicked()), pedestSpeedProfileGraph, SLOT(show()) );

    paCtrlSpeedProfile = new QTableWidget();
    paCtrlSpeedProfile->setColumnCount(2);

    tableLabels.clear();
    tableLabels << "Time[s]";
    tableLabels << "Speed[m/s]";

    paCtrlSpeedProfile->setHorizontalHeaderLabels( tableLabels );
    paCtrlSpeedProfile->setColumnWidth(0,TABLE_TIME_WIDTH);
    paCtrlSpeedProfile->setColumnWidth(1,TABLE_VAL_WIDTH);

    paCtrlSpeedProfile->setMinimumWidth( TABLE_MIN_WIDTH );

    QVBoxLayout *paControlGrid = new QVBoxLayout();

    paCtrlRunOutDirection = new QDoubleSpinBox();
    paCtrlRunOutDirection->setMinimum( -180.0 );
    paCtrlRunOutDirection->setMaximum( 180.0 );
    paCtrlRunOutDirection->setValue( 0.0 );
    paCtrlRunOutDirection->setPrefix("Run-Out Direction: ");
    paCtrlRunOutDirection->setSuffix("[deg]");

    paCtrlAllowRepeat = new QCheckBox("Apply again");


    paControlGrid->addWidget( paCtrlSelControlMode );
    paControlGrid->addSpacing( 10 );
    paControlGrid->addWidget( paCtrlTargetSpeed );
    paControlGrid->addSpacing( 10 );

    QHBoxLayout *paCtrlSpeedProfileBtnsLayout = new QHBoxLayout();
    paCtrlSpeedProfileBtnsLayout->addWidget( addRowPedestSpeedProfileTable );
    paCtrlSpeedProfileBtnsLayout->addWidget( delRowPedestSpeedProfileTable );
    paCtrlSpeedProfileBtnsLayout->addWidget( showPedestSpeedProfileTableAsGraph );
    paCtrlSpeedProfileBtnsLayout->addStretch( 1 );

    paControlGrid->addLayout( paCtrlSpeedProfileBtnsLayout );

    paControlGrid->addWidget( paCtrlSpeedProfile );
    paControlGrid->addSpacing( 10 );
    paControlGrid->addWidget( paCtrlRunOutDirection );
    paControlGrid->addSpacing( 10 );
    paControlGrid->addWidget( paCtrlAllowRepeat );

    paControlGrid->addStretch(1);

    paControlWidget->setLayout( paControlGrid );

    if( maxHeight < paControlWidget->sizeHint().height() ){
        maxHeight = paControlWidget->sizeHint().height();
    }
    if( maxWidth < paControlWidget->sizeHint().width() ){
        maxWidth = paControlWidget->sizeHint().width();
    }


    //--------
    paUDPWidget = new QWidget();
    paUDPWidget->hide();

    paUDPIPAddr = new QLineEdit();

    QString paIPrange = "(([ 0]+)|([ 0]*[0-9] *)|([0-9][0-9] )|([ 0][0-9][0-9])|(1[0-9][0-9])|([2][0-4][0-9])|(25[0-5]))";
    QRegExp paIPRegex ("^" + paIPrange
                     + "\\." + paIPrange
                     + "\\." + paIPrange
                     + "\\." + paIPrange + "$");
    QRegExpValidator *paIPValidator = new QRegExpValidator(paIPRegex, this);
    paUDPIPAddr->setValidator( paIPValidator );
    paUDPIPAddr->setInputMask("000.000.000.000");

    paUDPPortSB = new QSpinBox();
    paUDPPortSB->setMinimum( 0 );
    paUDPPortSB->setMaximum( 100000 );
    paUDPPortSB->setValue( 3000 );

    paUDPSendData = new QLineEdit();

    paRepeatSend = new QCheckBox("Repeat sedning the data");

    QGridLayout *paUDPGrid = new QGridLayout();
    paUDPGrid->addWidget( new QLabel("IP-Address"), 0 , 0 );
    paUDPGrid->addWidget( new QLabel("Port"), 1 , 0 );
    paUDPGrid->addWidget( new QLabel("IP-Address"), 0 , 0 );
    paUDPGrid->addWidget( paUDPIPAddr, 0 , 1 );
    paUDPGrid->addWidget( new QLabel("Port"), 1 , 0 );
    paUDPGrid->addWidget( paUDPPortSB, 1 , 1 );
    paUDPGrid->addWidget( new QLabel("Data"), 2 , 0 );
    paUDPGrid->addWidget( paUDPSendData, 2 , 1 );
    paUDPGrid->addWidget( paRepeatSend, 3 , 1 );

    paUDPGrid->setRowStretch( 4, 1 );

    paUDPWidget->setLayout( paUDPGrid );

    if( maxHeight < paUDPWidget->sizeHint().height() ){
        maxHeight = paUDPWidget->sizeHint().height();
    }
    if( maxWidth < paUDPWidget->sizeHint().width() ){
        maxWidth = paUDPWidget->sizeHint().width();
    }

    //--------
    paDisappearWidget = new QWidget();
    paDisappearWidget->hide();

    paDisappearAppearSoon = new QCheckBox("Apply if appeared again");

    QGridLayout *paDisappearGrid = new QGridLayout();
    paDisappearGrid->addWidget( paDisappearAppearSoon, 0 , 0 );
    paDisappearGrid->setRowStretch( 1, 1 );

    paDisappearWidget->setLayout( paDisappearGrid );

    if( maxHeight < paDisappearWidget->sizeHint().height() ){
        maxHeight = paDisappearWidget->sizeHint().height();
    }
    if( maxWidth < paDisappearWidget->sizeHint().width() ){
        maxWidth = paDisappearWidget->sizeHint().width();
    }

    //--------
    pedestActDataGrid->addWidget( paAppearWidget,    1 , 1 );
    pedestActDataGrid->addWidget( paControlWidget,   2 , 1 );
    pedestActDataGrid->addWidget( paUDPWidget,       3 , 1 );
    pedestActDataGrid->addWidget( paDisappearWidget, 4 , 1 );
    pedestActDataGrid->setColumnStretch(2,1);
    pedestActDataGrid->setRowStretch( 5, 1 );

    if( maxWidth < pedestActDataGrid->sizeHint().width() ){
        maxWidth = pedestActDataGrid->sizeHint().width();
    }


    //----- Action Layout
    sysActDataWidget = new QWidget();
    sysActDataWidget->setLayout( sysActDataGrid );

    vehicleActDataWidget = new QWidget();
    vehicleActDataWidget->setLayout( vehicleActDataGrid );
    vehicleActDataWidget->hide();

    pedestActDataWidget = new QWidget();
    pedestActDataWidget->setLayout( pedestActDataGrid );
    pedestActDataWidget->hide();


    QVBoxLayout *actDataTopLayout = new QVBoxLayout();
    actDataTopLayout->addWidget( sysActDataWidget );
    actDataTopLayout->addWidget( vehicleActDataWidget );
    actDataTopLayout->addWidget( pedestActDataWidget );

    actDataGB->setLayout( actDataTopLayout );

    actDataGB->setMinimumHeight( maxHeight );
    actDataGB->setMinimumWidth( maxWidth + QLabel("---Actions-----").sizeHint().width() );


    //----- Data Layout
    QHBoxLayout *GBLayout = new QHBoxLayout();
    GBLayout->addWidget( condDataGB );
    GBLayout->addWidget( actDataGB );

    applyBtn = new QPushButton("Apply");
    applyBtn->setIcon( QIcon(":images/accept.png") );
    applyBtn->setFixedSize( applyBtn->sizeHint() );
    connect( applyBtn, SIGNAL(clicked(bool)),this,SLOT(ApplyDataClicked()));


    QHBoxLayout *applyBtnLayout = new QHBoxLayout();
    applyBtnLayout->addStretch( 1 );
    applyBtnLayout->addWidget( applyBtn );
    applyBtnLayout->addStretch( 1 );

    QVBoxLayout *dataLayout = new QVBoxLayout();
    dataLayout->addLayout( GBLayout );
    dataLayout->addLayout( applyBtnLayout );


    //----- Tab Layout
    QHBoxLayout *middleLayout = new QHBoxLayout();
    middleLayout->addWidget( tabW );
    middleLayout->addLayout( dataLayout );


    //----- Main Layout
    QVBoxLayout *mainLay = new QVBoxLayout();
    mainLay->addLayout( toolbarLayout );
    mainLay->addLayout( btnLayout );
    mainLay->addLayout( middleLayout );
    mainLay->addStretch( 1 );

    setLayout( mainLay );
}


void ScenarioEditor::NewData()
{
    if( sSys.size() > 0 || sVehicle.size() > 0 || sPedest.size() > 0 ){
        QMessageBox msgBox;
        msgBox.setText("Clear Scenario Data.");
        msgBox.setInformativeText("Sure?");
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        if( ret == QMessageBox::Cancel ){
            return;
        }
    }

    ClearData();

    tabW->setCurrentIndex(0);
}


void ScenarioEditor::ClearData()
{
    tabW->setCurrentIndex(0);

    int sN = sysList->count();
    for(int i=sN-1;i>=0;i--){
        sysList->setCurrentRow( i );
        int sysItemID = sysList->currentItem()->text().remove("Item").trimmed().toInt();
        DelScenarioSystem( sysItemID );
    }

    tabW->setCurrentIndex(1);

    int vN = vehicleList->count();
    for(int i=vN-1;i>=0;i--){
        vehicleList->setCurrentRow( i );
        int vID = vehicleList->currentItem()->text().remove("Vehicle").trimmed().toInt();
        DelScenarioVehicle( vID );
    }
    vehicleSlot->clear();

    tabW->setCurrentIndex(2);

    int pN = pedestList->count();
    for(int i=pN-1;i>=0;i--){
        pedestList->setCurrentRow( i );
        int pID = pedestList->currentItem()->text().remove("Pedest").trimmed().toInt();
        DelScenarioPedestrian( pID );
    }
    pedestSlot->clear();
}


void ScenarioEditor::LoadData()
{
    if( road->roadDataFileName.isNull() || road->roadDataFileName.isEmpty() ){
        qDebug() << "Road data should be loaded before opening Scenario data.";
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose SEdit Scenario File"),
                                                    ".",
                                                    tr("SEdit Scenario file(*.ss.txt)"));

    if( fileName.isNull() == false ){
        qDebug() << "filename = " << fileName;
    }
    else{
        qDebug() << "Open action canceled.";
        return;
    }

    LoadDataWithFilename(fileName);

    UpdateScenarioSystemList();
    UpdateScenarioVehicleList();
    UpdateScenarioPedestList();

    tabW->setCurrentIndex( 0 );
}


void ScenarioEditor::SaveData()
{
    if( currentScenarioFile.isNull() == true || currentScenarioFile.isEmpty() == true ){
        SaveAsData();
        return;
    }

    QFile file(currentScenarioFile);
    if( file.open( QIODevice::WriteOnly | QIODevice::Text) == false ){
        qDebug() << "[ScenarioEditor::SaveData] cannot open file: "<< currentScenarioFile << " to write data.";
        currentScenarioFile = QString();
        return;
    }

    QTextStream out(&file);

    out << "#---------------------------------------\n";
    out << "#   SEdit Scenario Data File\n";
    out << "#---------------------------------------\n";
    out << "\n";

    out << "#--- Scenario System \n";
    out << "\n";

    for(int i=0;i<sSys.size();++i){

        out << "System Scenario; ID : " << sSys[i]->ID << "\n";
        out << "Trigger Info : "
            << sSys[i]->sItem.cond.combination << " , "
            << sSys[i]->sItem.cond.noTrigger << " , "
            << sSys[i]->sItem.cond.timeTrigger << " , "
            << sSys[i]->sItem.cond.positionTrigger << " , "
            << sSys[i]->sItem.cond.velocityTrigger << " , "
            << sSys[i]->sItem.cond.TTCTrigger << " , "
            << sSys[i]->sItem.cond.FETrigger << " , "
            << sSys[i]->sItem.cond.externalTrigger << " , "
            << sSys[i]->sItem.cond.funcKey << "\n";

        if( sSys[i]->sItem.cond.timeTrigger == true ){

            out << "Trigger Data : "
                << sSys[i]->sItem.cond.ttMin << " , "
                << sSys[i]->sItem.cond.ttSec << " , "
                << sSys[i]->sItem.cond.ttAbsOrRel << "\n";
        }

        if( sSys[i]->sItem.cond.positionTrigger == true ){

            out << "Trigger Data : "
                << sSys[i]->sItem.cond.ptX << " , "
                << sSys[i]->sItem.cond.ptY << " , "
                << sSys[i]->sItem.cond.ptPassAngle << " , "
                << sSys[i]->sItem.cond.ptTargetObjID << " , "
                << sSys[i]->sItem.cond.ptWidth << "\n";
        }

        if( sSys[i]->sItem.cond.velocityTrigger == true ){

            out << "Trigger Data : "
                << sSys[i]->sItem.cond.vtSpeed << " , "
                << sSys[i]->sItem.cond.vtLowOrHigh << " , "
                << sSys[i]->sItem.cond.vtTargetObjID << "\n";
        }

        if( sSys[i]->sItem.cond.TTCTrigger == true ){

            out << "Trigger Data : "
                << sSys[i]->sItem.cond.ttcVal << " , "
                << sSys[i]->sItem.cond.ttcCalTargetObjID << " , "
                << sSys[i]->sItem.cond.ttcCalType << " , "
                << sSys[i]->sItem.cond.ttcCalObjectID << " , "
                << sSys[i]->sItem.cond.ttcCalPosX << " , "
                << sSys[i]->sItem.cond.ttcCalPosY << "\n";
        }

        out << "Action Info : " << sSys[i]->sItem.act.actionType << "\n";

        if( sSys[i]->sItem.act.iParams.size() > 0 ){

            out << "Action i-Data : ";
            for(int j=0;j<sSys[i]->sItem.act.iParams.size();++j){
                out << sSys[i]->sItem.act.iParams[j];
                if( j < sSys[i]->sItem.act.iParams.size() - 1 ){
                    out << " , ";
                }
            }
            out << "\n";

        }

        if( sSys[i]->sItem.act.fParams.size() > 0 ){

            out << "Action f-Data : ";
            for(int j=0;j<sSys[i]->sItem.act.fParams.size();++j){
                out << sSys[i]->sItem.act.fParams[j];
                if( j < sSys[i]->sItem.act.fParams.size() - 1 ){
                    out << " , ";
                }
            }
            out << "\n";

        }

        if( sSys[i]->sItem.act.bParams.size() > 0 ){

            out << "Action b-Data : ";
            for(int j=0;j<sSys[i]->sItem.act.bParams.size();++j){
                out << sSys[i]->sItem.act.bParams[j];
                if( j < sSys[i]->sItem.act.bParams.size() - 1 ){
                    out << " , ";
                }
            }
            out << "\n";

        }

        out << "#---\n";
        out << "\n";
    }

    out << "\n";
    out << "#--- Scenario Vehicle \n";
    out << "\n";

    for(int i=0;i<sVehicle.size();++i){

        out << "Vehicle Scenario; ID : " << sVehicle[i]->ID << "\n";

        for(int j=0;j<sVehicle[i]->sItem.size();++j){

            out << "#--- slot " << j << "\n";

            out << "Trigger Info : "
                << sVehicle[i]->sItem[j]->cond.combination << " , "
                << sVehicle[i]->sItem[j]->cond.noTrigger << " , "
                << sVehicle[i]->sItem[j]->cond.timeTrigger << " , "
                << sVehicle[i]->sItem[j]->cond.positionTrigger << " , "
                << sVehicle[i]->sItem[j]->cond.velocityTrigger << " , "
                << sVehicle[i]->sItem[j]->cond.TTCTrigger << " , "
                << sVehicle[i]->sItem[j]->cond.FETrigger << " , "
                << sVehicle[i]->sItem[j]->cond.externalTrigger << " , "
                << sVehicle[i]->sItem[j]->cond.funcKey << "\n";

            if( sVehicle[i]->sItem[j]->cond.timeTrigger == true ){

                out << "Trigger Data : "
                    << sVehicle[i]->sItem[j]->cond.ttMin << " , "
                    << sVehicle[i]->sItem[j]->cond.ttSec << " , "
                    << sVehicle[i]->sItem[j]->cond.ttAbsOrRel << "\n";
            }

            if( sVehicle[i]->sItem[j]->cond.positionTrigger == true ){

                out << "Trigger Data : "
                    << sVehicle[i]->sItem[j]->cond.ptX << " , "
                    << sVehicle[i]->sItem[j]->cond.ptY << " , "
                    << sVehicle[i]->sItem[j]->cond.ptPassAngle << " , "
                    << sVehicle[i]->sItem[j]->cond.ptTargetObjID << " , "
                    << sVehicle[i]->sItem[j]->cond.ptWidth << "\n";
            }

            if( sVehicle[i]->sItem[j]->cond.velocityTrigger == true ){

                out << "Trigger Data : "
                    << sVehicle[i]->sItem[j]->cond.vtSpeed << " , "
                    << sVehicle[i]->sItem[j]->cond.vtLowOrHigh << " , "
                    << sVehicle[i]->sItem[j]->cond.vtTargetObjID << "\n";
            }

            if( sVehicle[i]->sItem[j]->cond.TTCTrigger == true ){

                out << "Trigger Data : "
                    << sVehicle[i]->sItem[j]->cond.ttcVal << " , "
                    << sVehicle[i]->sItem[j]->cond.ttcCalType << " , "
                    << sVehicle[i]->sItem[j]->cond.ttcCalTargetObjID << " , "
                    << sVehicle[i]->sItem[j]->cond.ttcCalPosX << " , "
                    << sVehicle[i]->sItem[j]->cond.ttcCalPosY << "\n";
            }

            out << "Action Info : " << sVehicle[i]->sItem[j]->act.actionType << "\n";

            if( sVehicle[i]->sItem[j]->act.iParams.size() > 0 ){

                out << "Action i-Data : ";
                for(int k=0;k<sVehicle[i]->sItem[j]->act.iParams.size();++k){
                    out << sVehicle[i]->sItem[j]->act.iParams[k];
                    if( k < sVehicle[i]->sItem[j]->act.iParams.size() - 1 ){
                        out << " , ";
                    }
                }
                out << "\n";
            }

            if( sVehicle[i]->sItem[j]->act.fParams.size() > 0 ){

                out << "Action f-Data : ";
                for(int k=0;k<sVehicle[i]->sItem[j]->act.fParams.size();++k){
                    out << sVehicle[i]->sItem[j]->act.fParams[k];
                    if( k < sVehicle[i]->sItem[j]->act.fParams.size() - 1 ){
                        out << " , ";
                    }
                }
                out << "\n";
            }

            if( sVehicle[i]->sItem[j]->act.bParams.size() > 0 ){

                out << "Action b-Data : ";
                for(int k=0;k<sVehicle[i]->sItem[j]->act.bParams.size();++k){
                    out << sVehicle[i]->sItem[j]->act.bParams[k];
                    if( k < sVehicle[i]->sItem[j]->act.bParams.size() - 1 ){
                        out << " , ";
                    }
                }
                out << "\n";
            }

        }
        out << "#---\n";
        out << "\n";
    }

    out << "\n";
    out << "#--- Scenario Pedestrian \n";
    out << "\n";

    for(int i=0;i<sPedest.size();++i){

        out << "Pedestrian Scenario; ID : " << sPedest[i]->ID << "\n";

        for(int j=0;j<sPedest[i]->sItem.size();++j){

            out << "Trigger Info : "
                << sPedest[i]->sItem[j]->cond.combination << " , "
                << sPedest[i]->sItem[j]->cond.noTrigger << " , "
                << sPedest[i]->sItem[j]->cond.timeTrigger << " , "
                << sPedest[i]->sItem[j]->cond.positionTrigger << " , "
                << sPedest[i]->sItem[j]->cond.velocityTrigger << " , "
                << sPedest[i]->sItem[j]->cond.TTCTrigger << " , "
                << sPedest[i]->sItem[j]->cond.FETrigger << " , "
                << sPedest[i]->sItem[j]->cond.externalTrigger << " , "
                << sPedest[i]->sItem[j]->cond.funcKey << "\n";

            if( sPedest[i]->sItem[j]->cond.timeTrigger == true ){

                out << "Trigger Data : "
                    << sPedest[i]->sItem[j]->cond.ttMin << " , "
                    << sPedest[i]->sItem[j]->cond.ttSec << " , "
                    << sPedest[i]->sItem[j]->cond.ttAbsOrRel << "\n";
            }

            if( sPedest[i]->sItem[j]->cond.positionTrigger == true ){

                out << "Trigger Data : "
                    << sPedest[i]->sItem[j]->cond.ptX << " , "
                    << sPedest[i]->sItem[j]->cond.ptY << " , "
                    << sPedest[i]->sItem[j]->cond.ptPassAngle << " , "
                    << sPedest[i]->sItem[j]->cond.ptTargetObjID << " , "
                    << sPedest[i]->sItem[j]->cond.ptWidth << "\n";
            }

            if( sPedest[i]->sItem[j]->cond.velocityTrigger == true ){

                out << "Trigger Data : "
                    << sPedest[i]->sItem[j]->cond.vtSpeed << " , "
                    << sPedest[i]->sItem[j]->cond.vtLowOrHigh << " , "
                    << sPedest[i]->sItem[j]->cond.vtTargetObjID << "\n";
            }

            if( sPedest[i]->sItem[j]->cond.TTCTrigger == true ){

                out << "Trigger Data : "
                    << sPedest[i]->sItem[j]->cond.ttcVal << " , "
                    << sPedest[i]->sItem[j]->cond.ttcCalType << " , "
                    << sPedest[i]->sItem[j]->cond.ttcCalTargetObjID << " , "
                    << sPedest[i]->sItem[j]->cond.ttcCalPosX << " , "
                    << sPedest[i]->sItem[j]->cond.ttcCalPosY << "\n";
            }

            out << "Action Info : " << sPedest[i]->sItem[j]->act.actionType << "\n";

            if( sPedest[i]->sItem[j]->act.iParams.size() > 0 ){

                out << "Action i-Data : ";
                for(int k=0;k<sPedest[i]->sItem[j]->act.iParams.size();++k){
                    out << sPedest[i]->sItem[j]->act.iParams[k];
                    if( k < sPedest[i]->sItem[j]->act.iParams.size() - 1 ){
                        out << " , ";
                    }
                }
                out << "\n";
            }

            if( sPedest[i]->sItem[j]->act.fParams.size() > 0 ){

                out << "Action f-Data : ";
                for(int k=0;k<sPedest[i]->sItem[j]->act.fParams.size();++k){
                    out << sPedest[i]->sItem[j]->act.fParams[k];
                    if( k < sPedest[i]->sItem[j]->act.fParams.size() - 1 ){
                        out << " , ";
                    }
                }
                out << "\n";
            }

            if( sPedest[i]->sItem[j]->act.bParams.size() > 0 ){

                out << "Action b-Data : ";
                for(int k=0;k<sPedest[i]->sItem[j]->act.bParams.size();++k){
                    out << sPedest[i]->sItem[j]->act.bParams[k];
                    if( k < sPedest[i]->sItem[j]->act.bParams.size() - 1 ){
                        out << " , ";
                    }
                }
                out << "\n";
            }
        }
        out << "#---\n";
        out << "\n";
    }


    file.close();

    qDebug() << "Scenario Data Saved to " << currentScenarioFile;
}


void ScenarioEditor::SaveAsData()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Sedit Scenario Data File"),
                                                    ".",
                                                    tr("SEdit Scenario file(*.ss.txt)"));

    if( fileName.isNull() == false ){

        if( fileName.endsWith(".ss.txt") == false ){
            fileName += QString(".ss.txt");
        }

        qDebug() << "filename = " << fileName;
    }
    else{
        qDebug() << "SaveAs Scenario Data action canceled.";
        return;
    }

    SaveDataWithFilename( fileName );
}


void ScenarioEditor::SaveDataWithFilename(QString filename)
{
    currentScenarioFile = filename;
    SaveData();
}

void ScenarioEditor::LoadDataWithFilename(QString fileName)
{
    QFile file(fileName);
    if( file.open( QIODevice::ReadOnly | QIODevice::Text) == false ){
        qDebug() << "[ScenarioEditor::LoadData] cannot open file: "<< fileName << " to read data.";
        return;
    }

    QTextStream in(&file);
    QString Line;

    Line = in.readLine();
    Line = in.readLine();
    if( Line.contains("SEdit Scenario Data File") == false ){
        qDebug() << "[ScenarioEditor::LoadData] the file: "<< fileName << " is not SEdit Scenario file.";
        return;
    }
    Line = in.readLine();

    currentScenarioFile = fileName;

    ClearData();

    int kind = -1;
    int triggerDataCount = 0;

    while( in.atEnd() == false ){

        Line = in.readLine();
        if( Line.startsWith("#") == true || Line.isEmpty() == true ){
            continue;
        }

        if( Line.contains("System Scenario; ID") == true ){

            struct ScenarioSystem *s = new ScenarioSystem;

            s->ID = Line.remove("System Scenario; ID :").trimmed().toInt();

            memset( &(s->sItem.cond), 0, sizeof( s->sItem.cond) );
            s->sItem.act.actionType = 0;

            sSys.append( s );

            kind = 0;
        }
        else if( Line.contains("Vehicle Scenario; ID") == true ){

            struct ScenarioVehicle *v = new ScenarioVehicle;

            v->ID = Line.remove("Vehicle Scenario; ID :").trimmed().toInt();
            sVehicle.append( v );

            kind = 1;
        }
        else if( Line.contains("Pedestrian Scenario; ID") == true ){

            struct ScenarioPedestrian *p = new ScenarioPedestrian;

            p->ID = Line.remove("Pedestrian Scenario; ID :").trimmed().toInt();
            sPedest.append( p );

            kind = 2;
        }
        else if( Line.contains("Trigger Info") == true ){

            QStringList divLine = Line.remove("Trigger Info :").split(",");

            struct ScenarioItemCondition *cond = NULL;
            if( kind == 0 ){
                cond = &(sSys.last()->sItem.cond);
            }
            else if( kind == 1 ){
                struct ScenarioItem *item = new struct ScenarioItem;

                memset( &(item->cond), 0, sizeof(item->cond) );
                item->act.actionType = 0;
                item->act.route = NULL;

                sVehicle.last()->sItem.append( item );
                cond = &(item->cond);
            }
            else if( kind == 2 ){
                struct ScenarioItem *item = new struct ScenarioItem;

                memset( &(item->cond), 0, sizeof(item->cond) );
                item->act.actionType = 0;
                item->act.route = NULL;

                sPedest.last()->sItem.append( item );
                cond = &(item->cond);
            }

            if( cond != NULL ){
                cond->combination     =   QString( divLine[0] ).trimmed().toInt();
                cond->noTrigger       = ( QString( divLine[1] ).trimmed().toInt() == 1 ? true : false );
                cond->timeTrigger     = ( QString( divLine[2] ).trimmed().toInt() == 1 ? true : false );
                cond->positionTrigger = ( QString( divLine[3] ).trimmed().toInt() == 1 ? true : false );
                cond->velocityTrigger = ( QString( divLine[4] ).trimmed().toInt() == 1 ? true : false );
                cond->TTCTrigger      = ( QString( divLine[5] ).trimmed().toInt() == 1 ? true : false );

                if( divLine.size() >= 9 ){
                   cond->FETrigger       = ( QString( divLine[6] ).trimmed().toInt() == 1 ? true : false );
                   cond->externalTrigger = ( QString( divLine[7] ).trimmed().toInt() == 1 ? true : false );
                   cond->funcKey         = QString( divLine[8] ).trimmed().toInt();
                }
            }
            triggerDataCount = 0;
        }
        else if( Line.contains("Trigger Data") == true ){

            QStringList divLine = Line.remove("Trigger Data :").split(",");

            struct ScenarioItemCondition *cond = NULL;
            if( kind == 0 ){
                cond = &(sSys.last()->sItem.cond);
            }
            else if( kind == 1 ){
                cond = &( sVehicle.last()->sItem.last()->cond );
            }
            else if( kind == 2 ){
                cond = &( sPedest.last()->sItem.last()->cond );
            }

            if( cond != NULL ){
                int count = 0;
                if( cond->timeTrigger == true ){

                    if( count == triggerDataCount ){
                        if( divLine.size() == 3 ){
                            cond->ttMin      = QString( divLine[0] ).trimmed().toInt();
                            cond->ttSec      = QString( divLine[1] ).trimmed().toFloat();
                            cond->ttAbsOrRel = QString( divLine[2] ).trimmed().toInt();
                        }
                    }

                    count++;
                }

                if( cond->positionTrigger == true ){

                    if( count == triggerDataCount ){
                        if( divLine.size() >= 4 ){
                            cond->ptX           = QString( divLine[0] ).trimmed().toFloat();
                            cond->ptY           = QString( divLine[1] ).trimmed().toFloat();
                            cond->ptPassAngle   = QString( divLine[2] ).trimmed().toFloat();
                            cond->ptTargetObjID = QString( divLine[3] ).trimmed().toInt();
                            cond->ptWidth       = 10.0;
                        }
                        if( divLine.size() >= 5 ){
                            cond->ptWidth = QString( divLine[4] ).trimmed().toInt();
                        }
                    }

                    count++;
                }

                if( cond->velocityTrigger == true ){

                    if( count == triggerDataCount ){
                        if( divLine.size() == 3 ){
                            cond->vtSpeed       = QString( divLine[0] ).trimmed().toFloat();
                            cond->vtLowOrHigh   = QString( divLine[1] ).trimmed().toInt();
                            cond->vtTargetObjID = QString( divLine[2] ).trimmed().toInt();
                        }
                    }

                    count++;
                }

                if( cond->TTCTrigger == true ){

                    if( count == triggerDataCount ){
                        if( divLine.size() == 6 ){
                            cond->ttcVal            = QString( divLine[0] ).trimmed().toFloat();
                            cond->ttcCalTargetObjID = QString( divLine[1] ).trimmed().toInt();
                            cond->ttcCalType        = QString( divLine[2] ).trimmed().toInt();
                            cond->ttcCalObjectID    = QString( divLine[3] ).trimmed().toInt();
                            cond->ttcCalPosX        = QString( divLine[4] ).trimmed().toFloat();
                            cond->ttcCalPosY        = QString( divLine[5] ).trimmed().toFloat();
                        }
                    }

                    count++;
                }
            }

            triggerDataCount++;
        }
        else if( Line.contains("Action Info") == true ){

            int actionType = Line.remove("Action Info :").trimmed().toInt();

            struct ScenarioItemBehavior *act = NULL;
            if( kind == 0 ){
                act = &(sSys.last()->sItem.act);
            }
            else if( kind == 1 ){
                act = &(sVehicle.last()->sItem.last()->act);
            }
            else if( kind == 2 ){
                act = &(sPedest.last()->sItem.last()->act);
            }

            if( act != NULL ){
                act->actionType = actionType;
            }
        }
        else if( Line.contains("Action i-Data") == true ){

            QStringList divLine = Line.remove("Action i-Data :").split(",");

            struct ScenarioItemBehavior *act = NULL;
            if( kind == 0 ){
                act = &(sSys.last()->sItem.act);
            }
            else if( kind == 1 ){
                act = &(sVehicle.last()->sItem.last()->act);
            }
            else if( kind == 2 ){
                act = &(sPedest.last()->sItem.last()->act);
            }

            if( act != NULL ){
                for(int i=0;i<divLine.size();++i){
                    act->iParams.append( QString(divLine[i]).trimmed().toInt() );
                }
            }

        }
        else if( Line.contains("Action f-Data") == true ){

            QStringList divLine = Line.remove("Action f-Data :").split(",");

            struct ScenarioItemBehavior *act = NULL;
            if( kind == 0 ){
                act = &(sSys.last()->sItem.act);
            }
            else if( kind == 1 ){
                act = &(sVehicle.last()->sItem.last()->act);
            }
            else if( kind == 2 ){
                act = &(sPedest.last()->sItem.last()->act);
            }

            if( act != NULL ){
                for(int i=0;i<divLine.size();++i){
                    act->fParams.append( QString(divLine[i]).trimmed().toFloat() );
                }
            }

        }
        else if( Line.contains("Action b-Data") == true ){

            QStringList divLine = Line.remove("Action b-Data :").split(",");

            struct ScenarioItemBehavior *act = NULL;
            if( kind == 0 ){
                act = &(sSys.last()->sItem.act);
            }
            else if( kind == 1 ){
                act = &(sVehicle.last()->sItem.last()->act);
            }
            else if( kind == 2 ){
                act = &(sPedest.last()->sItem.last()->act);
            }

            if( act != NULL ){
                for(int i=0;i<divLine.size();++i){
                    act->bParams.append( ( QString(divLine[i]).trimmed().toInt() == 1 ? true : false) );
                }
            }

        }
    }

    file.close();

    for(int i=0;i<sVehicle.size();++i){
        for(int j=0;j<sVehicle[i]->sItem.size();++j){
            if( sVehicle[i]->sItem[j]->act.actionType == 0 &&
                    sVehicle[i]->sItem[j]->act.iParams.size() >= 3 &&
                    sVehicle[i]->sItem[j]->act.fParams.size() > 0  ){

                if( sVehicle[i]->sItem[j]->act.iParams[2] == 0 ){
                    SetNodeRouteLaneLists( i, j );
                }
                else if( sVehicle[i]->sItem[j]->act.iParams[2] == 1 ){
                    SetPathRouteLaneShape( i, j );
                }
            }
        }
    }

    for(int i=0;i<sPedest.size();++i){
        for(int j=0;j<sPedest[i]->sItem.size();++j){
            if( sPedest[i]->sItem[j]->act.actionType == 0 &&
                    sPedest[i]->sItem[j]->act.iParams.size() >= 3 &&
                    sPedest[i]->sItem[j]->act.fParams.size() > 0  ){

                SetPedestRouteLaneShape( i, j );
            }
        }
    }
}


void ScenarioEditor::AddScenarioVehicle()
{
    qDebug() << "[ScenarioEditor::AddScenarioVehicle]";

    int ID = 10;
    for(int i=0;i<sVehicle.size();++i){
        if( ID <= sVehicle[i]->ID ){
            ID = sVehicle[i]->ID + 1;
        }
    }
    for(int i=0;i<sPedest.size();++i){
        if( ID <= sPedest[i]->ID ){
            ID = sPedest[i]->ID + 1;
        }
    }

    struct ScenarioVehicle *s = new struct ScenarioVehicle;
    s->ID = ID;

    sVehicle.append( s );

    tabW->setCurrentIndex( 1 );
    UpdateScenarioVehicleList();
}


void ScenarioEditor::AddScenarioPedestrian()
{
    qDebug() << "[ScenarioEditor::AddScenarioPedestrian]";

    int ID = 10;
    for(int i=0;i<sVehicle.size();++i){
        if( ID <= sVehicle[i]->ID ){
            ID = sVehicle[i]->ID + 1;
        }
    }
    for(int i=0;i<sPedest.size();++i){
        if( ID <= sPedest[i]->ID ){
            ID = sPedest[i]->ID + 1;
        }
    }

    struct ScenarioPedestrian *s = new struct ScenarioPedestrian;
    s->ID = ID;

    sPedest.append( s );

    tabW->setCurrentIndex( 2 );
    UpdateScenarioPedestList();
}


void ScenarioEditor::AddScenarioSystem()
{
    qDebug() << "[ScenarioEditor::AddScenarioSystem]";

    int ID = 0;
    for(int i=0;i<sSys.size();++i){
        if( ID <= sSys[i]->ID ){
            ID = sSys[i]->ID + 1;
        }
    }

    struct ScenarioSystem *s = new struct ScenarioSystem;
    s->ID = ID;

    s->sItem.cond.combination = 0;

    s->sItem.cond.noTrigger = false;

    s->sItem.cond.timeTrigger = false;
    s->sItem.cond.ttMin = 0;
    s->sItem.cond.ttSec = 0.0;
    s->sItem.cond.ttAbsOrRel = 0;

    s->sItem.cond.positionTrigger = false;
    s->sItem.cond.ptX = 0.0;
    s->sItem.cond.ptY = 0.0;
    s->sItem.cond.ptPassAngle = 0.0;
    s->sItem.cond.ptWidth = 10.0;
    s->sItem.cond.ptTargetObjID = -1;

    s->sItem.cond.velocityTrigger = false;
    s->sItem.cond.vtSpeed = 0.0;
    s->sItem.cond.vtLowOrHigh = 0;
    s->sItem.cond.vtTargetObjID = -1;

    s->sItem.cond.TTCTrigger = false;
    s->sItem.cond.ttcVal = 0.0;
    s->sItem.cond.ttcCalType = 0;
    s->sItem.cond.ttcCalPosX = 0.0;
    s->sItem.cond.ttcCalPosY = 0.0;
    s->sItem.cond.ttcCalTargetObjID = -1;
    s->sItem.cond.ttcCalObjectID = -1;

    s->sItem.cond.FETrigger = false;
    s->sItem.cond.externalTrigger = false;
    s->sItem.cond.funcKey = 0;

    s->sItem.act.actionType = 0;


    sSys.append( s );

    tabW->setCurrentIndex( 0 );
    UpdateScenarioSystemList();
}


void ScenarioEditor::DelScenarioClicked()
{
    if( confirmToDelete->isChecked() == true ){
        QMessageBox msgBox;
        msgBox.setText("Delete Scenario Data.");
        msgBox.setInformativeText("Sure?");
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        if( ret == QMessageBox::Cancel ){
            return;
        }
    }

    int tabIdx = tabW->currentIndex();
    if( tabIdx == 0 ){
        int cRow = sysList->currentRow();
        if( cRow < 0 ){
            return;
        }
        int sysItemID = sysList->currentItem()->text().remove("Item").trimmed().toInt();
        DelScenarioSystem( sysItemID );
    }
    else if( tabIdx == 1 ){
        int cRow = vehicleList->currentRow();
        if( cRow < 0 ){
            return;
        }
        int vehicleID = vehicleList->currentItem()->text().remove("Vehicle").trimmed().toInt();
        DelScenarioVehicle( vehicleID );
    }
    else if( tabIdx == 2 ){
        int cRow = pedestList->currentRow();
        if( cRow < 0 ){
            return;
        }
        int pedestID = pedestList->currentItem()->text().remove("Pedest").trimmed().toInt();
        DelScenarioPedestrian( pedestID );
    }
}

void ScenarioEditor::DelScenarioVehicle(int ID)
{
    for(int i=0;i<sVehicle.size();++i){
        if( sVehicle[i]->ID == ID ){

            for(int j=0;j<sVehicle[i]->sItem.size();++j){

                for(int k=0;k<sVehicle[i]->sItem[j]->act.laneShape.size();++k){

                    road->ClearLaneShape( sVehicle[i]->sItem[j]->act.laneShape[k] );

                    delete sVehicle[i]->sItem[j]->act.laneShape[k];
                }
                sVehicle[i]->sItem[j]->act.laneShape.clear();

                if( sVehicle[i]->sItem[j]->act.route != NULL ){
                    for(int k=0;k<sVehicle[i]->sItem[j]->act.route->nodeList.size();++k){
                        delete sVehicle[i]->sItem[j]->act.route->nodeList[k];
                    }
                    sVehicle[i]->sItem[j]->act.route->nodeList.clear();

                    for(int k=0;k<sVehicle[i]->sItem[j]->act.route->laneList.size();++k){
                        sVehicle[i]->sItem[j]->act.route->laneList[k].clear();
                    }
                    sVehicle[i]->sItem[j]->act.route->laneList.clear();

                    for(int k=0;k<sVehicle[i]->sItem[j]->act.route->routeLaneLists.size();++k){
                        for(int l=0;l<sVehicle[i]->sItem[j]->act.route->routeLaneLists[k]->laneList.size();++l){
                            sVehicle[i]->sItem[j]->act.route->routeLaneLists[k]->laneList[l].clear();
                        }
                        sVehicle[i]->sItem[j]->act.route->routeLaneLists[k]->laneList.clear();

                        delete sVehicle[i]->sItem[j]->act.route->routeLaneLists[k];
                    }
                    sVehicle[i]->sItem[j]->act.route->routeLaneLists.clear();
                }

                delete sVehicle[i]->sItem[j];
            }
            sVehicle[i]->sItem.clear();

            delete sVehicle[i];
            sVehicle.removeAt( i );

            break;
        }
    }

    UpdateScenarioVehicleList();
}


void ScenarioEditor::DelScenarioPedestrian(int ID)
{
    for(int i=0;i<sPedest.size();++i){
        if( sPedest[i]->ID == ID ){

            for(int j=0;j<sPedest[i]->sItem.size();++j){

                for(int k=0;k<sPedest[i]->sItem[j]->act.laneShape.size();++k){

                    road->ClearLaneShape( sPedest[i]->sItem[j]->act.laneShape[k] );

                    delete sPedest[i]->sItem[j]->act.laneShape[k];
                }
                sPedest[i]->sItem[j]->act.laneShape.clear();

                delete sPedest[i]->sItem[j];
            }
            sPedest[i]->sItem.clear();

            delete sPedest[i];
            sPedest.removeAt( i );

            break;
        }
    }

    UpdateScenarioPedestList();
}


void ScenarioEditor::DelScenarioSystem(int ID)
{
    for(int i=0;i<sSys.size();++i){
        if( sSys[i]->ID == ID ){

            delete sSys[i];
            sSys.removeAt( i );

            break;
        }
    }

    UpdateScenarioSystemList();
}


void ScenarioEditor::ChangeActDataContents(int wIdx)
{
    wIdx == 0 ? sysActDataWidget->show() : sysActDataWidget->hide();
    wIdx == 1 ? vehicleActDataWidget->show() : vehicleActDataWidget->hide();
    wIdx == 2 ? pedestActDataWidget->show() : pedestActDataWidget->hide();

    SetScenarioDataToGUI();

    this->update();

    emit UpdateGraphic();
}


void ScenarioEditor::ChangeVehicleActSeletion(int idx)
{
    idx == 0 ? vaAppearWidget->show() : vaAppearWidget->hide();
    idx == 1 ? vaControlWidget->show() : vaControlWidget->hide();
    idx == 2 ? vaUDPWidget->show() : vaUDPWidget->hide();
    idx == 3 ? vaDisappearWidget->show() : vaDisappearWidget->hide();

    this->update();
}


void ScenarioEditor::ChangeSystemActSeletion(int idx)
{
    idx == 0 ? saTeleportWidget->show() : saTeleportWidget->hide();
    idx == 1 ? saChangeTSWidget->show() : saChangeTSWidget->hide();
    idx == 2 ? saChangeSpeedInfoWidget->show() : saChangeSpeedInfoWidget->hide();
    idx == 3 ? saSendUDPWidget->show() : saSendUDPWidget->hide();

    this->update();
}


void ScenarioEditor::ChangePedestActSeletion(int idx)
{
    idx == 0 ? paAppearWidget->show() : paAppearWidget->hide();
    idx == 1 ? paControlWidget->show() : paControlWidget->hide();
    idx == 2 ? paUDPWidget->show() : paUDPWidget->hide();
    idx == 3 ? paDisappearWidget->show() : paDisappearWidget->hide();

    this->update();
}


void ScenarioEditor::UpdateScenarioSystemList()
{
    disconnect( sysList, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );

    sysList->clear();
    for(int i=0;i<sSys.size();++i){
        sysList->addItem(QString("Item %1").arg(sSys[i]->ID));
    }

    connect( sysList, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );

    emit UpdateGraphic();
}

void ScenarioEditor::UpdateScenarioVehicleList()
{
    disconnect( vehicleList, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );

    vehicleList->clear();
    for(int i=0;i<sVehicle.size();++i){
        vehicleList->addItem(QString("Vehicle %1").arg(sVehicle[i]->ID));
    }

    connect( vehicleList, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );

    emit UpdateGraphic();
}


void ScenarioEditor::UpdateScenarioVehicleSlotList()
{
    disconnect( vehicleSlot, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );
    if( vehicleList->currentRow() < 0 ){
        return;
    }

    int currentVID = vehicleList->currentItem()->text().remove("Vehicle").trimmed().toInt();

    vehicleSlot->clear();
    for(int i=0;i<sVehicle.size();++i){
        if( sVehicle[i]->ID != currentVID ){
            continue;
        }
        for(int j=0;j<sVehicle[i]->sItem.size();++j){
            vehicleSlot->addItem( QString("Slot %1").arg(j) );
        }
        break;
    }

    connect( vehicleSlot, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );

    emit UpdateGraphic();
}


void ScenarioEditor::UpdateScenarioPedestList()
{
    disconnect( pedestList, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );

    pedestList->clear();
    for(int i=0;i<sPedest.size();++i){
        pedestList->addItem(QString("Pedest %1").arg(sPedest[i]->ID));
    }

    connect( pedestList, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );

    emit UpdateGraphic();
}


void ScenarioEditor::UpdateScenarioPedestSlotList()
{
    disconnect( pedestSlot, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );
    if( pedestList->currentRow() < 0 ){
        return;
    }

    int currentPID = pedestList->currentItem()->text().remove("Pedest").trimmed().toInt();

    pedestSlot->clear();
    for(int i=0;i<sPedest.size();++i){
        if( sPedest[i]->ID != currentPID ){
            continue;
        }
        for(int j=0;j<sPedest[i]->sItem.size();++j){
            pedestSlot->addItem( QString("Slot %1").arg(j) );
        }
        break;
    }

    connect( pedestSlot, SIGNAL(currentRowChanged(int)), this, SLOT(SetScenarioDataToGUI()) );

    emit UpdateGraphic();
}


void ScenarioEditor::ApplyDataClicked()
{
    qDebug() << "[ScenarioEditor::ApplyDataClicked]";

    int tabIndex = tabW->currentIndex();
    if( tabIndex == 0 ){
        int dIdx = sysList->currentRow();
        if( dIdx < 0 ){
            QMessageBox msgBox;
            msgBox.setText("Information");
            msgBox.setInformativeText("No System Event is assigned. Data will not be set.");
            msgBox.exec();
            return;
        }

        int id = sysList->currentItem()->text().remove("Item").trimmed().toInt();

        int sIdx = -1;
        for(int i=0;i<sSys.size();++i){
            if( sSys[i]->ID == id ){
                sIdx = i;
                break;
            }
        }
        if( sIdx < 0 ){
            return;
        }

        sSys[sIdx]->sItem.cond.combination = AND_OR->currentIndex();

        sSys[sIdx]->sItem.cond.noTrigger = noTrigger->isChecked();

        sSys[sIdx]->sItem.cond.timeTrigger = timeTrigger->isChecked();
        if( sSys[sIdx]->sItem.cond.timeTrigger == true ){

            sSys[sIdx]->sItem.cond.ttMin = ttMinSB->value();
            sSys[sIdx]->sItem.cond.ttSec = ttSecSB->value();
            sSys[sIdx]->sItem.cond.ttAbsOrRel = 0;
            if( ttRelative->isChecked() == true ){
                sSys[sIdx]->sItem.cond.ttAbsOrRel = 1;
            }
        }
        else{

            sSys[sIdx]->sItem.cond.ttMin = 0;
            sSys[sIdx]->sItem.cond.ttSec = 0.0;
            sSys[sIdx]->sItem.cond.ttAbsOrRel = 0;
        }

        sSys[sIdx]->sItem.cond.positionTrigger = positionTrigger->isChecked();
        if( sSys[sIdx]->sItem.cond.positionTrigger == true ){

            sSys[sIdx]->sItem.cond.ptX = ptX->value();
            sSys[sIdx]->sItem.cond.ptY = ptY->value();
            sSys[sIdx]->sItem.cond.ptPassAngle = ptPsi->value();
            sSys[sIdx]->sItem.cond.ptWidth = ptWidth->value();
            sSys[sIdx]->sItem.cond.ptTargetObjID = ptTargetID->value();
        }
        else{

            sSys[sIdx]->sItem.cond.ptX = 0.0;
            sSys[sIdx]->sItem.cond.ptY = 0.0;
            sSys[sIdx]->sItem.cond.ptPassAngle = 0.0;
            sSys[sIdx]->sItem.cond.ptWidth = 10.0;
            sSys[sIdx]->sItem.cond.ptTargetObjID = -1;
        }

        sSys[sIdx]->sItem.cond.velocityTrigger = velocityTrigger->isChecked();
        if( sSys[sIdx]->sItem.cond.velocityTrigger == true ){

            sSys[sIdx]->sItem.cond.vtSpeed = vtSpeed->value();
            sSys[sIdx]->sItem.cond.vtLowOrHigh = 0;
            if( vtSlower->isChecked() == true ){
                sSys[sIdx]->sItem.cond.vtLowOrHigh = 1;
            }
            sSys[sIdx]->sItem.cond.vtTargetObjID = vtTargetID->value();
        }
        else{

            sSys[sIdx]->sItem.cond.vtSpeed = 0.0;
            sSys[sIdx]->sItem.cond.vtLowOrHigh = 0;
            sSys[sIdx]->sItem.cond.vtTargetObjID = -1;
        }

        sSys[sIdx]->sItem.cond.TTCTrigger = TTCTrigger->isChecked();
        if( sSys[sIdx]->sItem.cond.TTCTrigger == true ){

            sSys[sIdx]->sItem.cond.ttcVal = ttcVal->value();
            sSys[sIdx]->sItem.cond.ttcCalType = 0;
            if( ttcObject->isChecked() == true ){
                sSys[sIdx]->sItem.cond.ttcCalType = 1;
            }
            sSys[sIdx]->sItem.cond.ttcCalPosX = ttcX->value();
            sSys[sIdx]->sItem.cond.ttcCalPosY = ttcY->value();
            sSys[sIdx]->sItem.cond.ttcCalObjectID = ttcCalObjectID->value();
            sSys[sIdx]->sItem.cond.ttcCalTargetObjID = ttcTargetID->value();
        }
        else{

            sSys[sIdx]->sItem.cond.ttcVal = 0.0;
            sSys[sIdx]->sItem.cond.ttcCalType = 0;
            sSys[sIdx]->sItem.cond.ttcCalPosX = 0.0;
            sSys[sIdx]->sItem.cond.ttcCalPosY = 0.0;
            sSys[sIdx]->sItem.cond.ttcCalObjectID = -1;
            sSys[sIdx]->sItem.cond.ttcCalTargetObjID = -1;
        }

        sSys[sIdx]->sItem.cond.FETrigger = FETrigger->isChecked();

        sSys[sIdx]->sItem.cond.externalTrigger = externalTrigger->isChecked();
        sSys[sIdx]->sItem.cond.funcKey = etKeys->currentIndex();

        sSys[sIdx]->sItem.act.actionType = sysActSelCB->currentIndex();

        sSys[sIdx]->sItem.act.fParams.clear();
        sSys[sIdx]->sItem.act.iParams.clear();
        sSys[sIdx]->sItem.act.bParams.clear();

        if( sSys[sIdx]->sItem.act.actionType == 0 ){ // Teleport

            sSys[sIdx]->sItem.act.fParams.append( saMoveToX->value() );
            sSys[sIdx]->sItem.act.fParams.append( saMoveToY->value() );
            sSys[sIdx]->sItem.act.fParams.append( saMoveToPsi->value() );

            sSys[sIdx]->sItem.act.iParams.append( saMoveToTargetID->value() );
            sSys[sIdx]->sItem.act.iParams.append( saMoveToNearLaneID->value() );

            sSys[sIdx]->sItem.act.bParams.append( saMoveToWithSurroundingVehicles->isChecked() );
            sSys[sIdx]->sItem.act.bParams.append( saMoveToOnlyOnce->isChecked() );
            sSys[sIdx]->sItem.act.bParams.append( saMoveToClearLateralOffset->isChecked() );
        }
        else if( sSys[sIdx]->sItem.act.actionType == 1 ){ // Change Traffic Signal

            sSys[sIdx]->sItem.act.iParams.append( saChangeTSTargetTSID->value() );
            sSys[sIdx]->sItem.act.iParams.append( saChangeTSChangeToIndex->value() );

            sSys[sIdx]->sItem.act.bParams.append( saChangeTSSystemDown->isChecked() );
            sSys[sIdx]->sItem.act.bParams.append( saChangeTSApplyAll->isChecked() );
        }
        else if( sSys[sIdx]->sItem.act.actionType == 2 ){ // Change Speed Info

            sSys[sIdx]->sItem.act.fParams.append( saChangeSpeedInfoSpeedVal->value() );

            QStringList lanesList = saChangeSpeedInfoTargetLanes->text().split(",");
            for(int i=0;i<lanesList.size();++i){
                sSys[sIdx]->sItem.act.iParams.append( QString(lanesList[i]).trimmed().toInt() );
            }

            sSys[sIdx]->sItem.act.bParams.append( saChangeSpeedInfoSpeedLimit->isChecked() );
            sSys[sIdx]->sItem.act.bParams.append( saChangeSpeedInfoActualSpeed->isChecked() );
        }
        else if( sSys[sIdx]->sItem.act.actionType == 3 ){ // Send UDP Data

            QStringList ipAddrNums = saUDPIPAddr->text().split(".");
            for(int i=0;i<ipAddrNums.size();++i){
                sSys[sIdx]->sItem.act.iParams.append( QString(ipAddrNums[i]).trimmed().toInt() );
            }
            sSys[sIdx]->sItem.act.iParams.append( saUDPPortSB->value() );

            QStringList sendDataVals = saUDPSendData->text().split(",");
            for(int i=0;i<sendDataVals.size();++i){
                sSys[sIdx]->sItem.act.fParams.append( QString(sendDataVals[i]).trimmed().toFloat() );
            }

            sSys[sIdx]->sItem.act.bParams.append( saRepeatSend->isChecked() );
        }

    }
    else if( tabIndex == 1 ){
        int dIdx = vehicleList->currentRow();
        if( dIdx < 0 ){
            QMessageBox msgBox;
            msgBox.setText("Information");
            msgBox.setInformativeText("Vehicle Object is not selected. Data will not be set.");
            msgBox.exec();
            return;
        }
        int rIdx = vehicleSlot->currentRow();
        if( rIdx < 0 ){
            QMessageBox msgBox;
            msgBox.setText("Information");
            msgBox.setInformativeText("Vehicle Slot is not assigned. Data will not be set.");
            msgBox.exec();
            return;
        }

        int id = vehicleList->currentItem()->text().remove("Vehicle").trimmed().toInt();

        int sIdx = -1;
        for(int i=0;i<sVehicle.size();++i){
            if( sVehicle[i]->ID == id ){
                sIdx = i;
                break;
            }
        }
        if( sIdx < 0 ){
            return;
        }

        sVehicle[sIdx]->sItem[rIdx]->cond.combination = AND_OR->currentIndex();

        sVehicle[sIdx]->sItem[rIdx]->cond.noTrigger = noTrigger->isChecked();

        sVehicle[sIdx]->sItem[rIdx]->cond.timeTrigger = timeTrigger->isChecked();
        if( sVehicle[sIdx]->sItem[rIdx]->cond.timeTrigger == true ){

            sVehicle[sIdx]->sItem[rIdx]->cond.ttMin = ttMinSB->value();
            sVehicle[sIdx]->sItem[rIdx]->cond.ttSec = ttSecSB->value();
            sVehicle[sIdx]->sItem[rIdx]->cond.ttAbsOrRel = 0;
            if( ttRelative->isChecked() == true ){
                sVehicle[sIdx]->sItem[rIdx]->cond.ttAbsOrRel = 1;
            }
        }
        else{

            sVehicle[sIdx]->sItem[rIdx]->cond.ttMin = 0;
            sVehicle[sIdx]->sItem[rIdx]->cond.ttSec = 0.0;
            sVehicle[sIdx]->sItem[rIdx]->cond.ttAbsOrRel = 0;
        }

        sVehicle[sIdx]->sItem[rIdx]->cond.positionTrigger = positionTrigger->isChecked();
        if( sVehicle[sIdx]->sItem[rIdx]->cond.positionTrigger == true ){

            sVehicle[sIdx]->sItem[rIdx]->cond.ptX = ptX->value();
            sVehicle[sIdx]->sItem[rIdx]->cond.ptY = ptY->value();
            sVehicle[sIdx]->sItem[rIdx]->cond.ptPassAngle = ptPsi->value();
            sVehicle[sIdx]->sItem[rIdx]->cond.ptWidth = ptWidth->value();
            sVehicle[sIdx]->sItem[rIdx]->cond.ptTargetObjID = ptTargetID->value();
        }
        else{

            sVehicle[sIdx]->sItem[rIdx]->cond.ptX = 0.0;
            sVehicle[sIdx]->sItem[rIdx]->cond.ptY = 0.0;
            sVehicle[sIdx]->sItem[rIdx]->cond.ptPassAngle = 0.0;
            sVehicle[sIdx]->sItem[rIdx]->cond.ptWidth = 10.0;
            sVehicle[sIdx]->sItem[rIdx]->cond.ptTargetObjID = -1;
        }

        sVehicle[sIdx]->sItem[rIdx]->cond.velocityTrigger = velocityTrigger->isChecked();
        if( sVehicle[sIdx]->sItem[rIdx]->cond.velocityTrigger == true ){

            sVehicle[sIdx]->sItem[rIdx]->cond.vtSpeed = vtSpeed->value();
            sVehicle[sIdx]->sItem[rIdx]->cond.vtLowOrHigh = 0;
            if( vtSlower->isChecked() == true ){
                sVehicle[sIdx]->sItem[rIdx]->cond.vtLowOrHigh = 1;
            }
            sVehicle[sIdx]->sItem[rIdx]->cond.vtTargetObjID = vtTargetID->value();
        }
        else{

            sVehicle[sIdx]->sItem[rIdx]->cond.vtSpeed = 0.0;
            sVehicle[sIdx]->sItem[rIdx]->cond.vtLowOrHigh = 0;
            sVehicle[sIdx]->sItem[rIdx]->cond.vtTargetObjID = -1;
        }

        sVehicle[sIdx]->sItem[rIdx]->cond.TTCTrigger = TTCTrigger->isChecked();
        if( sVehicle[sIdx]->sItem[rIdx]->cond.TTCTrigger == true ){

            sVehicle[sIdx]->sItem[rIdx]->cond.ttcVal = ttcVal->value();
            sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalType = 0;
            if( ttcObject->isChecked() == true ){
                sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalType = 1;
            }
            sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalPosX = ttcX->value();
            sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalPosY = ttcY->value();
            sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalObjectID = ttcCalObjectID->value();
            sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalTargetObjID = ttcTargetID->value();
        }
        else{

            sVehicle[sIdx]->sItem[rIdx]->cond.ttcVal = 0.0;
            sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalType = 0;
            sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalPosX = 0.0;
            sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalPosY = 0.0;
            sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalObjectID = -1;
            sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalTargetObjID = -1;
        }

        sVehicle[sIdx]->sItem[rIdx]->cond.FETrigger = FETrigger->isChecked();

        sVehicle[sIdx]->sItem[rIdx]->cond.externalTrigger = externalTrigger->isChecked();
        sVehicle[sIdx]->sItem[rIdx]->cond.funcKey = etKeys->currentIndex();

        sVehicle[sIdx]->sItem[rIdx]->act.actionType = vehicleActSelCB->currentIndex();

        sVehicle[sIdx]->sItem[rIdx]->act.fParams.clear();
        sVehicle[sIdx]->sItem[rIdx]->act.iParams.clear();
        sVehicle[sIdx]->sItem[rIdx]->act.bParams.clear();

        if( sVehicle[sIdx]->sItem[rIdx]->act.actionType == 0 ){ // Appear

            sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( vaAppearX->value() );
            sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( vaAppearY->value() );
            sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( vaAppearPsi->value() );
            sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( vaAppearSpeed->value() );

            sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( vaVehicleModelID->value() );
            sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( vaAppearNearLaneID->value() );

            if( vaAppearNodeRoute->isChecked() == true ){
                sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( 0 );

                // iParams[3]: size of node list
                // iParams[4]: InDir_1, iParams[5]: Node_1, iParams[6]: OutDir_1
                // iParams[7]: InDir_2, iParams[8]: Node_2, iParams[9]: OutDir_2

                // parse route info
                QStringList divStr = vaAppearRouteInfo->text().split("\n");
                if( divStr.size() > 1 ){

                    int nPoint = 0;
                    for(int i=1;i<divStr.size()-1;++i){
                        nPoint++;
                    }
                    sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( nPoint );

                    for(int i=1;i<divStr.size()-1;++i){
                        QStringList divStr2 = QString(divStr[i]).split(",");
                        for(int j=0;j<divStr2.size();++j){
                            QStringList divStr3 = QString(divStr2[j]).split("=");
                            int val = QString( divStr3[1] ).trimmed().toInt();
                            sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( val );
                        }
                    }
                }
                else{
                    sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( 0 );
                }

                SetNodeRouteLaneLists( sIdx, rIdx );
            }
            else{
                sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( 1 );

                // fParams[4]: X1, fParam[5]: Y1, fParam[6]: Z1, fParam[7]: Tht1
                // fParams[8]: X2, fParam[9]: Y2, fParam[10]: Z2, fParam[11]: Tht2
                // ...

                // parse route info
                QStringList divStr = vaAppearRouteInfo->text().split("\n");
                if( divStr.size() > 1 ){

                    int nPoint = 0;
                    for(int i=1;i<divStr.size()-1;++i){
                        QStringList divStr2 = QString(divStr[i]).split(",");
                        for(int j=0;j<divStr2.size();++j){
                            QStringList divStr3 = QString(divStr2[j]).split("=");
                            float val = QString( divStr3[1] ).trimmed().toFloat();
                            sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( val );
                        }
                        nPoint++;
                    }
                    sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( nPoint );
                }
                else{
                    sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( 0 );
                }

                SetPathRouteLaneShape( sIdx, rIdx );
            }

            sVehicle[sIdx]->sItem[rIdx]->act.bParams.append( vaAppearAllowRepeat->isChecked() );
        }
        else if( sVehicle[sIdx]->sItem[rIdx]->act.actionType == 1 ){ // Control

            sVehicle[sIdx]->sItem[rIdx]->act.bParams.append( vaCtrlChangeControlMode->isChecked() );
            sVehicle[sIdx]->sItem[rIdx]->act.bParams.append( vaCtrlAccelDecel->isChecked() );
            sVehicle[sIdx]->sItem[rIdx]->act.bParams.append( vaCtrlSteer->isChecked() );
            sVehicle[sIdx]->sItem[rIdx]->act.bParams.append( vaCtrlAllowRepeat->isChecked() );

            if( vaCtrlChangeControlMode->isChecked() == true ){

                sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( vaCtrlSelControlMode->currentIndex() );
                sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( vaCtrlTargetID->value() );

                sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( vaCtrlTargetSpeed->value() );
                sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( vaCtrlTargetHeadwayTime->value() );
                sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( vaCtrlTargetHeadwayDistance->value() );
                sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( vaCtrlStopX->value() );
                sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( vaCtrlStopY->value() );

                int nRow = vaCtrlSpeedProfile->rowCount();
                sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( nRow );

                for(int i=0;i<nRow;++i){

                    float t = vaCtrlSpeedProfile->item(i,0)->text().toFloat();
                    float v = vaCtrlSpeedProfile->item(i,1)->text().toFloat();  // [km/h]

                    sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( t );
                    sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( v );
                }
            }

            if( vaCtrlAccelDecel->isChecked() == true ){

                int nRow = vaCtrlAccelDecelTable->rowCount();

                sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( nRow );

                for(int i=0;i<nRow;++i){

                    float t = vaCtrlAccelDecelTable->item(i,0)->text().toFloat();
                    float a = vaCtrlAccelDecelTable->item(i,1)->text().toFloat();

                    sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( t );
                    sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( a );
                }
            }

            if( vaCtrlSteer->isChecked() == true ){

                int nRow = vaCtrlSteerTable->rowCount();

                sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( nRow );

                for(int i=0;i<nRow;++i){

                    float t = vaCtrlSteerTable->item(i,0)->text().toFloat();
                    float a = vaCtrlSteerTable->item(i,1)->text().toFloat();

                    sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( t );
                    sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( a );
                }
            }

            SetDataToGraph(0);
            SetDataToGraph(1);
            SetDataToGraph(2);
            SetDataToGraph(3);
        }
        else if( sVehicle[sIdx]->sItem[rIdx]->act.actionType == 2 ){ // UDP

            QStringList ipAddrNums = vaUDPIPAddr->text().split(".");
            for(int i=0;i<ipAddrNums.size();++i){
                sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( QString(ipAddrNums[i]).trimmed().toInt() );
            }
            sVehicle[sIdx]->sItem[rIdx]->act.iParams.append( vaUDPPortSB->value() );

            QStringList sendDataVals = vaUDPSendData->text().split(",");
            for(int i=0;i<sendDataVals.size();++i){
                sVehicle[sIdx]->sItem[rIdx]->act.fParams.append( QString(sendDataVals[i]).trimmed().toFloat() );
            }

            sVehicle[sIdx]->sItem[rIdx]->act.bParams.append( vaRepeatSend->isChecked() );

        }
        else if( sVehicle[sIdx]->sItem[rIdx]->act.actionType == 3 ){ // Disappear

            sVehicle[sIdx]->sItem[rIdx]->act.bParams.append( vaDisappearAppearSoon->isChecked() );

        }

    }
    else if( tabIndex == 2 ){
        int dIdx = pedestList->currentRow();
        if( dIdx < 0 ){
            QMessageBox msgBox;
            msgBox.setText("Information");
            msgBox.setInformativeText("Pedestrian Object is not selected. Data will not be set.");
            msgBox.exec();
            return;
        }

        int rIdx = pedestSlot->currentRow();
        if( rIdx < 0 ){
            QMessageBox msgBox;
            msgBox.setText("Information");
            msgBox.setInformativeText("Pedestrian Slot is not assigned. Data will not be set.");
            msgBox.exec();
            return;
        }

        int id = pedestList->currentItem()->text().remove("Pedest").trimmed().toInt();

        int sIdx = -1;
        for(int i=0;i<sPedest.size();++i){
            if( sPedest[i]->ID == id ){
                sIdx = i;
                break;
            }
        }
        if( sIdx < 0 ){
            return;
        }

        sPedest[sIdx]->sItem[rIdx]->cond.combination = AND_OR->currentIndex();

        sPedest[sIdx]->sItem[rIdx]->cond.noTrigger = noTrigger->isChecked();

        sPedest[sIdx]->sItem[rIdx]->cond.timeTrigger = timeTrigger->isChecked();
        if( sPedest[sIdx]->sItem[rIdx]->cond.timeTrigger == true ){

            sPedest[sIdx]->sItem[rIdx]->cond.ttMin = ttMinSB->value();
            sPedest[sIdx]->sItem[rIdx]->cond.ttSec = ttSecSB->value();
            sPedest[sIdx]->sItem[rIdx]->cond.ttAbsOrRel = 0;
            if( ttRelative->isChecked() == true ){
                sPedest[sIdx]->sItem[rIdx]->cond.ttAbsOrRel = 1;
            }
        }
        else{

            sPedest[sIdx]->sItem[rIdx]->cond.ttMin = 0;
            sPedest[sIdx]->sItem[rIdx]->cond.ttSec = 0.0;
            sPedest[sIdx]->sItem[rIdx]->cond.ttAbsOrRel = 0;
        }

        sPedest[sIdx]->sItem[rIdx]->cond.positionTrigger = positionTrigger->isChecked();
        if( sPedest[sIdx]->sItem[rIdx]->cond.positionTrigger == true ){

            sPedest[sIdx]->sItem[rIdx]->cond.ptX = ptX->value();
            sPedest[sIdx]->sItem[rIdx]->cond.ptY = ptY->value();
            sPedest[sIdx]->sItem[rIdx]->cond.ptPassAngle = ptPsi->value();
            sPedest[sIdx]->sItem[rIdx]->cond.ptWidth = ptWidth->value();
            sPedest[sIdx]->sItem[rIdx]->cond.ptTargetObjID = ptTargetID->value();
        }
        else{

            sPedest[sIdx]->sItem[rIdx]->cond.ptX = 0.0;
            sPedest[sIdx]->sItem[rIdx]->cond.ptY = 0.0;
            sPedest[sIdx]->sItem[rIdx]->cond.ptPassAngle = 0.0;
            sPedest[sIdx]->sItem[rIdx]->cond.ptWidth = 0.0;
            sPedest[sIdx]->sItem[rIdx]->cond.ptTargetObjID = -1;
        }

        sPedest[sIdx]->sItem[rIdx]->cond.velocityTrigger = velocityTrigger->isChecked();
        if( sPedest[sIdx]->sItem[rIdx]->cond.velocityTrigger == true ){

            sPedest[sIdx]->sItem[rIdx]->cond.vtSpeed = vtSpeed->value();
            sPedest[sIdx]->sItem[rIdx]->cond.vtLowOrHigh = 0;
            if( vtSlower->isChecked() == true ){
                sPedest[sIdx]->sItem[rIdx]->cond.vtLowOrHigh = 1;
            }
            sPedest[sIdx]->sItem[rIdx]->cond.vtTargetObjID = vtTargetID->value();
        }
        else{

            sPedest[sIdx]->sItem[rIdx]->cond.vtSpeed = 0.0;
            sPedest[sIdx]->sItem[rIdx]->cond.vtLowOrHigh = 0;
            sPedest[sIdx]->sItem[rIdx]->cond.vtTargetObjID = -1;
        }

        sPedest[sIdx]->sItem[rIdx]->cond.TTCTrigger = TTCTrigger->isChecked();
        if( sPedest[sIdx]->sItem[rIdx]->cond.TTCTrigger == true ){

            sPedest[sIdx]->sItem[rIdx]->cond.ttcVal = ttcVal->value();
            sPedest[sIdx]->sItem[rIdx]->cond.ttcCalType = 0;
            if( ttcObject->isChecked() == true ){
                sPedest[sIdx]->sItem[rIdx]->cond.ttcCalType = 1;
            }
            sPedest[sIdx]->sItem[rIdx]->cond.ttcCalPosX = ttcX->value();
            sPedest[sIdx]->sItem[rIdx]->cond.ttcCalPosY = ttcY->value();
            sPedest[sIdx]->sItem[rIdx]->cond.ttcCalObjectID = ttcCalObjectID->value();
            sPedest[sIdx]->sItem[rIdx]->cond.ttcCalTargetObjID = ttcTargetID->value();
        }
        else{

            sPedest[sIdx]->sItem[rIdx]->cond.ttcVal = 0.0;
            sPedest[sIdx]->sItem[rIdx]->cond.ttcCalType = 0;
            sPedest[sIdx]->sItem[rIdx]->cond.ttcCalPosX = 0.0;
            sPedest[sIdx]->sItem[rIdx]->cond.ttcCalPosY = 0.0;
            sPedest[sIdx]->sItem[rIdx]->cond.ttcCalObjectID = -1;
            sPedest[sIdx]->sItem[rIdx]->cond.ttcCalTargetObjID = -1;
        }

        sPedest[sIdx]->sItem[rIdx]->cond.FETrigger = FETrigger->isChecked();

        sPedest[sIdx]->sItem[rIdx]->cond.externalTrigger = externalTrigger->isChecked();
        sPedest[sIdx]->sItem[rIdx]->cond.funcKey = etKeys->currentIndex();

        sPedest[sIdx]->sItem[rIdx]->act.actionType = pedestActSelCB->currentIndex();

        sPedest[sIdx]->sItem[rIdx]->act.fParams.clear();
        sPedest[sIdx]->sItem[rIdx]->act.iParams.clear();
        sPedest[sIdx]->sItem[rIdx]->act.bParams.clear();

        if( sPedest[sIdx]->sItem[rIdx]->act.actionType == 0 ){ // Appear

            sPedest[sIdx]->sItem[rIdx]->act.fParams.append( paAppearX->value() );
            sPedest[sIdx]->sItem[rIdx]->act.fParams.append( paAppearY->value() );
            sPedest[sIdx]->sItem[rIdx]->act.fParams.append( paAppearPsi->value() );
            sPedest[sIdx]->sItem[rIdx]->act.fParams.append( paAppearSpeed->value() );

            sPedest[sIdx]->sItem[rIdx]->act.iParams.append( paPedestModelID->value() );
            sPedest[sIdx]->sItem[rIdx]->act.iParams.append( paAppearNearLaneID->value() );

            //
            // parse route info

            // fParams[4]: X1, fParam[5]: Y1, fParam[6]: Z1, fParam[7]: Tht1
            // fParams[8]: X2, fParam[9]: Y2, fParam[10]: Z2, fParam[11]: Tht2
            // ...
            QStringList divStr = paAppearRouteInfo->text().split("\n");
            if( divStr.size() > 1 ){

                int nPoint = 0;
                for(int i=1;i<divStr.size()-1;++i){
                    QStringList divStr2 = QString(divStr[i]).split(",");
                    for(int j=0;j<divStr2.size();++j){
                        QStringList divStr3 = QString(divStr2[j]).split("=");
                        float val = QString( divStr3[1] ).trimmed().toFloat();
                        sPedest[sIdx]->sItem[rIdx]->act.fParams.append( val );
                    }
                    nPoint++;
                }
                sPedest[sIdx]->sItem[rIdx]->act.iParams.append( nPoint );
            }
            else{
                sPedest[sIdx]->sItem[rIdx]->act.iParams.append( 0 );
            }

            SetPedestRouteLaneShape( sIdx, rIdx );

            sPedest[sIdx]->sItem[rIdx]->act.bParams.append( paAppearAllowRepeat->isChecked() );

        }
        else if( sPedest[sIdx]->sItem[rIdx]->act.actionType == 1 ){ // Control

            sPedest[sIdx]->sItem[rIdx]->act.iParams.append( paCtrlSelControlMode->currentIndex() );

            sPedest[sIdx]->sItem[rIdx]->act.fParams.append( paCtrlTargetSpeed->value() );
            sPedest[sIdx]->sItem[rIdx]->act.fParams.append( paCtrlRunOutDirection->value() );

            int nRow = paCtrlSpeedProfile->rowCount();
            sPedest[sIdx]->sItem[rIdx]->act.iParams.append( nRow );

            for(int i=0;i<nRow;++i){

                float t = paCtrlSpeedProfile->item(i,0)->text().toFloat();
                float v = paCtrlSpeedProfile->item(i,1)->text().toFloat();  // [km/h]

                sPedest[sIdx]->sItem[rIdx]->act.fParams.append( t );
                sPedest[sIdx]->sItem[rIdx]->act.fParams.append( v );
            }

            sPedest[sIdx]->sItem[rIdx]->act.bParams.append( paCtrlAllowRepeat->isChecked() );
        }
        else if( sPedest[sIdx]->sItem[rIdx]->act.actionType == 2 ){ // UDP

            QStringList ipAddrNums = paUDPIPAddr->text().split(".");
            for(int i=0;i<ipAddrNums.size();++i){
                sPedest[sIdx]->sItem[rIdx]->act.iParams.append( QString(ipAddrNums[i]).trimmed().toInt() );
            }
            sPedest[sIdx]->sItem[rIdx]->act.iParams.append( paUDPPortSB->value() );

            QStringList sendDataVals = paUDPSendData->text().split(",");
            for(int i=0;i<sendDataVals.size();++i){
                sPedest[sIdx]->sItem[rIdx]->act.fParams.append( QString(sendDataVals[i]).trimmed().toFloat() );
            }

            sPedest[sIdx]->sItem[rIdx]->act.bParams.append( paRepeatSend->isChecked() );

        }
        else if( sPedest[sIdx]->sItem[rIdx]->act.actionType == 3 ){ // Disappear

            sPedest[sIdx]->sItem[rIdx]->act.bParams.append( paDisappearAppearSoon->isChecked() );

        }
    }

    emit UpdateGraphic();
}


void ScenarioEditor::SetScenarioDataToGUI()
{
    //qDebug() << "[ScenarioEditor::SetScenarioDataToGUI]";

    int tabIndex = tabW->currentIndex();
    if( tabIndex == 0 ){
        int dIdx = sysList->currentRow();
        if( dIdx < 0 ){
            return;
        }

        int id = sysList->currentItem()->text().remove("Item").trimmed().toInt();

        int sIdx = -1;
        for(int i=0;i<sSys.size();++i){
            if( sSys[i]->ID == id ){
                sIdx = i;
                break;
            }
        }
        if( sIdx < 0 ){
            return;
        }

        AND_OR->setCurrentIndex( sSys[sIdx]->sItem.cond.combination );

        noTrigger->setChecked( sSys[sIdx]->sItem.cond.noTrigger );

        timeTrigger->setChecked( sSys[sIdx]->sItem.cond.timeTrigger );
        ttMinSB->setValue( sSys[sIdx]->sItem.cond.ttMin );
        ttSecSB->setValue( sSys[sIdx]->sItem.cond.ttSec );
        if( sSys[sIdx]->sItem.cond.ttAbsOrRel == 0 ){
            ttAbsolute->setChecked( true );
        }
        else{
            ttRelative->setChecked( true );
        }

        positionTrigger->setChecked( sSys[sIdx]->sItem.cond.positionTrigger );
        ptX->setValue( sSys[sIdx]->sItem.cond.ptX );
        ptY->setValue( sSys[sIdx]->sItem.cond.ptY );
        ptPsi->setValue( sSys[sIdx]->sItem.cond.ptPassAngle );
        ptWidth->setValue( sSys[sIdx]->sItem.cond.ptWidth );
        ptTargetID->setValue( sSys[sIdx]->sItem.cond.ptTargetObjID );

        velocityTrigger->setChecked( sSys[sIdx]->sItem.cond.velocityTrigger );
        vtSpeed->setValue( sSys[sIdx]->sItem.cond.vtSpeed );
        if( sSys[sIdx]->sItem.cond.vtLowOrHigh == 0 ){
            vtHigher->setChecked( true );
        }
        else{
            vtSlower->setChecked( true );
        }
        vtTargetID->setValue( sSys[sIdx]->sItem.cond.vtTargetObjID );

        TTCTrigger->setChecked( sSys[sIdx]->sItem.cond.TTCTrigger );
        ttcVal->setValue( sSys[sIdx]->sItem.cond.ttcVal );
        if( sSys[sIdx]->sItem.cond.ttcCalType == 0 ){
            ttcPoint->setChecked( true );
        }
        else{
            ttcObject->setChecked( true );
        }
        ttcX->setValue( sSys[sIdx]->sItem.cond.ttcCalPosX );
        ttcY->setValue( sSys[sIdx]->sItem.cond.ttcCalPosY );
        ttcCalObjectID->setValue( sSys[sIdx]->sItem.cond.ttcCalObjectID );
        ttcTargetID->setValue( sSys[sIdx]->sItem.cond.ttcCalTargetObjID );

        FETrigger->setChecked( sSys[sIdx]->sItem.cond.FETrigger );

        externalTrigger->setChecked( sSys[sIdx]->sItem.cond.externalTrigger );
        etKeys->setCurrentIndex( sSys[sIdx]->sItem.cond.funcKey );

        sysActSelCB->setCurrentIndex( sSys[sIdx]->sItem.act.actionType );

        if( sSys[sIdx]->sItem.act.actionType == 0 ){ // Teleport

            if( sSys[sIdx]->sItem.act.fParams.size() == 3 ){
                saMoveToX->setValue( sSys[sIdx]->sItem.act.fParams[0] );
                saMoveToY->setValue( sSys[sIdx]->sItem.act.fParams[1] );
                saMoveToPsi->setValue( sSys[sIdx]->sItem.act.fParams[2] );
            }
            else{
                saMoveToX->setValue( 0.0 );
                saMoveToY->setValue( 0.0 );
                saMoveToPsi->setValue( 0.0 );
            }

            if( sSys[sIdx]->sItem.act.iParams.size() >= 2 ){
                saMoveToTargetID->setValue( sSys[sIdx]->sItem.act.iParams[0] );
                saMoveToNearLaneID->setValue( sSys[sIdx]->sItem.act.iParams[1] );
            }
            else{
                saMoveToTargetID->setValue( -1 );
                saMoveToNearLaneID->setValue( -1 );
            }

            if( sSys[sIdx]->sItem.act.bParams.size() == 3 ){
                saMoveToWithSurroundingVehicles->setChecked( sSys[sIdx]->sItem.act.bParams[0] );
                saMoveToOnlyOnce->setChecked( sSys[sIdx]->sItem.act.bParams[1] );
                saMoveToClearLateralOffset->setChecked( sSys[sIdx]->sItem.act.bParams[2] );
            }
            else{
                saMoveToWithSurroundingVehicles->setChecked( false );
                saMoveToOnlyOnce->setChecked( false );
                saMoveToClearLateralOffset->setChecked( false );
            }
        }
        else if( sSys[sIdx]->sItem.act.actionType == 1 ){ // Change Traffic Signal

            if( sSys[sIdx]->sItem.act.iParams.size() == 2 ){
                saChangeTSTargetTSID->setValue( sSys[sIdx]->sItem.act.iParams[0] );
                saChangeTSChangeToIndex->setValue( sSys[sIdx]->sItem.act.iParams[1] );
            }

            if( sSys[sIdx]->sItem.act.bParams.size() == 2 ){
                saChangeTSSystemDown->setChecked( sSys[sIdx]->sItem.act.bParams[0] );
                saChangeTSApplyAll->setChecked( sSys[sIdx]->sItem.act.bParams[1] );
            }

        }
        else if( sSys[sIdx]->sItem.act.actionType == 2 ){ // Change Speed Info

            if( sSys[sIdx]->sItem.act.fParams.size() == 1 ){
                saChangeSpeedInfoSpeedVal->setValue( sSys[sIdx]->sItem.act.fParams[0] );
            }
            if( sSys[sIdx]->sItem.act.iParams.size() > 0 ){
                QString setStr = QString();
                for(int i=0;i<sSys[sIdx]->sItem.act.iParams.size() ;++i){
                    setStr += QString("%1").arg( sSys[sIdx]->sItem.act.iParams[i] );
                    if( i < sSys[sIdx]->sItem.act.iParams.size() - 1 ){
                        setStr += QString(",");
                    }
                }
                saChangeSpeedInfoTargetLanes->setText( setStr );
            }
            else{
                saChangeSpeedInfoTargetLanes->clear();
            }
            if( sSys[sIdx]->sItem.act.bParams.size() == 2 ){
                saChangeSpeedInfoSpeedLimit->setChecked( sSys[sIdx]->sItem.act.bParams[0] );
                saChangeSpeedInfoActualSpeed->setChecked( sSys[sIdx]->sItem.act.bParams[1] );
            }
        }
        else if( sSys[sIdx]->sItem.act.actionType == 3 ){ // Send UDP Data

            if( sSys[sIdx]->sItem.act.iParams.size() == 5 ){
                QString setStr = QString();
                for(int i=0;i<4;++i){
                    setStr += QString("%1").arg( sSys[sIdx]->sItem.act.iParams[i] );
                    if( i < 3 ){
                        setStr += QString(".");
                    }
                }
                saUDPIPAddr->setText( setStr );
                saUDPPortSB->setValue( sSys[sIdx]->sItem.act.iParams[4] );
            }

            if( sSys[sIdx]->sItem.act.fParams.size() > 0 ){
                QString setStr = QString();
                for(int i=0;i<sSys[sIdx]->sItem.act.fParams.size() ;++i){
                    setStr += QString("%1").arg( sSys[sIdx]->sItem.act.fParams[i] );
                    if( i < sSys[sIdx]->sItem.act.fParams.size() - 1 ){
                        setStr += QString(",");
                    }
                }
                saUDPSendData->setText( setStr );
            }
            else{
                saUDPSendData->clear();
            }

            if( sSys[sIdx]->sItem.act.bParams.size() > 0 ){
                saRepeatSend->setChecked( sSys[sIdx]->sItem.act.bParams[0] );
            }

        }
    }
    else if( tabIndex == 1 ){
        int dIdx = vehicleList->currentRow();
        if( dIdx < 0 ){
            return;
        }

        int rIdx = vehicleSlot->currentRow();
        if( rIdx < 0 ){
            return;
        }

        int id = vehicleList->currentItem()->text().remove("Vehicle").trimmed().toInt();

        int sIdx = -1;
        for(int i=0;i<sVehicle.size();++i){
            if( sVehicle[i]->ID == id ){
                sIdx = i;
                break;
            }
        }
        if( sIdx < 0 ){
            return;
        }

        AND_OR->setCurrentIndex( sVehicle[sIdx]->sItem[rIdx]->cond.combination );

        noTrigger->setChecked( sVehicle[sIdx]->sItem[rIdx]->cond.noTrigger );

        timeTrigger->setChecked( sVehicle[sIdx]->sItem[rIdx]->cond.timeTrigger );
        ttMinSB->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ttMin );
        ttSecSB->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ttSec );
        if( sVehicle[sIdx]->sItem[rIdx]->cond.ttAbsOrRel == 0 ){
            ttAbsolute->setChecked( true );
        }
        else{
            ttRelative->setChecked( true );
        }

        positionTrigger->setChecked( sVehicle[sIdx]->sItem[rIdx]->cond.positionTrigger );
        ptX->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ptX );
        ptY->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ptY );
        ptPsi->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ptPassAngle );
        ptWidth->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ptWidth );
        ptTargetID->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ptTargetObjID );

        velocityTrigger->setChecked( sVehicle[sIdx]->sItem[rIdx]->cond.velocityTrigger );
        vtSpeed->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.vtSpeed );
        if( sVehicle[sIdx]->sItem[rIdx]->cond.vtLowOrHigh == 0 ){
            vtHigher->setChecked( true );
        }
        else{
            vtSlower->setChecked( true );
        }
        vtTargetID->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.vtTargetObjID );

        TTCTrigger->setChecked( sVehicle[sIdx]->sItem[rIdx]->cond.TTCTrigger );
        ttcVal->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ttcVal );
        if( sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalType == 0 ){
            ttcPoint->setChecked( true );
        }
        else{
            ttcObject->setChecked( true );
        }
        ttcX->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalPosX );
        ttcY->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalPosY );
        ttcCalObjectID->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalObjectID );
        ttcTargetID->setValue( sVehicle[sIdx]->sItem[rIdx]->cond.ttcCalTargetObjID );

        FETrigger->setChecked( sVehicle[sIdx]->sItem[rIdx]->cond.FETrigger );

        externalTrigger->setChecked( sVehicle[sIdx]->sItem[rIdx]->cond.externalTrigger );
        etKeys->setCurrentIndex( sVehicle[sIdx]->sItem[rIdx]->cond.funcKey );

        vehicleActSelCB->setCurrentIndex( sVehicle[sIdx]->sItem[rIdx]->act.actionType );

        if( sVehicle[sIdx]->sItem[rIdx]->act.actionType == 0 ){ // Appear

            if( sVehicle[sIdx]->sItem[rIdx]->act.fParams.size() >= 4 ){

                vaAppearX->setValue( sVehicle[sIdx]->sItem[rIdx]->act.fParams[0] );
                vaAppearY->setValue( sVehicle[sIdx]->sItem[rIdx]->act.fParams[1] );
                vaAppearPsi->setValue( sVehicle[sIdx]->sItem[rIdx]->act.fParams[2] );
                vaAppearSpeed->setValue( sVehicle[sIdx]->sItem[rIdx]->act.fParams[3] );
            }

            if( sVehicle[sIdx]->sItem[rIdx]->act.iParams.size() >= 3 ){

                vaVehicleModelID ->setValue( sVehicle[sIdx]->sItem[rIdx]->act.iParams[0] );
                vaAppearNearLaneID->setValue( sVehicle[sIdx]->sItem[rIdx]->act.iParams[1] );

                if( sVehicle[sIdx]->sItem[rIdx]->act.iParams[2] == 0 ){
                    vaAppearNodeRoute->setChecked( true );

                    QString routeStr = QString("Route Info:\n");
                    int no = 1;
                    for(int i=4;i<sVehicle[sIdx]->sItem[rIdx]->act.iParams.size();i+=3){

                        QString tStr = QString("[%1] In-Dir = %2, Node = %3, Out-Dir = %4\n")
                                .arg( no )
                                .arg( sVehicle[sIdx]->sItem[rIdx]->act.iParams[i] )
                                .arg( sVehicle[sIdx]->sItem[rIdx]->act.iParams[i+1] )
                                .arg( sVehicle[sIdx]->sItem[rIdx]->act.iParams[i+2] );

                        routeStr += tStr;
                        no++;
                    }
                    vaAppearRouteInfo->setText( routeStr );
                }
                else{
                    vaAppearPathRoute->setChecked( true );

                    QString routeStr = QString("Route Info:\n");
                    int no = 1;
                    for(int i=4;i<sVehicle[sIdx]->sItem[rIdx]->act.fParams.size();i+=4){

                        QString tStr = QString("[%1] X = %2, Y = %3, Z = %4, D = %5\n")
                                .arg( no )
                                .arg( sVehicle[sIdx]->sItem[rIdx]->act.fParams[i] )
                                .arg( sVehicle[sIdx]->sItem[rIdx]->act.fParams[i+1] )
                                .arg( sVehicle[sIdx]->sItem[rIdx]->act.fParams[i+2] )
                                .arg( sVehicle[sIdx]->sItem[rIdx]->act.fParams[i+3] );

                        routeStr += tStr;
                        no++;
                    }
                    vaAppearRouteInfo->setText( routeStr );
                }
            }

            if( sVehicle[sIdx]->sItem[rIdx]->act.bParams.size() == 1 ){
                vaAppearAllowRepeat->setChecked( sVehicle[sIdx]->sItem[rIdx]->act.bParams[0] );
            }
        }
        else if( sVehicle[sIdx]->sItem[rIdx]->act.actionType == 1 ){ // Control

//            qDebug() << "sIdx = " << sIdx;
//            qDebug() << "rIdx = " << rIdx;
//            qDebug() << "bParams = " << sVehicle[sIdx]->sItem[rIdx]->act.bParams;
//            qDebug() << "iParams = " << sVehicle[sIdx]->sItem[rIdx]->act.iParams;
//            qDebug() << "fParams = " << sVehicle[sIdx]->sItem[rIdx]->act.fParams;

            if( sVehicle[sIdx]->sItem[rIdx]->act.bParams.size() >= 3 ){

                vaCtrlChangeControlMode->setChecked( sVehicle[sIdx]->sItem[rIdx]->act.bParams[0] );
                vaCtrlAccelDecel->setChecked( sVehicle[sIdx]->sItem[rIdx]->act.bParams[1] );
                vaCtrlSteer->setChecked( sVehicle[sIdx]->sItem[rIdx]->act.bParams[2] );

                if( sVehicle[sIdx]->sItem[rIdx]->act.bParams.size() >= 4 ){
                    vaCtrlAllowRepeat->setChecked( sVehicle[sIdx]->sItem[rIdx]->act.bParams[3] );
                }
            }

            int fParaIdx = 0;
            int iParaIdx = 0;
            if( vaCtrlChangeControlMode->isChecked() == true ){

                if( sVehicle[sIdx]->sItem[rIdx]->act.iParams.size() >= 2 ){
                    vaCtrlSelControlMode->setCurrentIndex( sVehicle[sIdx]->sItem[rIdx]->act.iParams[iParaIdx++] );
                    vaCtrlTargetID->setValue( sVehicle[sIdx]->sItem[rIdx]->act.iParams[iParaIdx++] );
                }

                if( sVehicle[sIdx]->sItem[rIdx]->act.fParams.size() >= 5 ){

                    vaCtrlTargetSpeed->setValue( sVehicle[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                    vaCtrlTargetHeadwayTime->setValue( sVehicle[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                    vaCtrlTargetHeadwayDistance->setValue( sVehicle[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                    vaCtrlStopX->setValue( sVehicle[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                    vaCtrlStopY->setValue( sVehicle[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );

                    qDebug() << "iParams.size = " << sVehicle[sIdx]->sItem[rIdx]->act.iParams.size();

                    if( sVehicle[sIdx]->sItem[rIdx]->act.iParams.size() >= 3 ){

                        int nRow = sVehicle[sIdx]->sItem[rIdx]->act.iParams[iParaIdx++];
                        qDebug() << "nRow = " << nRow << " iParaIdx = " << iParaIdx;

                        qDebug() << "fParams.size = " << sVehicle[sIdx]->sItem[rIdx]->act.fParams.size();

                        if( sVehicle[sIdx]->sItem[rIdx]->act.fParams.size() >= 5 + 2 * nRow ){

                            QList<float> t;
                            QList<float> val;

                            for(int i=0;i<nRow;++i){

                                t.append( sVehicle[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                                val.append( sVehicle[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                            }

                            SetCtrlDataToTable( 0, t, val );
                        }
                        else{
                            ClearCtrlTable(0);
                        }
                    }
                }
            }
            else{
                ClearCtrlTable(0);
            }

            if( vaCtrlAccelDecel->isChecked() == true ){

                if( sVehicle[sIdx]->sItem[rIdx]->act.iParams.size() >= iParaIdx + 1 ){

                    int nRow = sVehicle[sIdx]->sItem[rIdx]->act.iParams[iParaIdx++];

                    if( sVehicle[sIdx]->sItem[rIdx]->act.fParams.size() >= fParaIdx + 2 * nRow ){

                        QList<float> t;
                        QList<float> val;

                        for(int i=0;i<nRow;++i){

                            t.append( sVehicle[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                            val.append( sVehicle[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                        }

                        SetCtrlDataToTable( 1, t, val );
                    }
                }
            }
            else{
                ClearCtrlTable(1);
            }

            if( vaCtrlSteer->isChecked() == true ){

                if( sVehicle[sIdx]->sItem[rIdx]->act.iParams.size() >= iParaIdx + 1 ){

                    int nRow = sVehicle[sIdx]->sItem[rIdx]->act.iParams[iParaIdx++];

                    if( sVehicle[sIdx]->sItem[rIdx]->act.fParams.size() >= fParaIdx + 2 * nRow ){

                        QList<float> t;
                        QList<float> val;

                        for(int i=0;i<nRow;++i){

                            t.append( sVehicle[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                            val.append( sVehicle[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                        }

                        SetCtrlDataToTable( 2, t, val );
                    }
                }
            }
            else{
                ClearCtrlTable(2);
            }
        }
        else if( sVehicle[sIdx]->sItem[rIdx]->act.actionType == 2 ){ // UDP

            if( sVehicle[sIdx]->sItem[rIdx]->act.iParams.size() == 5 ){
                QString setStr = QString();
                for(int i=0;i<4;++i){
                    setStr += QString("%1").arg( sVehicle[sIdx]->sItem[rIdx]->act.iParams[i] );
                    if( i < 3 ){
                        setStr += QString(".");
                    }
                }
                vaUDPIPAddr->setText( setStr );
                vaUDPPortSB->setValue( sVehicle[sIdx]->sItem[rIdx]->act.iParams[4] );
            }

            if( sVehicle[sIdx]->sItem[rIdx]->act.fParams.size() > 0 ){
                QString setStr = QString();
                for(int i=0;i<sVehicle[sIdx]->sItem[rIdx]->act.fParams.size() ;++i){
                    setStr += QString("%1").arg( sVehicle[sIdx]->sItem[rIdx]->act.fParams[i] );
                    if( i < sVehicle[sIdx]->sItem[rIdx]->act.fParams.size() - 1 ){
                        setStr += QString(",");
                    }
                }
                vaUDPSendData->setText( setStr );
            }
            else{
                vaUDPSendData->clear();
            }

            if( sVehicle[sIdx]->sItem[rIdx]->act.bParams.size() > 0 ){
                vaRepeatSend->setChecked( sVehicle[sIdx]->sItem[rIdx]->act.bParams[0] );
            }
        }
        else if( sVehicle[sIdx]->sItem[rIdx]->act.actionType == 3 ){ // Disappear

            if( sVehicle[sIdx]->sItem[rIdx]->act.bParams.size() > 0 ){
                vaDisappearAppearSoon->setChecked( sVehicle[sIdx]->sItem[rIdx]->act.bParams[0] );
            }
        }

    }
    else if( tabIndex == 2 ){
        int dIdx = pedestList->currentRow();
        if( dIdx < 0 ){
            return;
        }

        int rIdx = pedestSlot->currentRow();
        if( rIdx < 0 ){
            return;
        }

        int id = pedestList->currentItem()->text().remove("Pedest").trimmed().toInt();

        int sIdx = -1;
        for(int i=0;i<sPedest.size();++i){
            if( sPedest[i]->ID == id ){
                sIdx = i;
                break;
            }
        }
        if( sIdx < 0 ){
            return;
        }

        AND_OR->setCurrentIndex( sPedest[sIdx]->sItem[rIdx]->cond.combination );

        noTrigger->setChecked( sPedest[sIdx]->sItem[rIdx]->cond.noTrigger );

        timeTrigger->setChecked( sPedest[sIdx]->sItem[rIdx]->cond.timeTrigger );
        ttMinSB->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ttMin );
        ttSecSB->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ttSec );
        if( sPedest[sIdx]->sItem[rIdx]->cond.ttAbsOrRel == 0 ){
            ttAbsolute->setChecked( true );
        }
        else{
            ttRelative->setChecked( true );
        }

        positionTrigger->setChecked( sPedest[sIdx]->sItem[rIdx]->cond.positionTrigger );
        ptX->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ptX );
        ptY->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ptY );
        ptPsi->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ptPassAngle );
        ptWidth->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ptWidth );
        ptTargetID->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ptTargetObjID );

        velocityTrigger->setChecked( sPedest[sIdx]->sItem[rIdx]->cond.velocityTrigger );
        vtSpeed->setValue( sPedest[sIdx]->sItem[rIdx]->cond.vtSpeed );
        if( sPedest[sIdx]->sItem[rIdx]->cond.vtLowOrHigh == 0 ){
            vtHigher->setChecked( true );
        }
        else{
            vtSlower->setChecked( true );
        }
        vtTargetID->setValue( sPedest[sIdx]->sItem[rIdx]->cond.vtTargetObjID );

        TTCTrigger->setChecked( sPedest[sIdx]->sItem[rIdx]->cond.TTCTrigger );
        ttcVal->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ttcVal );
        if( sPedest[sIdx]->sItem[rIdx]->cond.ttcCalType == 0 ){
            ttcPoint->setChecked( true );
        }
        else{
            ttcObject->setChecked( true );
        }
        ttcX->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ttcCalPosX );
        ttcY->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ttcCalPosY );
        ttcCalObjectID->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ttcCalObjectID );
        ttcTargetID->setValue( sPedest[sIdx]->sItem[rIdx]->cond.ttcCalTargetObjID );

        FETrigger->setChecked( sPedest[sIdx]->sItem[rIdx]->cond.FETrigger );

        externalTrigger->setChecked( sPedest[sIdx]->sItem[rIdx]->cond.externalTrigger );
        etKeys->setCurrentIndex( sPedest[sIdx]->sItem[rIdx]->cond.funcKey );

        pedestActSelCB->setCurrentIndex( sPedest[sIdx]->sItem[rIdx]->act.actionType );

        if( sPedest[sIdx]->sItem[rIdx]->act.actionType == 0 ){ // Appear

            if( sPedest[sIdx]->sItem[rIdx]->act.fParams.size() >= 4 ){

                paAppearX->setValue( sPedest[sIdx]->sItem[rIdx]->act.fParams[0] );
                paAppearY->setValue( sPedest[sIdx]->sItem[rIdx]->act.fParams[1] );
                paAppearPsi->setValue( sPedest[sIdx]->sItem[rIdx]->act.fParams[2] );
                paAppearSpeed->setValue( sPedest[sIdx]->sItem[rIdx]->act.fParams[3] );
            }

            if( sPedest[sIdx]->sItem[rIdx]->act.iParams.size() >= 1 ){

                paPedestModelID->setValue( sPedest[sIdx]->sItem[rIdx]->act.iParams[0] );
            }


            QString routeStr = QString("Route Info:\n");
            int no = 1;
            for(int i=4;i<sPedest[sIdx]->sItem[rIdx]->act.fParams.size();i+=4){

                QString tStr = QString("[%1] X = %2, Y = %3, Z = %4, D = %5\n")
                        .arg( no )
                        .arg( sPedest[sIdx]->sItem[rIdx]->act.fParams[i] )
                        .arg( sPedest[sIdx]->sItem[rIdx]->act.fParams[i+1] )
                        .arg( sPedest[sIdx]->sItem[rIdx]->act.fParams[i+2] )
                        .arg( sPedest[sIdx]->sItem[rIdx]->act.fParams[i+3] );

                routeStr += tStr;
                no++;
            }
            paAppearRouteInfo->setText( routeStr );

            if( sPedest[sIdx]->sItem[rIdx]->act.bParams.size() >= 1 ){
                paAppearAllowRepeat->setChecked( sPedest[sIdx]->sItem[rIdx]->act.bParams[0] );
            }
        }
        else if( sPedest[sIdx]->sItem[rIdx]->act.actionType == 1 ){ // Control

            int nRow = 0;
            if( sPedest[sIdx]->sItem[rIdx]->act.iParams.size() >= 2 ){
                paCtrlSelControlMode->setCurrentIndex( sPedest[sIdx]->sItem[rIdx]->act.iParams[0] );
                nRow = sPedest[sIdx]->sItem[rIdx]->act.iParams[1];
            }

            if( sPedest[sIdx]->sItem[rIdx]->act.bParams.size() >= 0 ){
                paCtrlAllowRepeat->setChecked( sPedest[sIdx]->sItem[rIdx]->act.bParams[0] );
            }

            if( sPedest[sIdx]->sItem[rIdx]->act.iParams[0] == 2 &&
                    sPedest[sIdx]->sItem[rIdx]->act.fParams.size() >= 2 + 2 * nRow ){

                int fParaIdx = 0;

                paCtrlTargetSpeed->setValue( sPedest[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                paCtrlRunOutDirection->setValue( sPedest[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );

                QList<float> t;
                QList<float> val;

                for(int i=0;i<nRow;++i){

                    t.append( sPedest[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                    val.append( sPedest[sIdx]->sItem[rIdx]->act.fParams[fParaIdx++] );
                }

                SetCtrlDataToTable( 3, t, val );
            }
            else{
                ClearCtrlTable(3);
            }
        }
        else if( sPedest[sIdx]->sItem[rIdx]->act.actionType == 2 ){ // UDP

            if( sPedest[sIdx]->sItem[rIdx]->act.iParams.size() == 5 ){
                QString setStr = QString();
                for(int i=0;i<4;++i){
                    setStr += QString("%1").arg( sPedest[sIdx]->sItem[rIdx]->act.iParams[i] );
                    if( i < 3 ){
                        setStr += QString(".");
                    }
                }
                paUDPIPAddr->setText( setStr );
                paUDPPortSB->setValue( sPedest[sIdx]->sItem[rIdx]->act.iParams[4] );
            }

            if( sPedest[sIdx]->sItem[rIdx]->act.fParams.size() > 0 ){
                QString setStr = QString();
                for(int i=0;i<sPedest[sIdx]->sItem[rIdx]->act.fParams.size() ;++i){
                    setStr += QString("%1").arg( sPedest[sIdx]->sItem[rIdx]->act.fParams[i] );
                    if( i < sPedest[sIdx]->sItem[rIdx]->act.fParams.size() - 1 ){
                        setStr += QString(",");
                    }
                }
                paUDPSendData->setText( setStr );
            }
            else{
                paUDPSendData->clear();
            }

            if( sPedest[sIdx]->sItem[rIdx]->act.bParams.size() > 0  ){
                paRepeatSend->setChecked( sPedest[sIdx]->sItem[rIdx]->act.bParams[0] );
            }
        }
        else if( sPedest[sIdx]->sItem[rIdx]->act.actionType == 3 ){ // Disappear

            if( sPedest[sIdx]->sItem[rIdx]->act.bParams.size() > 0 ){
                paDisappearAppearSoon->setChecked( sPedest[sIdx]->sItem[rIdx]->act.bParams[0] );
            }
        }

    }
}


void ScenarioEditor::AddScenarioVehicleSlot()
{
    if( vehicleList->currentRow() < 0 ){
        return;
    }

    int currentVID = vehicleList->currentItem()->text().remove("Vehicle").trimmed().toInt();
    for(int i=0;i<sVehicle.size();++i){
        if( sVehicle[i]->ID != currentVID ){
            continue;
        }

        struct ScenarioItem *s = new struct ScenarioItem;

        s->cond.combination = 0;

        s->cond.noTrigger = false;

        s->cond.timeTrigger = false;
        s->cond.ttMin = 0;
        s->cond.ttSec = 0.0;
        s->cond.ttAbsOrRel = 0;

        s->cond.positionTrigger = false;
        s->cond.ptX = 0.0;
        s->cond.ptY = 0.0;
        s->cond.ptPassAngle = 0.0;
        s->cond.ptWidth = 10.0;
        s->cond.ptTargetObjID = -1;

        s->cond.velocityTrigger = false;
        s->cond.vtSpeed = 0.0;
        s->cond.vtLowOrHigh = 0;
        s->cond.vtTargetObjID = -1;

        s->cond.TTCTrigger = false;
        s->cond.ttcVal = 0.0;
        s->cond.ttcCalType = 0;
        s->cond.ttcCalPosX = 0.0;
        s->cond.ttcCalPosY = 0.0;
        s->cond.ttcCalTargetObjID = -1;
        s->cond.ttcCalObjectID = -1;

        s->cond.FETrigger = false;
        s->cond.externalTrigger = false;
        s->cond.funcKey = 0;

        s->act.actionType = 0;
        s->act.route = NULL;

        ChangeVehicleActSeletion(0);

        sVehicle[i]->sItem.append( s );

        break;
    }

    ClearCtrlTable(0);
    ClearCtrlTable(1);
    ClearCtrlTable(2);

    UpdateScenarioVehicleSlotList();
}


void ScenarioEditor::AddScenarioPedestSlot()
{
    if( pedestList->currentRow() < 0 ){
        return;
    }

    int currentPID = pedestList->currentItem()->text().remove("Pedest").trimmed().toInt();
    for(int i=0;i<sPedest.size();++i){
        if( sPedest[i]->ID != currentPID ){
            continue;
        }

        struct ScenarioItem *s = new struct ScenarioItem;

        s->cond.combination = 0;

        s->cond.noTrigger = false;

        s->cond.timeTrigger = false;
        s->cond.ttMin = 0;
        s->cond.ttSec = 0.0;
        s->cond.ttAbsOrRel = 0;

        s->cond.positionTrigger = false;
        s->cond.ptX = 0.0;
        s->cond.ptY = 0.0;
        s->cond.ptPassAngle = 0.0;
        s->cond.ptWidth = 10.0;
        s->cond.ptTargetObjID = -1;

        s->cond.velocityTrigger = false;
        s->cond.vtSpeed = 0.0;
        s->cond.vtLowOrHigh = 0;
        s->cond.vtTargetObjID = -1;

        s->cond.TTCTrigger = false;
        s->cond.ttcVal = 0.0;
        s->cond.ttcCalType = 0;
        s->cond.ttcCalPosX = 0.0;
        s->cond.ttcCalPosY = 0.0;
        s->cond.ttcCalTargetObjID = -1;
        s->cond.ttcCalObjectID = -1;

        s->cond.FETrigger = false;
        s->cond.externalTrigger = false;
        s->cond.funcKey = 0;

        s->act.actionType = 0;
        s->act.route = NULL;

        ChangePedestActSeletion(0);

        sPedest[i]->sItem.append( s );

        break;
    }

    ClearCtrlTable(3);

    UpdateScenarioPedestSlotList();
}


void ScenarioEditor::DelScenarioVehicleSlot()
{
    if( vehicleList->currentRow() < 0 ){
        return;
    }

    if( vehicleSlot->currentRow() < 0 ){
        return;
    }

    if( confirmToDelete->isChecked() == true ){
        QMessageBox msgBox;
        msgBox.setText("Delete Scenario Data.");
        msgBox.setInformativeText("Sure?");
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        if( ret == QMessageBox::Cancel ){
            return;
        }
    }

    int currentVID = vehicleList->currentItem()->text().remove("Vehicle").trimmed().toInt();
    int currentSID = vehicleSlot->currentItem()->text().remove("Slot").trimmed().toInt();

    for(int i=0;i<sVehicle.size();++i){
        if( sVehicle[i]->ID != currentVID ){
            continue;
        }

        for(int j=0;j<sVehicle[i]->sItem[currentSID]->act.laneShape.size();++j){

            road->ClearLaneShape( sVehicle[i]->sItem[currentSID]->act.laneShape[j] );

            delete sVehicle[i]->sItem[currentSID]->act.laneShape[j];
        }

        sVehicle[i]->sItem[currentSID]->act.laneShape.clear();

        if( sVehicle[i]->sItem[currentSID]->act.route != NULL ){
            for(int j=0;j<sVehicle[i]->sItem[currentSID]->act.route->nodeList.size();++j){
                delete sVehicle[i]->sItem[currentSID]->act.route->nodeList[j];
            }
            sVehicle[i]->sItem[currentSID]->act.route->nodeList.clear();

            for(int j=0;j<sVehicle[i]->sItem[currentSID]->act.route->laneList.size();++j){
                sVehicle[i]->sItem[currentSID]->act.route->laneList[j].clear();
            }
            sVehicle[i]->sItem[currentSID]->act.route->laneList.clear();

            for(int j=0;j<sVehicle[i]->sItem[currentSID]->act.route->routeLaneLists.size();++j){

                for(int k=0;k<sVehicle[i]->sItem[currentSID]->act.route->routeLaneLists[j]->laneList.size();++k){
                    sVehicle[i]->sItem[currentSID]->act.route->routeLaneLists[j]->laneList[k].clear();
                }
                sVehicle[i]->sItem[currentSID]->act.route->routeLaneLists[j]->laneList.clear();

                delete sVehicle[i]->sItem[currentSID]->act.route->routeLaneLists[j];
            }
            sVehicle[i]->sItem[currentSID]->act.route->routeLaneLists.clear();

            delete sVehicle[i]->sItem[currentSID]->act.route;
        }

        sVehicle[i]->sItem[currentSID]->act.bParams.clear();
        sVehicle[i]->sItem[currentSID]->act.iParams.clear();
        sVehicle[i]->sItem[currentSID]->act.fParams.clear();

        delete sVehicle[i]->sItem[currentSID];
        sVehicle[i]->sItem.removeAt( currentSID );
        break;
    }

    UpdateScenarioVehicleSlotList();
}


void ScenarioEditor::DelScenarioPedestSlot()
{
    if( pedestList->currentRow() < 0 ){
        return;
    }

    if( pedestSlot->currentRow() < 0 ){
        return;
    }

    if( confirmToDelete->isChecked() == true ){
        QMessageBox msgBox;
        msgBox.setText("Delete Scenario Data.");
        msgBox.setInformativeText("Sure?");
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        if( ret == QMessageBox::Cancel ){
            return;
        }
    }

    int currentPID = pedestList->currentItem()->text().remove("Pedest").trimmed().toInt();
    int currentSID = pedestSlot->currentItem()->text().remove("Slot").trimmed().toInt();

    for(int i=0;i<sPedest.size();++i){
        if( sPedest[i]->ID != currentPID ){
            continue;
        }

        for(int j=0;j<sPedest[i]->sItem[currentSID]->act.laneShape.size();++j){

            road->ClearLaneShape( sPedest[i]->sItem[currentSID]->act.laneShape[j] );

            delete sPedest[i]->sItem[currentSID]->act.laneShape[j];
        }

        delete sPedest[i]->sItem[currentSID];
        sPedest[i]->sItem.removeAt( currentSID );
        break;
    }

    UpdateScenarioPedestSlotList();
}


void ScenarioEditor::SetPickMode()
{
    QString senderName = sender()->objectName();

    if( senderName == QString("PositionTrigger") ){

        emit SetScenarioPickMode(1);
    }
    else if( senderName == QString("TTCTrigger") ){

        emit SetScenarioPickMode(2);
    }
    else if( senderName == QString("Teleport") ){

        emit SetScenarioPickMode(3);
    }
    else if( senderName == QString("VehicleAppear") ){

        emit SetScenarioPickMode(4);
    }
    else if( senderName == QString("VehicleStopPoint") ){

        emit SetScenarioPickMode(5);
    }
    else if( senderName == QString("VehicleSetPathRoute") ){

        emit SetScenarioPickMode(6);
    }
    else if( senderName == QString("pickTS") ){

        emit SetScenarioPickMode(7);
    }
    else if( senderName == QString("pickLane") ){

        emit SetScenarioPickMode(8);
    }
    else if( senderName == QString("VehicleSetNodeRoute") ){

        emit SetScenarioPickMode(9);
    }
    else if( senderName == QString("PedestrianSetPathRoute") ){

        emit SetScenarioPickMode(10);
    }
    else if( senderName == QString("PedestrianAppear") ){

        emit SetScenarioPickMode(11);
    }
}

void ScenarioEditor::GetPointsPicked(int mode, float x1, float y1, float x2, float y2)
{
    if( mode == 1 ){

        float dx = x2 - x1;
        float dy = y2 - y1;
        float angle = atan2( dy, dx ) * 57.3;

        ptX->setValue( x1 );
        ptY->setValue( y1 );
        ptPsi->setValue( angle );

    }
    else if( mode == 2 ){

        ttcX->setValue( x1 );
        ttcY->setValue( y1 );
    }
    else if( mode == 3 ){

        float dx = x2 - x1;
        float dy = y2 - y1;
        float angle = atan2( dy, dx ) * 57.3;

        saMoveToX->setValue( x1 );
        saMoveToY->setValue( y1 );
        saMoveToPsi->setValue( angle );

        QVector2D pos;
        pos.setX( x1 );
        pos.setY( y1 );

        int laneID = road->GetNearestLane( pos );
        if( laneID >= 0 ){
            saMoveToNearLaneID->setValue( laneID );
        }

    }
    else if( mode == 4 ){

        float dx = x2 - x1;
        float dy = y2 - y1;
        float angle = atan2( dy, dx ) * 57.3;

        vaAppearX->setValue( x1 );
        vaAppearY->setValue( y1 );
        vaAppearPsi->setValue( angle );

        QVector2D pos;
        pos.setX( x1 );
        pos.setY( y1 );

        int laneID = road->GetNearestLane( pos );
        if( laneID >= 0 ){
            vaAppearNearLaneID->setValue( laneID );
        }
    }
    else if( mode == 5 ){

        vaCtrlStopX->setValue( x1 );
        vaCtrlStopY->setValue( y1 );
    }
    else if( mode == 7 ){

        QVector2D pos;
        pos.setX( x1 );
        pos.setY( y1 );
        float dist = 0.0;

        int nearTS = road->GetNearestTrafficSignal( pos, dist );
        if( nearTS >= 0 ){
            saChangeTSTargetTSID->setValue( nearTS );
        }
    }
    else if( mode == 8 ){

        QVector2D pos;
        pos.setX( x1 );
        pos.setY( y1 );

        int nearLane = road->GetNearestLane( pos );
        if( nearLane >= 0 ){

            bool addOK = true;
            bool isFirst = false;

            QString currentText = saChangeSpeedInfoTargetLanes->text();
            if( currentText.isNull() == false && currentText.isEmpty() == false ){
                QStringList ctDiv = currentText.split(",");
                for(int i=0;i<ctDiv.size();++i){
                    if( QString( ctDiv[i]).trimmed().toInt() == nearLane ){
                        addOK = false;
                        break;
                    }
                }
            }
            else{
                isFirst = true;
            }

            if( addOK == true ){
                if( isFirst == true ){
                    QString setTxt = QString("%1").arg( nearLane );
                    saChangeSpeedInfoTargetLanes->setText( setTxt );
                }
                else{
                    QString setTxt = currentText + QString(",%1").arg( nearLane );
                    saChangeSpeedInfoTargetLanes->setText( setTxt );
                }
            }
        }
    }
    else if( mode == 11 ){

        float dx = x2 - x1;
        float dy = y2 - y1;
        float angle = atan2( dy, dx ) * 57.3;

        paAppearX->setValue( x1 );
        paAppearY->setValue( y1 );
        paAppearPsi->setValue( angle );

        QVector2D pos;
        pos.setX( x1 );
        pos.setY( y1 );
    }

}


void ScenarioEditor::GetPointListPicked(int mode, QList<QPointF> points)
{
    if( mode == 6 ){

        QList<float> tht;
        QList<float> tht_bk;
        for(int i=0;i<points.size()-1;++i){

            float dx = points[i+1].x() - points[i].x();
            float dy = points[i+1].y() - points[i].y();
            float angle = atan2( dy, dx ) * 57.3;

            tht.append( angle );
            tht_bk.append( angle );
            if( i == points.size()-2 ){
                tht.append( angle );
                tht_bk.append( tht_bk );
            }
        }
        for(int i=1;i<points.size()-1;++i){
            float tht_m = (tht_bk[i-1] + tht_bk[i]) * 0.5;
            tht[i] = tht_m;
        }

        QString routeStr = QString("Route Info:\n");
        for(int i=0;i<points.size();++i){

            QString tStr = QString("[%1] X = %2, Y = %3, Z = 0.0, D = %4\n")
                    .arg( i+1 )
                    .arg( points[i].x() )
                    .arg( points[i].y() )
                    .arg( tht[i] );

            routeStr += tStr;
        }

        vaAppearRouteInfo->setText( routeStr );
    }
    else if( mode == 10 ){

        QList<float> tht;
        QList<float> tht_bk;
        for(int i=0;i<points.size()-1;++i){

            float dx = points[i+1].x() - points[i].x();
            float dy = points[i+1].y() - points[i].y();
            float angle = atan2( dy, dx ) * 57.3;

            tht.append( angle );
            tht_bk.append( angle );
            if( i == points.size()-2 ){
                tht.append( angle );
                tht_bk.append( tht_bk );
            }
        }
        for(int i=1;i<points.size()-1;++i){
            float tht_m = (tht_bk[i-1] + tht_bk[i]) * 0.5;
            tht[i] = tht_m;
        }

        QString routeStr = QString("Route Info:\n");
        for(int i=0;i<points.size();++i){

            QString tStr = QString("[%1] X = %2, Y = %3, Z = 0.0, D = %4\n")
                    .arg( i+1 )
                    .arg( points[i].x() )
                    .arg( points[i].y() )
                    .arg( tht[i] );

            routeStr += tStr;
        }

        paAppearRouteInfo->setText( routeStr );
    }
    else if( mode == 9 ){

        QList<int> nodeList;
        QList<int> nodeInList;
        QList<int> nodeOutList;

        for(int i=0;i<points.size();++i){

            QVector2D pos;
            pos.setX( points[i].x() );
            pos.setY( points[i].y() );

            int ndID = road->GetNearestNode( pos );
            if( ndID >= 0 ){
                nodeList.append( ndID );
                nodeInList.append( -1 );
                nodeOutList.append( -1 );
            }
        }

        for(int i=0;i<nodeList.size()-1;++i){

            int nIdx = road->indexOfNode( nodeList[i] );

            bool foundConnection = false;

            qDebug() << "Check connection from Node " << nodeList[i] << " to " << nodeList[i+1];

            for(int j=0;j<road->nodes[nIdx]->legInfo.size();++j){
                if( road->nodes[nIdx]->legInfo[j]->connectingNode == nodeList[i+1] ){
                    nodeOutList[i] = road->nodes[nIdx]->legInfo[j]->legID;
                    nodeInList[i+1] = road->nodes[nIdx]->legInfo[j]->connectingNodeInDirect;
                    foundConnection = true;
                    break;
                }
            }

            if( foundConnection == false ){

                qDebug() << "Failed.";

                QMessageBox msgBox;
                msgBox.setText("Caution");
                msgBox.setInformativeText("Sorry, selected nodes have miss connection. See console for detail.");
                msgBox.exec();
                return;
            }

            if( i == 0 && road->nodes[nIdx]->legInfo.size() > 1 ){

                for(int j=0;j<road->nodes[nIdx]->legInfo.size();++j){
                    if( nodeOutList[i] == road->nodes[nIdx]->legInfo[j]->legID ){
                        if( road->nodes[nIdx]->legInfo[j]->oncomingLegID >= 0 ){
                            nodeInList[i] = road->nodes[nIdx]->legInfo[j]->oncomingLegID;
                        }
                        else if( road->nodes[nIdx]->legInfo[j]->leftTurnLegID.size() > 0 ){
                            nodeInList[i] = road->nodes[nIdx]->legInfo[j]->leftTurnLegID.at(0);
                        }
                        else if( road->nodes[nIdx]->legInfo[j]->rightTurnLegID.size() > 0 ){
                            nodeInList[i] = road->nodes[nIdx]->legInfo[j]->rightTurnLegID.at(0);
                        }
                    }
                }

            }
        }

        int nIdx = road->indexOfNode( nodeList.last() );
        if( road->nodes[nIdx]->legInfo.size() > 1 ){

            for(int j=0;j<road->nodes[nIdx]->legInfo.size();++j){
                if( nodeInList.last() == road->nodes[nIdx]->legInfo[j]->legID ){
                    if( road->nodes[nIdx]->legInfo[j]->oncomingLegID >= 0 ){
                        nodeOutList.last() = road->nodes[nIdx]->legInfo[j]->oncomingLegID;
                    }
                    else if( road->nodes[nIdx]->legInfo[j]->leftTurnLegID.size() > 0 ){
                        nodeOutList.last() = road->nodes[nIdx]->legInfo[j]->leftTurnLegID.at(0);
                    }
                    else if( road->nodes[nIdx]->legInfo[j]->rightTurnLegID.size() > 0 ){
                        nodeOutList.last() = road->nodes[nIdx]->legInfo[j]->rightTurnLegID.at(0);
                    }
                }
            }
        }


        QString routeStr = QString("Route Info:\n");
        for(int i=0;i<nodeList.size();++i){

            QString tStr = QString("[%1] In-Dir = %2, Node = %3, Out-Dir = %4\n")
                    .arg( i+1 )
                    .arg( nodeInList.at(i) )
                    .arg( nodeList.at(i) )
                    .arg( nodeOutList.at(i) );

            routeStr += tStr;
        }

        vaAppearRouteInfo->setText( routeStr );
    }
}


void ScenarioEditor::SetToNearestLane()
{
    QString senderName = sender()->objectName();

    if( senderName == QString("VehicleAppear") && vaAppearPathRoute->isChecked() == true ){

        float xc = vaAppearX->value();
        float yc = vaAppearY->value();

        int dIdx = vehicleList->currentRow();
        if( dIdx < 0 ){
            return;
        }
        int rIdx = vehicleSlot->currentRow();
        if( rIdx < 0 ){
            return;
        }

        int id = vehicleList->currentItem()->text().remove("Vehicle").trimmed().toInt();

        int sIdx = -1;
        for(int i=0;i<sVehicle.size();++i){
            if( sVehicle[i]->ID == id ){
                sIdx = i;
                break;
            }
        }
        if( sIdx < 0 ){
            return;
        }

        for(int i=0;i<sVehicle[sIdx]->sItem[rIdx]->act.laneShape.size();++i){

            float xt = 0.0;
            float yt = 0.0;
            float angle = 0.0;
            int ret = road->GetNearestLanePointShapeInfo( sVehicle[sIdx]->sItem[rIdx]->act.laneShape[i],
                                                          xc, yc, xt, yt, angle );

            if( ret == 1 ){
                vaAppearX->setValue( xt );
                vaAppearY->setValue( yt );
                vaAppearPsi->setValue( angle );
                vaAppearNearLaneID->setValue( -1 );
                break;
            }
        }
    }


    if( senderName == QString("Teleport") || (senderName == QString("VehicleAppear") && vaAppearNodeRoute->isChecked() == true) ){

        float xc = 0.0;
        float yc = 0.0;
        if( tabW->currentIndex() == 0 ){
            xc = saMoveToX->value();
            yc = saMoveToY->value();
        }
        else if( tabW->currentIndex() == 1 ){
            xc = vaAppearX->value();
            yc = vaAppearY->value();
        }
        else if( tabW->currentIndex() == 2 ){
            xc = paAppearX->value();
            yc = paAppearY->value();
        }
        else{
            return;
        }

        QVector2D pos;
        pos.setX( xc );
        pos.setY( yc );

        int laneID = road->GetNearestLane( pos );

        float xt = 0.0;
        float yt = 0.0;
        float angle = 0.0;
        if( laneID >= 0 && road->GetNearestLanePoint(laneID, xc, yc, xt, yt, angle) == laneID ){

            if( tabW->currentIndex() == 0 ){
                saMoveToX->setValue( xt );
                saMoveToY->setValue( yt );
                saMoveToPsi->setValue( angle );
                saMoveToNearLaneID->setValue( laneID );
            }
            else if( tabW->currentIndex() == 1 ){
                vaAppearX->setValue( xt );
                vaAppearY->setValue( yt );
                vaAppearPsi->setValue( angle );
                vaAppearNearLaneID->setValue( laneID );
            }
            else if( tabW->currentIndex() == 2 ){
                paAppearX->setValue( xt );
                paAppearY->setValue( yt );
                paAppearPsi->setValue( angle );
                paAppearNearLaneID->setValue( laneID );
            }
        }
    }

    emit UpdateGraphic();
}


void ScenarioEditor::ClearRouteData()
{
    if( sender()->objectName() == QString("VehicleClearNodeRoute") || sender()->objectName() == QString("VehicleClearPathRoute") ){
        QString routeStr = QString("Route Info:\n");
        vaAppearRouteInfo->setText( routeStr );
    }
    else{
        QString routeStr = QString("Route Info:\n");
        paAppearRouteInfo->setText( routeStr );
    }
}


void ScenarioEditor::SetNodeRouteLaneLists(int sIdx, int rIdx)
{
    if( sVehicle[sIdx]->sItem[rIdx]->act.route != NULL ){

        for(int i=0;i<sVehicle[sIdx]->sItem[rIdx]->act.route->nodeList.size();++i){
            delete sVehicle[sIdx]->sItem[rIdx]->act.route->nodeList[i];
        }
        sVehicle[sIdx]->sItem[rIdx]->act.route->nodeList.clear();

        for(int i=0;i<sVehicle[sIdx]->sItem[rIdx]->act.route->laneList.size();++i){
            sVehicle[sIdx]->sItem[rIdx]->act.route->laneList[i].clear();
        }
        sVehicle[sIdx]->sItem[rIdx]->act.route->laneList.clear();

        for(int i=0;i<sVehicle[sIdx]->sItem[rIdx]->act.route->routeLaneLists.size();++i){

            for(int j=0;j<sVehicle[sIdx]->sItem[rIdx]->act.route->routeLaneLists[i]->laneList.size();++j){
                sVehicle[sIdx]->sItem[rIdx]->act.route->routeLaneLists[i]->laneList[j].clear();
            }
            sVehicle[sIdx]->sItem[rIdx]->act.route->routeLaneLists[i]->laneList.clear();

            delete sVehicle[sIdx]->sItem[rIdx]->act.route->routeLaneLists[i];
        }
        sVehicle[sIdx]->sItem[rIdx]->act.route->routeLaneLists.clear();
    }
    else{
        struct RouteData *rd = new struct RouteData;
        sVehicle[sIdx]->sItem[rIdx]->act.route = rd;
    }

    int iIdx = 4;
    for(int i=0;i<sVehicle[sIdx]->sItem[rIdx]->act.iParams[3];++i){

        struct RouteElem *re = new struct RouteElem;

        re->inDir  = sVehicle[sIdx]->sItem[rIdx]->act.iParams[iIdx++];
        re->node   = sVehicle[sIdx]->sItem[rIdx]->act.iParams[iIdx++];
        re->outDir = sVehicle[sIdx]->sItem[rIdx]->act.iParams[iIdx++];

        sVehicle[sIdx]->sItem[rIdx]->act.route->nodeList.append( re );
    }


    // extract lane-list
    road->GetLaneListForScenarioNodeRoute( sVehicle[sIdx]->sItem[rIdx]->act.route );
}


void ScenarioEditor::SetPathRouteLaneShape(int sIdx, int rIdx)
{
    for(int k=0;k<sVehicle[sIdx]->sItem[rIdx]->act.laneShape.size();++k){
        road->ClearLaneShape( sVehicle[sIdx]->sItem[rIdx]->act.laneShape[k] );
        delete sVehicle[sIdx]->sItem[rIdx]->act.laneShape[k];
    }
    sVehicle[sIdx]->sItem[rIdx]->act.laneShape.clear();

    for(int k=4;k<sVehicle[sIdx]->sItem[rIdx]->act.fParams.size()-4;k+=4){

        struct LaneShapeInfo *lsi = new struct LaneShapeInfo;

        QVector3D *sp = new QVector3D();
        sp->setX( sVehicle[sIdx]->sItem[rIdx]->act.fParams[k] );
        sp->setY( sVehicle[sIdx]->sItem[rIdx]->act.fParams[k+1] );
        sp->setZ( sVehicle[sIdx]->sItem[rIdx]->act.fParams[k+2] );
        lsi->pos.append( sp );

        float tht1 = sVehicle[sIdx]->sItem[rIdx]->act.fParams[k+3] * 0.017452;
        QVector2D *sd = new QVector2D();
        sd->setX( cos(tht1) );
        sd->setY( sin(tht1) );
        lsi->derivative.append( sd );

        QVector3D *ep = new QVector3D();
        ep->setX( sVehicle[sIdx]->sItem[rIdx]->act.fParams[k+4] );
        ep->setY( sVehicle[sIdx]->sItem[rIdx]->act.fParams[k+5] );
        ep->setZ( sVehicle[sIdx]->sItem[rIdx]->act.fParams[k+6] );
        lsi->pos.append( ep );

        float tht2 = sVehicle[sIdx]->sItem[rIdx]->act.fParams[k+7] * 0.017452;
        QVector2D *ed = new QVector2D();
        ed->setX( cos(tht2) );
        ed->setY( sin(tht2) );
        lsi->derivative.append( ed );

        road->CalculateShape( lsi );

        sVehicle[sIdx]->sItem[rIdx]->act.laneShape.append( lsi );
    }
}


void ScenarioEditor::SetPedestRouteLaneShape(int sIdx, int rIdx)
{
    for(int k=0;k<sPedest[sIdx]->sItem[rIdx]->act.pedestLaneshape.size();++k){
        delete sPedest[sIdx]->sItem[rIdx]->act.pedestLaneshape[k];
    }
    sPedest[sIdx]->sItem[rIdx]->act.pedestLaneshape.clear();

    for(int k=4;k<sPedest[sIdx]->sItem[rIdx]->act.fParams.size();k+=4){

        struct PedestrianLaneShapeElement *lse = new struct PedestrianLaneShapeElement;

        lse->pos.setX( sPedest[sIdx]->sItem[rIdx]->act.fParams[k] );
        lse->pos.setY( sPedest[sIdx]->sItem[rIdx]->act.fParams[k+1] );
        lse->pos.setZ( sPedest[sIdx]->sItem[rIdx]->act.fParams[k+2] );

        lse->angleToNextPos = 0.0;
        lse->width = 1.0;
        lse->distanceToNextPos = 0.0;
        lse->isCrossWalk = false;
        lse->controlPedestSignalID = -1;
        lse->runOutDirect = 0;
        lse->runOutProb = 0.0;

        sPedest[sIdx]->sItem[rIdx]->act.pedestLaneshape.append( lse );
    }

    for(int i=0;i<sPedest[sIdx]->sItem[rIdx]->act.pedestLaneshape.size()-1;++i){

        float dx = sPedest[sIdx]->sItem[rIdx]->act.pedestLaneshape[i+1]->pos.x() - sPedest[sIdx]->sItem[rIdx]->act.pedestLaneshape[i]->pos.x();
        float dy = sPedest[sIdx]->sItem[rIdx]->act.pedestLaneshape[i+1]->pos.y() - sPedest[sIdx]->sItem[rIdx]->act.pedestLaneshape[i]->pos.y();

        sPedest[sIdx]->sItem[rIdx]->act.pedestLaneshape[i]->angleToNextPos    = atan2( dy, dx );   // [rad]
        sPedest[sIdx]->sItem[rIdx]->act.pedestLaneshape[i]->distanceToNextPos = sqrt( dx * dx + dy * dy );
    }
}


bool ScenarioEditor::isScenarioVehicleSelected(int id)
{
    if( tabW->currentIndex() != 1 ){
        return false;
    }

    if( vehicleList->currentRow() < 0 ){
        return false;
    }

    if( vehicleList->currentItem()->text() == QString("Vehicle %1").arg(id) ){
        return true;
    }

    return false;
}


bool ScenarioEditor::isScenarioPedestrianSelected(int id)
{
    if( tabW->currentIndex() != 2 ){
        return false;
    }

    if( pedestList->currentRow() < 0 ){
        return false;
    }

    if( pedestList->currentItem()->text() == QString("Pedest %1").arg(id) ){
        return true;
    }

    return false;
}


void ScenarioEditor::EmitUpdateGraphic()
{
    emit UpdateGraphic();
}


void ScenarioEditor::AddRowTable()
{
    qDebug() << "[ScenarioEditor::AddRowTable] sender = " << sender()->objectName();

    if( sender()->objectName() == QString("SpeedProfile") ){

        if( vehicleList->currentRow() < 0 ){
            return;
        }
        if( vehicleSlot->currentRow() < 0 ){
            return;
        }

        int cRow = vaCtrlSpeedProfile->currentRow();
        if( cRow < 0 ){
            cRow = vaCtrlSpeedProfile->rowCount() - 1;
        }

        vaCtrlSpeedProfile->insertRow( cRow + 1 );

        QTableWidgetItem *titem = new QTableWidgetItem();
        vaCtrlSpeedProfile->setItem( cRow + 1, 0, titem);

        QTableWidgetItem *vitem = new QTableWidgetItem();
        vaCtrlSpeedProfile->setItem( cRow + 1, 1, vitem);

    }
    else if( sender()->objectName() == QString("AccelDecel") ){

        if( vehicleList->currentRow() < 0 ){
            return;
        }
        if( vehicleSlot->currentRow() < 0 ){
            return;
        }

        int cRow = vaCtrlAccelDecelTable->currentRow();
        if( cRow < 0 ){
            cRow = vaCtrlAccelDecelTable->rowCount() - 1;
        }

        vaCtrlAccelDecelTable->insertRow( cRow + 1 );

        QTableWidgetItem *titem = new QTableWidgetItem();
        vaCtrlAccelDecelTable->setItem( cRow + 1, 0, titem);

        QTableWidgetItem *vitem = new QTableWidgetItem();
        vaCtrlAccelDecelTable->setItem( cRow + 1, 1, vitem);

    }
    else if( sender()->objectName() == QString("Steer") ){

        if( vehicleList->currentRow() < 0 ){
            return;
        }
        if( vehicleSlot->currentRow() < 0 ){
            return;
        }

        int cRow = vaCtrlSteerTable->currentRow();
        if( cRow < 0 ){
            cRow = vaCtrlSteerTable->rowCount() - 1;
        }

        vaCtrlSteerTable->insertRow( cRow + 1 );

        QTableWidgetItem *titem = new QTableWidgetItem();
        vaCtrlSteerTable->setItem( cRow + 1, 0, titem);

        QTableWidgetItem *vitem = new QTableWidgetItem();
        vaCtrlSteerTable->setItem( cRow + 1, 1, vitem);
    }
    else if( sender()->objectName() == QString("PedestSpeedProfile") ){

        if( pedestList->currentRow() < 0 ){
            return;
        }
        if( pedestSlot->currentRow() < 0 ){
            return;
        }

        int cRow = paCtrlSpeedProfile->currentRow();
        if( cRow < 0 ){
            cRow = paCtrlSpeedProfile->rowCount() - 1;
        }

        paCtrlSpeedProfile->insertRow( cRow + 1 );

        QTableWidgetItem *titem = new QTableWidgetItem();
        paCtrlSpeedProfile->setItem( cRow + 1, 0, titem);

        QTableWidgetItem *vitem = new QTableWidgetItem();
        paCtrlSpeedProfile->setItem( cRow + 1, 1, vitem);

    }
}


void ScenarioEditor::DelRowTable()
{
    qDebug() << "[ScenarioEditor::DelRowTable] sender = " << sender()->objectName();

    if( sender()->objectName() == QString("SpeedProfile") ){

        if( vehicleList->currentRow() < 0 ){
            return;
        }
        if( vehicleSlot->currentRow() < 0 ){
            return;
        }

        int cRow = vaCtrlSpeedProfile->currentRow();
        if( cRow < 0 ){
            return;
        }

        vaCtrlSpeedProfile->removeRow( cRow );

    }
    else if( sender()->objectName() == QString("AccelDecel") ){

        if( vehicleList->currentRow() < 0 ){
            return;
        }
        if( vehicleSlot->currentRow() < 0 ){
            return;
        }

        int cRow = vaCtrlAccelDecelTable->currentRow();
        if( cRow < 0 ){
            return;
        }

        vaCtrlAccelDecelTable->removeRow( cRow );
    }
    else if( sender()->objectName() == QString("Steer") ){

        if( vehicleList->currentRow() < 0 ){
            return;
        }
        if( vehicleSlot->currentRow() < 0 ){
            return;
        }

        int cRow = vaCtrlSteerTable->currentRow();
        if( cRow < 0 ){
            return;
        }

        vaCtrlSteerTable->removeRow( cRow );
    }
    else if( sender()->objectName() == QString("PedestSpeedProfile") ){

        if( pedestList->currentRow() < 0 ){
            return;
        }
        if( pedestSlot->currentRow() < 0 ){
            return;
        }

        int cRow = paCtrlSpeedProfile->currentRow();
        if( cRow < 0 ){
            return;
        }

        paCtrlSpeedProfile->removeRow( cRow );
    }
}


void ScenarioEditor::ClearCtrlTable(int tblNo)
{
    if( tblNo == 0 ){  // Speed Profile
        vaCtrlSpeedProfile->clear();

        QStringList tableLabels;
        tableLabels << "Time[s]";
        tableLabels << "Speed[km/h]";

        vaCtrlSpeedProfile->setHorizontalHeaderLabels( tableLabels );
        vaCtrlSpeedProfile->setColumnWidth(0,TABLE_TIME_WIDTH);
        vaCtrlSpeedProfile->setColumnWidth(1,TABLE_VAL_WIDTH);

        //qDebug() << "ClearCtrlTable: tblNo = " << tblNo << " rowCount = " << vaCtrlSpeedProfile->rowCount();
    }
    else if( tblNo == 1 ){  // AccelDecel

        vaCtrlAccelDecelTable->clear();

        QStringList tableLabels;
        tableLabels << "Time[s]";
        tableLabels << "Accel[G]";

        vaCtrlAccelDecelTable->setHorizontalHeaderLabels( tableLabels );
        vaCtrlAccelDecelTable->setColumnWidth(0,TABLE_TIME_WIDTH);
        vaCtrlAccelDecelTable->setColumnWidth(1,TABLE_VAL_WIDTH);
    }
    else if( tblNo == 2 ){  // Steer

        vaCtrlSteerTable->clear();

        QStringList tableLabels;
        tableLabels << "Time[s]";
        tableLabels << "Steer[deg]";

        vaCtrlSteerTable->setHorizontalHeaderLabels( tableLabels );
        vaCtrlSteerTable->setColumnWidth(0,TABLE_TIME_WIDTH);
        vaCtrlSteerTable->setColumnWidth(1,TABLE_VAL_WIDTH);
    }
    else if( tblNo == 3 ){  // Pedest Speed Profile
        paCtrlSpeedProfile->clear();

        QStringList tableLabels;
        tableLabels << "Time[s]";
        tableLabels << "Speed[m/s]";

        paCtrlSpeedProfile->setHorizontalHeaderLabels( tableLabels );
        paCtrlSpeedProfile->setColumnWidth(0,TABLE_TIME_WIDTH);
        paCtrlSpeedProfile->setColumnWidth(1,TABLE_VAL_WIDTH);
    }
}

void ScenarioEditor::SetCtrlDataToTable(int tblNo,QList<float> t,QList<float> val)
{
    if( t.size() != val.size() ){
        return;
    }

    //qDebug() << "[SetCtrlDataToTable] tblNo = " << tblNo;

    if( tblNo == 0 ){  // Speed Profile

        ClearCtrlTable(tblNo);

        vaCtrlSpeedProfile->setRowCount( t.size() );

        //qDebug() << "vaCtrlSpeedProfile.rowCount = " << vaCtrlSpeedProfile->rowCount();
        //qDebug() << "t.size = " << t.size();

        for(int i=0;i<t.size();++i){
            QTableWidgetItem *titem = new QTableWidgetItem();
            titem->setText( QString("%1").arg( t[i]) );
            vaCtrlSpeedProfile->setItem( i, 0, titem);

            QTableWidgetItem *vitem = new QTableWidgetItem();
            vitem->setText( QString("%1").arg( val[i]) );
            vaCtrlSpeedProfile->setItem( i, 1, vitem);
        }

        SetDataToGraph(0);
    }
    else if( tblNo == 1 ){  // AccelDecel

        ClearCtrlTable(tblNo);

        vaCtrlAccelDecelTable->setRowCount( t.size() );

        for(int i=0;i<t.size();++i){
            QTableWidgetItem *titem = new QTableWidgetItem();
            titem->setText( QString("%1").arg( t[i]) );
            vaCtrlAccelDecelTable->setItem( i, 0, titem);

            QTableWidgetItem *vitem = new QTableWidgetItem();
            vitem->setText( QString("%1").arg( val[i]) );
            vaCtrlAccelDecelTable->setItem( i, 1, vitem);
        }

        SetDataToGraph(1);
    }
    else if( tblNo == 2 ){  // Steer

        ClearCtrlTable(tblNo);

        vaCtrlSteerTable->setRowCount( t.size() );

        for(int i=0;i<t.size();++i){
            QTableWidgetItem *titem = new QTableWidgetItem();
            titem->setText( QString("%1").arg( t[i]) );
            vaCtrlSteerTable->setItem( i, 0, titem);

            QTableWidgetItem *vitem = new QTableWidgetItem();
            vitem->setText( QString("%1").arg( val[i]) );
            vaCtrlSteerTable->setItem( i, 1, vitem);
        }

        SetDataToGraph(2);
    }
    else if( tblNo == 3 ){  // Pedest Speed Profile

        ClearCtrlTable(tblNo);

        paCtrlSpeedProfile->setRowCount( t.size() );

        //qDebug() << "vaCtrlSpeedProfile.rowCount = " << vaCtrlSpeedProfile->rowCount();
        //qDebug() << "t.size = " << t.size();

        for(int i=0;i<t.size();++i){
            QTableWidgetItem *titem = new QTableWidgetItem();
            titem->setText( QString("%1").arg( t[i]) );
            paCtrlSpeedProfile->setItem( i, 0, titem);

            QTableWidgetItem *vitem = new QTableWidgetItem();
            vitem->setText( QString("%1").arg( val[i]) );
            paCtrlSpeedProfile->setItem( i, 1, vitem);
        }

        SetDataToGraph(3);
    }
}


void ScenarioEditor::CloseDialogs()
{
    speedProfileGraph->close();
    accelDecelGraph->close();
    steerGraph->close();
    pedestSpeedProfileGraph->close();
}


void ScenarioEditor::SetDataToGraph(int type)
{
    if( type == 0 ){

        int nRow = vaCtrlSpeedProfile->rowCount();
        if( nRow == 0 ){
            return;
        }

        QList<float> t;
        QList<float> val;
        for(int i=0;i<nRow;++i){
            t.append( vaCtrlSpeedProfile->item(i,0)->text().toFloat() );
            val.append( vaCtrlSpeedProfile->item(i,1)->text().toFloat() );
        }

        speedProfileGraph->SetData(t,val);
    }
    else if( type == 1 ){

        int nRow = vaCtrlAccelDecelTable->rowCount();
        if( nRow == 0 ){
            return;
        }

        QList<float> t;
        QList<float> val;
        for(int i=0;i<nRow;++i){
            t.append( vaCtrlAccelDecelTable->item(i,0)->text().toFloat() );
            val.append( vaCtrlAccelDecelTable->item(i,1)->text().toFloat() );
        }

        accelDecelGraph->SetData(t,val);
    }
    else if( type == 2 ){

        int nRow = vaCtrlSteerTable->rowCount();
        if( nRow == 0 ){
            return;
        }

        QList<float> t;
        QList<float> val;
        for(int i=0;i<nRow;++i){
            t.append( vaCtrlSteerTable->item(i,0)->text().toFloat() );
            val.append( vaCtrlSteerTable->item(i,1)->text().toFloat() );
        }

        steerGraph->SetData(t,val);
    }
    else if( type == 3 ){

        int nRow = paCtrlSpeedProfile->rowCount();
        if( nRow == 0 ){
            return;
        }

        QList<float> t;
        QList<float> val;
        for(int i=0;i<nRow;++i){
            t.append( paCtrlSpeedProfile->item(i,0)->text().toFloat() );
            val.append( paCtrlSpeedProfile->item(i,1)->text().toFloat() );
        }

        pedestSpeedProfileGraph->SetData(t,val);
    }
}

