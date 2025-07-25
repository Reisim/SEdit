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


#include "roadinfo.h"
#include <QProgressDialog>
#include <QApplication>
#include <QDebug>

#include "workingthread.h"



int RoadInfo::CreateStopLine(int assignId, int relatedNodeID, int relatedNodeDir, int SLType)
{
    int rndIdx = indexOfNode( relatedNodeID );
    if( rndIdx < 0 ){
        qDebug() << "[RoadInfo::CreateStopLine] invalid relatedNodeID, no Node found.";
        return -1;
    }

    bool nDirCheck = false;
    float angle = 0.0;
    for(int i=0;i<nodes[rndIdx]->legInfo.size();++i){
        if( nodes[rndIdx]->legInfo[i]->legID == relatedNodeDir ){
            nDirCheck = true;
            angle = nodes[rndIdx]->legInfo[i]->angle * 0.017452;
            break;
        }
    }
    if( nDirCheck == false ){
        qDebug() << "[RoadInfo::CreateStopLine] invalid relatedNodeDir, no Leg found.";
        return -1;
    }

    // Id check
    int cId = -1;
    if( assignId < 0 ){
        cId = 0;
        for(int i=0;i<nodes.size();++i){
            for(int j=0;j<nodes[i]->stopLines.size();++j){
                if( cId <= nodes[i]->stopLines[j]->id ){
                    cId = nodes[i]->stopLines[j]->id + 1;
                }
            }
        }
    }
    else{
        if( indexOfSL(assignId,-1) < 0 ){
            cId = assignId;
        }
        else{
            qDebug() << "[CreateStopLine] assigned ID already exists.";
            return -1;
        }
    }


    struct StopLineInfo *sl = new StopLineInfo;

    sl->id = cId;
    sl->stopLineType   = SLType;
    sl->relatedNode    = relatedNodeID;
    sl->relatedNodeDir = relatedNodeDir;

    int nIdx = indexOfNode( relatedNodeID );
    if( nIdx >= 0 ){

        float angle = GetNodeLegAngle( relatedNodeID, relatedNodeDir );
        angle *= 0.017452;
        float cp = cos( angle );
        float sp = sin( angle );
        QVector2D nodeCenter = GetNodePosition( relatedNodeID );

        float L = 0.0;
        float yMax = -100.0;
        float yMin = 100.0;

        for(int i=0;i<nodes[nIdx]->relatedLanes.size();++i){
            int lidx = indexOfLane( nodes[nIdx]->relatedLanes[i] );
            if( lidx >= 0 ){

                if( lanes[lidx]->eWPInNode == relatedNodeID && lanes[lidx]->sWPInNode != relatedNodeID
                        && lanes[lidx]->eWPNodeDir == relatedNodeDir && lanes[lidx]->eWPBoundary == true ){

                    float dx = lanes[lidx]->shape.pos.last()->x() - nodeCenter.x();
                    float dy = lanes[lidx]->shape.pos.last()->y() - nodeCenter.y();
                    float Lt = dx * cp + dy * sp;
                    float Yt = dx * (-sp) + dy * cp;

                    if( L < Lt ){
                        L = Lt;
                    }
                    if( yMax < Yt ){
                        yMax = Yt;
                    }
                    if( yMin > Yt ){
                        yMin = Yt;
                    }
                }
            }
        }

        float xSLc = nodeCenter.x() + cp * (L + 2.0);
        float ySLc = nodeCenter.y() + sp * (L + 2.0);

        sl->rightEdge.setX( xSLc - sp * (yMax + 1.0) );
        sl->rightEdge.setY( ySLc + cp * (yMax + 1.0) );

        sl->leftEdge.setX( xSLc - sp * (yMin - 1.0) );
        sl->leftEdge.setY( ySLc + cp * (yMin - 1.0) );

        //qDebug() << "Stopline : (" << sl->leftEdge.x() << "," << sl->leftEdge.y() << ") and (" << sl->rightEdge.x() << "," << sl->rightEdge.y() << ")";
    }

    nodes[rndIdx]->stopLines.append( sl );

    return cId;
}


int RoadInfo::CreateStopLineAtLAne(int assignId,int onLane,float X,float Y,float angle,int SLType)
{
    int lnIdx = indexOfLane(onLane);
    if( lnIdx < 0 || lnIdx >= lanes.size() ){
        qDebug() << "[CreateStopLineAtLAne] Invalid onLane ID = " << onLane;
        return -1;
    }

    //qDebug() << " [CreateStopLineAtLAne] lnIdx = " << lnIdx;

    int relatedNodeID = lanes[lnIdx]->connectedNode;

    //qDebug() << " [CreateStopLineAtLAne] relatedNodeID = " << relatedNodeID;

    int rndIdx = indexOfNode( relatedNodeID );
    if( rndIdx < 0 ){
        qDebug() << "[CreateStopLineAtLAne] Invalid related Node ID = " << relatedNodeID;
        return -1;
    }

    //qDebug() << " [CreateStopLineAtLAne] rndIdx = " << rndIdx;

    int relatedNodeDir = lanes[lnIdx]->connectedNodeInDirect;

    //qDebug() << " [CreateStopLineAtLAne] relatedNodeDir = " << relatedNodeDir;


    // // Id check
    int cId = -1;
    if( assignId < 0 ){
        cId = 0;
        for(int i=0;i<nodes.size();++i){
            for(int j=0;j<nodes[i]->stopLines.size();++j){
                if( cId <= nodes[i]->stopLines[j]->id ){
                    cId = nodes[i]->stopLines[j]->id + 1;
                }
            }
        }
    }
    else{
        if( indexOfSL(assignId,-1) < 0 ){
            cId = assignId;
        }
        else{
            qDebug() << "[CreateStopLineAtLAne] assigned ID already exists.";
            return -1;
        }
    }

    //qDebug() << " [CreateStopLineAtLAne] cId = " << cId;


    struct StopLineInfo *sl = new StopLineInfo;

    sl->id = cId;
    sl->stopLineType   = SLType;
    sl->relatedNode    = relatedNodeID;
    sl->relatedNodeDir = relatedNodeDir;

    float cp = cos(angle);
    float sp = sin(angle);

    //qDebug() << " [CreateStopLineAtLAne] angle = " << angle * 57.3;

    float LW = 1.25;
    sl->rightEdge.setX( X - sp * LW );
    sl->rightEdge.setY( Y + cp * LW );

    sl->leftEdge.setX( X + sp * LW );
    sl->leftEdge.setY( Y - cp * LW );

    if( nodes.size() > rndIdx ){
        nodes[rndIdx]->stopLines.append( sl );
    }
    else{
        qDebug() << "Size of nodes = " << nodes.size() << ", smaller than rndIdx = " << rndIdx;
    }
    //qDebug() << " [CreateStopLineAtLAne] return";

    return cId;
}


int RoadInfo::GetNearestStopLine(QVector2D pos, float &dist)
{
    int ret = -1;
    float nearDist = 0.0;

    for(int i=0;i<nodes.size();++i){

        float dx = nodes[i]->pos.x() - pos.x();
        float dy = nodes[i]->pos.y() - pos.y();
        float D = dx * dx + dy * dy;
        if( D > 40000.0 ){
            continue;
        }

        for(int j=0;j<nodes[i]->stopLines.size();++j){

            dx = nodes[i]->stopLines[j]->leftEdge.x() - pos.x();
            dy = nodes[i]->stopLines[j]->leftEdge.y() - pos.y();
            D = dx * dx + dy * dy;
            if( ret < 0 || nearDist > D ){
                ret = nodes[i]->stopLines[j]->id;
                nearDist = D;
            }

            dx = nodes[i]->stopLines[j]->rightEdge.x() - pos.x();
            dy = nodes[i]->stopLines[j]->rightEdge.y() - pos.y();
            D = dx * dx + dy * dy;
            if( ret < 0 || nearDist > D ){
                ret = nodes[i]->stopLines[j]->id;
                nearDist = D;
            }
        }
    }

    qDebug() << "[GetNearestStopLine] ret = " << ret << " nearDist = " << nearDist;

    dist = nearDist;

    return ret;
}


void RoadInfo::CheckAllStopLineCrossLane()
{

    QList<int> SLList;
    for(int i=0;i<nodes.size();++i){
        for(int j=0;j<nodes[i]->stopLines.size();++j){
            SLList.append( nodes[i]->stopLines[j]->id );
        }
    }
    if( SLList.size() == 0 ){
        return;
    }


    // Clear stopline info of Lanes
    for(int i=0;i<lanes.size();++i){
        if( lanes[i]->stopPoints.size() == 0 ){
            continue;
        }
        for(int j=0;j<lanes[i]->stopPoints.size();++j){
            delete lanes[i]->stopPoints[j];
        }
        lanes[i]->stopPoints.clear();
    }


    int nThread = 8;
    WorkingThread *wt = new WorkingThread[nThread];
    int nSLWT = (SLList.size() / nThread + 1);

    int SLIndex = 0;
    for(int i=0;i<nThread;++i){
        int nSLCount = 0;

        wt[i].mode = 1;
        wt[i].road = this;
        wt[i].wtID = i;

        for(int j=0;j<nSLWT;++j){
            if( SLIndex < SLList.size() ){
                wt[i].params.append( SLList[SLIndex] );
                SLIndex++;
            }
        }
    }


    for(int i=0;i<nThread;++i){
        wt[i].start();
    }

    QProgressDialog *pd = new QProgressDialog("CheckAllStopLineCrossLane", "Cancel", 0, nThread, 0);

    pd->setWindowModality(Qt::WindowModal);
    pd->setAttribute( Qt::WA_DeleteOnClose );
    pd->setWindowIcon(QIcon(":images/SEdit-icon.png"));
    pd->show();

    pd->setValue(0);
    QApplication::processEvents();

    while(1){

        int nFinish = 0;
        for(int i=0;i<nThread;++i){

            if( wt[i].mode < 0 ){
                nFinish++;
            }
        }

        pd->setValue(nFinish);
        QApplication::processEvents();
        if( pd->wasCanceled() ){
            qDebug() << "Canceled.";
            break;
        }
        else if( nFinish == nThread ){
            qDebug() << "Finished.";
            break;
        }
    }

    pd->close();

    delete [] wt;

}


void RoadInfo::CheckStopLineCrossLanes(int id)
{
    int rNode = indexOfSL(id,-1);
    if( rNode < 0 ){
        qDebug() << "[RoadInfo::CheckStopLineCrossLanes] can not find node contain this stopline: id = " << id;
        return;
    }
    int nIdx = indexOfNode( rNode );
    if( nIdx < 0 ){
        qDebug() << "[RoadInfo::CheckStopLineCrossLanes] can not get index of node = " << rNode;
        return;
    }
    int idx = indexOfSL(id, rNode);
    if( idx < 0 ){
        qDebug() << "[RoadInfo::CheckStopLineCrossLanes] can not find stopline: id = " << id;
        return;
    }

    nodes[nIdx]->stopLines[idx]->crossLanes.clear();

    for(int i=0;i<nodes[nIdx]->relatedLanes.size();++i){

        int laneID = nodes[nIdx]->relatedLanes[i];

        struct CrossPointInfo* cp = CheckLaneCrossPoint(laneID,
                                                        QPointF(nodes[nIdx]->stopLines[idx]->leftEdge.x(),nodes[nIdx]->stopLines[idx]->leftEdge.y()),
                                                        QPointF(nodes[nIdx]->stopLines[idx]->rightEdge.x(),nodes[nIdx]->stopLines[idx]->rightEdge.y()) );

//        if( cp == NULL ){
//            qDebug() << "No Cross.";
//            continue;
//        }

        if( cp != NULL ){

//            qDebug() << "cp @ " << cp->pos;

            nodes[nIdx]->stopLines[idx]->crossLanes.append( laneID );

            int lidx = indexOfLane( laneID );
            if( lidx >= 0 ){

                bool alreadyExist = false;
                for(int j=0;j<lanes[lidx]->stopPoints.size();++j){
                    if( lanes[lidx]->stopPoints[j]->stoplineID == nodes[nIdx]->stopLines[idx]->id ){

                        lanes[lidx]->stopPoints[j]->stoplineType = nodes[nIdx]->stopLines[idx]->stopLineType;

                        lanes[lidx]->stopPoints[j]->pos = cp->pos;
                        lanes[lidx]->stopPoints[j]->derivative = cp->derivative;
                        lanes[lidx]->stopPoints[j]->distanceFromLaneStartPoint = cp->distanceFromLaneStartPoint;

                        alreadyExist = true;
                        break;
                    }
                }

                if( alreadyExist == false ){
                    struct StopPointInfo* sp = new StopPointInfo;

                    sp->stoplineID = nodes[nIdx]->stopLines[idx]->id;
                    sp->stoplineType = nodes[nIdx]->stopLines[idx]->stopLineType;
                    sp->pos = cp->pos;
                    sp->derivative = cp->derivative;
                    sp->distanceFromLaneStartPoint = cp->distanceFromLaneStartPoint;

                    lanes[lidx]->stopPoints.append( sp );
                }
            }

            delete cp;
        }
    }
}


void RoadInfo::MoveStopLine(int id, float moveX, float moveY)
{
    int rNd = indexOfSL(id,-1);
    if( rNd < 0 ){
        qDebug() << "[RoadInfo::MoveStopLine] cannot find node contain SL id = " << id;
        return;
    }
    int idx = indexOfSL(id,rNd);
    if( idx < 0 ){
        qDebug() << "[RoadInfo::MoveStopLine] cannot find index of SL id = " << id;
        return;
    }
    int ndIdx = indexOfNode(rNd);
    if( ndIdx < 0 ){
        qDebug() << "[RoadInfo::MoveStopLine] cannot find index of Node id = " << rNd;
        return;
    }

    nodes[ndIdx]->stopLines[idx]->leftEdge.setX( nodes[ndIdx]->stopLines[idx]->leftEdge.x() + moveX );
    nodes[ndIdx]->stopLines[idx]->leftEdge.setY( nodes[ndIdx]->stopLines[idx]->leftEdge.y() + moveY );

    nodes[ndIdx]->stopLines[idx]->rightEdge.setX( nodes[ndIdx]->stopLines[idx]->rightEdge.x() + moveX );
    nodes[ndIdx]->stopLines[idx]->rightEdge.setY( nodes[ndIdx]->stopLines[idx]->rightEdge.y() + moveY );
}


void RoadInfo::ChangeStopLineLength(int id,float fact)
{
    int rNd = indexOfSL(id,-1);
    if( rNd < 0 ){
        qDebug() << "[RoadInfo::ChangeStopLineLength] cannot find node contain SL id = " << id;
        return;
    }
    int idx = indexOfSL(id,rNd);
    if( idx < 0 ){
        qDebug() << "[RoadInfo::ChangeStopLineLength] cannot find index of SL id = " << id;
        return;
    }
    int ndIdx = indexOfNode(rNd);
    if( ndIdx < 0 ){
        qDebug() << "[RoadInfo::ChangeStopLineLength] cannot find index of Node id = " << rNd;
        return;
    }

    QVector2D center = (nodes[ndIdx]->stopLines[idx]->leftEdge + nodes[ndIdx]->stopLines[idx]->rightEdge) / 2.0;

    nodes[ndIdx]->stopLines[idx]->leftEdge  = (nodes[ndIdx]->stopLines[idx]->leftEdge - center) * fact + center;
    nodes[ndIdx]->stopLines[idx]->rightEdge = (nodes[ndIdx]->stopLines[idx]->rightEdge - center) * fact + center;
}


void RoadInfo::DeleteStopLine(int id)
{
    int rNd = indexOfSL(id,-1);
    if( rNd < 0 ){
        qDebug() << "[RoadInfo::DeleteStopLine] cannot find node contain SL id = " << id;
        return;
    }
    int idx = indexOfSL(id,rNd);
    if( idx < 0 ){
        qDebug() << "[RoadInfo::DeleteStopLine] cannot find index of SL id = " << id;
        return;
    }
    int ndIdx = indexOfNode(rNd);
    if( ndIdx < 0 ){
        qDebug() << "[RoadInfo::DeleteStopLine] cannot find index of Node id = " << rNd;
        return;
    }

    nodes[ndIdx]->stopLines[idx]->crossLanes.clear();

    delete nodes[ndIdx]->stopLines[idx];

    nodes[ndIdx]->stopLines.removeAt(idx);
}
