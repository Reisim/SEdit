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


void DataManipulator::MigrateData(QString filename)
{
    qDebug() << "[DataManipulator::MigrateData] filename = " << filename;

    QFile file(filename);
    if( !file.open(QIODevice::ReadOnly | QIODevice::Text) ){
        qDebug() << "Cannot open file for read: " << qPrintable(file.errorString());
        return;
    }

    QTextStream in(&file);

    QStringList commands;

    while( in.atEnd() == false ){
        commands.append( in.readLine() );
    }

    file.close();

    int num = commands.size()-1;

    for(int i=num;i>=0;i--){
        if( commands[i].contains("NODE_CONNECT:") == true ){
            commands.append( commands[i] );
            commands.removeAt(i);
        }
    }
    for(int i=num;i>=0;i--){
        if( commands[i].contains("EDGE_INFO:") == true ){
            commands.append( commands[i] );
            commands.removeAt(i);
        }
    }
    for(int i=num;i>=0;i--){
        if( commands[i].contains("CREATE_STOPLINE:") == true ){
            commands.append( commands[i] );
            commands.removeAt(i);
        }
    }
    for(int i=num;i>=0;i--){
        if( commands[i].contains("SET_TS_NODE:") == true ){
            commands.append( commands[i] );
            commands.removeAt(i);
        }
    }

    qDebug() << "Data read: size of commands = " << commands.size();


    QList< QList<QPointF> > lanePoints;

    for(int i=0;i<commands.size();++i){

        QString comStr = QString(commands[i]);
        if( comStr.isEmpty() == true ){
            continue;
        }

        if( comStr.contains("CREATE_NODE:") ){
            QStringList divCom = QString(commands[i]).replace("CREATE_NODE:","").split(",");

            int id   = QString(divCom[0]).trimmed().toInt();
            float xc = QString(divCom[1]).trimmed().toFloat();
            float yc = QString(divCom[2]).trimmed().toFloat();
            int Ncrs = QString(divCom[3]).trimmed().toInt();

            QList<int> lanes;
            for(int n=0;n<Ncrs;++n){
                lanes << 1;
            }

            road->CreateNode(id, xc, yc, lanes, lanes );
        }
        if( comStr.contains("NODE_CONNECT:") ){
            QStringList divCom = QString(commands[i]).replace("NODE_CONNECT:","").split(",");

            int id   = QString(divCom[0]).trimmed().toInt();
            for(int n=1;n<divCom.size();n+=3){
                int cNode = QString(divCom[n]).trimmed().toInt();
                float angle = QString(divCom[n+1]).trimmed().toFloat();
                int inOrOut = QString(divCom[n+2]).trimmed().toInt();

                road->SetAngleNodeLeg(id, (n-1)/3, angle);

                if( inOrOut == 1 || inOrOut == 3 ){
                    road->SetNodeConnectInfo(id,(n-1)/3, cNode, "OutNode")   ;
                }

                if( inOrOut == 2 || inOrOut == 3 ){
                    road->SetNodeConnectInfo(id,(n-1)/3, cNode, "InNode")   ;
                }
            }
        }
        if( comStr.contains("CREATE_PATH:") ){
            QStringList divCom = QString(commands[i]).replace("CREATE_PATH:","").split(",");

            int id = QString(divCom[0]).trimmed().toInt();
            int nP = QString(divCom[1]).trimmed().toInt();

            float sx = QString(divCom[2]).trimmed().toFloat();
            float sy =  QString(divCom[3]).trimmed().toFloat();
            float sz = QString(divCom[4]).trimmed().toFloat();

            float sdx = QString(divCom[5]).trimmed().toFloat() - sx;
            float sdy = QString(divCom[6]).trimmed().toFloat() - sy;
            float stht = atan2(sdy,sdx);

            float ex = QString(divCom[2 + (nP-1) * 3]).trimmed().toFloat();
            float ey =  QString(divCom[3 + (nP-1) * 3]).trimmed().toFloat();
            float ez =  QString(divCom[4 + (nP-1) * 3]).trimmed().toFloat();

            float edx = ex - sx;
            float edy = ey - sy;
            if( nP >= 3 ){
                edx = ex - QString(divCom[2 + (nP-2) * 3]).trimmed().toFloat();
                edy = ey - QString(divCom[3 + (nP-2) * 3]).trimmed().toFloat();
            }
            float etht = atan2(edy,edx);

            QList<QPointF> points;

            for(int k=0;k<nP;k++){
                QPointF P;
                P.setX(  QString(divCom[2 + k * 3]).trimmed().toFloat() );
                P.setY(  QString(divCom[3 + k * 3]).trimmed().toFloat() );
                points.append( P );
            }

            lanePoints.append( points );

            QVector4D startPoint;
            startPoint.setX( sx );
            startPoint.setY( sy );
            startPoint.setZ( sz );
            startPoint.setW( stht );

            QVector4D endPoint;
            endPoint.setX( ex );
            endPoint.setY( ey );
            endPoint.setZ( ez );
            endPoint.setW( etht );

            //qDebug() << "CreateLane: id = " << id;
            road->CreateLane( id, startPoint, -1, 0, true, endPoint, -1, 0, true );
        }
        if( comStr.contains("EDGE_INFO:") ){
            QStringList divCom = QString(commands[i]).replace("EDGE_INFO:","").split(",");

            int id = QString(divCom[0]).trimmed().toInt();
            bool boundaryWPS = (QString(divCom[1]).trimmed().toInt() == 1 ? true : false);
            int NodeS = QString(divCom[2]).trimmed().toInt();
            int DirS = QString(divCom[3]).trimmed().toInt();
            bool boundaryWPE = (QString(divCom[4]).trimmed().toInt() == 1 ? true : false);
            int NodeE = QString(divCom[5]).trimmed().toInt();
            int DirE = QString(divCom[6]).trimmed().toInt();

            //qDebug() << "ID=" << id << " NodeS=" << NodeS << " NodeE=" << NodeE;

            int idx = road->indexOfLane( id );
            if( idx >= 0 ){
                road->lanes[idx]->sWPBoundary = boundaryWPS;
                road->lanes[idx]->sWPInNode = NodeS;
                road->lanes[idx]->sWPNodeDir =DirS;

                road->lanes[idx]->eWPBoundary = boundaryWPE;
                road->lanes[idx]->eWPInNode = NodeE;
                road->lanes[idx]->eWPNodeDir =DirE;

                road->lanes[idx]->connectedNode = NodeE;
                if( NodeE == NodeS ){
                    road->lanes[idx]->connectedNodeOutDirect = DirE;
                    road->lanes[idx]->connectedNodeInDirect  = DirS;
                    road->lanes[idx]->departureNode = -1;
                    road->lanes[idx]->departureNodeOutDirect = -1;
                }
                else{
                    road->lanes[idx]->connectedNodeOutDirect = -1;
                    road->lanes[idx]->connectedNodeInDirect  = DirE;
                    road->lanes[idx]->departureNode = NodeS;
                    road->lanes[idx]->departureNodeOutDirect = DirS;
                }

                road->SetNodeRelatedLane( NodeE, id );
            }
        }
        if( comStr.contains("CREATE_STOPLINE:") ){

            QStringList divCom = QString(commands[i]).replace("CREATE_STOPLINE:","").split(",");

            int id = QString(divCom[0]).trimmed().toInt();
            int onLane = QString(divCom[1]).trimmed().toInt();

            int lnIdx = road->indexOfLane( onLane );
            if( lnIdx >= 0 ){
                int cNd = road->lanes[lnIdx]->connectedNode;
                int ndIdx = road->indexOfNode( cNd );
                if( ndIdx >= 0 ){
                    if( road->nodes[ndIdx]->nLeg == 1 ){
                        continue;
                    }
                }
            }

            float atX = QString(divCom[2]).trimmed().toFloat();
            float atY = QString(divCom[3]).trimmed().toFloat();
            float atZ = QString(divCom[4]).trimmed().toFloat();
            int kind = QString(divCom[5]).trimmed().toFloat();

            float angle = 0.0;
            for(int n=0;n<lanePoints[onLane].size()-1;++n){
                float xs = lanePoints[onLane][n].x();
                float ys = lanePoints[onLane][n].y();
                float xe = lanePoints[onLane][n+1].x();
                float ye = lanePoints[onLane][n+1].y();
                float dx = xe - xs;
                float dy = ye - ys;
                float D = dx * dx + dy * dy;
                float rx = atX - xs;
                float ry = atY - ys;
                float R = rx * rx + ry * ry;
                if( R < D ){
                    angle = atan2( dy, dx );
                    break;
                }
            }

            int SLType = _STOPLINE_KIND::STOPLINE_TEMPSTOP;
            if( kind == 1 ){
                SLType = _STOPLINE_KIND::STOPLINE_SIGNAL;
            }
            else if( kind == 2 ){
                SLType = _STOPLINE_KIND::STOPLINE_TURNWAIT;
            }

            //qDebug() << "CREATE_STOPLINE:" << id << " " << onLane << " " << atX << " " << atY;

            road->CreateStopLineAtLAne( id, onLane, atX, atY, angle, SLType );
        }
        if( comStr.contains("SET_TS_NODE:") ){

            QStringList divCom = QString(commands[i]).replace("SET_TS_NODE:","").split(",");

            int id = QString(divCom[0]).trimmed().toInt();
            int ndIdx = road->indexOfNode( id );
            if( ndIdx >= 0 ){

                if( road->nodes[ndIdx]->nLeg == 1 ){
                    continue;
                }

                for(int j=0;j<road->nodes[ndIdx]->legInfo.size();++j){
                    bool hasTS = false;
                    for(int k=0;k<road->nodes[ndIdx]->trafficSignals.size();++k){
                        if( road->nodes[ndIdx]->trafficSignals[k]->controlNodeDirection == j ){
                            hasTS = true;
                            break;
                        }
                    }
                    if( hasTS == false ){
                        road->AddTrafficSignalToNode(id,-1,0,j);
                    }
                }
            }
        }
    }

    // Set In-Out Direction
    road->SetNodeConnectInOutDirect();


    qDebug() << "Check path tangent at edge points.";

    CheckLaneConnectionFull();

    int checkEdgeAngleTarget = -1;

    for(int i=0;i<road->lanes.size();i++){

        if( road->lanes[i]->nextLanes.size() > 0 ){

            int nP = lanePoints[i].size();

            float dx = lanePoints[i][nP-1].x() - lanePoints[i][nP-2].x();
            float dy = lanePoints[i][nP-1].y() - lanePoints[i][nP-2].y();
            float D = sqrt(dx * dx + dy * dy);
            dx /= D;
            dy /= D;

            if( i == checkEdgeAngleTarget ){
                qDebug() << " Check Edge Angle; i = " << i << " , dx = " << dx << " dy = " << dy;
            }

            float setAngle = 0.0;
            QList<float> DXf;
            QList<float> DYf;
            for(int j=0;j<road->lanes[i]->nextLanes.size();++j){
                int nextLL = road->lanes[i]->nextLanes[j];

                float dxf = lanePoints[nextLL][1].x() - lanePoints[nextLL][0].x();
                float dyf = lanePoints[nextLL][1].y() - lanePoints[nextLL][0].y();
                float Df = sqrt(dxf * dxf + dyf * dyf);
                dxf /= Df;
                dyf /= Df;

                DXf.append( dxf );
                DYf.append( dyf );

                if( i == checkEdgeAngleTarget ){
                    qDebug() << " nextLL = " << nextLL << " , dxf = " << dxf << " dyf = " << dyf;
                }
            }

            if( DXf.size() == 1 ){
                float ip = dx * DXf[0] + dy * DYf[0];

                if( i == checkEdgeAngleTarget ){
                    qDebug() << "[1] ip = " << ip;
                }

                if( ip > 0.9986 ){
                    float dxm = dx + DXf[0];
                    float dym = dy + DYf[0];
                    float angle = atan2( dym, dxm );
                    road->SetLaneEdgeAngle( i, angle, 1 );

                    setAngle = angle;

                    if( i == checkEdgeAngleTarget ){
                        qDebug() << "dxm = " << dxm << " dym = " << dym << " angle = " << angle;
                    }
                }
                else{
                    float angle = atan2( DYf[0], DXf[0] );
                    road->SetLaneEdgeAngle( i, angle, 1 );

                    setAngle = angle;

                    if( i == checkEdgeAngleTarget ){
                        qDebug() << "dx = " << DXf[0] << " dy = " << DYf[0] << " angle = " << angle;
                    }
                }
            }
            else if( DXf.size() > 1 ){
                int selLL = -1;
                float maxIP = 0.0;
                for(int j=0;j<DXf.size();++j){
                    float ip = dx * DXf[j] + dy * DYf[j];
                    if( selLL< 0 || ip > maxIP ){
                        selLL = j;
                        maxIP = ip;
                    }
                }

                if( i == checkEdgeAngleTarget ){
                    qDebug() << "[2] selLL = " << selLL << " maxIP = " << maxIP;
                }

                if( maxIP > 0.9986 ){
                    float dxm = dx + DXf[selLL];
                    float dym = dy + DYf[selLL];
                    float angle = atan2( dym, dxm );
                    road->SetLaneEdgeAngle( i, angle, 1 );

                    setAngle = angle;

                    if( i == checkEdgeAngleTarget ){
                        qDebug() << "dxm = " << dxm << " dym = " << dym << " angle = " << angle;
                    }
                }
                else{
                    float angle = atan2( dy, dx );
                    road->SetLaneEdgeAngle( i, angle, 1 );

                    setAngle = angle;

                    if( i == checkEdgeAngleTarget ){
                        qDebug() << "dx = " << dx << " dy = " << dy << " angle = " << angle;
                    }
                }
            }

            for(int j=0;j<road->lanes[i]->nextLanes.size();++j){
                int nextLL = road->lanes[i]->nextLanes[j];
                road->SetLaneEdgeAngle( nextLL, setAngle, 0 );
            }
        }
    }

    qDebug() << "Adjust lane shape by dividing lane";

    int maxLane = road->lanes.size();
    QList<int> newlyCreatedLane;
    for(int i=0;i<maxLane;++i){

        if( newlyCreatedLane.contains(i) == true ){
            continue;
        }

        int nP = lanePoints[i].size();
        if( nP < 4 ){
            continue;
        }
        int mid = nP / 2;
        float xm = lanePoints[i][mid].x();
        float ym = lanePoints[i][mid].y();

        float xt = 0.0;
        float yt = 0.0;
        float angle = 0.0;
        int ret = road->GetNearestLanePoint(i, xm, ym, xt, yt, angle);
        int newId = -1;
        if( ret == i || ret < 0 ){
            float dx = xm - xt;
            float dy = ym - yt;
            float D = dx * dx + dy * dy;
            if( D > 1.0 || ret < 0 ){

                //qDebug() << "Lane = " << i << " M(" << xm << "," << ym <<") P(" << xt << "," << yt << "), D = " << D;

                int nStep = 1;
                while(1){
                    int ns = nStep + 1;
                    if( mid + ns < lanePoints[i].size() && mid - ns >= 0){
                        nStep = ns;
                    }
                    else{
                        break;
                    }
                    if(nStep >= 5){
                        break;
                    }
                }

                float rx1 = lanePoints[i][mid+nStep].x() - xm;
                float ry1 = lanePoints[i][mid+nStep].y() - ym;
                float D1 = sqrt(rx1 * rx1 + ry1 * ry1);
                rx1 /= D1;
                ry1 /= D1;

                float rx2 = xm - lanePoints[i][mid-nStep].x();
                float ry2 = ym - lanePoints[i][mid-nStep].y();
                float D2 = sqrt(rx2 * rx2 + ry2 * ry2);
                rx2 /= D2;
                ry2 /= D2;

                float a1 = D2 / (D1 + D2);
                float a2 = D1 / (D1 + D2);
                float rx = rx1 * a1 + rx2 * a2;
                float ry = ry1 * a1 + ry2 * a2;
                float angle = atan2( ry, rx );

                //qDebug() << "D1 = " << D1 << " D2 = " << D2 << "angle = " << angle * 57.3;

                newId = road->DivideLaneAndMove( i, xm, ym, angle, false );
                newlyCreatedLane.append( newId );
            }
        }

        int midh = mid / 2;
        int mid1 = mid - midh;
        if( mid1 > 0 && mid1 < lanePoints[i].size() -1 ){
            xm = lanePoints[i][mid1].x();
            ym = lanePoints[i][mid1].y();

            float xt = 0.0;
            float yt = 0.0;
            float angle = 0.0;
            int ret = road->GetNearestLanePoint(i, xm, ym, xt, yt, angle);
            if( ret == i || ret < 0 ){
                float dx = xm - xt;
                float dy = ym - yt;
                float D = dx * dx + dy * dy;
                if( D > 1.0 || ret < 0 ){

                    //qDebug() << "Lane = " << i << " M(" << xm << "," << ym <<") P(" << xt << "," << yt << "), D = " << D;

                    int nStep = 1;
                    while(1){
                        int ns = nStep + 1;
                        if( mid1 + ns < lanePoints[i].size() && mid1 - ns >= 0){
                            nStep = ns;
                        }
                        else{
                            break;
                        }
                        if(nStep >= 5){
                            break;
                        }
                    }

                    float rx1 = lanePoints[i][mid1+nStep].x() - xm;
                    float ry1 = lanePoints[i][mid1+nStep].y() - ym;
                    float D1 = sqrt(rx1 * rx1 + ry1 * ry1);
                    rx1 /= D1;
                    ry1 /= D1;

                    float rx2 = xm - lanePoints[i][mid1-nStep].x();
                    float ry2 = ym - lanePoints[i][mid1-nStep].y();
                    float D2 = sqrt(rx2 * rx2 + ry2 * ry2);
                    rx2 /= D2;
                    ry2 /= D2;

                    float a1 = D2 / (D1 + D2);
                    float a2 = D1 / (D1 + D2);
                    float rx = rx1 * a1 + rx2 * a2;
                    float ry = ry1 * a1 + ry2 * a2;
                    float angle = atan2( ry, rx );

                    //qDebug() << "D1 = " << D1 << " D2 = " << D2 << "angle = " << angle * 57.3;

                    int newId1 = road->DivideLaneAndMove( i, xm, ym, angle, false );
                    newlyCreatedLane.append( newId1 );
                }
            }
        }

        int mid2 = mid + midh;
        if( newId >= 0 && mid2 > 0 && mid2 < lanePoints[i].size() - 1 ){

            xm = lanePoints[i][mid2].x();
            ym = lanePoints[i][mid2].y();

            float xt = 0.0;
            float yt = 0.0;
            float angle = 0.0;
            int ret = road->GetNearestLanePoint(newId, xm, ym, xt, yt, angle);
            if( ret == newId || ret < 0 ){
                float dx = xm - xt;
                float dy = ym - yt;
                float D = dx * dx + dy * dy;
                if( D > 1.0 || ret < 0 ){

                    //qDebug() << "Lane = " << i << " M(" << xm << "," << ym <<") P(" << xt << "," << yt << "), D = " << D;

                    int nStep = 1;
                    while(1){
                        int ns = nStep + 1;
                        if( mid2 + ns < lanePoints[i].size() && mid2 - ns >= 0){
                            nStep = ns;
                        }
                        else{
                            break;
                        }
                        if(nStep >= 5){
                            break;
                        }
                    }

                    float rx1 = lanePoints[i][mid2+nStep].x() - xm;
                    float ry1 = lanePoints[i][mid2+nStep].y() - ym;
                    float D1 = sqrt(rx1 * rx1 + ry1 * ry1);
                    rx1 /= D1;
                    ry1 /= D1;

                    float rx2 = xm - lanePoints[i][mid2-nStep].x();
                    float ry2 = ym - lanePoints[i][mid2-nStep].y();
                    float D2 = sqrt(rx2 * rx2 + ry2 * ry2);
                    rx2 /= D2;
                    ry2 /= D2;

                    //qDebug() << " F(" << lanePoints[i][mid2+nStep].x() << "," << lanePoints[i][mid2+nStep].y() << ")"
                    //         << " R(" << lanePoints[i][mid2-nStep].x() << "," << lanePoints[i][mid2-nStep].y() << ")";

                    float a1 = D2 / (D1 + D2);
                    float a2 = D1 / (D1 + D2);
                    float rx = rx1 * a1 + rx2 * a2;
                    float ry = ry1 * a1 + ry2 * a2;
                    float angle = atan2( ry, rx );

                    //qDebug() << "D1 = " << D1 << " D2 = " << D2 << "angle = " << angle * 57.3;

                    int newId2 = road->DivideLaneAndMove( newId, xm, ym, angle, false );
                    newlyCreatedLane.append( newId2 );
                }
            }
        }
    }

    CreateWPData();

    FindInconsistentData();

    SetAllLaneLists();

    SetTurnDirectionInfo();

    CheckAllStopLineCrossLane();

    CheckLaneCrossPoints();

    qDebug() << "End of Migratation";
}


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

            mapImageMng->AddMapImageFromFile( mapFileList[i], xMaps[i], yMaps[i], sMaps[i], 0.0 );

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
                    if( tmpPathWidth.size() > i ){
                        road->lanes[lIdx]->laneWidth = tmpPathWidth[i];
                    }
                    else{
                        road->lanes[lIdx]->laneWidth = 2.7;
                    }
                    if( tmpPathSpeedLimit.size() > i ){
                        road->lanes[lIdx]->speedInfo = tmpPathSpeedLimit[i];
                    }
                    else{
                        road->lanes[lIdx]->speedInfo = 40.0;
                    }
                    road->lanes[lIdx]->actualSpeed = road->lanes[lIdx]->speedInfo;
                    if( tmpPathAutomaticDriving.size() > i ){
                        road->lanes[lIdx]->automaticDrivingEnabled = tmpPathAutomaticDriving[i];
                    }
                    else{
                        road->lanes[lIdx]->automaticDrivingEnabled = false;
                    }
                    if( tmpPathDriverErrorProb.size() > i ){
                        road->lanes[lIdx]->driverErrorProb = 0.0;
                    }

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
                qDebug() << "Traffic Signals created.";
            }
        }

    }


    for(int i=0;i<road->nodes.size();++i){
        road->nodes[i]->nLeg  = road->nodes[i]->legInfo.size();
        road->nodes[i]->hasTS = road->nodes[i]->trafficSignals.size() > 0 ? true : false;
    }


    bool ret;

    // Set Next and Previous Lanes

    qDebug() << "Set Next and Previous Lanes.";

    ret = road->CheckLaneConnectionFull();

    qDebug() << "CheckLaneConnectionFull: ret = " << ret;

    if( ret == true ){

        int maxInter = 10;
        int iter = 0;

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

            iter++;
            if( iter >= maxInter ){
                qDebug() << "iter exceed max; failed to set data.";
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


    // Check related node data in Lane

    road->CheckLaneRelatedNodeAllLanes();


    // Calculate Stop Point Data

    qDebug() << "Calculate Stop Point Data.";

    road->CheckAllStopLineCrossLane();


    // Calculate Lane Cross Points

    qDebug() << "Calculate Lane Cross Points.";

    road->CheckLaneCrossPoints();


    // Create WP and Boundary WP Check

    qDebug() << "Create WP and Boundary WP Check.";

    road->CreateWPData();


    // Check WPNode info again
    while(1){

        int nChecked = 0;

        for(int i=0;i<road->lanes.size();++i){

            if( road->lanes[i]->sWPInNode <= 0 && road->lanes[i]->sWPNodeDir < 0 &&
                  road->lanes[i]->eWPInNode >=0 && road->lanes[i]->eWPNodeDir >= 0 ){

                nChecked++;

                int nIdx = road->indexOfNode( road->lanes[i]->eWPInNode );
                if( nIdx >= 0 ){
                    for(int j=0;j<road->nodes[nIdx]->legInfo.size();++j){
                        if( road->nodes[nIdx]->legInfo[j]->legID == road->lanes[i]->eWPNodeDir ){
                            road->lanes[i]->sWPInNode = road->nodes[nIdx]->legInfo[j]->connectedNode;
                            road->lanes[i]->sWPNodeDir = road->nodes[nIdx]->legInfo[j]->connectedNodeOutDirect;
                            break;
                        }
                    }
                }
            }
            else if( road->lanes[i]->eWPInNode <= 0 && road->lanes[i]->eWPNodeDir < 0 &&
                  road->lanes[i]->sWPInNode >=0 && road->lanes[i]->sWPNodeDir >= 0 ){

                nChecked++;

                int nIdx = road->indexOfNode( road->lanes[i]->sWPInNode );
                if( nIdx >= 0 ){
                    for(int j=0;j<road->nodes[nIdx]->legInfo.size();++j){
                        if( road->nodes[nIdx]->legInfo[j]->legID == road->lanes[i]->sWPNodeDir ){
                            road->lanes[i]->eWPInNode = road->nodes[nIdx]->legInfo[j]->connectingNode;
                            road->lanes[i]->eWPNodeDir = road->nodes[nIdx]->legInfo[j]->connectingNodeInDirect;
                            break;
                        }
                    }
                }
            }
        }

        for(int i=0;i<road->lanes.size();++i){

            if( road->lanes[i]->sWPInNode < 0 && road->lanes[i]->sWPNodeDir < 0 &&
                  road->lanes[i]->eWPInNode <0 && road->lanes[i]->eWPNodeDir < 0 ){

                nChecked++;

                bool isSet = false;
                if( road->lanes[i]->nextLanes.size() > 0 ){
                    for(int j=0;j<road->lanes[i]->nextLanes.size();++j){
                        int nlIdx = road->indexOfLane( road->lanes[i]->nextLanes[j] );
                        if( nlIdx >= 0){

                            if( road->lanes[nlIdx]->sWPBoundary == false ){

                                road->lanes[i]->sWPInNode = road->lanes[nlIdx]->sWPInNode;
                                road->lanes[i]->sWPNodeDir = road->lanes[nlIdx]->sWPNodeDir;

                                road->lanes[i]->eWPInNode = road->lanes[nlIdx]->eWPInNode;
                                road->lanes[i]->eWPNodeDir = road->lanes[nlIdx]->eWPNodeDir;

                                isSet = true;
                            }
                            else{

                                road->lanes[i]->eWPInNode = road->lanes[nlIdx]->sWPInNode;
                                road->lanes[i]->eWPNodeDir = road->lanes[nlIdx]->sWPNodeDir;

                                int nIdx = road->indexOfNode( road->lanes[i]->eWPInNode );
                                if( nIdx >= 0 ){
                                    for(int k=0;k<road->nodes[nIdx]->legInfo.size();++k){
                                        if( road->nodes[nIdx]->legInfo[k]->legID == road->lanes[i]->eWPNodeDir ){
                                            road->lanes[i]->sWPInNode = road->nodes[nIdx]->legInfo[k]->connectedNode;
                                            road->lanes[i]->sWPNodeDir = road->nodes[nIdx]->legInfo[k]->connectedNodeOutDirect;
                                            break;
                                        }
                                    }
                                }

                                isSet = true;
                            }
                            break;
                        }
                    }
                }
                if( isSet == true ){
                    continue;
                }

                if( road->lanes[i]->previousLanes.size() > 0 ){
                    for(int j=0;j<road->lanes[i]->previousLanes.size();++j){
                        int plIdx = road->indexOfLane( road->lanes[i]->previousLanes[j] );
                        if( plIdx >= 0){

                            if( road->lanes[plIdx]->eWPBoundary == false ){

                                road->lanes[i]->sWPInNode = road->lanes[plIdx]->sWPInNode;
                                road->lanes[i]->sWPNodeDir = road->lanes[plIdx]->sWPNodeDir;

                                road->lanes[i]->eWPInNode = road->lanes[plIdx]->eWPInNode;
                                road->lanes[i]->eWPNodeDir = road->lanes[plIdx]->eWPNodeDir;

                                isSet = true;
                            }
                            else{

                                road->lanes[i]->sWPInNode = road->lanes[plIdx]->eWPInNode;
                                road->lanes[i]->sWPNodeDir = road->lanes[plIdx]->eWPNodeDir;

                                int nIdx = road->indexOfNode( road->lanes[i]->sWPInNode );
                                if( nIdx >= 0 ){
                                    for(int k=0;k<road->nodes[nIdx]->legInfo.size();++k){
                                        if( road->nodes[nIdx]->legInfo[k]->legID == road->lanes[i]->sWPNodeDir ){
                                            road->lanes[i]->eWPInNode = road->nodes[nIdx]->legInfo[k]->connectingNode;
                                            road->lanes[i]->eWPNodeDir = road->nodes[nIdx]->legInfo[k]->connectingNodeInDirect;
                                            break;
                                        }
                                    }
                                }

                                isSet = true;
                            }
                            break;
                        }
                    }
                }
            }
        }

        for(int i=0;i<road->lanes.size();++i){

            if( road->lanes[i]->sWPInNode >= 0 && road->lanes[i]->sWPInNode == road->lanes[i]->eWPInNode &&
                   road->lanes[i]->sWPNodeDir >= 0 && road->lanes[i]->eWPNodeDir < 0 ){

                nChecked++;

                if( road->lanes[i]->nextLanes.size() > 0 ){
                    for(int j=0;j<road->lanes[i]->nextLanes.size();++j){
                        int nlIdx = road->indexOfLane( road->lanes[i]->nextLanes[j] );
                        if( nlIdx >= 0){
                            if( road->lanes[nlIdx]->eWPNodeDir >= 0 ){
                                road->lanes[i]->eWPNodeDir = road->lanes[nlIdx]->eWPNodeDir;
                            }
                            if( road->lanes[nlIdx]->sWPNodeDir < 0 ){
                                road->lanes[nlIdx]->sWPNodeDir = road->lanes[i]->sWPNodeDir;
                            }
                        }
                    }
                }
            }

            if( road->lanes[i]->sWPInNode >= 0 && road->lanes[i]->sWPInNode == road->lanes[i]->eWPInNode &&
                   road->lanes[i]->sWPNodeDir < 0 && road->lanes[i]->eWPNodeDir >= 0 ){

                nChecked++;

                if( road->lanes[i]->previousLanes.size() > 0 ){
                    for(int j=0;j<road->lanes[i]->previousLanes.size();++j){
                        int plIdx = road->indexOfLane( road->lanes[i]->previousLanes[j] );
                        if( plIdx >= 0){
                            if( road->lanes[plIdx]->sWPNodeDir >= 0 ){
                                road->lanes[i]->sWPNodeDir = road->lanes[plIdx]->sWPNodeDir;
                            }
                            if( road->lanes[plIdx]->eWPNodeDir < 0 ){
                                road->lanes[plIdx]->eWPNodeDir = road->lanes[i]->eWPNodeDir;
                            }
                        }
                    }
                }
            }
        }

        if( nChecked == 0 ){
            break;
        }
    }

    road->CheckLaneRelatedNodeAllLanes();



    // Set Lane List

    qDebug() << "Set Lane List.";

    road->SetAllLaneLists();


    // Set Turn Direction Info

    qDebug() << "Set Turn Direction Info.";

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




