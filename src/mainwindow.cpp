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


#include "mainwindow.h"
#include <QApplication>
#include <QtGui>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    statusBar()->showMessage(QString("SEdit"));

    setFocusPolicy(Qt::StrongFocus);

    QWidget *cWidget = new QWidget();

    currentSEditFilename = QString();

    //-------------------------
    road = new RoadInfo;

    //-------------------------
    canvas = new GraphicCanvas();
    //canvas->setMinimumSize(1600,1200);
    canvas->road = road;
    connect( canvas, SIGNAL(UpdateStatusBar(QString)), this, SLOT(UpdateStatusBar(QString)));

    //-------------------------
    setDlg = new SettingDialog();
    connect(setDlg,SIGNAL(TrafficDirectionSettingChanged(int)),this,SLOT(SetTrafficDirection(int)));
    setDlg->hide();

    road->setDlg = setDlg;

    //-------------------------
    dtManip = new DataManipulator();
    dtManip->road   = road;
    dtManip->canvas = canvas;
    dtManip->setDlg = setDlg;
    connect( dtManip, SIGNAL(UpdateStatusBar(QString)), this, SLOT(UpdateStatusBar(QString)));
    connect( canvas, SIGNAL(PedestLanePointPicked()), dtManip, SLOT(CreatePedestPath()) );


    //-------------------------
    mapImageMng = new BaseMapImageManager();
    mapImageMng->hide();

    dtManip->mapImageMng = mapImageMng;
    road->mapImageMng    = mapImageMng;
    canvas->mapImageMng  = mapImageMng;

    connect( mapImageMng, SIGNAL(MapImageAdded(struct baseMapImage *)), canvas, SLOT(LoadMapImage(struct baseMapImage *)) );
    connect( mapImageMng, SIGNAL(MapImageDeleted(struct baseMapImage *)), canvas, SLOT(DeleteMapImage(struct baseMapImage *)) );
    connect( mapImageMng, SIGNAL(UpdateGraphic()), canvas, SLOT(update()) );


    //-------------------------
    resimOut = new ResimFilesOutput();
    resimOut->road = road;
    resimOut->hide();

    //-------------------------
    dispCtrl = new DisplayControl();

    connect( dispCtrl->OrthogonalView, SIGNAL(toggled(bool)), canvas, SLOT(SetProjectionOrthogonal(bool)) );
    connect( dispCtrl, SIGNAL(ViewMoveTo(float,float)), canvas, SLOT(MoveTo(float,float)) );
    connect( dispCtrl->resetRotate, SIGNAL(clicked()), canvas, SLOT(ResetRotate()) );

    connect( dispCtrl->showMapImage, SIGNAL(toggled(bool)), canvas, SLOT(SetMapVisibility(bool)) );
    connect( dispCtrl->backMapImage, SIGNAL(toggled(bool)), canvas, SLOT(SetBackMap(bool)) );
    connect( dispCtrl->showNodes, SIGNAL(toggled(bool)), canvas, SLOT(SetNodeVisibility(bool)) );
    connect( dispCtrl->showNodeLaneList, SIGNAL(toggled(bool)), canvas, SLOT(SetNodeLaneListlVisibility(bool)) );
    connect( dispCtrl->showRelatedLanes, SIGNAL(toggled(bool)), canvas, SLOT(SetRelatedLaneslVisibility(bool)) );
    connect( dispCtrl->showRouteLaneList, SIGNAL(toggled(bool)), canvas, SLOT(SetRouteLaneListlVisibility(bool)) );
    connect( dispCtrl->prevLaneList, SIGNAL(clicked()), canvas, SLOT(ShowPrevLaneList()) );
    connect( dispCtrl->nextLaneList, SIGNAL(clicked()), canvas, SLOT(ShowNextLaneList()) );
    connect( dispCtrl->showNodeLabels, SIGNAL(toggled(bool)), canvas, SLOT(SetNodeLabelVisibility(bool)) );
    connect( dispCtrl->showLanes, SIGNAL(toggled(bool)), canvas, SLOT(SetLaneVisibility(bool)) );
    connect( dispCtrl->showLaneLabels, SIGNAL(toggled(bool)), canvas, SLOT(SetLaneLabelVisibility(bool)) );
    connect( dispCtrl->laneWidth, SIGNAL(valueChanged(int)), canvas, SLOT(SetLaneDrawWidth(int)) );
    connect( dispCtrl->showTrafficSignals, SIGNAL(toggled(bool)), canvas, SLOT(SetTrafficSignalVisibility(bool)) );
    connect( dispCtrl->showTrafficSignalLabels, SIGNAL(toggled(bool)), canvas, SLOT(SetTrafficSignalLabelVisibility(bool)) );
    connect( dispCtrl->showStopLines, SIGNAL(toggled(bool)), canvas, SLOT(SetStopLineVisibility(bool)) );
    connect( dispCtrl->showStopLineLabels, SIGNAL(toggled(bool)), canvas, SLOT(SetStopLineLabelVisibility(bool)) );
    connect( dispCtrl->showPedestLanes, SIGNAL(toggled(bool)), canvas, SLOT(SetPedestLaneVisibility(bool)) );
    connect( dispCtrl->showPedestLaneLabels, SIGNAL(toggled(bool)), canvas, SLOT(SetPedestLaneLabelVisibility(bool)) );
    connect( dispCtrl->showLabels, SIGNAL(toggled(bool)), canvas, SLOT(SetLabelVisibility(bool)) );
    connect( dispCtrl->colorMapOfLaneSpeedLimit, SIGNAL(toggled(bool)), canvas, SLOT(SetLaneColorBySpeedLimitFlag(bool)) );
    connect( dispCtrl->colorMapOfLaneActualSpeed, SIGNAL(toggled(bool)), canvas, SLOT(SetLaneColorByActualSpeedFlag(bool)) );

    connect( dispCtrl->selectNode, SIGNAL(toggled(bool)), canvas, SLOT(SetNodeSelection(bool)) );
    connect( dispCtrl->selectLane, SIGNAL(toggled(bool)), canvas, SLOT(SetLaneSelection(bool)) );
    connect( dispCtrl->selectTrafficSignal, SIGNAL(toggled(bool)), canvas, SLOT(SetTrafficSignalSelection(bool)) );
    connect( dispCtrl->selectStopLine, SIGNAL(toggled(bool)), canvas, SLOT(SetStopLineSelection(bool)) );
    connect( dispCtrl->selectPedestLane, SIGNAL(toggled(bool)), canvas, SLOT(SetPedestLaneSelection(bool)) );

    dispCtrl->road = road;
    dispCtrl->move(50,50);
    dispCtrl->setWindowIcon(QIcon(":images/SEdit-icon.png"));
    dispCtrl->show();


    road->LeftOrRight = setDlg->GetCurrentLeftRightIndex();
    road->useRelativePath = setDlg->GetUseRelativePath();

    //-------------------------
    roadObjProp = new RoadObjectProperty();
    roadObjProp->road = road;

    roadObjProp->move(50 + dispCtrl->sizeHint().width() + 10 ,50);
    roadObjProp->setWindowIcon(QIcon(":images/SEdit-icon.png"));
    roadObjProp->show();

    roadObjProp->setDlg  = setDlg;
    canvas->roadProperty = roadObjProp;
    dtManip->roadObjProp = roadObjProp;

    //-------------------------
    odRoute = new ODRouteEditor();
    odRoute->road = road;
    odRoute->propRO = roadObjProp;
    odRoute->setDlg = setDlg;
    odRoute->SetHeaderTrafficVolumeTable();
    odRoute->move(50 + dispCtrl->sizeHint().width() + roadObjProp->sizeHint().width() + 10, 50);
    odRoute->hide();

    canvas->odRoute = odRoute;

    connect( roadObjProp, SIGNAL(ShowODRouteEdit()), odRoute, SLOT(show()) );
    connect( roadObjProp, SIGNAL(HideODRouteEdit()), odRoute, SLOT(hide()) );
    connect( roadObjProp, SIGNAL(DestinationNodeChanged(int,bool)), odRoute, SLOT(SetCurrentODRouteData(int,bool)) );
    connect( roadObjProp, SIGNAL(OriginNodeChanged(int,bool)), odRoute, SLOT(SetCurrentODRouteData(int,bool)) );
    connect( roadObjProp, SIGNAL(ResetLaneListIndex()), canvas, SLOT(ResetLaneListIndex()) );

    connect( odRoute, SIGNAL(ShowMessageStatusBar(QString)), this, SLOT(UpdateStatusBar(QString)) );
    connect( odRoute, SIGNAL(SetNodePickMode(int,int)), canvas, SLOT(SetNodePickMode(int,int)) );
    connect( odRoute, SIGNAL(ResetNodePickMode()), canvas, SLOT(ResetNodePickMode()) );
    connect( odRoute, SIGNAL(SetNodeSelected(int)), canvas, SLOT(SetNodeSelected(int)) );
    connect( odRoute, SIGNAL(UpdateGraphic()), canvas, SLOT(update()) );
    connect( canvas, SIGNAL(SetNodeListForRoute(QList<int>)), odRoute, SLOT(GetNodeListForRoute(QList<int>)) );

    //-------------------------
    configMgr = new ConfigFileManager();
    configMgr->hide();


    //-------------------------
    scenarioEdit = new ScenarioEditor(setDlg,road);
    scenarioEdit->hide();
    scenarioEdit->move( QGuiApplication::screens().at(0)->size().width() - scenarioEdit->sizeHint().width() - 50, 50);

    connect( scenarioEdit, SIGNAL(SetScenarioPickMode(int)), canvas, SLOT(SetScenarioPickMode(int)) );
    connect( scenarioEdit, SIGNAL(UpdateGraphic()), canvas, SLOT(update()) );
    connect( canvas, SIGNAL(PointsPickedForScenario(int,float,float,float,float)), scenarioEdit, SLOT(GetPointsPicked(int,float,float,float,float)));
    connect( canvas, SIGNAL(PointListForScenario(int,QList<QPointF>)), scenarioEdit, SLOT(GetPointListPicked(int,QList<QPointF>)));

    canvas->scnrEdit = scenarioEdit;



    //
    //  Menubar
    //
    QMenu* fileMenu = menuBar()->addMenu( tr("&File") );

    QAction* newAct = new QAction( tr("&New"), this );
    newAct->setIcon(QIcon(":/images/new.png"));
    newAct->setShortcut( tr("Ctrl+N") );
    newAct->setStatusTip( tr("Clear All Data nad Create a New Map Data") );
    connect( newAct, SIGNAL(triggered()), this, SLOT(NewFile()));
    fileMenu->addAction( newAct );


    QAction* openAct = new QAction( tr("&Open"), this );
    openAct->setIcon(QIcon(":/images/open.png"));
    openAct->setShortcut( tr("Ctrl+O") );
    openAct->setStatusTip( tr("Open SEdit Data File") );
    connect( openAct, SIGNAL(triggered()), this, SLOT(OpenFile()));
    fileMenu->addAction( openAct );


    QAction* saveAct = new QAction( tr("&Save"), this );
    saveAct->setIcon(QIcon(":/images/save.png"));
    saveAct->setShortcut( tr("Ctrl+S") );
    saveAct->setStatusTip( tr("Save SEdit Data File") );
    connect( saveAct, SIGNAL(triggered()), this, SLOT(SaveFile()));
    fileMenu->addAction( saveAct );


    QAction* saveAsAct = new QAction( tr("&SaveAs"), this );
    saveAsAct->setIcon(QIcon(":/images/saveas.png"));
    saveAsAct->setStatusTip( tr("Save SEdit Data File by Another Name") );
    connect( saveAsAct, SIGNAL(triggered()), this, SLOT(SaveAsFile()));
    fileMenu->addAction( saveAsAct );

    fileMenu->addSeparator();


    recentFileOpen= fileMenu->addMenu("Recent Files");
    GetRecentDataFile();

    fileMenu->addSeparator();

    QAction* importAct = new QAction( tr("&Import"), this );
    importAct->setStatusTip( tr("Import Other Data File") );
    connect( importAct, SIGNAL(triggered()), this, SLOT(ImportOtherData()));
    fileMenu->addAction( importAct );

    fileMenu->addSeparator();

    QAction* closeAct = new QAction( tr("&Exit"), this );
    closeAct->setIcon(QIcon(":/images/exit.png"));
    closeAct->setShortcut( tr("Ctrl+Q") );
    connect( closeAct, SIGNAL(triggered()), this, SLOT(close()));
    fileMenu->addAction( closeAct );


    QMenu* toolMenu = menuBar()->addMenu( tr("&Tool") );

    QAction* showMapImageDialogAct = new QAction( tr("&Show Map Image Dialog"), this );
    connect( showMapImageDialogAct, SIGNAL(triggered()), mapImageMng, SLOT(show()) );
    toolMenu->addAction( showMapImageDialogAct );

    QAction* showScenarioEditorAct = new QAction( tr("&Show Scenario Editor"), this );
    connect( showScenarioEditorAct, SIGNAL(triggered()), scenarioEdit, SLOT(show()) );
    toolMenu->addAction( showScenarioEditorAct );

    QAction* showResimOutputAct = new QAction( tr("&Show Resim Files Output Dialog"), this );
    connect( showResimOutputAct, SIGNAL(triggered()), resimOut, SLOT(show()) );
    toolMenu->addAction( showResimOutputAct );

    toolMenu->addSeparator();

    QAction* showConfigMgrAct = new QAction( tr("&Show Config-File Manager"), this );
    connect( showConfigMgrAct, SIGNAL(triggered()), configMgr, SLOT(show()) );
    toolMenu->addAction( showConfigMgrAct );

    toolMenu->addSeparator();

    QAction* showSettingDialogAct = new QAction( tr("&Show Setting Dialog"), this );
    connect( showSettingDialogAct, SIGNAL(triggered()), setDlg, SLOT(show()) );
    toolMenu->addAction( showSettingDialogAct );




    //
    //   Layout
    //
    QHBoxLayout *mainLayout = new QHBoxLayout();



    mainLayout->addWidget( canvas );

    cWidget->setLayout( mainLayout );

    setCentralWidget( cWidget );


    //
    //   Popup menu preparation
    //
    createObjectPopup = new QMenu();

    QAction *createNode_4x1x1_NoTS = new QAction();
    createNode_4x1x1_NoTS->setText("4-Leg 1x1 noTS");
    connect( createNode_4x1x1_NoTS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_4x1x1_noTS()));
    connect( createNode_4x1x1_NoTS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_4x1x1_NoTS );

    QAction *createNode_4x2x1_NoTS = new QAction();
    createNode_4x2x1_NoTS->setText("4-Leg 2x1 noTS");
    connect( createNode_4x2x1_NoTS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_4x2x1_noTS()));
    connect( createNode_4x2x1_NoTS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_4x2x1_NoTS );

    QAction *createNode_3x1x1_NoTS = new QAction();
    createNode_3x1x1_NoTS->setText("3-Leg 1x1 noTS");
    connect( createNode_3x1x1_NoTS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3x1x1_noTS()));
    connect( createNode_3x1x1_NoTS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3x1x1_NoTS );

    QAction *createNode_3x2x1_NoTS = new QAction();
    createNode_3x2x1_NoTS->setText("3-Leg 2x1 noTS");
    connect( createNode_3x2x1_NoTS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3x2x1_noTS()));
    connect( createNode_3x2x1_NoTS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3x2x1_NoTS );

    QAction *createNode_3x1x1_tr_NoTS = new QAction();
    createNode_3x1x1_tr_NoTS->setText("3-Leg 1x1 noTS, turn restriction");
    connect( createNode_3x1x1_tr_NoTS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3x1x1_tr_noTS()));
    connect( createNode_3x1x1_tr_NoTS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3x1x1_tr_NoTS );

    QAction *createNode_3x2x1_tr_NoTS = new QAction();
    createNode_3x2x1_tr_NoTS->setText("3-Leg 2x1 noTS, turn restriction");
    connect( createNode_3x2x1_tr_NoTS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3x2x1_tr_noTS()));
    connect( createNode_3x2x1_tr_NoTS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3x2x1_tr_NoTS );

    QAction *createNode_3x3x1_tr_NoTS = new QAction();
    createNode_3x3x1_tr_NoTS->setText("3-Leg 3x1 noTS, turn restriction");
    connect( createNode_3x3x1_tr_NoTS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3x3x1_tr_noTS()));
    connect( createNode_3x3x1_tr_NoTS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3x3x1_tr_NoTS );


    createObjectPopup->addSeparator();

    QAction *createNode_4x1x1_TS = new QAction();
    createNode_4x1x1_TS->setText("4-Leg 1x1 TS");
    connect( createNode_4x1x1_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_4x1x1_TS()));
    connect( createNode_4x1x1_TS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_4x1x1_TS );

    QAction *createNode_4x1x1_r_TS = new QAction();
    createNode_4x1x1_r_TS->setText("4-Leg 1x1 with Turn-Lane, TS");
    connect( createNode_4x1x1_r_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_4x1x1_r_TS()));
    connect( createNode_4x1x1_r_TS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_4x1x1_r_TS );

    QAction *createNode_4x2x1_TS = new QAction();
    createNode_4x2x1_TS->setText("4-Leg 2x1 TS");
    connect( createNode_4x2x1_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_4x2x1_TS()));
    connect( createNode_4x2x1_TS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_4x2x1_TS );

    QAction *createNode_4x2x1_r_TS = new QAction();
    createNode_4x2x1_r_TS->setText("4-Leg 2x1 with Turn-Lane, TS");
    connect( createNode_4x2x1_r_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_4x2x1_r_TS()));
    connect( createNode_4x2x1_r_TS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_4x2x1_r_TS );

    QAction *createNode_4x2x2_TS = new QAction();
    createNode_4x2x2_TS->setText("4-Leg 2x2 TS");
    connect( createNode_4x2x2_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_4x2x2_TS()));
    connect( createNode_4x2x2_TS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_4x2x2_TS );

    QAction *createNode_4x2x2_r_TS = new QAction();
    createNode_4x2x2_r_TS->setText("4-Leg 2x2 with Turn-Lane, TS");
    connect( createNode_4x2x2_r_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_4x2x2_r_TS()));
    connect( createNode_4x2x2_r_TS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_4x2x2_r_TS );

    createObjectPopup->addSeparator();

    QAction *createNode_3x1x1_TS = new QAction();
    createNode_3x1x1_TS->setText("3-Leg 1x1 TS");
    connect( createNode_3x1x1_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3x1x1_TS()));
    connect( createNode_3x1x1_TS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3x1x1_TS );

    QAction *createNode_3x1x1_r_TS = new QAction();
    createNode_3x1x1_r_TS->setText("3-Leg 1x1 with Turn-Lane, TS");
    connect( createNode_3x1x1_r_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3x1x1_r_TS()));
    connect( createNode_3x1x1_r_TS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3x1x1_r_TS );

    QAction *createNode_3x2x1_TS = new QAction();
    createNode_3x2x1_TS->setText("3-Leg 2x1 TS");
    connect( createNode_3x2x1_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3x2x1_TS()));
    connect( createNode_3x2x1_TS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3x2x1_TS );

    QAction *createNode_3x2x1_rm_TS = new QAction();
    createNode_3x2x1_rm_TS->setText("3-Leg 2x1 with Turn-Lane for only Primary Lane, TS");
    connect( createNode_3x2x1_rm_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3x2x1_rm_TS()));
    connect( createNode_3x2x1_rm_TS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3x2x1_rm_TS );

    QAction *createNode_3x2x1_r_TS = new QAction();
    createNode_3x2x1_r_TS->setText("3-Leg 2x1 with Turn-Lane, TS");
    connect( createNode_3x2x1_r_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3x2x1_r_TS()));
    connect( createNode_3x2x1_r_TS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3x2x1_r_TS );

    createObjectPopup->addSeparator();

    QAction *createNode_2L_exist = new QAction();
    createNode_2L_exist->setText("2-Lanes and 1 exist Lane");
    connect( createNode_2L_exist, SIGNAL(triggered()),dtManip,SLOT(CreateNode_2L_exist()));
    connect( createNode_2L_exist, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_2L_exist );

    QAction *createNode_3L_exist = new QAction();
    createNode_3L_exist->setText("3-Lanes and 1 exist Lane");
    connect( createNode_3L_exist, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3L_exist()));
    connect( createNode_3L_exist, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3L_exist );

    QAction *createNode_2L_merge = new QAction();
    createNode_2L_merge->setText("2-Lanes and 1 merging Lane");
    connect( createNode_2L_merge, SIGNAL(triggered()),dtManip,SLOT(CreateNode_2L_merge()));
    connect( createNode_2L_merge, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_2L_merge );

    QAction *createNode_3L_merge = new QAction();
    createNode_3L_merge->setText("3-Lanes and 1 merging Lane");
    connect( createNode_3L_merge, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3L_merge()));
    connect( createNode_3L_merge, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_3L_merge );

    createObjectPopup->addSeparator();

    QAction *createNode_straightcourse_1 = new QAction();
    createNode_straightcourse_1->setText("Straight Course, 1 Lane");
    connect( createNode_straightcourse_1, SIGNAL(triggered()),dtManip,SLOT(CreateNode_straight_1()));
    connect( createNode_straightcourse_1, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_straightcourse_1 );

    QAction *createNode_straightcourse_2 = new QAction();
    createNode_straightcourse_2->setText("Straight Course, 2 Lanes");
    connect( createNode_straightcourse_2, SIGNAL(triggered()),dtManip,SLOT(CreateNode_straight_2()));
    connect( createNode_straightcourse_2, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_straightcourse_2 );

    QAction *createNode_straightcourse_3 = new QAction();
    createNode_straightcourse_3->setText("Straight Course, 3 Lanes");
    connect( createNode_straightcourse_3, SIGNAL(triggered()),dtManip,SLOT(CreateNode_straight_3()));
    connect( createNode_straightcourse_3, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createNode_straightcourse_3 );

    createObjectPopup->addSeparator();

    QAction *createNode_assignAll = new QAction();
    createNode_assignAll->setText("Show Create Node Dialog");
    connect( createNode_assignAll, SIGNAL(triggered()),dtManip,SLOT(CreateNode_Dialog()));
    createObjectPopup->addAction( createNode_assignAll );

    createObjectPopup->addSeparator();

    QAction *createPedestLane = new QAction();
    createPedestLane->setText("Pedest Lane");
    connect( createPedestLane, SIGNAL(triggered()),dtManip,SLOT(StartCreatePedestPath()));
    connect( createPedestLane, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    createObjectPopup->addAction( createPedestLane );


    //
    insertNodePopup = new QMenu();

    //
    utilityPopup = new QMenu();

    QAction *createNode_duplicate = new QAction();
    createNode_duplicate->setText("Duplicate Selected Nodes and Lanes");
    connect( createNode_duplicate, SIGNAL(triggered()),dtManip,SLOT(DuplicateNodes()));
    connect( createNode_duplicate, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( createNode_duplicate );

    utilityPopup->addSeparator();

    QAction *checkLaneConnect = new QAction();
    checkLaneConnect->setText("Check Lane Connection");
    connect( checkLaneConnect, SIGNAL(triggered()),dtManip,SLOT(CheckLaneConnectionFull()));
    connect( checkLaneConnect, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( checkLaneConnect );

    QAction *createWPData = new QAction();
    createWPData->setText("Create WP Data");
    connect( createWPData, SIGNAL(triggered()),dtManip,SLOT(CreateWPData()));
    connect( createWPData, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( createWPData );

    QAction *setODTerm = new QAction();
    setODTerm->setText("Terminal Node -> OD Node");
    connect( setODTerm, SIGNAL(triggered()),dtManip,SLOT(SetODFlagOfTerminalNode()));
    connect( setODTerm, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( setODTerm );

    QAction *setAllLaneLists = new QAction();
    setAllLaneLists->setText("Set All Lane-Lists");
    connect( setAllLaneLists, SIGNAL(triggered()),dtManip,SLOT(SetAllLaneLists()));
    connect( setAllLaneLists, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( setAllLaneLists );

    QAction *setSelectedNodeLaneLists = new QAction();
    setSelectedNodeLaneLists->setText("Set Lane-Lists for Selected Node");
    connect( setSelectedNodeLaneLists, SIGNAL(triggered()),dtManip,SLOT(SetSelectedNodeLaneLists()));
    connect( setSelectedNodeLaneLists, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( setSelectedNodeLaneLists );

    QAction *setTurnDirection = new QAction();
    setTurnDirection->setText("Set Turn-Direction Info");
    connect( setTurnDirection, SIGNAL(triggered()),dtManip,SLOT(SetTurnDirectionInfo()));
    connect( setTurnDirection, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( setTurnDirection );

    QAction *checkAllSLCrossLane = new QAction();
    checkAllSLCrossLane->setText("Check Cross Point of StopLines and Lanes");
    connect( checkAllSLCrossLane, SIGNAL(triggered()),dtManip,SLOT(CheckAllStopLineCrossLane()));
    connect( checkAllSLCrossLane, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( checkAllSLCrossLane );

    QAction *checkAllCPOfLane = new QAction();
    checkAllCPOfLane->setText("Check Cross Points of All Lanes");
    connect( checkAllCPOfLane, SIGNAL(triggered()),dtManip,SLOT(CheckLaneCrossPoints()));
    connect( checkAllCPOfLane, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( checkAllCPOfLane );

    QAction *checkCPOfSelectedLane = new QAction();
    checkCPOfSelectedLane->setText("Check Cross Points of Selected Lane");
    connect( checkCPOfSelectedLane, SIGNAL(triggered()),dtManip,SLOT(CheckCrossPointsOfSelectedLane()));
    connect( checkCPOfSelectedLane, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( checkCPOfSelectedLane );

    QAction *changeSpeedLimitOfSelectedLanes = new QAction();
    changeSpeedLimitOfSelectedLanes->setText("Change Speed Limit of Selected Lane");
    connect( changeSpeedLimitOfSelectedLanes, SIGNAL(triggered()),dtManip,SLOT(ChangeSpeedLimitOfSelectedLanes()));
    connect( changeSpeedLimitOfSelectedLanes, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( changeSpeedLimitOfSelectedLanes );

    QAction *changeActualSpeedOfSelectedLanes = new QAction();
    changeActualSpeedOfSelectedLanes->setText("Change Actual Speed of Selected Lane");
    connect( changeActualSpeedOfSelectedLanes, SIGNAL(triggered()),dtManip,SLOT(ChangeActualSpeedOfSelectedLanes()));
    connect( changeActualSpeedOfSelectedLanes, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( changeActualSpeedOfSelectedLanes );

    QAction *checkCPOfLaneAndPedestLane = new QAction();
    checkCPOfLaneAndPedestLane->setText("Check Cross Points of Lanes and Pedestrian Lanes");
    connect( checkCPOfLaneAndPedestLane, SIGNAL(triggered()),dtManip,SLOT(CheckLaneAndPedestLaneCrossPoint()));
    connect( checkCPOfLaneAndPedestLane, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    utilityPopup->addAction( checkCPOfLaneAndPedestLane );

    utilityPopup->addSeparator();

    QAction *readLineCSV = new QAction();
    readLineCSV->setText("Read Line CSV");
    connect( readLineCSV, SIGNAL(triggered()),dtManip,SLOT(ReadLineCSV()));
    utilityPopup->addAction( readLineCSV );

    QAction *clearLineCSV = new QAction();
    clearLineCSV->setText("Clear Line Data");
    connect( clearLineCSV, SIGNAL(triggered()),dtManip,SLOT(ClearLineData()));
    utilityPopup->addAction( clearLineCSV );

    QAction *changeLineObjCoordInfo = new QAction();
    changeLineObjCoordInfo->setText("Change Line Coord Info");
    connect( changeLineObjCoordInfo, SIGNAL(triggered()),dtManip,SLOT(ChangeLineCoordInfo()));
    utilityPopup->addAction( changeLineObjCoordInfo );

    //
    searchPopup = new QMenu();

    QAction *searchNode = new QAction();
    searchNode->setText("Search Node");
    connect( searchNode, SIGNAL(triggered()),dtManip,SLOT(SearchNode()));
    searchPopup->addAction( searchNode );

    QAction *searchLane = new QAction();
    searchLane->setText("Search Lane");
    connect( searchLane, SIGNAL(triggered()),dtManip,SLOT(SearchLane()));
    searchPopup->addAction( searchLane );

    QAction *searchTS = new QAction();
    searchTS->setText("Search Traffic Signal");
    connect( searchTS, SIGNAL(triggered()),dtManip,SLOT(SearchTrafficSignal()));
    searchPopup->addAction( searchTS );

    QAction *moveXY = new QAction();
    moveXY->setText("Move to XY");
    connect( moveXY, SIGNAL(triggered()),dtManip,SLOT(MoveXY()));
    searchPopup->addAction( moveXY );

    searchPopup->addSeparator();

    QAction *setNodePos = new QAction();
    setNodePos->setText("Set Node Position");
    connect( setNodePos, SIGNAL(triggered()),dtManip,SLOT(SetNodePos()));
    searchPopup->addAction( setNodePos );

    QAction *selectAllLanes = new QAction();
    selectAllLanes->setText("Select All Lanes");
    connect( selectAllLanes, SIGNAL(triggered()),dtManip,SLOT(SelectAllLanes()));
    searchPopup->addAction( selectAllLanes );

    //
    connect( canvas->createVTS, SIGNAL(triggered()),dtManip,SLOT(CreateTrafficSignalForVehicle()));
    connect( canvas->createVTS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    connect( canvas->createPTS, SIGNAL(triggered()),dtManip,SLOT(CreateTrafficSignalForPedestrian()));
    connect( canvas->createPTS, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
    connect( canvas->createSL, SIGNAL(triggered()),dtManip,SLOT(CreateStopLineForInDir()));
    connect( canvas->createSL, SIGNAL(triggered()), this, SLOT(WrapWinModified()));
}


MainWindow::~MainWindow()
{

}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if( okToContinue() ){

        if( dispCtrl ){
            dispCtrl->close();
        }

        if( roadObjProp ){
            roadObjProp->close();
        }

        if( odRoute ){
            odRoute->close();
        }

        if( resimOut ){
            resimOut->close();
        }

        if( configMgr ){
            configMgr->close();
        }

        if( mapImageMng ){
            mapImageMng->close();
        }

        if( scenarioEdit ){
            scenarioEdit->CloseDialogs();
            scenarioEdit->close();
        }

        event->accept();

    }
    else{
        event->ignore();
    }
}


bool MainWindow::okToContinue()
{
    if( isWindowModified() ){

        int ret = QMessageBox::warning(this, tr("S-Edit"),
                                       tr("The Data has been modified.\n"
                                          "Do you want to save your changes?"),
                                       QMessageBox::Yes | QMessageBox::Default,
                                       QMessageBox::No,
                                       QMessageBox::Cancel | QMessageBox::Escape);
        if( ret == QMessageBox::Yes ){
            SaveAsFile();
        }
        else if( ret == QMessageBox::Cancel ){
            return false;
        }
    }
    return true;
}


void MainWindow::NewFile()
{
    qDebug() << "[MainWindow::NewFile]";

    if( (currentSEditFilename.isNull() == false && currentSEditFilename.isEmpty() == false ) || isWindowModified() == true ){
        int ret = QMessageBox::warning(this, tr("S-Edit"),
                                       tr("All Data will be cleared.\n"
                                          "Do you want to continue?"),
                                       QMessageBox::Yes | QMessageBox::Default,
                                       QMessageBox::Cancel | QMessageBox::Escape);
        if( ret == QMessageBox::Cancel ){
            qDebug() << "Canceled.";
            return ;
        }
    }

    qDebug() << "Clear Data...";

    currentSEditFilename = QString();

    road->ClearAllData();
    qDebug() << "Road Data cleared.";

    dtManip->operationHistory.clear();

    mapImageMng->ClearAll();
    qDebug() << "Map Image Data cleared.";

    dispCtrl->InitSetting();

    roadObjProp->ChangeTabPage(0);
    odRoute->hide();

    resimOut->Clear();
    resimOut->hide();

    configMgr->ClearDataWithoutConfirm();

    canvas->ResetPedestLanePointPickMode();
    canvas->ResetNodePickMode();
    canvas->selectedObj.selObjKind.clear();
    canvas->selectedObj.selObjID.clear();
    canvas->SetNumberKeyPressed(-1);
    canvas->MoveTo(0.0,0.0);
    canvas->ResetRotate();

    setWindowModified(false);
    setWindowTitle(QString("MDS02-Canopus | S-Edit[*]"));

    qDebug() << "End.";
}

void MainWindow::OpenFile()
{
    QString fileName = QString();

    if( isWindowModified() ){

        QMessageBox msgBox;
        msgBox.setText("Opening a file clears the existing data.");
        msgBox.setInformativeText("Sure to open the file?");
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        if( ret == QMessageBox::Cancel ){
            return;
        }
        else{

            fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose SEdit Data File"),
                                                    ".",
                                                    tr("SEdit Data file(*.se.txt)"));

            if( fileName.isNull() == false ){
                qDebug() << "filename = " << fileName;
            }
            else{
                qDebug() << "Open action canceled.";
                return;
            }

            //
            // clear data
            NewFile();
        }
    }
    else{
        fileName = QFileDialog::getOpenFileName(this,
                                                tr("Choose SEdit Data File"),
                                                ".",
                                                tr("SEdit Data file(*.se.txt)"));

        if( fileName.isNull() == false ){
            qDebug() << "filename = " << fileName;
        }
        else{
            qDebug() << "Open action canceled.";
            return;
        }
    }

    // open
    OpenRoadDataWithFilename( fileName );
}


bool MainWindow::SaveFile()
{
    if( currentSEditFilename == QString() || currentSEditFilename.isNull() || currentSEditFilename.isEmpty() ){
        SaveAsFile();
        return false;
    }

    road->useRelativePath = setDlg->GetUseRelativePath();

    if( road->SaveRoadData( currentSEditFilename ) == true ){

        setWindowModified(false);
        setWindowTitle(QString("MDS02-Canopus | S-Edit - %1[*]").arg(currentSEditFilename));

        SetRecentDataFile();

        qDebug() << "Data file saved. Filename = " << currentSEditFilename;

        UpdateStatusBar( QString("Data file saved.") );

        return true;
    }

    return false;
}

bool MainWindow::SaveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save SEdit Data File"),
                                                    ".",
                                                    tr("SEdit Data file(*.se.txt)"));

    if( fileName.isNull() == false ){

        if( fileName.endsWith(".se.txt") == false ){
            fileName += QString(".se.txt");
        }

        qDebug() << "filename = " << fileName;

    }
    else{
        qDebug() << "saveAsConfigSetting action canceled.";
        return false;
    }

    currentSEditFilename = fileName;
    SaveFile();

    return true;
}


void MainWindow::ImportOtherData()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose Import Data File"),
                                                    ".",
                                                    tr("Data file(*.eris3 *.spd *.ods)"));

    if( fileName.isNull() == false ){
        qDebug() << "filename = " << fileName;
    }
    else{
        qDebug() << "Import action canceled.";
        return;
    }


    if( fileName.endsWith(".eris3") == true ){

        dtManip->ImportERIS3Data( fileName );

        UpdateStatusBar(QString("data imported."));
    }
    else if( fileName.endsWith(".spd") == true ){

        dtManip->ImportERIS3TrafficSignalData( fileName );

        UpdateStatusBar(QString("data imported."));
    }
    else if( fileName.endsWith(".ods") == true ){

        dtManip->ImportERIS3ODData( fileName );

        UpdateStatusBar(QString("data imported."));

    }
}


void MainWindow::SetTrafficDirection(int dir)
{
    if( road ){
        road->LeftOrRight = dir;
        if( dir == 0 ){
            qDebug() << "[MainWindow::SetTrafficDirection] road->LeftOrRight = LEFT-HAND";
        }
        else if( dir == 1 ){
            qDebug() << "[MainWindow::SetTrafficDirection] road->LeftOrRight = RIGHT-HAND";
        }
    }
}


void MainWindow::keyPressEvent(QKeyEvent *e)
{
    Qt::KeyboardModifiers modifi = QApplication::keyboardModifiers();
    int key = e->key();


    if( key == Qt::Key_Escape ){

        qDebug() << "[MainWindow::keyPressEvent] key = ESCAPE";

        canvas->ResetPedestLanePointPickMode();
        canvas->ResetNodePickMode();
        canvas->selectedObj.selObjKind.clear();
        canvas->selectedObj.selObjID.clear();
        canvas->SetNumberKeyPressed(-1);
        canvas->update();

        return;
    }

    if( key == Qt::Key_Delete ){
        canvas->RemovePickedPedestLanePoint();
    }

    if( modifi & Qt::AltModifier ){
        if( key == Qt::Key_N ){
            if( canvas->selectedObj.selObjKind.size() == 0 ){
                createObjectPopup->exec( QCursor::pos() );
            }
        }
        else if( key == Qt::Key_U ){
            utilityPopup->exec( QCursor::pos() );
        }
        else if( key == Qt::Key_S ){
            searchPopup->exec( QCursor::pos() );
        }
        else if( key == Qt::Key_Delete ){
            dtManip->DeleteSelectedObject();
        }
        else if( key == Qt::Key_M ){
            dtManip->MergeSelectedObject();
        }
        else if( key == Qt::Key_D ){
            dtManip->SplitSelectedLane();
        }
        else if( key == Qt::Key_I ){

            int checkInsertCondition = 0;
            bool selectedIsAllLane = true;
            for(int i=0;i<canvas->selectedObj.selObjKind.size();++i){
                if( canvas->selectedObj.selObjKind[i] != canvas->SEL_LANE ){
                    selectedIsAllLane = false;
                    break;
                }
            }
            if( selectedIsAllLane == false ){
                return;
            }

            QStringList relNodes;
            for(int i=0;i<canvas->selectedObj.selObjKind.size();++i){
                int lidx = road->indexOfLane( canvas->selectedObj.selObjID[i] );
                if( lidx >= 0 ){
                    if( road->lanes[lidx]->sWPInNode == road->lanes[lidx]->eWPInNode ){
                        continue;
                    }
                    QString snInfo = QString("%1-%2").arg( road->lanes[lidx]->sWPInNode ).arg( road->lanes[lidx]->sWPNodeDir);
                    if( relNodes.indexOf( snInfo ) < 0 ){
                        relNodes.append( snInfo );
                    }
                    QString enInfo = QString("%1-%2").arg( road->lanes[lidx]->eWPInNode ).arg( road->lanes[lidx]->eWPNodeDir);
                    if( relNodes.indexOf( enInfo ) < 0 ){
                        relNodes.append( enInfo );
                    }
                }
            }
            if( relNodes.size() != 2 ){
                return;
            }


            dtManip->insertMode = checkInsertCondition;
            dtManip->insertNode1 = canvas->selectedObj.selObjID[0];
            dtManip->insertNode2 = canvas->selectedObj.selObjID[1];

            insertNodePopup->clear();

            QAction *insert4Legx1 = new QAction();
            insert4Legx1->setText("4-Leg 1-Lane noTS");
            connect( insert4Legx1, SIGNAL(triggered()),dtManip,SLOT(InsertNode_4x1_noTS()));
            insertNodePopup->addAction( insert4Legx1 );

//            QAction *insert4Legx2 = new QAction();
//            insert4Legx2->setText("4-Leg 2-Lanes noTS");
//            connect( insert4Legx2, SIGNAL(triggered()),dtManip,SLOT(InsertNode_4x2_noTS()));
//            insertNodePopup->addAction( insert4Legx2 );

            QAction *insert3Legx1Left = new QAction();
            insert3Legx1Left->setText("3-Leg(B-Direction) 1-Lanes noTS");
            connect( insert3Legx1Left, SIGNAL(triggered()),dtManip,SLOT(InsertNode_3Lx1_noTS()));
            insertNodePopup->addAction( insert3Legx1Left );

//            QAction *insert3Legx2Left = new QAction();
//            insert3Legx2Left->setText("3-Leg(B-Direction) 2-Lanes noTS");
//            connect( insert3Legx2Left, SIGNAL(triggered()),dtManip,SLOT(InsertNode_3Lx2_noTS()));
//            insertNodePopup->addAction( insert3Legx2Left );


            QAction *insert3Legx1Right = new QAction();
            insert3Legx1Right->setText("3-Leg(D-Direction) 1-Lanes noTS");
            connect( insert3Legx1Right, SIGNAL(triggered()),dtManip,SLOT(InsertNode_3Rx1_noTS()));
            insertNodePopup->addAction( insert3Legx1Right );

//            QAction *insert3Legx2Right = new QAction();
//            insert3Legx2Right->setText("3-Leg(D-Direction) 2-Lanes noTS");
//            connect( insert3Legx2Right, SIGNAL(triggered()),dtManip,SLOT(InsertNode_3Rx2_noTS()));
//            insertNodePopup->addAction( insert3Legx2Right );

            insertNodePopup->exec( QCursor::pos() );
        }
    }
    else if( modifi & Qt::ControlModifier ){
        if( key == Qt::Key_Z ){
            dtManip->UndoOperation();
        }
        else if( key == Qt::Key_A ){
            dtManip->SelectAllNode();
        }
    }

    if( key == Qt::Key_1 ){
        canvas->SetNumberKeyPressed( 1 );
    }
    else if( key == Qt::Key_2 ){
        canvas->SetNumberKeyPressed( 2 );
    }
    else if( key == Qt::Key_3 ){
        canvas->SetNumberKeyPressed( 3 );
    }
    else if( key == Qt::Key_4 ){
        canvas->SetNumberKeyPressed( 4 );
    }
    else if( key == Qt::Key_5 ){
        canvas->SetNumberKeyPressed( 5 );
    }
    else if( key == Qt::Key_6 ){
        canvas->SetNumberKeyPressed( 6 );
    }
    else if( key == Qt::Key_7 ){
        canvas->SetNumberKeyPressed( 7 );
    }
    else if( key == Qt::Key_8 ){
        canvas->SetNumberKeyPressed( 8 );
    }
}



void MainWindow::SetRecentDataFile()
{
    recentOpenFiles.clear();

    QFile file("recentlyUsedSEditFiles.txt");
    if( file.open(QIODevice::ReadOnly | QIODevice::Text) ){

        QTextStream in(&file);

        while( in.atEnd() == false ){

            QString line = in.readLine();
            if( line.startsWith("#") || line.isEmpty() ){
                continue;
            }
            if( line.contains(".se.txt") == true ){
                if( recentOpenFiles.contains( line ) == false ){
                    recentOpenFiles.append( line );
                }
            }
        }

        file.close();
    }

    if( recentOpenFiles.contains(currentSEditFilename) == true ){
        int idx = recentOpenFiles.indexOf(currentSEditFilename);
        recentOpenFiles.removeAt(idx);
    }
    recentOpenFiles.prepend( currentSEditFilename );


    if( recentOpenFiles.size() >= 20 ){
        recentOpenFiles.removeLast();
    }

    if( file.open(QIODevice::WriteOnly | QIODevice::Text) ){

        QTextStream out(&file);

        for(int i=0;i<recentOpenFiles.size();++i){
            out << QString(recentOpenFiles[i]) << "\n";
        }

        file.close();
    }

    GetRecentDataFile();
}


void MainWindow::GetRecentDataFile()
{
    recentOpenFiles.clear();

    QFile file("recentlyUsedSEditFiles.txt");
    if( file.open(QIODevice::ReadOnly | QIODevice::Text) ){

        QTextStream in(&file);

        while( in.atEnd() == false ){

            QString line = in.readLine();
            if( line.startsWith("#") || line.isEmpty() ){
                continue;
            }
            if( line.contains(".se.txt") == true ){
                if( recentOpenFiles.contains( line ) == false ){
                    recentOpenFiles.append( line );
                }
            }
        }

        file.close();
    }

    if( recentFileOpen ){
        recentFileOpen->clear();
    }

    if( recentOpenFiles.size() > 0 ){

        QList<QAction*> actList;
        for(int i=0;i<recentOpenFiles.size();++i){
            QAction *act = new QAction( QString( recentOpenFiles[i] ) );
            act->setObjectName( QString( recentOpenFiles[i] ) );
            actList.append( act );
            connect(act, SIGNAL(triggered(bool)),this,SLOT(OpenRecentFile()));
        }
        recentFileOpen->addActions( actList );
    }
}


void MainWindow::OpenRecentFile()
{
    QString fileName = sender()->objectName();

    qDebug() << "[OpenRecentFile] fileName = " << fileName;


    if( currentSEditFilename == fileName ){
        return;
    }

    if( currentSEditFilename != QString() ){

        QMessageBox msgBox;
        msgBox.setText("Opening recent file clears the existing data.");
        msgBox.setInformativeText("Sure to open the file?");
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        if( ret == QMessageBox::Cancel ){
            return;
        }
        else{
            //
            // clear data
            NewFile();
        }
    }

    //
    // Open File
    OpenRoadDataWithFilename( fileName );
}


void MainWindow::OpenRoadDataWithFilename(QString fileName)
{
    if( road->LoadRoadData( fileName ) == true )
    {
        currentSEditFilename = fileName;

        setWindowModified(false);
        setWindowTitle(QString("MDS02-Canopus | S-Edit - %1[*]").arg(currentSEditFilename));

        SetRecentDataFile();

        if( road->nodes.size() > 0 ){

            if( roadObjProp->nodeIDSB->value() == road->nodes[0]->id ){
                roadObjProp->ChangeNodeInfo( road->nodes[0]->id );
                roadObjProp->update();
            }
            else{
                roadObjProp->nodeIDSB->setValue( road->nodes[0]->id );
            }

            if( road->nodes[0]->trafficSignals.size() > 0 ){
                if( roadObjProp->tsIDSB->value() == road->nodes[0]->trafficSignals[0]->id ){
                    roadObjProp->ChangeTrafficSignalInfo( road->nodes[0]->trafficSignals[0]->id );
                    roadObjProp->update();
                }
                else{
                    roadObjProp->tsIDSB->setValue( road->nodes[0]->trafficSignals[0]->id );
                }
            }
            if( road->nodes[0]->stopLines.size() > 0 ){
                if( roadObjProp->slIDSB->value() == road->nodes[0]->stopLines[0]->id ){
                    roadObjProp->ChangeStopLineInfo( road->nodes[0]->stopLines[0]->id );
                    roadObjProp->update();
                }
                else{
                    roadObjProp->slIDSB->setValue( road->nodes[0]->stopLines[0]->id );
                }
            }
        }
        if( road->lanes.size() > 0 ){
            if( roadObjProp->laneIDSB->value() == road->lanes[0]->id ){
                roadObjProp->ChangeLaneInfo( road->lanes[0]->id );
                roadObjProp->update();
            }
            else{
                roadObjProp->laneIDSB->setValue( road->lanes[0]->id );
            }
        }

        canvas->update();
    }
}


void MainWindow::UpdateStatusBar(QString message)
{
    statusBar()->showMessage(message);
}


