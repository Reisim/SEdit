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
#include <QApplication>
#include <QDebug>

int RoadInfo::CreateRoadBoundaryInfo(int assignId, QList<QVector3D *> posData, QList<float> height)
{
    // Id check
    int cId = -1;
    if( assignId < 0 ){
        cId = 0;
        for(int i=0;i<roadBoundary.size();++i){
            if( cId <= roadBoundary[i]->id ){
                cId = roadBoundary[i]->id + 1;
            }
        }
    }
    else{
        if( indexOfRoadBoundary(assignId) < 0 ){
            cId = assignId;
        }
        else{
            qDebug() << "[CreateRoadBoundaryInfo] assigned ID already exists.";
            return -1;
        }
    }


    struct RoadBoundaryInfo *rb = new RoadBoundaryInfo;

    rb->id = cId;

    for(int i=0;i<posData.size();++i){
        rb->pos.append( new QVector3D( posData[i]->x(), posData[i]->y() ,posData[i]->z()) );
    }

    for(int i=1;i<posData.size();++i){
        float dx = posData[i]->x() - posData[i-1]->x();
        float dy = posData[i]->y() - posData[i-1]->y();
        rb->diff.append( new QVector2D(dx,dy) );

        float len = sqrt( dx * dx + dy * dy );
        rb->length.append( len );

        float angle = atan2( dy, dx );
        rb->angles.append( angle );
    }

    for(int i=0;i<height.size();++i){
        rb->height.append( height[i] );
    }

    rb->RoadSide = 0;   // right side is road as default

    roadBoundary.append( rb );

    return cId;
}


void RoadInfo::ClearRoadBoundaryInfo()
{
    QList<int> allRoadBoundaryIDs;
    for(int i=0;i<roadBoundary.size();++i){
        allRoadBoundaryIDs.append( roadBoundary[i]->id );
    }

    for(int i=0;i<allRoadBoundaryIDs.size();++i){
        DeleteRoadBoundaryInfo( allRoadBoundaryIDs[i] );
    }

    roadBoundary.clear();
}


void RoadInfo::DeleteRoadBoundaryInfo(int id)
{
    int index = indexOfRoadBoundary(id);
    if( index < 0 ){
        qDebug() << "[DeleteRoadBoundaryInfo] cannot find index of id = " << id;
        return;
    }

    for(int i=0;i<roadBoundary[index]->pos.size();++i){
        delete roadBoundary[index]->pos[i];
    }
    roadBoundary[index]->pos.clear();

    for(int i=0;i<roadBoundary[index]->diff.size();++i){
        delete roadBoundary[index]->diff[i];
    }
    roadBoundary[index]->diff.clear();

    roadBoundary[index]->angles.clear();
    roadBoundary[index]->length.clear();
    roadBoundary[index]->height.clear();

    delete roadBoundary[index];

    roadBoundary.removeAt(index);
}


void RoadInfo::MoveRoadBoundaryInfo(int id,float moveX,float moveY,float moveZ)
{
    int index = indexOfRoadBoundary(id);
    if( index < 0 ){
        qDebug() << "[MoveRoadBoundaryInfo] cannot find index of id = " << id;
        return;
    }

    for(int i=0;i<roadBoundary[index]->pos.size();++i){
        roadBoundary[index]->pos[i]->setX( roadBoundary[index]->pos[i]->x() + moveX );
        roadBoundary[index]->pos[i]->setY( roadBoundary[index]->pos[i]->y() + moveY );
        roadBoundary[index]->pos[i]->setZ( roadBoundary[index]->pos[i]->z() + moveZ );
    }
}


void RoadInfo::MoveRoadBoundaryPoint(int id,int pIdx,float moveX,float moveY,float moveZ)
{
    int index = indexOfRoadBoundary(id);
    if( index < 0 ){
        qDebug() << "[MoveRoadBoundaryPoint] cannot find index of id = " << id;
        return;
    }

    if( pIdx < 0 || pIdx >= roadBoundary[index]->pos.size() ){
        qDebug() << "[MoveRoadBoundaryPoint] invalid point index of pIdx = " << pIdx;
        return;
    }

    roadBoundary[index]->pos[pIdx]->setX( roadBoundary[index]->pos[pIdx]->x() + moveX );
    roadBoundary[index]->pos[pIdx]->setY( roadBoundary[index]->pos[pIdx]->y() + moveY );
    roadBoundary[index]->pos[pIdx]->setZ( roadBoundary[index]->pos[pIdx]->z() + moveZ );

    UpdateRoadBoundaryInfo( id );
}


void RoadInfo::UpdateRoadBoundaryInfo(int id)
{
    int index = indexOfRoadBoundary(id);
    if( index < 0 ){
        qDebug() << "[UpdateRoadBoundaryInfo] cannot find index of id = " << id;
        return;
    }

    for(int i=1;i<roadBoundary[index]->pos.size();++i){
        float dx = roadBoundary[index]->pos[i]->x() - roadBoundary[index]->pos[i-1]->x();
        float dy = roadBoundary[index]->pos[i]->y() - roadBoundary[index]->pos[i-1]->y();

        roadBoundary[index]->diff[i-1]->setX( dx );
        roadBoundary[index]->diff[i-1]->setY( dy );

        float len = sqrt( dx * dx + dy * dy );
        roadBoundary[index]->length[i-1] = len;

        float angle = atan2( dy, dx );
        roadBoundary[index]->angles[i-1] = angle;
    }
}


int RoadInfo::GetNearestRoadBoundaryInfo(QVector2D pos,float &dist)
{
    int ret = -1;
    float nearDist = 0.0;

    for(int i=0;i<roadBoundary.size();++i){

        for(int j=0;j<roadBoundary[i]->pos.size()-1;++j){

            float dx = pos.x() - roadBoundary[i]->pos[j]->x();
            float dy = pos.y() - roadBoundary[i]->pos[j]->y();

            float ct = cos(roadBoundary[i]->angles[j]);
            float st = sin(roadBoundary[i]->angles[j]);

            float ip = dx * ct + dy * st;
            float cp = dx * (-st) + dy * ct;

            if( ip < 0 || ip > roadBoundary[i]->length[j] ){
                continue;
            }

            if( fabs(cp) > 2.0 ){
                continue;
            }

            float D = fabs(cp);
            if( ret < 0 || nearDist > D ){
                ret = roadBoundary[i]->id;
                nearDist = D;
            }
        }
    }

    qDebug() << "[GetNearestRoadBoundaryInfo] ret = " << ret << " nearDist = " << nearDist;

    dist = nearDist;

    return ret;
}


void RoadInfo::GetNearestRoadBoundaryPoint(QVector2D pos, float &dist,int &nearRoadBaundaryID,int &nearRoadBoundaryPointIndex)
{
    int ret = -1;
    float nearDist = 0.0;

    nearRoadBaundaryID = GetNearestRoadBoundaryInfo(pos, nearDist);
    if( nearRoadBaundaryID < 0 ){
        nearRoadBoundaryPointIndex = -1;
        return;
    }

    nearDist = 0.0;
    int plIdx = indexOfRoadBoundary( nearRoadBaundaryID );

    for(int j=0;j<roadBoundary[plIdx]->pos.size();++j){

        float dx = pos.x() - roadBoundary[plIdx]->pos[j]->x();
        float dy = pos.y() - roadBoundary[plIdx]->pos[j]->y();
        float D = dx * dx + dy * dy;
        if( D > 9.0 ){
            continue;
        }
        if( ret < 0 || nearDist > D ){
            ret = j;
            nearDist = D;
        }
    }

    qDebug() << "[GetNearestRoadBoundaryPoint] ret = " << ret << " nearDist = " << nearDist;

    dist = nearDist;
    nearRoadBoundaryPointIndex = ret;
}


void RoadInfo::DivideRoadBoundarySection(int id, int pIdx1, int pIdx2)
{
    int index = indexOfRoadBoundary(id);
    if( index < 0 ){
        qDebug() << "[DivideRoadBoundarySection] cannot find index of id = " << id;
        return;
    }

    if( pIdx1 < 0 || pIdx1 >= roadBoundary[index]->pos.size() ){
        qDebug() << "[DivideRoadBoundarySection] invalid point index of pIdx1 = " << pIdx1;
        return;
    }

    if( pIdx2 < 0 || pIdx2 >= roadBoundary[index]->pos.size() ){
        qDebug() << "[DivideRoadBoundarySection] invalid point index of pIdx2 = " << pIdx2;
        return;
    }

    if( abs( pIdx2 - pIdx1 ) != 1 ){
        qDebug() << "[DivideRoadBoundarySection] invalid point index, | pIdx1 - pIdx2 | should be 1.";
        return;
    }

    int minIdx = pIdx1;
    int maxIdx = pIdx2;
    if( maxIdx < minIdx ){
        minIdx = pIdx2;
        maxIdx = pIdx1;
    }

    QVector3D* newPos = new QVector3D;

    newPos->setX( (roadBoundary[index]->pos[minIdx]->x() + roadBoundary[index]->pos[maxIdx]->x()) * 0.5 );
    newPos->setY( (roadBoundary[index]->pos[minIdx]->y() + roadBoundary[index]->pos[maxIdx]->y()) * 0.5 );
    newPos->setZ( (roadBoundary[index]->pos[minIdx]->z() + roadBoundary[index]->pos[maxIdx]->z()) * 0.5 );

    roadBoundary[index]->pos.insert( maxIdx, newPos );

    roadBoundary[index]->height.insert( maxIdx, roadBoundary[index]->height[minIdx] );

    roadBoundary[index]->diff.append( new QVector2D( 0.0, 0.0 ) );
    roadBoundary[index]->angles.append( 0.0 );
    roadBoundary[index]->length.append( 0.0 );

    UpdateRoadBoundaryInfo(id);
}


void RoadInfo::MergeRoadBoundarySection(int id, int pIdx)
{
    int index = indexOfRoadBoundary(id);
    if( index < 0 ){
        qDebug() << "[MergeRoadBoundarySection] cannot find index of id = " << id;
        return;
    }

    if( pIdx <= 0 || pIdx >= roadBoundary[index]->pos.size() - 1 ){
        qDebug() << "[MergeRoadBoundarySection] invalid point index of pIdx = " << pIdx;
        return;
    }

    delete roadBoundary[index]->pos[pIdx];
    roadBoundary[index]->pos.removeAt( pIdx );

    delete roadBoundary[index]->diff[pIdx-1];
    roadBoundary[index]->diff.removeAt( pIdx - 1);

    roadBoundary[index]->height.removeAt( pIdx - 1 );
    roadBoundary[index]->angles.removeAt( pIdx - 1 );
    roadBoundary[index]->length.removeAt( pIdx - 1 );

    UpdateRoadBoundaryInfo( id );
}


void RoadInfo::SetRoadBoudaryInfoRoadSide(int id,int roadSide)
{
    int index = indexOfRoadBoundary(id);
    if( index < 0 ){
        qDebug() << "[SetRoadBoudaryInfoRoadSide] cannot find index of id = " << id;
        return;
    }

    roadBoundary[index]->RoadSide = roadSide;
    if( roadBoundary[index]->RoadSide < 0 ){
        roadBoundary[index]->RoadSide = 0;
    }
    else if( roadBoundary[index]->RoadSide > 1 ){
        roadBoundary[index]->RoadSide = 1;
    }
}

