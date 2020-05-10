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



#include "datamanipulator.h"

#include <QFile>
#include <QDataStream>
#include <QProgressDialog>
#include <QMessageBox>

#include <QDebug>


void DataManipulator::ImportERIS3Data(QString filename)
{
    qDebug() << "[DataManipulator::ImportERIS3Data] filename = " << filename;

    bool firstDataSet = false;
    float maxDataX = 0.0;
    float maxDataY = 0.0;
    float minDataX = 0.0;
    float minDataY = 0.0;

    QFile file(filename);
    if( !file.open(QIODevice::ReadOnly) ){
        qDebug() << "Cannot open file for read: " << qPrintable(file.errorString()) << endl;
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_6);

    int   itmp;
    float ftmp;
    bool  btmp;
    QString stmp;

    in >> itmp;   //  Drawing Dimension
    in >> ftmp;   //  Drawing(View) Scale

    in >> ftmp;   //  Perspective Parameters
    in >> ftmp;
    in >> ftmp;
    in >> ftmp;
    in >> ftmp;
    in >> ftmp;

    in >> ftmp;
    in >> ftmp;
    in >> ftmp;

    // Map Data
    int N;
    in >> N;         // Number of Map

    qDebug() << "Number of Map = " << N;

    int numMap = N;

    QStringList mapFileList;
    in >> mapFileList;

    QList<float> xMaps;
    QList<float> yMaps;
    QList<float> sMaps;

    for(int i=0;i<numMap;++i){
        float xmap,ymap,smap;
        in >> xmap;
        in >> ymap;
        in >> smap;

        xMaps.append( xmap );
        yMaps.append( ymap );
        sMaps.append( smap );

        if( firstDataSet == false ){
            maxDataX = xmap;
            minDataX = xmap;
            maxDataY = ymap;
            minDataY = ymap;
            firstDataSet = true;
        }
        else{
            maxDataX = maxDataX < xmap ? xmap : maxDataX;
            maxDataY = maxDataY < ymap ? ymap : maxDataY;
            minDataX = minDataX > xmap ? xmap : minDataX;
            minDataY = minDataY > ymap ? ymap : minDataY;
        }
    }

    if( numMap > 0 ){
        QProgressDialog *pd = new QProgressDialog("Loading Image ...", "Cancel", 0, numMap, 0);
        pd->setWindowModality(Qt::WindowModal);
        pd->setWindowIcon(QIcon(":images/SEdit-icon.png"));
        pd->show();

        pd->setValue(0);
        QApplication::processEvents();

        for(int i=0;i<numMap;++i){

            mapImageMng->AddMapImageFromFile( mapFileList.at(i), xMaps[i], yMaps[i], sMaps[i], 0.0 );

            pd->setValue(i+1);
            QApplication::processEvents();

            if( pd->wasCanceled() ){
                qDebug() << "Canceled.";
                break;
            }

            if( i + 1 == numMap ){
                qDebug() << "All Images loaded.";
            }
        }

        xMaps.clear();
        yMaps.clear();
        sMaps.clear();
    }


    // WP Data
    in >> N;
    in >> itmp;

    qDebug() << "Number of WP Data = " << N;

    QList<float> tmpWPx;
    QList<float> tmpWPy;
    QList<float> tmpWPz;
    QList<float> tmpWPpsi;
    QList<int> tmpWPRelatedNode;
    QList<int> tmpWPRelatedNodeDir;
    QList<bool> tmpWPIsWPin;
    QList<bool> tmpWPIsWPout;

    for(int i=0;i<N;++i){

        QVector4D *wp = new QVector4D;
        in >> btmp;

        in >> ftmp;
        tmpWPx.append( ftmp );

        in >> ftmp;
        tmpWPy.append( ftmp );

        in >> ftmp;
        tmpWPz.append( ftmp );

        in >> ftmp;
        tmpWPpsi.append( ftmp );

        in >> ftmp;
        in >> ftmp;

        in >> btmp;
        tmpWPIsWPin.append(btmp);

        in >> btmp;
        tmpWPIsWPout.append(btmp);

        in >> itmp;
        tmpWPRelatedNode.append( itmp );

        in >> itmp;
        tmpWPRelatedNodeDir.append( itmp );

        in >> itmp;
        for(int j=0;j<itmp;++j){
            int itmp2 = 0;
            in >> itmp2;
        }
        in >> itmp;
        for(int j=0;j<itmp;++j){
            int itmp2 = 0;
            in >> itmp2;
        }
        in >> itmp;
        for(int j=0;j<itmp;++j){
            int itmp2 = 0;
            in >> itmp2;
        }
        in >> itmp;
        for(int j=0;j<itmp;++j){
            int itmp2 = 0;
            in >> itmp2;
        }

        in >> btmp;
        in >> btmp;
    }

    qDebug() << "WP Data Read.";


    // Path Data
    in >> N;
    in >> itmp;

    qDebug() << "Number of Path Data = " << N;

    QList<int> tmpPathFromWP;
    QList<int> tmpPathToWP;
    QList<float> tmpPathWidth;
    QList<float> tmpPathSpeedLimit;

    for(int i=0;i<N;++i){

        in >> btmp;

        int sWPidx;
        in >> sWPidx;
        tmpPathFromWP.append( sWPidx );

        int eWPidx;
        in >> eWPidx;
        tmpPathToWP.append( eWPidx );

        float tLaneWidth;
        in >> tLaneWidth;
        tmpPathWidth.append( tLaneWidth );

        float tSpeedLimit;
        in >> tSpeedLimit;
        tmpPathSpeedLimit.append( tSpeedLimit );

        in >> ftmp;
    }

    qDebug() << "Path Data Read.";


    // Node Data
    in >> N;
    in >> itmp;

    qDebug() << "Number of Node Data = " << N;

    for(int i=0;i<N;++i){

        in >> btmp;

        float Xc;
        in >> Xc;

        float Yc;
        in >> Yc;

        int nCross;
        in >> nCross;

        QList<int> inlanes;
        QList<int> outlanes;
        QList<float> angleDir;
        QList<int> CNodeIn;
        QList<int> CNodeInArc;
        QList<int> CNodeOut;
        QList<int> CNodeOutArc;

        QList<QList<int>> inWPs;
        QList<QList<int>> outWPs;

        for(int j=0;j<nCross;++j){

            int itmp1,itmp2,itmp3,itmp4;
            float ftmp1;
            in >> itmp1;

            in >> ftmp1;
            angleDir.append( ftmp1 );

            in >> itmp2;   // nWPin
            inlanes.append( itmp2 );

            QList<int> inwp;
            for(int k=0;k<itmp2;++k){
                in >> itmp1;  // WPin
                inwp.append(itmp2);
            }
            inWPs.append( inwp );


            in >> itmp1;   // nWPout
            outlanes.append( itmp1 );

            QList<int> outwp;
            for(int k=0;k<itmp1;++k){
                in >> itmp2;  // WPout
                outwp.append( itmp2 );
            }
            outWPs.append( outwp );


            in >> itmp1;  // CNodeIn
            CNodeIn.append( itmp1 );

            in >> itmp2;  // CNodeInArc
            CNodeInArc.append( itmp2 );

            in >> itmp3;  // CNodeOut
            CNodeOut.append( itmp3 );

            in >> itmp4;  // CNodeOutArc
            CNodeOutArc.append( itmp4 );
        }

        in >> btmp;
        in >> btmp;
        in >> itmp;

        in >> btmp;  // roundAbout

        int MC;
        in >> MC;
        for(int j=0;j<MC;++j){
            in >> itmp;  // RoundAboutInsideWPs
        }


        road->CreateNode( i, Xc, Yc, inlanes, outlanes );
        int nIdx = road->indexOfNode( i );
        if( nIdx >= 0 ){
            for(int j=0;j<road->nodes[nIdx]->legInfo.size();++j){
                road->nodes[nIdx]->legInfo[j]->legID = j;
                road->nodes[nIdx]->legInfo[j]->angle = angleDir[j];

                road->nodes[nIdx]->legInfo[j]->connectedNode = CNodeIn[j];
                road->nodes[nIdx]->legInfo[j]->connectedNodeOutDirect = CNodeInArc[j];
                road->nodes[nIdx]->legInfo[j]->connectingNode = CNodeOut[j];
                road->nodes[nIdx]->legInfo[j]->connectingNodeInDirect = CNodeOutArc[j];

                for(int k=0;k<inWPs[j].size();++k){
                    road->nodes[nIdx]->legInfo[j]->inWPs.append( inWPs[j][k] );
                }
                for(int k=0;k<outWPs[j].size();++k){
                    road->nodes[nIdx]->legInfo[j]->outWPs.append( outWPs[j][k] );
                }
            }
        }

        if( firstDataSet == false ){
            maxDataX = Xc;
            minDataX = Xc;
            maxDataY = Yc;
            minDataY = Yc;
            firstDataSet = true;
        }
        else{
            maxDataX = maxDataX < Xc ? Xc : maxDataX;
            maxDataY = maxDataY < Yc ? Yc : maxDataY;
            minDataX = minDataX > Xc ? Xc : minDataX;
            minDataY = minDataY > Yc ? Yc : minDataY;
        }

        inlanes.clear();
        outlanes.clear();
        angleDir.clear();
        CNodeIn.clear();
        CNodeInArc.clear();
        CNodeOut.clear();
        CNodeOutArc.clear();
    }

    qDebug() << "Node Data Read.";


    // Stopline Data
    in >> N;
    in >> itmp;

    qDebug() << "Number of StopLine Data = " << N;

    QList<float> tmpStopLineXL;
    QList<float> tmpStopLineXR;
    QList<float> tmpStopLineYL;
    QList<float> tmpStopLineYR;
    QList<int> tmpStopLineType;
    QList<int> tmpStopLineRelatedNode;

    for(int i=0;i<N;++i){

        in >> btmp;
        qint8 temp8qi;
        in >> temp8qi;
        tmpStopLineType.append( (int)temp8qi );

        in >> ftmp;  // x;
        tmpStopLineXL.append( ftmp );

        in >> ftmp;  // xr
        tmpStopLineXR.append( ftmp );

        in >> ftmp;  // yl
        tmpStopLineYL.append( ftmp );

        in >> ftmp;  // yr
        tmpStopLineYR.append( ftmp );

        in >> itmp;  // relatedNode
        tmpStopLineRelatedNode.append( itmp );
    }

    qDebug() << "StopLine Data Read.";


    // Traffic Signal Data
    in >> N;
    in >> itmp;

    qDebug() << "Number of Traffic Signal Data  = " << N;

    QList<float> tmpTSPosX;
    QList<float> tmpTSPosY;
    QList<float> tmpTSPosZ;
    QList<float> tmpTSPsi;

    for(int i=0;i<N;++i){


        in >> btmp;

        in >> ftmp;   // x
        tmpTSPosX.append( ftmp );

        in >> ftmp;   // y
        tmpTSPosY.append( ftmp );

        in >> ftmp;   // z
        tmpTSPosZ.append( ftmp );

        in >> ftmp;   // psi
        tmpTSPsi.append( ftmp );


        qint8 temp8qi;
        in >> temp8qi;  // TSType

//        if( temp8qi == 0 )
//            pts->TSType = 'v';
//        else
//            pts->TSType = (char)temp8qi;

        in >> ftmp;  // height
        in >> ftmp;  // width
        in >> ftmp;  // length
        in >> itmp;


    }

    qDebug() << "Traffic Signal Data Read.";


    // Static Object Data
    in >> N;
    in >> itmp;

    for(int i=0;i<N;++i){
        in >> btmp;
        in >> itmp;
        in >> itmp;
        in >> ftmp;
        in >> ftmp;
        in >> ftmp;
        in >> ftmp;
        for(int j=0;j<4;++j){
            in >> ftmp;
            in >> ftmp;
            in >> ftmp;
        }
    }


    // Traffic Signal Pattern & Assignment Data
    in >> N;
    for(int i=0;i<N;++i){
        int M ;
        in >> M;
        for(int j=0;j<M;++j){
            for(int k=0;k<8;++k){
                bool ON_OFF;
                in >> ON_OFF;
            }
        }
    }
    in >> N;
    for(int i=0;i<N;++i){
        in >> itmp;
        in >> itmp;
        int M;
        in >> M;
        for(int j=0;j<M;++j){
            int n;
            in >> n;
            for(int k=0;k<n;++k){
                float dt;
                in >> dt;
            }
            in >> itmp;
            in >> itmp;
        }
    }
    in >> N;
    for(int i=0;i<N;++i){
        in >> itmp;
        in >> itmp;
        in >> itmp;
        for(int j=0;j<itmp;++j){
            in >> stmp;
        }
    }


    // Agent Generate Data
    in >> N;
    for(int i=0;i<N;++i){
        in >> stmp;
    }
    in >> N;
    for(int i=0;i<N;++i){
        in >> itmp;
        in >> itmp;
        int M;
        in >> M;
        for(int j=0;j<M;++j){
            in >> ftmp;
            in >> ftmp;
            in >> itmp;
            in >> itmp;
        }
    }


    // OD Data
    int nO = 0;
    in >> nO;
    for(int i=0;i<nO;++i){
        in >> itmp;
    }
    int nD = 0;
    in >> nD;
    for(int i=0;i<nD;++i){
        in >> itmp;
    }
    in >> N;
    for(int i=0;i<N;++i){
        for(int j=0;j<nO;++j){
            in >> itmp;
            for(int k=0;k<nD;++k){
                in >> itmp;
                in >> ftmp;
                in >> ftmp;
                in >> ftmp;
                in >> ftmp;
            }
        }
    }
    in >> N;
    for(int i=0;i<N;++i){
        in >> itmp;
        in >> ftmp;
        in >> itmp;
        in >> itmp;
    }


    // Route Data
    in >> N;
    for(int i=0;i<N;++i){
        in >> itmp;
        int M;
        in >> M;
        for(int j=0;j<M;++j){
            in >> itmp;
            in >> ftmp;
            int L;
            in >> L;
            for(int k=0;k<L;++k){
                in >> itmp;
                in >> itmp;
                in >> itmp;
            }
        }
    }


    // Polygon Data
    in >> N;
    if( N > 0 ){
        for(int i=0;i<N;++i){
            in >> stmp;
        }
    }
    in >> N;
    if( N > 0 ){
        for(int i=0;i<N;++i){
            in >> stmp;
            in >> stmp;
            float xm = 0.0,ym = 0.0,zm = 0.0,pm = 0.0,sm = 1.0;
            in >> xm;
            in >> ym;
            in >> zm;
            in >> pm;
            in >> sm;
            in >> itmp;
        }
    }


    // Vehicle Data
    in >> N;
    for(int i=0;i<N;++i){
        int M;
        in >> M;
        for(int j=0;j<M;++j){
            in >> stmp;
            in >> ftmp;
            in >> ftmp;
            in >> ftmp;
            in >> ftmp;
            in >> ftmp;
            in >> ftmp;
            in >> itmp;
            if( itmp > 0 )
                in >> stmp;
        }
    }

    // Map Rotation
    for(int i=0;i<numMap;++i){
        in >> ftmp;
    }


    if( file.atEnd() ){
        goto SKIP_POSITION;
    }

    // related Direction of stopLine
    for(int i=0;i<tmpStopLineType.size();++i){
        in >> itmp;
    }


    // Extended Static Objects
    in >> N;
    in >> itmp;

    for(int i=0;i<N;++i){
        in >> btmp;
        in >> itmp;
        int AS = 0;
        in >> AS;
        for(int j=0;j<AS;++j){
            in >> ftmp;
            in >> ftmp;
            in >> ftmp;
            in >> ftmp;
            in >> itmp;
        }
    }

    if( file.atEnd() ){
        goto SKIP_POSITION;
    }

    for(int i=0;i<N;++i){
        in >> itmp;
    }

    if( file.atEnd() ){
        goto SKIP_POSITION;
    }

    // Path Edge Information
    in >> N;
    int max_lane_marker_path;
    in >> max_lane_marker_path;

    for(int i=0;i<N;++i){
        for(int j=0;j<max_lane_marker_path;++j){
            in >> itmp;
            in >> ftmp;
            in >> itmp;
            in >> ftmp;
        }
    }

    if( file.atEnd() ){
        goto SKIP_POSITION;
    }

    // Layer Data
    in >> N;
    int max_layer;
    in >> max_layer;
    for(int i=0;i<N;++i){
        int layerNo;
        in >> layerNo;
        in >> ftmp;
        in >> ftmp;
        in >> ftmp;
        in >> itmp;
    }

    if( file.atEnd() ){
        goto SKIP_POSITION;
    }


    // Object Layer Data
    in >> N;
    for(int i=0;i<N;++i){
        in >> itmp;
    }

    in >> N;
    for(int i=0;i<N;++i){
        in >> itmp;
    }

    in >> N;
    for(int i=0;i<N;++i){
        in >> itmp;
    }

    if( file.atEnd() ){
        goto SKIP_POSITION;
    }


    // Additional Layer Data
    in >> N;
    for(int i=0;i<N;++i){
        in >> ftmp;
    }

    if( file.atEnd() ){
        goto SKIP_POSITION;
    }


    // polygon index
    in >> N;
    for(int i=0;i<N;++i){
        in >> itmp;
    }

    if( file.atEnd() ){
        goto SKIP_POSITION;
    }


    // traffic signal data: offset
    in >> N;
    for(int i=0;i<N;++i){
        in >> btmp;
        in >> itmp;
        in >> itmp;
    }

    if( file.atEnd() ){
        goto SKIP_POSITION;
    }


    // Traffic Signal CG Model
    for(int i=0;i<N;++i){
        in >> itmp;
        int idx = itmp;
        in >> itmp;
    }

    if( file.atEnd() ){
        goto SKIP_POSITION;
    }


    // Pedest Path
    in >> N;
    in >> itmp;
    for(int i=0;i<N;++i){

        in >> btmp;
        in >> ftmp;  // x1
        in >> ftmp;  // y1
        in >> ftmp;  // tht1
        in >> ftmp;  // x2
        in >> ftmp;  // y2
        in >> ftmp;  // tht2
        in >> ftmp;  // width
        in >> ftmp;  // adjust param
        in >> itmp;  // isCrossWalk
        in >> btmp;  // isRightSideRoad;
        in >> btmp;  // isLeftSideRoad;

    }


    SKIP_POSITION:

    in >> N;
    QList<int> tmpWPIDs;
    for(int i=0;i<N;++i){
        in >> itmp;
        tmpWPIDs.append( itmp );
        in >> btmp;
    }

    in >> N;
    QList<int> tmpPathIDs;
    for(int i=0;i<N;++i){
        in >> itmp;
        tmpPathIDs.append( itmp );
    }

    in >> N;
    QList<int> tmpNodeIDs;
    for(int i=0;i<N;++i){
        in >> itmp;
        in >> ftmp;
        tmpNodeIDs.append( itmp );
        road->nodes[i]->id = itmp;
    }

    in >> N;
    QList<int> tmpStopLineIDs;
    QList<int> tmpStopLineRelatedDirect;
    for(int i=0;i<N;++i){
        in >> itmp;
        tmpStopLineIDs.append( itmp );
        in >> itmp;
        tmpStopLineRelatedDirect.append( itmp );
    }

    in >> N;
    QList<int> tmpTSIDs;
    QList<int> tmpTSRelatedNode;
    QList<int> tmpTSRelatedDirect;
    QList<int> tmpTSRelatedLane;
    for(int i=0;i<N;++i){
        in >> itmp;
        tmpTSIDs.append( itmp );
        in >> itmp;
        tmpTSRelatedNode.append( itmp );
        in >> itmp;
        tmpTSRelatedDirect.append( itmp );
        in >> itmp;
        tmpTSRelatedLane.append( itmp );
    }

    in >> btmp;
    in >> stmp;

    in >> N;
    for(int i=0;i<N;++i){
        in >> itmp;
        in >> ftmp;
        in >> ftmp;
    }

    in >> N;
    for(int i=0;i<N;++i){
        float w,h;
        in >> w;
        in >> h;
    }

    // LineRoad data
    if( in.atEnd() == false ){
        for(int n=0;n<5;++n){
            int nL;
            in >> nL;
            for(int j=0;j<nL;++j){
                int id2,nP;
                in >> id2;
                in >> nP;
                for(int k=0;k<nP;++k){
                    int pn;
                    float x,y,z;
                    in >> pn;
                    in >> x;
                    in >> y;
                    in >> z;
                }
            }
        }
    }

    if( in.atEnd() == false ){
        in >> N;
        for(int i=0;i<N;++i){
            int type,id;
            in >> type;
            in >> id;
        }
    }

    if( in.atEnd() == false ){
        in >> N;
        for(int i=0;i<N;++i){
            bool bv1,bv2;
            in >> bv1;
            in >> bv2;
        }
    }

    if( in.atEnd() == false ){
        in >> N;
        for(int i=0;i<N;++i){
            int order;
            in >> order;
        }
    }

    if( in.atEnd() == false ){
        in >> N;
        for(int i=0;i<N;++i){
            in >> btmp;    // heightSet
            if( btmp == true ){
                for(int j=0;j<=10;++j){
                    in >> ftmp;   // Zp
                }
            }
        }
    }

    if( in.atEnd() == false ){
        in >> N;
        for(int i=0;i<N;++i){
            in >> btmp;   // isInvisiblePPath
        }
    }

    QList<float> tmpPathDriverErrorProb;
    QList<bool> tmpPathAutomaticDriving;
    if( in.atEnd() == false ){
        in >> N;
        for(int i=0;i<N;++i){
            in >> ftmp;
            tmpPathDriverErrorProb.append( ftmp );
            in >> btmp;
            tmpPathAutomaticDriving.append( btmp );
        }
    }

    file.close();



    QApplication::processEvents();

    {
        QProgressDialog *pd = new QProgressDialog("Creating Lanes ...", "Cancel", 0, tmpPathFromWP.size(), 0);
        pd->setWindowModality(Qt::WindowModal);
        pd->setWindowIcon(QIcon(":images/SEdit-icon.png"));
        pd->show();

        pd->setValue(0);
        QApplication::processEvents();

        for(int i=0;i<tmpPathIDs.size();++i){

            QVector4D startWP;
            QVector4D endWP;

            int sWPidx = tmpWPIDs.indexOf( tmpPathFromWP[i] );
            int eWPidx = tmpWPIDs.indexOf( tmpPathToWP[i] );

            startWP.setX( tmpWPx[sWPidx] );
            startWP.setY( tmpWPy[sWPidx] );
            startWP.setZ( tmpWPz[sWPidx] );
            startWP.setW( tmpWPpsi[sWPidx] * 0.017452 );

            endWP.setX( tmpWPx[eWPidx] );
            endWP.setY( tmpWPy[eWPidx] );
            endWP.setZ( tmpWPz[eWPidx] );
            endWP.setW( tmpWPpsi[eWPidx] * 0.017452 );

            int lId = road->CreateLane( tmpPathIDs[i],
                                        startWP,
                                        tmpWPRelatedNode[sWPidx],
                                        tmpWPRelatedNodeDir[sWPidx],
                                        (tmpWPIsWPin[sWPidx] == true || tmpWPIsWPout[sWPidx] == true ) ? true : false,
                                        endWP,
                                        tmpWPRelatedNode[eWPidx],
                                        tmpWPRelatedNodeDir[eWPidx],
                                        (tmpWPIsWPin[eWPidx] == true || tmpWPIsWPout[eWPidx] == true ) ? true : false );

            if( lId >= 0 ){
                int lIdx = road->indexOfLane(lId);
                if( lIdx >= 0 ){
                    road->lanes[lIdx]->laneWidth = tmpPathWidth[i];
                    road->lanes[lIdx]->speedInfo = tmpPathSpeedLimit[i];
                    road->lanes[lIdx]->actualSpeed = road->lanes[lIdx]->speedInfo;
                    road->lanes[lIdx]->automaticDrivingEnabled = tmpPathAutomaticDriving[i];
                    road->lanes[lIdx]->driverErrorProb = tmpPathDriverErrorProb[i];

                    if( road->lanes[lIdx]->connectedNode >= 0 ){
                        road->SetNodeRelatedLane( road->lanes[lIdx]->connectedNode, lId );
                    }
                    if( road->lanes[lIdx]->departureNode >= 0 ){
                        road->SetNodeRelatedLane( road->lanes[lIdx]->departureNode, lId );
                    }
                }
            }

            pd->setValue(i+1);
            QApplication::processEvents();

            if( pd->wasCanceled() ){
                qDebug() << "Canceled.";
                break;
            }

            if( i + 1 == tmpPathIDs.size() ){
                qDebug() << "Lanes created.";
            }
        }
    }


    {
        QProgressDialog *pd = new QProgressDialog("Creating StopLines ...", "Cancel", 0, tmpStopLineType.size(), 0);
        pd->setWindowModality(Qt::WindowModal);
        pd->setWindowIcon(QIcon(":images/SEdit-icon.png"));
        pd->show();

        pd->setValue(0);
        QApplication::processEvents();

        for(int i=0;i<tmpStopLineType.size();++i){

    //        qDebug() << "ID=" << tmpStopLineIDs[i] <<  " SLType = " << tmpStopLineType[i] << " Node=" << tmpStopLineRelatedNode[i] << " Dir=" << tmpStopLineRelatedDirect[i];

            pd->setValue(i+1);
            QApplication::processEvents();


            if( tmpStopLineType[i] != 0 && tmpStopLineType[i] != 4 && tmpStopLineType[i] != 5 ){
                continue;
            }

            road->CreateStopLine( tmpStopLineIDs[i],
                                  tmpStopLineRelatedNode[i],
                                  tmpStopLineRelatedDirect[i],
                                  _STOPLINE_KIND::STOPLINE_SIGNAL );

            int nIdx = road->indexOfNode( tmpStopLineRelatedNode[i] );
            int slIdx = road->indexOfSL( tmpStopLineIDs[i], tmpStopLineRelatedNode[i] );

    //        qDebug() << "Create SL: nIdx = " << nIdx << " slIdx = " << slIdx;

            if( nIdx >= 0 && slIdx >= 0 ){
                road->nodes[nIdx]->stopLines[slIdx]->leftEdge.setX( tmpStopLineXL[i] );
                road->nodes[nIdx]->stopLines[slIdx]->leftEdge.setY( tmpStopLineYL[i] );
                road->nodes[nIdx]->stopLines[slIdx]->rightEdge.setX( tmpStopLineXR[i] );
                road->nodes[nIdx]->stopLines[slIdx]->rightEdge.setY( tmpStopLineYR[i] );

    //            qDebug() << "leftEdge = " << road->nodes[nIdx]->stopLines[slIdx]->leftEdge;
    //            qDebug() << "rightEdge = " << road->nodes[nIdx]->stopLines[slIdx]->rightEdge;

            }


            if( pd->wasCanceled() ){
                qDebug() << "Canceled.";
                break;
            }
        }
    }


    {
        QProgressDialog *pd = new QProgressDialog("Creating Traffic Signals ...", "Cancel", 0, tmpTSIDs.size(), 0);
        pd->setWindowModality(Qt::WindowModal);
        pd->setWindowIcon(QIcon(":images/SEdit-icon.png"));
        pd->show();

        pd->setValue(0);
        QApplication::processEvents();

        for(int i=0;i<tmpTSIDs.size();++i){

            road->CreateTrafficSignal( tmpTSIDs[i], tmpTSRelatedNode[i], tmpTSRelatedDirect[i], 0 );

            int nIdx = road->indexOfNode( tmpTSRelatedNode[i] );
            int tsIdx = road->indexOfTS( tmpTSIDs[i], tmpTSRelatedNode[i] );
            if( nIdx >= 0 && tsIdx >= 0 ){
                road->nodes[nIdx]->trafficSignals[tsIdx]->pos.setX( tmpTSPosX[i] );
                road->nodes[nIdx]->trafficSignals[tsIdx]->pos.setY( tmpTSPosY[i] );
                road->nodes[nIdx]->trafficSignals[tsIdx]->pos.setZ( tmpTSPosZ[i] );
                road->nodes[nIdx]->trafficSignals[tsIdx]->facingDirect = tmpTSPsi[i];
            }

            pd->setValue(i+1);
            QApplication::processEvents();

            if( pd->wasCanceled() ){
                qDebug() << "Canceled.";
                break;
            }

            if( i + 1 == tmpTSIDs.size() ){
                qDebug() << "Lanes created.";
            }
        }

    }


    for(int i=0;i<road->nodes.size();++i){
        road->nodes[i]->nLeg  = road->nodes[i]->legInfo.size();
        road->nodes[i]->hasTS = road->nodes[i]->trafficSignals.size() > 0 ? true : false;
    }


    bool ret;

    // Set Next and Previous Lanes
    ret = road->CheckLaneConnectionFull();
    if( ret == true ){

        while(1){

            bool CNAllSet = true;
            for(int i=0;i<road->lanes.size();++i){
                if( road->lanes[i]->connectedNode < 0 ){
                    CNAllSet = false;

                    for(int j=0;j<road->lanes[i]->nextLanes.size();++j){
                        int nlIdx = road->indexOfLane( road->lanes[i]->nextLanes[j] );
                        if( nlIdx >= 0 ){
                            if( road->lanes[nlIdx]->connectedNode >= 0 ){
                                road->lanes[i]->connectedNode = road->lanes[nlIdx]->connectedNode;
                                road->lanes[i]->connectedNodeInDirect = road->lanes[nlIdx]->connectedNodeInDirect;
                                road->SetNodeRelatedLane( road->lanes[i]->connectedNode, road->lanes[i]->id );
                                break;
                            }
                        }
                    }
                }
                if( road->lanes[i]->departureNode < 0 ){
                    CNAllSet = false;

                    for(int j=0;j<road->lanes[i]->previousLanes.size();++j){
                        int plIdx = road->indexOfLane( road->lanes[i]->previousLanes[j] );
                        if( plIdx >= 0 ){
                            if( road->lanes[plIdx]->departureNode >= 0 ){
                                road->lanes[i]->departureNode = road->lanes[plIdx]->departureNode;
                                road->lanes[i]->departureNodeOutDirect = road->lanes[plIdx]->departureNodeOutDirect;
                                road->SetNodeRelatedLane( road->lanes[i]->departureNode, road->lanes[i]->id );
                                break;
                            }
                        }
                    }
                }
            }

            if( CNAllSet == true ){
                break;
            }
        }

        for(int i=0;i<road->lanes.size();++i){
            road->SetNodeRelatedLane( road->lanes[i]->connectedNode, road->lanes[i]->id );
            if( road->lanes[i]->connectedNode != road->lanes[i]->departureNode ){
                road->SetNodeRelatedLane( road->lanes[i]->departureNode, road->lanes[i]->id );
            }
        }

    }
    else {
        return;
    }


    // Calculate Stop Point Data
    road->CheckAllStopLineCrossLane();


    // Calculate Lane Cross Points
    road->CheckLaneCrossPoints();


    // Create WP and Boundary WP Check
    road->CreateWPData();


    // Set Lane List
    road->SetAllLaneLists();


    // Set Turn Direction Info
    {
        QList<int> nodeList;
        for(int i=0;i<road->nodes.size();++i){
            nodeList.append( i );
        }
        road->SetTurnDirectionInfo(nodeList);
    }


    float xmean = (maxDataX + minDataX) / 2;
    float ymean = (maxDataY + minDataY) / 2;

    qDebug() << "Move to " << xmean << "," << ymean;

    canvas->MoveTo( xmean, ymean );
}


void DataManipulator::ImportERIS3TrafficSignalData(QString filename)
{
    // Check if Traffic Signals exist
    QList<int> TSnodeIdx;

    struct TSIndexData
    {
        int TSID;
        int ndIdx;
        int tsIdx;
    };

    QList<struct TSIndexData> TSIDList;

    for(int i=0;i<road->nodes.size();++i){
        if( road->nodes[i]->trafficSignals.size() > 0 ){
            TSnodeIdx.append( i );
            for(int j=0;j<road->nodes[i]->trafficSignals.size();++j){

                struct TSIndexData t;
                t.TSID = road->nodes[i]->trafficSignals[j]->id;
                t.ndIdx = i;
                t.tsIdx = j;

                TSIDList.append( t );
            }
        }
    }
    if( TSnodeIdx.size() == 0 ){
        return;
    }


    QFile file(filename);
    if( !file.open(QIODevice::ReadOnly) ){
        qDebug() << "Cannot open file for writing: " << qPrintable(file.errorString()) << endl;
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_6);

    QString stmp;

    in >> stmp;

    int nNodeTS;
    in >> nNodeTS;

    QStringList NodeIDs;
    for(int i=0;i<nNodeTS;++i){
        QString strTmp;
        in >> strTmp;
        NodeIDs << strTmp;
    }

    if( TSnodeIdx.size() != NodeIDs.size() ){
        QMessageBox::critical(NULL,"Data Mismatch","Number of Nodes with Traffic Signal is not the same in eris3 and spd data.");
        return;
    }


    int Nheader;
    in >> Nheader;

    QStringList HeaderStrs;
    for(int i=0;i<Nheader;++i){
        QString tmpStr;
        in >> tmpStr;
        HeaderStrs << tmpStr;
    }

    bool headerCheck = true;
    for(int i=0;i<Nheader;++i){
        bool isHit = false;
        for(int j=0;j<TSIDList.size();++j){
            if( QString(HeaderStrs[i]).remove("TS ").trimmed().toInt() == TSIDList[j].TSID ){
                isHit = true;
                break;
            }
            if( isHit == false ){
                headerCheck = false;
                break;
            }
        }
    }
    if( headerCheck == false ){
        QMessageBox::critical(NULL,"Data Mismatch","List of Traffic Signal Data is not the same in eris3 and spd data.");
        return;
    }


    int N;
    in >> N;

    qDebug() << "N = " << N;

    QList<int> offsetTimes;
    for(int i=0;i<N;++i){
        int offsetTime;
        in >> offsetTime;

        qDebug() << "offsetTime = " << offsetTime;

        offsetTimes.append( offsetTime );
    }


    in >> N;

    qDebug() << "N = " << N;
    QList<int> numTSInNode;
    int idx = 0;
    for(int i=0;i<N;++i){
        int rC;
        int cC;
        in >> rC >> cC;

        for(int j=0;j<TSIDList.size();++j){
            if( QString(HeaderStrs[i]).remove("TS ").trimmed().toInt() == TSIDList[j].TSID ){
                idx = j;
                break;
            }
        }

        //qDebug() << " rC = " << rC << " cC = " << cC;
        numTSInNode.append( rC );

        for(int j=0;j<rC;++j){

            int ndIdx = TSIDList[idx].ndIdx;
            int tsIdx = TSIDList[idx].tsIdx;

            for(int k=0;k<road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.size();++k){
                delete road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[k];
            }
            road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.clear();

            for(int k=0;k<cC;++k){

                QString txt;
                in >> txt;
                //qDebug() << "j=" << j << " k=" << k << " txt=" << txt;  // display

                struct SignalPatternData* sp = new struct SignalPatternData;
                if( txt.trimmed() == QString("B") ){
                    sp->signal = 1;
                }
                else if( txt.trimmed() == QString("Y") ){
                    sp->signal = 2;
                }
                else if( txt.trimmed() == QString("R") ){
                    sp->signal = 4;
                }
                else if( txt.trimmed() == QString("LA/Y") ){
                    sp->signal = 2 + 8;
                }
                else if( txt.trimmed() == QString("SA/Y") ){
                    sp->signal = 2 + 16;
                }
                else if( txt.trimmed() == QString("RA/Y") ){
                    sp->signal = 2 + 32;
                }
                else if( txt.trimmed() == QString("LS/Y") ){
                    sp->signal = 2 + 8 + 16;
                }
                else if( txt.trimmed() == QString("SR/Y") ){
                    sp->signal = 2 + 16 + 32;
                }
                else if( txt.trimmed() == QString("LR/Y") ){
                    sp->signal = 2 + 8 + 32;
                }
                else if( txt.trimmed() == QString("AA/Y") ){
                    sp->signal = 2 + 8 + 16 + 32;
                }
                else if( txt.trimmed() == QString("LA") ){
                    sp->signal = 4 + 8;
                }
                else if( txt.trimmed() == QString("SA") ){
                    sp->signal = 4 + 16;
                }
                else if( txt.trimmed() == QString("RA") ){
                    sp->signal = 4 + 32;
                }
                else if( txt.trimmed() == QString("LS") ){
                    sp->signal = 4 + 8 + 16;
                }
                else if( txt.trimmed() == QString("SR") ){
                    sp->signal = 4 + 16 + 32;
                }
                else if( txt.trimmed() == QString("LR") ){
                    sp->signal = 4 + 8 + 32;
                }
                else if( txt.trimmed() == QString("AA") ){
                    sp->signal = 4 + 8 + 16 + 32;
                }

                road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.append( sp );

                if( txt.compare("NULL") == 0 )
                    continue;
            }
        }
    }


    in >> N;
    idx = 0;
    for(int i=0;i<N;++i){
        int rC;
        int cC;
        in >> rC >> cC;

        //qDebug() << " rC = " << rC << " cC = " << cC;
        QList<int> dur;

        for(int j=0;j<TSIDList.size();++j){
            if( QString(HeaderStrs[i]).remove("TS ").trimmed().toInt() == TSIDList[j].TSID ){
                idx = j;
                break;
            }
        }

        for(int j=0;j<rC;++j){
            for(int k=0;k<cC;++k){

                QString txt;
                in >> txt;
                //qDebug() << "j=" << j << " k=" << k << " txt=" << txt;  // cycle

                if( j == 0 ){
                    dur.append( txt.trimmed().toInt() );
                }
                if( txt.compare("NULL") == 0 )
                    continue;
            }
        }

        for(int j=0;j<numTSInNode[i];++j){

            int ndIdx = TSIDList[idx].ndIdx;
            int tsIdx = TSIDList[idx].tsIdx;

            for(int k=0;k<dur.size();++k){
                road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[k]->duration = dur[k];
            }

            road->nodes[ndIdx]->trafficSignals[tsIdx]->startOffset = offsetTimes[i];

            idx++;
        }
    }

    file.close();


    canvas->update();
}


void DataManipulator::ImportERIS3ODData(QString filename)
{
    QFile file(filename);
    if( !file.open(QIODevice::ReadOnly) ){
        qDebug() << "Cannot open file: " << filename;
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_6);

    QString roadDataFile;
    in >> roadDataFile;


    // load eris3 file to relate WP and Node
    struct tmpWPData
    {
        int id;
        float node;
        float xc;
        float yc;
    };

    QList<struct tmpWPData> wpOutData;

    {
        qDebug() << "Load : " << roadDataFile;

        QFile file_eris3(roadDataFile);
        if( !file_eris3.open(QIODevice::ReadOnly) ){
            qDebug() << "Cannot open file : " << file_eris3;

            roadDataFile = QFileDialog::getOpenFileName(NULL,
                                                        tr("Assign ERIS3 File"),
                                                        QString(),
                                                        tr("ERIS3 Data files(*.eris3)"));

            if( roadDataFile.isEmpty() ){
                qDebug() << "Canceled.";
                return;
            }

            file_eris3.setFileName( roadDataFile );
            if( !file_eris3.open(QIODevice::ReadOnly) ){
                qDebug() << "Cannot open file : " << file_eris3;
                return;
            }
        }

        QDataStream in_eris3(&file_eris3);
        in_eris3.setVersion(QDataStream::Qt_4_6);

        int   itmp;
        float ftmp;
        bool  btmp;
        int N;

        in_eris3 >> itmp;
        in_eris3 >> ftmp;

        for(int i=0;i<9;++i){
            in_eris3 >> ftmp;
        }

        in_eris3 >> N;
        qDebug() << "Number Map = " << N;

        QStringList mapFileList;
        in_eris3 >> mapFileList;

        for(int i=0;i<N;++i){
            float xmap,ymap,smap;
            in_eris3 >> xmap;
            in_eris3 >> ymap;
            in_eris3 >> smap;
        }

        in_eris3 >> N;
        in_eris3 >> itmp;
        qDebug() << "Number WP = " << N;

        for(int i=0;i<N;++i){
            in_eris3 >> btmp;
            for(int j=0;j<6;++j){
                in_eris3 >> ftmp;
            }
            in_eris3 >> btmp;
            in_eris3 >> btmp;
            in_eris3 >> itmp;
            in_eris3 >> itmp;
            in_eris3 >> itmp;
            for(int j=0;j<itmp;++j){
                int itmp2 = 0;
                in_eris3 >> itmp2;
            }
            in_eris3 >> itmp;
            for(int j=0;j<itmp;++j){
                int itmp2 = 0;
                in_eris3 >> itmp2;
            }
            in_eris3 >> itmp;
            for(int j=0;j<itmp;++j){
                int itmp2 = 0;
                in_eris3 >> itmp2;
            }
            in_eris3 >> itmp;
            for(int j=0;j<itmp;++j){
                int itmp2 = 0;
                in_eris3 >> itmp2;
            }
            in_eris3 >> btmp;
            in_eris3 >> btmp;
        }


        in_eris3 >> N;
        in_eris3 >> itmp;
        qDebug() << "Number PATH = " << N;
        for(int i=0;i<N;++i){
            in_eris3 >> btmp;
            in_eris3 >> itmp;
            in_eris3 >> itmp;
            in_eris3 >> ftmp;
            in_eris3 >> ftmp;
            in_eris3 >> ftmp;
        }


        in_eris3 >> N;
        qDebug() << "Number Node = " << N;
        in_eris3 >> itmp;
        for(int i=0;i<N;++i){
            in_eris3 >> btmp;
            float xc,yc;
            in_eris3 >> xc;
            in_eris3 >> yc;
            in_eris3 >> itmp;
            for(int j=0;j<itmp;++j){
                int itmp1,itmp2,itmp3,itmp4;
                float ftmp1;
                in_eris3 >> itmp1;
                in_eris3 >> ftmp1;
                in_eris3 >> itmp2;
                for(int k=0;k<itmp2;++k){
                    in_eris3 >> itmp1;
                }
                in_eris3 >> itmp1;
                for(int k=0;k<itmp1;++k){
                    in_eris3 >> itmp2;

                    struct tmpWPData w;
                    w.id = itmp2;
                    w.xc = xc;
                    w.yc = yc;
                    w.node = -1;

                    wpOutData.append( w );
                }
                in_eris3 >> itmp1;
                in_eris3 >> itmp2;
                in_eris3 >> itmp3;
                in_eris3 >> itmp4;
            }
            in_eris3 >> btmp;
            in_eris3 >> btmp;
            in_eris3 >> itmp;
            in_eris3 >> btmp;
            int MC;
            in_eris3 >> MC;
            for(int j=0;j<MC;++j){
                in_eris3 >> itmp;
            }
        }

        file_eris3.close();

        qDebug() << "Read.";
    }

    for(int i=0;i<wpOutData.size();++i){
        for(int j=0;j<road->nodes.size();++j){

            float dx = road->nodes[j]->pos.x() - wpOutData[i].xc;
            float dy = road->nodes[j]->pos.y() - wpOutData[i].yc;
            float D = dx * dx + dy * dy;
            if( D < 1.0 ){
                wpOutData[i].node = road->nodes[j]->id;
                break;
            }
        }
//        qDebug() << "WPOut: wp = " << wpOutData[i].id << " node = " << wpOutData[i].node;
    }


    //  Vehicle Data
    int nON;
    in >> nON;

    QList<int> OriginNode;
    for(int i=0;i<nON;++i){
        int nd;
        in >> nd;
        OriginNode.append( nd );

        int nIdx = road->indexOfNode( nd );
        if( nIdx >= 0 ){
            road->nodes[nIdx]->isOriginNode = true;
        }
    }


    int nDN;
    in >> nDN;

    QVector<int> DestinationNode;
    for(int i=0;i<nDN;++i){
        int nd;
        in >> nd;
        DestinationNode << nd;

        int nIdx = road->indexOfNode( nd );
        if( nIdx >= 0 ){
            road->nodes[nIdx]->isDestinationNode = true;
        }
    }


    int nOT;
    in >> nOT;

    QList<int> ODTableIDs;
    for(int i=0;i<nOT;++i){
        int id;
        in >> id;
        ODTableIDs.append( id );
    }

    int nODD;
    in >> nODD;

    struct ODDataElem{
        int onode;
        int dnode;
        float val;
    };

    QList<struct ODDataElem> ODDataMain;
    for(int i=0;i<nODD;++i){
        QString key;
        float val;
        in >> key;
        in >> val;

        if( val < 0.0001 ){
            continue;
        }

        QStringList divKey = key.split("-");

        struct ODDataElem e;
        e.onode = QString(divKey[1]).remove("O").trimmed().toInt();
        e.dnode = QString(divKey[2]).remove("D").trimmed().toInt();
        e.val = val;

        ODDataMain.append( e );
    }

    int nR;
    int nC;
    in >> nR;
    in >> nC;

    for(int i=0;i<nR;++i){
        for(int j=0;j<nC;++j){
            QString txt;
            in >> txt;  // sequence
        }
    }

    int nSink;
    in >> nSink;
    for(int i=0;i<nSink;++i){
        int tSink;
        in >> tSink;
    }


    int nSource;
    in >> nSource;

    struct tmpTrafficVolumeData
    {
        int node;
        int volume;
    };

    QList<struct tmpTrafficVolumeData> TVData;

    for(int i=0;i<nSource;++i){

        int wpid;
        in >> wpid;

        bool mode;
        in >> mode;


        int nRtw;
        int nCtw;
        in >> nRtw;
        in >> nCtw;

        qDebug() << "source WP: " << wpid;

        float meanArrival = 0.0;
        for(int j=0;j<nRtw;++j){
            for(int k=0;k<nCtw;++k){
                QString txt;
                in >> txt;
                if(j == 0){
                    meanArrival += txt.toFloat();
                }
            }
        }
        meanArrival /= nCtw;


        for(int l=0;l<wpOutData.size();++l){
            if( wpOutData[l].id == wpid ){

                int tvIdx = -1;
                int oNode = wpOutData[l].node;
                for(int m=0;m<TVData.size();++m){
                    if( TVData[m].node == oNode ){
                        tvIdx = m;
                        break;
                    }
                }
                if( tvIdx < 0 ){
                    struct tmpTrafficVolumeData v;
                    v.node = oNode;
                    v.volume = 0;
                    TVData.append( v );
                    tvIdx = TVData.size() - 1;
                }

                int vph = (int)(3600.0 / meanArrival);
                TVData[tvIdx].volume += vph;
            }
        }
    }

//    for(int i=0;i<TVData.size();++i){
//        qDebug() << "Node = " << TVData[i].node << " Volume = " << TVData[i].volume;
//    }

    int nRoute;
    in >> nRoute;

    for(int i=0;i<nRoute;++i){

        bool protectFlag;
        in >> protectFlag;

        float selProb;
        in >> selProb;

        int nElem;
        in >> nElem;

        QList<int> inDirs;
        QList<int> Nodes;
        QList<int> outDirs;

        for(int j=0;j<nElem;++j){

            int reDat;
            in >> reDat;  // inDir
            inDirs.append( reDat );

            in >> reDat;  // Node
            Nodes.append( reDat );

            in >> reDat;  // outDir
            outDirs.append( reDat );
        }


        int ONode = Nodes.first();
        int DNode = Nodes.last();

//        qDebug() << "Nodes: " << Nodes;
//        qDebug() << "ONode = " << ONode << " DNode = " << DNode;

        int onIdx = road->indexOfNode( ONode );
        if( onIdx >= 0 ){

            int dn = -1;
            for(int j=0;j<road->nodes[onIdx]->odData.size();++j){
                if( road->nodes[onIdx]->odData[j]->destinationNode == DNode ){
                    dn = j;
                    break;
                }
            }
            if( dn < 0 ){
                struct ODData *odd = new struct ODData;
                odd->destinationNode = DNode;
                road->nodes[onIdx]->odData.append( odd );
                dn = road->nodes[onIdx]->odData.size() - 1;
            }

            struct RouteData* route = new struct RouteData;

            route->selProb = selProb;
            for(int j=0;j<Nodes.size();++j){

                struct RouteElem *e = new struct RouteElem;

                e->inDir  = inDirs[j];
                e->node   = Nodes[j];
                e->outDir = outDirs[j];

                route->nodeList.append( e );
            }

            road->nodes[onIdx]->odData[dn]->route.append( route );

        }
    }


    for(int i=0;i<road->nodes.size();++i){
        for(int j=0;j<road->nodes[i]->odData.size();++j){
            int ONode = road->nodes[i]->id;
            int DNode = road->nodes[i]->odData[j]->destinationNode;

            int TVatONode = 0;
            for(int k=0;k<TVData.size();++k){
                if( TVData[k].node == ONode ){
                    TVatONode = TVData[k].volume;
                    break;
                }
            }

            float p = 0.0;
            for(int k=0;k<ODDataMain.size();++k){
                if(ODDataMain[k].onode == ONode && ODDataMain[k].dnode == DNode ){
                    p = ODDataMain[k].val;
                    break;
                }
            }

            for(int k=0;k<road->nodes[i]->odData[j]->route.size();++k){

                float r = road->nodes[i]->odData[j]->route[k]->selProb;

                int TV = (int)(r * p * TVatONode);

                struct TrafficVolumeData *tvd = new struct TrafficVolumeData;
                tvd->vehicleKind = 0;
                tvd->trafficVolume = TV;

                road->nodes[i]->odData[j]->route[k]->volume.append( tvd );
            }
        }
    }


    file.close();

}




