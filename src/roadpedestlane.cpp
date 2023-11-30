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
#include "workingthread.h"

#include <QProgressDialog>
#include <QApplication>
#include <QDebug>


int RoadInfo::CreatePedestLane(int assignId, QList<QVector3D *> posData)
{
    if( posData.size() < 2 ){
        return -1;
    }

    // Id check
    int cId = -1;
    if( assignId < 0 ){
        cId = 0;
        for(int i=0;i<pedestLanes.size();++i){
            if( cId <= pedestLanes[i]->id ){
                cId = pedestLanes[i]->id + 1;
            }
        }
    }
    else{
        if( indexOfPedestLane(assignId) < 0 ){
            cId = assignId;
        }
        else{
            qDebug() << "[CreatePedestLane] assigned ID already exists.";
            return -1;
        }
    }

    struct PedestrianLane *pl = new struct PedestrianLane;

    pl->id = cId;

    for(int i=0;i<posData.size();++i){

        struct PedestrianLaneShapeElement *ple = new PedestrianLaneShapeElement;

        ple->pos.setX( posData[i]->x() );
        ple->pos.setY( posData[i]->y() );
        ple->pos.setZ( posData[i]->z() );

        ple->width = 2.0;

        ple->runOutProb = 0.0;
        ple->runOutDirect = 1;   // 1 for Left, -1 for Right
        ple->marginToRoadForRunOut = 0.0;

        ple->isCrossWalk = false;
        ple->controlPedestSignalID = -1;

        ple->angleToNextPos = 0.0;
        ple->distanceToNextPos = 0.0;

        pl->shape.append( ple );
    }

    for(int i=0;i<pl->shape.size()-1;++i){

        float dx = pl->shape[i+1]->pos.x() - pl->shape[i]->pos.x();
        float dy = pl->shape[i+1]->pos.y() - pl->shape[i]->pos.y();

        pl->shape[i]->angleToNextPos    = atan2( dy, dx );   // [rad]
        pl->shape[i]->distanceToNextPos = sqrt( dx * dx + dy * dy );
    }

    pedestLanes.append( pl );

    return cId;
}


void RoadInfo::DeletePedestLane(int id)
{
    int index = indexOfPedestLane(id);
    if( index < 0 ){
        qDebug() << "[DeletePedestLane] cannot find index of id = " << id;
        return;
    }

    // shape
    for(int i=0;i<pedestLanes[index]->shape.size();++i){
        delete pedestLanes[index]->shape[i];
    }
    pedestLanes[index]->shape.clear();


    // pedestLanes
    delete pedestLanes[index];

    pedestLanes.removeAt(index);
}


void RoadInfo::UpdatePedestLaneShapeParams(int id)
{
    int index = indexOfPedestLane(id);
    if( index < 0 ){
        qDebug() << "[UpdatePedestLaneShapeParams] cannot find index of id = " << id;
        return;
    }

    struct PedestrianLane *pl = pedestLanes[index];

    for(int i=0;i<pl->shape.size()-1;++i){

        float dx = pl->shape[i+1]->pos.x() - pl->shape[i]->pos.x();
        float dy = pl->shape[i+1]->pos.y() - pl->shape[i]->pos.y();

        pl->shape[i]->angleToNextPos    = atan2( dy, dx );   // [rad]
        pl->shape[i]->distanceToNextPos = sqrt( dx * dx + dy * dy );
    }
}


void RoadInfo::MovePedestLane(int id, float moveX, float moveY)
{
    int index = indexOfPedestLane(id);
    if( index < 0 ){
        qDebug() << "[MovePedestLane] cannot find index of id = " << id;
        return;
    }

    for(int i=0;i<pedestLanes[index]->shape.size();++i){
        pedestLanes[index]->shape[i]->pos.setX( pedestLanes[index]->shape[i]->pos.x() + moveX );
        pedestLanes[index]->shape[i]->pos.setY( pedestLanes[index]->shape[i]->pos.y() + moveY );
    }

    UpdatePedestLaneShapeParams(id);
}


void RoadInfo::MovePedestLanePoint(int id, int pIdx, float moveX, float moveY)
{
    int index = indexOfPedestLane(id);
    if( index < 0 ){
        qDebug() << "[MovePedestLanePoint] cannot find index of id = " << id;
        return;
    }
    if( pIdx < 0 || pIdx >= pedestLanes[index]->shape.size() ){
        qDebug() << "[MovePedestLanePoint] pIdx = " << pIdx << ": invalid index for pedestLanes.shape ; size = " << pedestLanes[index]->shape.size();
        return;
    }

    pedestLanes[index]->shape[pIdx]->pos.setX( pedestLanes[index]->shape[pIdx]->pos.x() + moveX );
    pedestLanes[index]->shape[pIdx]->pos.setY( pedestLanes[index]->shape[pIdx]->pos.y() + moveY );

    UpdatePedestLaneShapeParams(id);
}


void RoadInfo::SetPedestLaneIsCrossWalk(int id, int pIdx, bool flag)
{
    int index = indexOfPedestLane(id);
    if( index < 0 ){
        qDebug() << "[SetPedestLaneIsCrossWalk] cannot find index of id = " << id;
        return;
    }
    if( pIdx < 0 || pIdx >= pedestLanes[index]->shape.size() ){
        qDebug() << "[SetPedestLaneIsCrossWalk] pIdx = " << pIdx << ": invalid index for pedestLanes.shape ; size = " << pedestLanes[index]->shape.size();
        return;
    }

    pedestLanes[index]->shape[pIdx]->isCrossWalk = flag;
}


void RoadInfo::SetPedestLaneRunOutData(int id, int pIdx, float prob, int dir)
{
    int index = indexOfPedestLane(id);
    if( index < 0 ){
        qDebug() << "[SetPedestLaneIsCrossWalk] cannot find index of id = " << id;
        return;
    }
    if( pIdx < 0 || pIdx >= pedestLanes[index]->shape.size() ){
        qDebug() << "[SetPedestLaneIsCrossWalk] pIdx = " << pIdx << ": invalid index for pedestLanes.shape ; size = " << pedestLanes[index]->shape.size();
        return;
    }

    pedestLanes[index]->shape[pIdx]->runOutProb   = prob;
    pedestLanes[index]->shape[pIdx]->runOutDirect = dir;
}


void RoadInfo::SetPedestLaneWidth(int id, int pIdx, float w)
{
    int index = indexOfPedestLane(id);
    if( index < 0 ){
        qDebug() << "[SetPedestLaneWidth] cannot find index of id = " << id;
        return;
    }
    if( pIdx < 0 || pIdx >= pedestLanes[index]->shape.size() ){
        qDebug() << "[SetPedestLaneWidth] pIdx = " << pIdx << ": invalid index for pedestLanes.shape ; size = " << pedestLanes[index]->shape.size();
        return;
    }

    pedestLanes[index]->shape[pIdx]->width = w;
}


int RoadInfo::GetNearestPedestLane(QVector2D pos, float &dist)
{
    int ret = -1;
    float nearDist = 0.0;

    for(int i=0;i<pedestLanes.size();++i){

//        qDebug() << "Test " << pedestLanes[i]->id;

        for(int j=0;j<pedestLanes[i]->shape.size()-1;++j){

            float dx = pos.x() - pedestLanes[i]->shape[j]->pos.x();
            float dy = pos.y() - pedestLanes[i]->shape[j]->pos.y();

            float ct = cos(pedestLanes[i]->shape[j]->angleToNextPos);
            float st = sin(pedestLanes[i]->shape[j]->angleToNextPos);

            float ip = dx * ct + dy * st;
            float cp = dx * (-st) + dy * ct;


//            qDebug() << "j=" << j << " ip = " << ip << " cp = " << cp
//                     << " length = " << pedestLanes[i]->shape[j]->distanceToNextPos
//                     << " width = " << pedestLanes[i]->shape[j]->width;


            if( ip < 0 || ip > pedestLanes[i]->shape[j]->distanceToNextPos ){
                continue;
            }

            if( fabs(cp) > pedestLanes[i]->shape[j]->width ){
                continue;
            }

            float D = fabs(cp);
            if( ret < 0 || nearDist > D ){
                ret = pedestLanes[i]->id;
                nearDist = D;
            }
        }
    }

    qDebug() << "[GetNearestPedestLane] ret = " << ret << " nearDist = " << nearDist;

    dist = nearDist;

    return ret;
}


void RoadInfo::GetNearestPedestLanePoint(QVector2D pos, float &dist,int &nearPedestLaneID,int &nearPedestLanePointIndex)
{
    int ret = -1;
    float nearDist = 0.0;

    nearPedestLaneID = GetNearestPedestLane(pos, nearDist);
    if( nearPedestLaneID < 0 ){
        nearPedestLanePointIndex = -1;
        return;
    }

    nearDist = 0.0;
    int plIdx = indexOfPedestLane( nearPedestLaneID );


    for(int j=0;j<pedestLanes[plIdx]->shape.size();++j){

        float dx = pos.x() - pedestLanes[plIdx]->shape[j]->pos.x();
        float dy = pos.y() - pedestLanes[plIdx]->shape[j]->pos.y();
        float D = dx * dx + dy * dy;
        if( D > 9.0 ){
            continue;
        }
        if( ret < 0 || nearDist > D ){
            ret = j;
            nearDist = D;
        }
    }

    qDebug() << "[GetNearestPedestLanePoint] ret = " << ret << " nearDist = " << nearDist;

    dist = nearDist;
    nearPedestLanePointIndex = ret;
}



void RoadInfo::FindPedestSignalFroCrossWalk()
{
    qDebug() << "[RoadInfo::FindPedestSignalFroCrossWalk]";

    for(int i=0;i<pedestLanes.size();++i){

        for(int j=0;j<pedestLanes[i]->shape.size();++j){

            if( pedestLanes[i]->shape[j]->isCrossWalk == false ){
                pedestLanes[i]->shape[j]->controlPedestSignalID = -1;
                continue;
            }

            qDebug() << " Checking pedestLane ID = " << pedestLanes[i]->id << " , sectionIndex = " << j;

            pedestLanes[i]->shape[j]->controlPedestSignalID = -1;

            float ct = cos( pedestLanes[i]->shape[j]->angleToNextPos );
            float st = sin( pedestLanes[i]->shape[j]->angleToNextPos );

            int nearNode = -1;
            int nearDir  = -1;
            int minDist  = 0.0;
            for(int k=0;k<nodes.size();++k){

                if( nodes[k]->hasTS == false ){
                    continue;
                }

                for(int l=0;l<nodes[k]->legInfo.size();++l){

                    int nWPin = nodes[k]->legInfo[l]->inWPs.size();
                    for(int m=0;m<nWPin;++m){

                        int wpIdx = indexOfWP( nodes[k]->legInfo[l]->inWPs[m] );
                        if( wpIdx >= 0 ){

                            float dx = wps[wpIdx]->pos.x() - pedestLanes[i]->shape[j]->pos.x();
                            float dy = wps[wpIdx]->pos.y() - pedestLanes[i]->shape[j]->pos.y();

                            float L = dx * ct + dy * st;
                            float e = dx * (-st) + dy * ct;

                            if( L < 0.0 || L > pedestLanes[i]->shape[j]->distanceToNextPos ){
                                continue;
                            }

                            if( nearNode < 0 || minDist > fabs(e) ){
                                nearNode = nodes[k]->id;
                                nearDir  = nodes[k]->legInfo[l]->legID;
                                minDist  = fabs(e);
                            }
                        }
                    }
                }
            }

            qDebug() << "nearNode = " << nearNode << " nearDir = " << nearDir;


            if( nearNode < 0 ){
                continue;
            }

            int ndIdx = indexOfNode( nearNode );
            if( ndIdx < 0 ){
                continue;
            }

            for(int k=0;k<nodes[ndIdx]->trafficSignals.size();++k){

                if( nodes[ndIdx]->trafficSignals[k]->type != 1 ){
                    continue;
                }

                if( nodes[ndIdx]->trafficSignals[k]->controlNodeDirection != nearDir ){
                    continue;
                }

                pedestLanes[i]->shape[j]->controlPedestSignalID   = nodes[ndIdx]->trafficSignals[k]->id;
                nodes[ndIdx]->trafficSignals[k]->controlCrossWalk = pedestLanes[i]->id;

                qDebug() << " Crosswalk: pedestPath id =" << pedestLanes[i]->id << " , section index = " << j;
                qDebug() << "   controlPedestSignalID = " << pedestLanes[i]->shape[j]->controlPedestSignalID;
            }
        }
    }
}


void RoadInfo::ClearPedestLanes()
{
    QList<int> allPedestLaneIDs;
    for(int i=0;i<pedestLanes.size();++i){
        allPedestLaneIDs.append( pedestLanes[i]->id );
    }

    for(int i=0;i<allPedestLaneIDs.size();++i){
        DeletePedestLane( allPedestLaneIDs[i] );
    }
}


bool RoadInfo::CheckPedestLaneCrossPoints()
{
    bool ret = true;

    for(int i=0;i<lanes.size();++i){

        for(int j=0;j<lanes[i]->pedestCrossPoints.size();++j){
            delete lanes[i]->pedestCrossPoints[j];
        }
        lanes[i]->pedestCrossPoints.clear();
    }


    int nThread = 8;
    WorkingThread *wt = new WorkingThread[nThread];
    for(int i=0;i<nThread;++i){
        wt[i].mode = 6;
        wt[i].road = this;
        wt[i].wtID = i;
    }

    int thrIdx = 0;
    for(int i=0;i<lanes.size();++i){
        wt[thrIdx].params.append( i );
        thrIdx++;
        if( thrIdx == nThread ){
            thrIdx = 0;
        }
    }

    for(int i=0;i<nThread;++i){
        wt[i].start();
    }


    QProgressDialog *pd = new QProgressDialog("CheckPedestLaneCrossPoints", "Cancel", 0, lanes.size(), 0);
    pd->setWindowModality(Qt::WindowModal);
    pd->setAttribute( Qt::WA_DeleteOnClose );
    pd->setWindowIcon(QIcon(":images/SEdit-icon.png"));
    pd->show();

    pd->setValue(0);
    QApplication::processEvents();

    qDebug() << "[RoadInfo::CheckPedestLaneCrossPoints]";

    while(1){

        int nFinish = 0;
        int nProcessed = 0;
        for(int i=0;i<nThread;++i){
            nProcessed += wt[i].nProcessed;
            if( wt[i].mode < 0 ){
                nFinish++;
            }
        }

        pd->setValue(nProcessed);
        QApplication::processEvents();
        if( pd->wasCanceled() ){
            qDebug() << "Canceled.";
            for(int i=0;i<nThread;++i){
                if( wt[i].mode > 0 ){
                    wt[i].SetStopFlag();
                }
            }

            if( nFinish == nThread ){
                break;
            }

        }
        else if( nFinish == nThread ){
            qDebug() << "Finished.";
            break;
        }
    }

    pd->setValue( lanes.size() );

    pd->close();

    delete [] wt;

    return ret;
}


void RoadInfo::DividePedestLaneHalf(int id,int sect)
{
    qDebug() << "[RoadInfo::DividePedestLaneHalf] id = " << id << " sect=" << sect;

    int index = indexOfPedestLane(id);
    if( index < 0 ){
        qDebug() << "[DividePedestLaneHalf] cannot find index of id = " << id;
        return;
    }
    if( sect < 0 || sect >= pedestLanes[index]->shape.size() - 1 ){
        qDebug() << "[DividePedestLaneHalf] sect = " << sect << ": invalid index for pedestLanes.shape ; size = " << pedestLanes[index]->shape.size();
        return;
    }

    struct PedestrianLaneShapeElement *plse = new struct PedestrianLaneShapeElement;

    plse->pos = (pedestLanes[index]->shape[sect]->pos + pedestLanes[index]->shape[sect+1]->pos) / 2;
    plse->angleToNextPos = pedestLanes[index]->shape[sect]->angleToNextPos;
    plse->width = pedestLanes[index]->shape[sect]->width;
    plse->isCrossWalk = false;
    plse->controlPedestSignalID = -1;
    plse->runOutDirect = 0;
    plse->runOutProb = 0.0;
    plse->marginToRoadForRunOut = 0.0;

    pedestLanes[index]->shape.insert(sect+1, plse);

    UpdatePedestLaneShapeParams(id);
}

void RoadInfo::DividePedestLaneAtPos(int id, int sect, QVector3D atPoint)
{
    qDebug() << "[RoadInfo::DividePedestLaneAtPos] id = " << id << " sect=" << sect;

    int index = indexOfPedestLane(id);
    if( index < 0 ){
        qDebug() << "[DividePedestLaneAtPos] cannot find index of id = " << id;
        return;
    }
    if( sect < 0 || sect >= pedestLanes[index]->shape.size() - 1 ){
        qDebug() << "[DividePedestLaneAtPos] sect = " << sect << ": invalid index for pedestLanes.shape ; size = " << pedestLanes[index]->shape.size();
        return;
    }

    struct PedestrianLaneShapeElement *plse = new struct PedestrianLaneShapeElement;

    plse->pos = atPoint;
    plse->angleToNextPos = pedestLanes[index]->shape[sect]->angleToNextPos;
    plse->width = pedestLanes[index]->shape[sect]->width;
    plse->isCrossWalk = false;
    plse->controlPedestSignalID = -1;
    plse->runOutDirect = 0;
    plse->runOutProb = 0.0;
    plse->marginToRoadForRunOut = 0.0;

    pedestLanes[index]->shape.insert(sect+1, plse);

    UpdatePedestLaneShapeParams(id);
}

void RoadInfo::ChangePedestLaneWidthByWheel(int id, int sect, float diff)
{
    qDebug() << "[RoadInfo::ChangePedestLaneWidthByWheel] id = " << id << " sect=" << sect;

    int index = indexOfPedestLane(id);
    if( index < 0 ){
        qDebug() << "[ChangePedestLaneWidthByWheel] cannot find index of id = " << id;
        return;
    }
    if( sect < 0 || sect >= pedestLanes[index]->shape.size() - 1 ){
        qDebug() << "[ChangePedestLaneWidthByWheel] sect = " << sect << ": invalid index for pedestLanes.shape ; size = " << pedestLanes[index]->shape.size();
        return;
    }

    pedestLanes[index]->shape[sect]->width += diff;
    if( pedestLanes[index]->shape[sect]->width < 0.5 ){
        pedestLanes[index]->shape[sect]->width = 0.5;
    }
}

