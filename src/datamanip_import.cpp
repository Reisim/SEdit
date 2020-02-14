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
#include <QDebug>
#include <QProgressDialog>


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

    in >> itmp;
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
            in >> btmp;
            if( btmp == true ){
                for(int j=0;j<=10;++j){
                    in >> ftmp;
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
                                        tmpWPRelatedNode[sWPidx],
                                        (tmpWPIsWPin[sWPidx] == true || tmpWPIsWPout[sWPidx] == true ) ? true : false,
                                        endWP,
                                        tmpWPRelatedNode[eWPidx],
                                        tmpWPRelatedNodeDir[eWPidx],
                                        tmpWPRelatedNode[eWPidx],
                                        (tmpWPIsWPin[eWPidx] == true || tmpWPIsWPout[eWPidx] == true ) ? true : false );

            if( lId >= 0 ){
                int lIdx = road->indexOfLane(lId);
                if( lIdx >= 0 ){
                    road->lanes[lIdx]->laneWidth = tmpPathWidth[i];
                    road->lanes[lIdx]->speedInfo = tmpPathSpeedLimit[i];

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

            if( i + 1 == N ){
                qDebug() << "Lanes created.";
            }
        }
    }


    for(int i=0;i<tmpStopLineType.size();++i){

//        qDebug() << "ID=" << tmpStopLineIDs[i] <<  " SLType = " << tmpStopLineType[i] << " Node=" << tmpStopLineRelatedNode[i] << " Dir=" << tmpStopLineRelatedDirect[i];

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
    }

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

    }


    QProgressDialog *pd = new QProgressDialog("Post processing ...", "Cancel", 0, 8, 0);
    pd->setWindowModality(Qt::WindowModal);
    pd->show();

    pd->setValue(0);
    QApplication::processEvents();



    for(int i=0;i<road->nodes.size();++i){
        road->nodes[i]->nLeg  = road->nodes[i]->legInfo.size();
        road->nodes[i]->hasTS = road->nodes[i]->trafficSignals.size() > 0 ? true : false;
    }


    pd->setValue(1);
    QApplication::processEvents();

    // Set Next and Previous Lanes
    road->CheckLaneConnection();

    pd->setValue(2);
    QApplication::processEvents();

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

    pd->setValue(3);
    QApplication::processEvents();

    // Calculate Stop Point Data
    road->CheckAllStopLineCrossLane();

    pd->setValue(4);
    QApplication::processEvents();

    // Calculate Lane Cross Points
    road->CheckLaneCrossPoints();

    pd->setValue(5);
    QApplication::processEvents();

    // Create WP and Boundary WP Check
    road->CreateWPData();

    pd->setValue(6);
    QApplication::processEvents();

    // Set Lane List
    road->SetAllLaneLists();

    pd->setValue(7);
    QApplication::processEvents();

    // Set Turn Direction Info
    road->SetTurnDirectionInfo();

    pd->setValue(8);
    QApplication::processEvents();


    float xmean = (maxDataX + minDataX) / 2;
    float ymean = (maxDataY + minDataY) / 2;

    qDebug() << "Move to " << xmean << "," << ymean;

    canvas->MoveTo( xmean, ymean );





}

