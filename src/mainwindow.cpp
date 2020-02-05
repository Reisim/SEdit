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
    canvas->setMinimumSize(800,600);
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

    //-------------------------
    mapImageMng = new BaseMapImageManager();
    mapImageMng->hide();

    //-------------------------
    resimOut = new ResimFilesOutput();
    resimOut->road = road;
    resimOut->hide();

    //-------------------------
    dispCtrl = new DisplayControl();

    connect( dispCtrl->OrthogonalView, SIGNAL(toggled(bool)), canvas, SLOT(SetProjectionOrthogonal(bool)) );
    connect( dispCtrl, SIGNAL(ViewMoveTo(float,float)), canvas, SLOT(MoveTo(float,float)) );
    connect( dispCtrl->resetRotate, SIGNAL(clicked()), canvas, SLOT(ResetRotate()) );

    connect( dispCtrl->showNodes, SIGNAL(toggled(bool)), canvas, SLOT(SetNodeVisibility(bool)) );
    connect( dispCtrl->showNodeLaneList, SIGNAL(toggled(bool)), canvas, SLOT(SetNodeLaneListlVisibility(bool)) );
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
    connect( dispCtrl->showLabels, SIGNAL(toggled(bool)), canvas, SLOT(SetLabelVisibility(bool)) );

    connect( dispCtrl->selectNode, SIGNAL(toggled(bool)), canvas, SLOT(SetNodeSelection(bool)) );
    connect( dispCtrl->selectLane, SIGNAL(toggled(bool)), canvas, SLOT(SetLaneSelection(bool)) );
    connect( dispCtrl->selectTrafficSignal, SIGNAL(toggled(bool)), canvas, SLOT(SetTrafficSignalSelection(bool)) );
    connect( dispCtrl->selectStopLine, SIGNAL(toggled(bool)), canvas, SLOT(SetStopLineSelection(bool)) );

    dispCtrl->move(50,50);
    dispCtrl->show();

    road->LeftOrRight = setDlg->GetCurrentLeftRightIndex();

    //-------------------------
    roadObjProp = new RoadObjectProperty();
    roadObjProp->road = road;

    roadObjProp->move(450,50);
    roadObjProp->show();

    canvas->roadProperty = roadObjProp;

    //-------------------------
    odRoute = new ODRouteEditor();
    odRoute->road = road;
    odRoute->propRO = roadObjProp;
    odRoute->setDlg = setDlg;
    odRoute->SetHeaderTrafficVolumeTable();
    odRoute->move(1010,50);
    odRoute->hide();

    connect( roadObjProp, SIGNAL(ShowODRouteEdit()), odRoute, SLOT(show()) );
    connect( roadObjProp, SIGNAL(HideODRouteEdit()), odRoute, SLOT(hide()) );
    connect( roadObjProp, SIGNAL(DestinationNodeChanged(int,bool)), odRoute, SLOT(SetCurrentODRouteData(int,bool)) );
    connect( roadObjProp, SIGNAL(OriginNodeChanged(int,bool)), odRoute, SLOT(SetCurrentODRouteData(int,bool)) );
    connect( roadObjProp, SIGNAL(ResetLaneListIndex()), canvas, SLOT(ResetLaneListIndex()) );

    connect( odRoute, SIGNAL(ShowMessageStatusBar(QString)), this, SLOT(UpdateStatusBar(QString)) );
    connect( odRoute, SIGNAL(SetNodePickMode(int,int)), canvas, SLOT(SetNodePickMode(int,int)) );
    connect( odRoute, SIGNAL(ResetNodePickMode()), canvas, SLOT(ResetNodePickMode()) );
    connect( canvas, SIGNAL(SetNodeListForRoute(QList<int>)), odRoute, SLOT(GetNodeListForRoute(QList<int>)) );

    //-------------------------
    configMgr = new ConfigFileManager();
    configMgr->hide();


    //
    //  Menubar
    //
    QMenu* fileMenu = menuBar()->addMenu( tr("&File") );

    QAction* newAct = new QAction( tr("&New"), this );
    newAct->setIcon(QIcon(":/images/new.png"));
    //newAct->setShortcut( tr("Ctrl+N") );
    //newAct->setStatusTip( tr("Create a New Map Data") );
    //connect( newAct, SIGNAL(triggered()), this, SLOT(newFile()));
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


    QAction* closeAct = new QAction( tr("&Exit"), this );
    closeAct->setIcon(QIcon(":/images/exit.png"));
    closeAct->setShortcut( tr("Ctrl+Q") );
    connect( closeAct, SIGNAL(triggered()), this, SLOT(close()));
    fileMenu->addAction( closeAct );


    QMenu* toolMenu = menuBar()->addMenu( tr("&Tool") );

    QAction* showMapImageDialogAct = new QAction( tr("&Show Map Image Dialog"), this );
    connect( showMapImageDialogAct, SIGNAL(triggered()), mapImageMng, SLOT(show()) );
    toolMenu->addAction( showMapImageDialogAct );

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
    createNodePopup = new QMenu();

    QAction *createNode_4x1x1_NoTS = new QAction();
    createNode_4x1x1_NoTS->setText("4-Leg 1x1 noTS");
    connect( createNode_4x1x1_NoTS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_4x1x1_noTS()));
    createNodePopup->addAction( createNode_4x1x1_NoTS );

    QAction *createNode_3x1x1_NoTS = new QAction();
    createNode_3x1x1_NoTS->setText("3-Leg 1x1 noTS");
    connect( createNode_3x1x1_NoTS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_3x1x1_noTS()));
    createNodePopup->addAction( createNode_3x1x1_NoTS );

    QAction *createNode_4x1x1_TS = new QAction();
    createNode_4x1x1_TS->setText("4-Leg 1x1 TS");
    connect( createNode_4x1x1_TS, SIGNAL(triggered()),dtManip,SLOT(CreateNode_4x1x1_TS()));
    createNodePopup->addAction( createNode_4x1x1_TS );

    insertNodePopup = new QMenu();

    utilityPopup = new QMenu();

    QAction *createWPData = new QAction();
    createWPData->setText("Create WP Data");
    connect( createWPData, SIGNAL(triggered()),dtManip,SLOT(CreateWPData()));
    utilityPopup->addAction( createWPData );

    QAction *setODTerm = new QAction();
    setODTerm->setText("Terminal Node -> OD Node");
    connect( setODTerm, SIGNAL(triggered()),dtManip,SLOT(SetODFlagOfTerminalNode()));
    utilityPopup->addAction( setODTerm );

    QAction *setAllLaneLists = new QAction();
    setAllLaneLists->setText("Set All Lane-Lists");
    connect( setAllLaneLists, SIGNAL(triggered()),dtManip,SLOT(SetAllLaneLists()));
    utilityPopup->addAction( setAllLaneLists );

    QAction *setTurnDirection = new QAction();
    setTurnDirection->setText("Set Turn-Direction Info");
    connect( setTurnDirection, SIGNAL(triggered()),dtManip,SLOT(SetTurnDirectionInfo()));
    utilityPopup->addAction( setTurnDirection );
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

        canvas->ResetNodePickMode();
        canvas->selectedObj.selObjKind.clear();
        canvas->selectedObj.selObjID.clear();
        canvas->SetNumberKeyPressed(-1);
        canvas->update();

        return;
    }

    if( modifi & Qt::AltModifier ){
        if( key == Qt::Key_N ){
            if( canvas->selectedObj.selObjKind.size() == 0 ){
                createNodePopup->exec( QCursor::pos() );
            }
        }
        else if( key == Qt::Key_U ){
            utilityPopup->exec( QCursor::pos() );
        }
        else if( key == Qt::Key_Delete ){
            dtManip->DeleteSelectedObject();
        }
        else if( key == Qt::Key_M ){
            dtManip->MergeSelectedObject();
        }
        else if( key == Qt::Key_I ){
            int checkInsertCondition = 0;
            if( canvas->selectedObj.selObjKind.size() == 2 &&
                    canvas->selectedObj.selObjKind[0] == canvas->SEL_NODE &&
                    canvas->selectedObj.selObjKind[1] == canvas->SEL_NODE ){

                // Selected two nodes should be connected
                int nd1 = canvas->selectedObj.selObjID[0];
                int nd2 = canvas->selectedObj.selObjID[1];
                int nd1Idx = road->indexOfNode( nd1 );
                int nd2Idx = road->indexOfNode( nd2 );
                if( nd1Idx >= 0 && nd2Idx >= 0 ){
                    for(int i=0;i<road->nodes[nd1Idx]->legInfo.size();++i){
                        if( road->nodes[nd1Idx]->legInfo[i]->connectingNode == nd2 ){
                            for(int j=0;j<road->nodes[nd2Idx]->legInfo.size();++j){
                                if( road->nodes[nd2Idx]->legInfo[j]->connectedNode == nd1 ){
                                    if( road->nodes[nd2Idx]->legInfo[j]->connectedNodeOutDirect == road->nodes[nd1Idx]->legInfo[i]->legID &&
                                            road->nodes[nd1Idx]->legInfo[i]->connectingNodeInDirect == road->nodes[nd2Idx]->legInfo[j]->legID ){
                                        checkInsertCondition += 1;
                                        break;
                                    }
                                }
                            }
                        }
                        if( checkInsertCondition > 0 ){
                            break;
                        }
                    }
                    for(int i=0;i<road->nodes[nd2Idx]->legInfo.size();++i){
                        if( road->nodes[nd2Idx]->legInfo[i]->connectingNode == nd1 ){
                            for(int j=0;j<road->nodes[nd1Idx]->legInfo.size();++j){
                                if( road->nodes[nd1Idx]->legInfo[j]->connectedNode == nd2 ){
                                    if( road->nodes[nd1Idx]->legInfo[j]->connectedNodeOutDirect == road->nodes[nd2Idx]->legInfo[i]->legID &&
                                            road->nodes[nd2Idx]->legInfo[i]->connectingNodeInDirect == road->nodes[nd1Idx]->legInfo[j]->legID ){
                                        checkInsertCondition += 2;
                                        break;
                                    }
                                }
                            }
                        }
                        if( checkInsertCondition > 1 ){
                            break;
                        }
                    }
                }
            }
            qDebug() << "checkInsertCondition = " << checkInsertCondition;

            if( checkInsertCondition > 0 ){
                dtManip->insertMode = checkInsertCondition;
                dtManip->insertNode1 = canvas->selectedObj.selObjID[0];
                dtManip->insertNode2 = canvas->selectedObj.selObjID[1];

                insertNodePopup->clear();

                QAction *insert4Legx1 = new QAction();
                insert4Legx1->setText("4-Leg 1-Lane noTS");
                connect( insert4Legx1, SIGNAL(triggered()),dtManip,SLOT(InsertNode_4x1_noTS()));
                insertNodePopup->addAction( insert4Legx1 );

                QAction *insert4Legx2 = new QAction();
                insert4Legx2->setText("4-Leg 2-Lanes noTS");
                connect( insert4Legx2, SIGNAL(triggered()),dtManip,SLOT(InsertNode_4x2_noTS()));
                insertNodePopup->addAction( insert4Legx2 );

                if( checkInsertCondition == 1 || checkInsertCondition == 3 ){

                    QAction *insert3Legx1Left = new QAction();
                    insert3Legx1Left->setText("3-Leg(Left-Side) 1-Lanes noTS");
                    connect( insert3Legx1Left, SIGNAL(triggered()),dtManip,SLOT(InsertNode_3Lx1_noTS()));
                    insertNodePopup->addAction( insert3Legx1Left );

                    QAction *insert3Legx2Left = new QAction();
                    insert3Legx2Left->setText("3-Leg(Left-Side) 2-Lanes noTS");
                    connect( insert3Legx2Left, SIGNAL(triggered()),dtManip,SLOT(InsertNode_3Lx2_noTS()));
                    insertNodePopup->addAction( insert3Legx2Left );

                }

                if( checkInsertCondition == 2 || checkInsertCondition == 3 ){

                    QAction *insert3Legx1Right = new QAction();
                    insert3Legx1Right->setText("3-Leg(Right-Side) 1-Lanes noTS");
                    connect( insert3Legx1Right, SIGNAL(triggered()),dtManip,SLOT(InsertNode_3Rx1_noTS()));
                    insertNodePopup->addAction( insert3Legx1Right );

                    QAction *insert3Legx2Right = new QAction();
                    insert3Legx2Right->setText("3-Leg(Right-Side) 2-Lanes noTS");
                    connect( insert3Legx2Right, SIGNAL(triggered()),dtManip,SLOT(InsertNode_3Rx2_noTS()));
                    insertNodePopup->addAction( insert3Legx2Right );

                }

                insertNodePopup->exec( QCursor::pos() );
            }
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


