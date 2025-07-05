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



void RoadInfo::ClearWPs()
{
    for(int i=0;i<wps.size();++i){
        wps[i]->relatedLanes.clear();
        delete wps[i];
    }
    wps.clear();
}


void RoadInfo::CreateWPData()
{
    ClearWPs();

    if( lanes.size() == 0 ){
        return;
    }

    for(int i=0;i<lanes.size();++i){
        lanes[i]->startWPID = -1;
        lanes[i]->endWPID = -1;
    }

    qDebug() << "Create WP Data";

    {
        QProgressDialog *pd = new QProgressDialog("CreateWPData - 1/2", "Cancel", 0, lanes.size(), 0);
        pd->setWindowModality(Qt::WindowModal);
        pd->setAttribute( Qt::WA_DeleteOnClose );
        pd->setWindowIcon(QIcon(":images/SEdit-icon.png"));
        pd->show();

        pd->setValue(0);
        QApplication::processEvents();

        for(int i=0;i<lanes.size();++i){

            if( lanes[i]->startWPID < 0 ){

                struct WayPoint* w = new struct WayPoint;

                w->id = wps.size();
                w->pos.setX( lanes[i]->shape.pos.first()->x() );
                w->pos.setY( lanes[i]->shape.pos.first()->y() );
                w->pos.setZ( lanes[i]->shape.pos.first()->z() );
                w->angle = atan2( lanes[i]->shape.derivative.first()->y(), lanes[i]->shape.derivative.first()->x() );  // [rad]

                w->relatedLanes.append( lanes[i]->id );

                lanes[i]->startWPID = w->id;

                for(int j=0;j<lanes[i]->previousLanes.size();++j){
                    if( w->relatedLanes.indexOf( lanes[i]->previousLanes[j] ) < 0 ){
                        w->relatedLanes.append( lanes[i]->previousLanes[j] );
                    }
                    int lidx = indexOfLane( lanes[i]->previousLanes[j] );
                    if(lidx >= 0){

                        lanes[lidx]->endWPID = w->id;

                        lanes[lidx]->shape.pos.last()->setX( lanes[i]->shape.pos.first()->x() );
                        lanes[lidx]->shape.pos.last()->setY( lanes[i]->shape.pos.first()->y() );
                        lanes[lidx]->shape.pos.last()->setZ( lanes[i]->shape.pos.first()->z() );

                        lanes[lidx]->shape.derivative.last()->setX( lanes[i]->shape.derivative.first()->x() );
                        lanes[lidx]->shape.derivative.last()->setY( lanes[i]->shape.derivative.first()->y() );


                        for(int k=0;k<lanes[lidx]->nextLanes.size();++k){
                            if( lanes[lidx]->nextLanes[k] == lanes[i]->id ){
                                continue;
                            }
                            if( w->relatedLanes.indexOf( lanes[lidx]->nextLanes[k] ) < 0 ){
                                w->relatedLanes.append( lanes[lidx]->nextLanes[k] );
                            }
                            int nlidx = indexOfLane( lanes[lidx]->nextLanes[k] );
                            if(nlidx >= 0){
                                lanes[nlidx]->startWPID = w->id;

                                lanes[nlidx]->shape.pos.first()->setX( lanes[i]->shape.pos.first()->x() );
                                lanes[nlidx]->shape.pos.first()->setY( lanes[i]->shape.pos.first()->y() );
                                lanes[nlidx]->shape.pos.first()->setZ( lanes[i]->shape.pos.first()->z() );

                                lanes[nlidx]->shape.derivative.first()->setX( lanes[i]->shape.derivative.first()->x() );
                                lanes[nlidx]->shape.derivative.first()->setY( lanes[i]->shape.derivative.first()->y() );
                            }
                        }
                    }
                }

                wps.append( w );
            }

            if( lanes[i]->endWPID < 0 ){

                struct WayPoint* w = new struct WayPoint;

                w->id = wps.size();
                w->pos.setX( lanes[i]->shape.pos.last()->x() );
                w->pos.setY( lanes[i]->shape.pos.last()->y() );
                w->pos.setZ( lanes[i]->shape.pos.last()->z() );
                w->angle = atan2( lanes[i]->shape.derivative.last()->y(), lanes[i]->shape.derivative.last()->x() );  // [rad]

                w->relatedLanes.append( lanes[i]->id );

                lanes[i]->endWPID = w->id;

                for(int j=0;j<lanes[i]->nextLanes.size();++j){
                    if( w->relatedLanes.indexOf( lanes[i]->nextLanes[j] ) < 0 ){
                        w->relatedLanes.append( lanes[i]->nextLanes[j] );
                    }
                    int lidx = indexOfLane( lanes[i]->nextLanes[j] );
                    if(lidx >= 0){
                        lanes[lidx]->startWPID = w->id;

                        lanes[lidx]->shape.pos.first()->setX( lanes[i]->shape.pos.last()->x() );
                        lanes[lidx]->shape.pos.first()->setY( lanes[i]->shape.pos.last()->y() );
                        lanes[lidx]->shape.pos.first()->setZ( lanes[i]->shape.pos.last()->z() );

                        lanes[lidx]->shape.derivative.first()->setX( lanes[i]->shape.derivative.last()->x() );
                        lanes[lidx]->shape.derivative.first()->setY( lanes[i]->shape.derivative.last()->y() );


                        for(int k=0;k<lanes[lidx]->previousLanes.size();++k){
                            if( lanes[lidx]->previousLanes[k] == lanes[i]->id ){
                                continue;
                            }
                            if( w->relatedLanes.indexOf( lanes[lidx]->previousLanes[k] ) < 0 ){
                                w->relatedLanes.append( lanes[lidx]->previousLanes[k] );
                            }
                            int plidx = indexOfLane( lanes[lidx]->previousLanes[k] );
                            if(plidx >= 0){
                                lanes[plidx]->endWPID = w->id;

                                lanes[plidx]->shape.pos.last()->setX( lanes[i]->shape.pos.last()->x() );
                                lanes[plidx]->shape.pos.last()->setY( lanes[i]->shape.pos.last()->y() );
                                lanes[plidx]->shape.pos.last()->setZ( lanes[i]->shape.pos.last()->z() );

                                lanes[plidx]->shape.derivative.last()->setX( lanes[i]->shape.derivative.last()->x() );
                                lanes[plidx]->shape.derivative.last()->setY( lanes[i]->shape.derivative.last()->y() );
                            }
                        }
                    }
                }

                wps.append( w );
            }

            pd->setValue(i+1);
            QApplication::processEvents();

            if( pd->wasCanceled() ){
                qDebug() << "Canceled.";
                break;
            }

        }
    }

    qDebug() << "Created " << wps.size() << " WP Data.";


    for(int i=0;i<nodes.size();++i){
        for(int j=0;j<nodes[i]->legInfo.size();++j){
            nodes[i]->legInfo[j]->inWPs.clear();
            nodes[i]->legInfo[j]->outWPs.clear();
            nodes[i]->legInfo[j]->nLaneIn  = 0;
            nodes[i]->legInfo[j]->nLaneOut = 0;
        }
    }


    {
        QProgressDialog *pd = new QProgressDialog("CreateWPData - 2/2", "Cancel", 0, lanes.size(), 0);
        pd->setWindowModality(Qt::WindowModal);
        pd->setAttribute( Qt::WA_DeleteOnClose );
        pd->setWindowIcon(QIcon(":images/SEdit-icon.png"));
        pd->show();

        pd->setValue(0);
        QApplication::processEvents();

        for(int i=0;i<lanes.size();++i){

            CalculateShape( &(lanes[i]->shape) );

            if( lanes[i]->sWPInNode >= 0 && lanes[i]->sWPNodeDir >= 0 && lanes[i]->sWPBoundary == true && lanes[i]->sWPInNode != lanes[i]->eWPInNode ){

                int ndIdx = indexOfNode( lanes[i]->sWPInNode );
                if( ndIdx >= 0 ){
                    for(int j=0;j<nodes[ndIdx]->legInfo.size();++j){
                        if( nodes[ndIdx]->legInfo[j]->legID == lanes[i]->sWPNodeDir ){
                            if( nodes[ndIdx]->legInfo[j]->outWPs.contains( lanes[i]->startWPID ) == false ){
                                nodes[ndIdx]->legInfo[j]->outWPs.append( lanes[i]->startWPID );
                            }
                            break;
                        }
                    }
                }
            }

            if( lanes[i]->eWPInNode >= 0 && lanes[i]->eWPNodeDir >= 0 && lanes[i]->eWPBoundary == true && lanes[i]->eWPInNode == lanes[i]->sWPInNode ){

                int ndIdx = indexOfNode( lanes[i]->eWPInNode );

                if( lanes[i]->eWPInNode == 311 ){
                    qDebug() << "check " << lanes[i]->eWPInNode << " lane = " << lanes[i]->id << " idx = " << i << " ndIdx = " << ndIdx;
                }

                if( ndIdx >= 0 ){

                    if( lanes[i]->eWPInNode == 311 ){
                        qDebug() << "legInfo size " << nodes[ndIdx]->legInfo.size() << ", eWPNodeDir = " << lanes[i]->eWPNodeDir;
                    }

                    for(int j=0;j<nodes[ndIdx]->legInfo.size();++j){
                        if( nodes[ndIdx]->legInfo[j]->legID == lanes[i]->eWPNodeDir ){

                            if( nodes[ndIdx]->legInfo[j]->outWPs.contains( lanes[i]->endWPID ) == false ){
                                nodes[ndIdx]->legInfo[j]->outWPs.append( lanes[i]->endWPID );
                            }

                            if( lanes[i]->eWPInNode == 311 ){
                                qDebug() << "outWPs " << nodes[ndIdx]->legInfo[j]->outWPs;
                            }

                            break;
                        }
                    }
                }
            }

            if( lanes[i]->eWPInNode >= 0 && lanes[i]->eWPNodeDir >= 0 && lanes[i]->eWPBoundary == true && lanes[i]->eWPInNode != lanes[i]->sWPInNode ){

                int ndIdx = indexOfNode( lanes[i]->eWPInNode );
                if( ndIdx >= 0 ){
                    for(int j=0;j<nodes[ndIdx]->legInfo.size();++j){
                        if( nodes[ndIdx]->legInfo[j]->legID == lanes[i]->eWPNodeDir ){
                            if( nodes[ndIdx]->legInfo[j]->inWPs.contains( lanes[i]->endWPID ) == false ){
                                nodes[ndIdx]->legInfo[j]->inWPs.append( lanes[i]->endWPID );
                            }
                            break;
                        }
                    }
                }
            }

            pd->setValue(i+1);
            QApplication::processEvents();

            if( pd->wasCanceled() ){
                qDebug() << "Canceled.";
                break;
            }
        }
    }


    for(int i=0;i<nodes.size();++i){
//        qDebug() << "Node " << nodes[i]->id << " :";
        for(int j=0;j<nodes[i]->legInfo.size();++j){
//            qDebug() << "  Leg " << nodes[i]->legInfo[j]->legID << " :";

            nodes[i]->legInfo[j]->nLaneIn  = nodes[i]->legInfo[j]->inWPs.size();
            nodes[i]->legInfo[j]->nLaneOut = nodes[i]->legInfo[j]->outWPs.size();

//            qDebug() << "    nLaneIn: " << nodes[i]->legInfo[j]->nLaneIn;
//            for(int k=0;k<nodes[i]->legInfo[j]->inWPs.size();++k){
//                qDebug() << "         wp " << nodes[i]->legInfo[j]->inWPs[k];
//            }
//            qDebug() << "    nLaneOut: " << nodes[i]->legInfo[j]->nLaneOut;
//            for(int k=0;k<nodes[i]->legInfo[j]->outWPs.size();++k){
//                qDebug() << "         wp " << nodes[i]->legInfo[j]->outWPs[k];
//            }
        }
    }
}


int RoadInfo::CreateWP(int assignID, QVector3D pos, float dir, QList<int> relatedLanes)
{
    // Id check
    int cId = -1;
    if( assignID < 0 ){
        cId = 0;
        for(int i=0;i<wps.size();++i){
            if( cId <= wps[i]->id ){
                cId = wps[i]->id + 1;
            }
        }
    }
    else{
        if( indexOfWP(assignID) < 0 ){
            cId = assignID;
        }
        else{
            qDebug() << "[CreateWP] assigned ID already exists.";
            return -1;
        }
    }

    struct WayPoint* w = new struct WayPoint;

    w->id = cId;
    w->pos = pos;
    w->angle = dir;  // [rad]

    for(int j=0;j<relatedLanes.size();++j){
        w->relatedLanes.append( relatedLanes[j] );
    }
    wps.append( w );

    return cId;
}




