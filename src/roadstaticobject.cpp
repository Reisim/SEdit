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


int RoadInfo::CreateStaticObject(int assignId)
{
    // Id check
    int cId = -1;
    if( assignId < 0 ){
        cId = 0;
        for(int i=0;i<staticObj.size();++i){
            if( cId <= staticObj[i]->id ){
                cId = staticObj[i]->id + 1;
            }
        }
    }
    else{
        if( indexOfStaticObject(assignId) < 0 ){
            cId = assignId;
        }
        else{
            qDebug() << "[CreateStaticObject] assigned ID already exists.";
            return -1;
        }
    }


    struct StaticObject *so = new StaticObject;

    so->id = cId;
    so->xc = 0.0;
    so->yc = 0.0;
    so->zc = 0.0;
    so->lenx = 1.0;
    so->leny = 1.0;
    so->height = 1.0;
    so->direction = 0.0;

    staticObj.append( so );

    SetCornerPointsStaticObject( cId );

    return cId;
}


void RoadInfo::SetCornerPointsStaticObject(int id)
{
    int index = indexOfStaticObject(id);
    if( index < 0 ){
        qDebug() << "[SetCornerPointsStaticObject] cannot find index of id = " << id;
        return;
    }

    float dirInDeg = staticObj[index]->direction * 0.017452;
    float cDir = cos( dirInDeg );
    float sDir = sin( dirInDeg );

    float x = staticObj[index]->xc;
    float y = staticObj[index]->yc;
    float Lx = staticObj[index]->lenx;
    float Ly = staticObj[index]->leny;

    staticObj[index]->x[0] = x + Lx * cDir - Ly * sDir;
    staticObj[index]->y[0] = y + Lx * sDir + Ly * cDir;

    staticObj[index]->x[1] = x - Lx * cDir - Ly * sDir;
    staticObj[index]->y[1] = y - Lx * sDir + Ly * cDir;

    staticObj[index]->x[2] = x - Lx * cDir + Ly * sDir;
    staticObj[index]->y[2] = y - Lx * sDir - Ly * cDir;

    staticObj[index]->x[3] = x + Lx * cDir + Ly * sDir;
    staticObj[index]->y[3] = y + Lx * sDir - Ly * cDir;
}


void RoadInfo::MoveStaticObject(int id, float moveX, float moveY, float moveZ)
{
    int index = indexOfStaticObject(id);
    if( index < 0 ){
        qDebug() << "[MoveStaticObject] cannot find index of id = " << id;
        return;
    }

    staticObj[index]->xc += moveX;
    staticObj[index]->yc += moveY;
    staticObj[index]->zc += moveZ;

    SetCornerPointsStaticObject( id );
}


void RoadInfo::RotateStaticObject(int id, float rot)
{
    int index = indexOfStaticObject(id);
    if( index < 0 ){
        qDebug() << "[MoveStaticObject] cannot find index of id = " << id;
        return;
    }

    staticObj[index]->direction += rot;
    if( staticObj[index]->direction > 180.0 ){
        staticObj[index]->direction -= 360.0;
    }
    else if( staticObj[index]->direction < -180.0 ){
        staticObj[index]->direction += 360.0;
    }

    SetCornerPointsStaticObject( id );
}


void RoadInfo::SetSizeStaticObject(int id, float lenx, float leny, float height)
{
    int index = indexOfStaticObject(id);
    if( index < 0 ){
        qDebug() << "[MoveStaticObject] cannot find index of id = " << id;
        return;
    }

    staticObj[index]->lenx = lenx;
    staticObj[index]->leny = leny;
    staticObj[index]->height = height;

    SetCornerPointsStaticObject( id );
}


void RoadInfo::DeleteStaticObject(int id)
{
    int index = indexOfStaticObject(id);
    if( index < 0 ){
        qDebug() << "[DeleteStaticObject] cannot find index of id = " << id;
        return;
    }

    delete staticObj[index];
    staticObj.removeAt(index);

    qDebug() << "[DeleteStaticObject] delete static object; id = " << id << ", size of staticObj = " << staticObj.size();
}


void RoadInfo::ClearStaticObject()
{
    QList<int> allStaticObjIDs;
    for(int i=0;i<staticObj.size();++i){
        allStaticObjIDs.append( staticObj[i]->id );
    }

    for(int i=0;i<allStaticObjIDs.size();++i){
        DeleteStaticObject( allStaticObjIDs[i] );
    }
}


int RoadInfo::GetNearestStaticObject(QVector2D pos, float &dist)
{
    int ret = -1;
    float nearDist = 0.0;

    for(int i=0;i<staticObj.size();++i){

        float tdist = (QVector2D( staticObj[i]->xc, staticObj[i]->yc ) - pos).length();
        if( ret < 0 || nearDist > tdist ){
            ret = staticObj[i]->id;
            nearDist = tdist;
        }
    }

    dist = nearDist;

    qDebug() << "[GetNearestStaticObject] ret = " << ret << " nearDist = " << dist;

    return ret;
}


