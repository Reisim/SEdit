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
#include <QDebug>


void RoadInfo::CreateWPData()
{
    for(int i=0;i<wps.size();++i){
        wps[i]->relatedLanes.clear();
        delete wps[i];
    }
    wps.clear();

    for(int i=0;i<lanes.size();++i){
        lanes[i]->startWPID = -1;
        lanes[i]->endWPID = -1;
    }

    for(int i=0;i<lanes.size();++i){

        bool createStartWP = true;
        if( lanes[i]->startWPID < 0 ){
            for(int j=0;j<lanes[i]->previousLanes.size();++j){
                int lidx = lanes[i]->previousLanes[j];
                if(lidx >= 0){
                    if( lanes[lidx]->endWPID >= 0 ){
                        lanes[i]->startWPID = lanes[lidx]->endWPID;
                        createStartWP = false;

                        int ewpIdx = indexOfWP( lanes[lidx]->endWPID );
                        if( ewpIdx >= 0 ){
                            wps[ewpIdx]->relatedLanes.append( lanes[i]->id );
                        }

                        break;
                    }
                }
            }
        }
        else{
            createStartWP = false;
        }
        if( createStartWP == true ){
            struct WayPoint* w = new struct WayPoint;

            w->id = wps.size();
            w->pos.setX( lanes[i]->shape.pos.first()->x() );
            w->pos.setY( lanes[i]->shape.pos.first()->y() );
            w->pos.setZ( lanes[i]->shape.pos.first()->z() );
            w->angle = lanes[i]->shape.angles.first();  // [rad]
            w->relatedLanes.append( lanes[i]->id );
            for(int j=0;j<lanes[i]->previousLanes.size();++j){
                w->relatedLanes.append( lanes[i]->previousLanes[j] );
            }
            wps.append( w );

            lanes[i]->startWPID = w->id;
            for(int j=0;j<lanes[i]->previousLanes.size();++j){
                int lidx = lanes[i]->previousLanes[j];
                if(lidx >= 0){
                    lanes[lidx]->endWPID = w->id;
                }
            }
        }

        bool createEndWP = true;
        if( lanes[i]->endWPID < 0 ){
            for(int j=0;j<lanes[i]->nextLanes.size();++j){
                int lidx = lanes[i]->nextLanes[j];
                if(lidx >= 0){
                    if( lanes[lidx]->startWPID >= 0 ){
                        lanes[i]->endWPID = lanes[lidx]->startWPID;
                        createEndWP = false;

                        int swpIdx = indexOfWP( lanes[lidx]->startWPID );
                        if( swpIdx >= 0 ){
                            wps[swpIdx]->relatedLanes.append( lanes[i]->id );
                        }

                        break;
                    }
                }
            }
        }
        else{
            createEndWP = false;
        }
        if( createEndWP == true ){
            struct WayPoint* w = new struct WayPoint;

            w->id = wps.size();
            w->pos.setX( lanes[i]->shape.pos.last()->x() );
            w->pos.setY( lanes[i]->shape.pos.last()->y() );
            w->pos.setZ( lanes[i]->shape.pos.last()->z() );
            w->angle = lanes[i]->shape.angles.last();  // [rad]
            w->relatedLanes.append( lanes[i]->id );
            for(int j=0;j<lanes[i]->nextLanes.size();++j){
                w->relatedLanes.append( lanes[i]->nextLanes[j] );
            }
            wps.append( w );

            lanes[i]->endWPID = w->id;
            for(int j=0;j<lanes[i]->nextLanes.size();++j){
                int lidx = lanes[i]->nextLanes[j];
                if(lidx >= 0){
                    lanes[lidx]->startWPID = w->id;
                }
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

    for(int i=0;i<lanes.size();++i){

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

    }

    for(int i=0;i<nodes.size();++i){
        qDebug() << "Node " << nodes[i]->id << " :";
        for(int j=0;j<nodes[i]->legInfo.size();++j){
            qDebug() << "  Leg " << nodes[i]->legInfo[j]->legID << " :";

            nodes[i]->legInfo[j]->nLaneIn  = nodes[i]->legInfo[j]->inWPs.size();
            nodes[i]->legInfo[j]->nLaneOut = nodes[i]->legInfo[j]->outWPs.size();

            qDebug() << "    nLaneIn: " << nodes[i]->legInfo[j]->nLaneIn;
            for(int k=0;k<nodes[i]->legInfo[j]->inWPs.size();++k){
                qDebug() << "         wp " << nodes[i]->legInfo[j]->inWPs[k];
            }
            qDebug() << "    nLaneOut: " << nodes[i]->legInfo[j]->nLaneOut;
            for(int k=0;k<nodes[i]->legInfo[j]->outWPs.size();++k){
                qDebug() << "         wp " << nodes[i]->legInfo[j]->outWPs[k];
            }
        }
    }
}

