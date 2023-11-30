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



RoadInfo::RoadInfo()
{
    roadDataFileName = QString();

    for(int i=0;i<=DEFAULT_LANE_SHAPE_POINTS;++i){
      float tht = 3.141592653 * 0.5 * (float)i / (float)(DEFAULT_LANE_SHAPE_POINTS);
      tbl_cos[i] = cos(tht);
      tbl_sin[i] = sin(tht);
    }

    LeftOrRight = LEFT_HAND_TRAFFIC;

    updateCPEveryOperation = false;
    updateWPDataEveryOperation = false;
}



int RoadInfo::indexOfLane(int id)
{
    int index = -1;
    for(int i=0;i<lanes.size();++i){
        if( lanes[i]->id == id ){
            index = i;
            break;
        }
    }
    return index;
}


int RoadInfo::indexOfNode(int id)
{
    int index = -1;
    for(int i=0;i<nodes.size();++i){
        if( nodes[i]->id == id ){
            index = i;
            break;
        }
    }
    return index;
}


int RoadInfo::indexOfWP(int id)
{
    int index = -1;
    for(int i=0;i<wps.size();++i){
        if( wps[i]->id == id ){
            index = i;
            break;
        }
    }
    return index;
}


// Return index of TS in trafficSignals data in node = relatedNodeID.
// If relatedNodeID = -1, nodeID containing the TS is returned.
int RoadInfo::indexOfTS(int id,int relatedNodeID)
{
    if( relatedNodeID >= 0 ){
        int index = -1;

        int ndIdx = indexOfNode( relatedNodeID );
        if( ndIdx >= 0 ){
            for(int i=0;i<nodes[ndIdx]->trafficSignals.size();++i){
                if( nodes[ndIdx]->trafficSignals[i]->id == id ){
                    index = i;
                    break;
                }
            }
        }

        return index;
    }
    else{

        int nodeContainTS = -1;

        for(int i=0;i<nodes.size();++i){
            for(int j=0;j<nodes[i]->trafficSignals.size();++j){
                if( nodes[i]->trafficSignals[j]->id == id ){
                    nodeContainTS = nodes[i]->id;
                    break;
                }
            }
        }

        return nodeContainTS;
    }
}


// Return index of StopLine in stoplines data in node = relatedNodeID.
// If relatedNodeID = -1, nodeID containing the SL is returned.
int RoadInfo::indexOfSL(int id,int relatedNodeID)
{
    if( relatedNodeID >= 0 ){
        int index = -1;

        int ndIdx = indexOfNode( relatedNodeID );
        if( ndIdx >= 0 ){
            for(int i=0;i<nodes[ndIdx]->stopLines.size();++i){
                if( nodes[ndIdx]->stopLines[i]->id == id ){
                    index = i;
                    break;
                }
            }
        }

        return index;
    }
    else{

        int nodeContainSL = -1;

        for(int i=0;i<nodes.size();++i){
            for(int j=0;j<nodes[i]->stopLines.size();++j){
                if( nodes[i]->stopLines[j]->id == id ){
                    nodeContainSL = nodes[i]->id;
                    break;
                }
            }
        }

        return nodeContainSL;
    }
}


int RoadInfo::indexOfPedestLane(int id)
{
    int index = -1;
    for(int i=0;i<pedestLanes.size();++i){
        if( pedestLanes[i]->id == id ){
            index = i;
            break;
        }
    }
    return index;
}


int RoadInfo::indexOfStaticObject(int id)
{
    int index = -1;
    for(int i=0;i<staticObj.size();++i){
        if( staticObj[i]->id == id ){
            index = i;
            break;
        }
    }
    return index;
}


void RoadInfo::ClearAllData()
{
    ClearWPs();
    for(int i=0;i<8;++i){
        ClearSearchHelper(i);
    }

    ClearPedestLanes();
    ClearLanes();
    ClearNodes();


    roadDataFileName = QString();
}


void RoadInfo::ClearLaneShape(LaneShapeInfo *s)
{
    for(int l=0;l<s->pos.size();++l){
        delete s->pos[l];
    }
    s->pos.clear();

    for(int l=0;l<s->derivative.size();++l){
        delete s->derivative[l];
    }
    s->derivative.clear();

    for(int l=0;l<s->diff.size();++l){
        delete s->diff[l];
    }
    s->diff.clear();

    s->angles.clear();
    s->curvature.clear();
    s->length.clear();
    s->segmentLength.clear();
}


void RoadInfo::FindInconsistentData()
{
    for(int i=0;i<nodes.size();++i){

        nodes[i]->suspectError = false;

        if( nodes[i]->nLeg != nodes[i]->legInfo.size() ){
            nodes[i]->nLeg = nodes[i]->legInfo.size();
        }

        if( nodes[i]->nLeg > 2 && nodes[i]->stopLines.size() == 0 ){
            nodes[i]->suspectError = true;
        }
    }

    for(int i=0;i<lanes.size();++i){

        if( lanes[i]->eWPInNode < 0  ){
            for(int j=0;j<lanes[i]->nextLanes.size();++j){
                int nIdx = indexOfLane(lanes[i]->nextLanes[j]);
                if( nIdx >= 0 ){
                    if( lanes[nIdx]->eWPInNode >= 0 ){
                        lanes[i]->eWPInNode = lanes[nIdx]->eWPInNode;
                        break;
                    }
                }
            }
        }

        if( lanes[i]->eWPNodeDir < 0  ){
            for(int j=0;j<lanes[i]->nextLanes.size();++j){
                int nIdx = indexOfLane(lanes[i]->nextLanes[j]);
                if( nIdx >= 0 ){
                    if( lanes[i]->eWPInNode == lanes[nIdx]->eWPInNode &&
                            lanes[nIdx]->eWPNodeDir >= 0 ){
                        lanes[i]->eWPNodeDir = lanes[nIdx]->eWPNodeDir;
                        break;
                    }
                }
            }
        }

        if( lanes[i]->sWPInNode < 0  ){
            for(int j=0;j<lanes[i]->previousLanes.size();++j){
                int pIdx = indexOfLane(lanes[i]->previousLanes[j]);
                if( pIdx >= 0 ){
                    if( lanes[pIdx]->sWPInNode >= 0 ){
                        lanes[i]->sWPInNode = lanes[pIdx]->sWPInNode;
                        break;
                    }
                }
            }
        }

        if( lanes[i]->sWPNodeDir < 0  ){
            for(int j=0;j<lanes[i]->previousLanes.size();++j){
                int pIdx = indexOfLane(lanes[i]->previousLanes[j]);
                if( pIdx >= 0 ){
                    if( lanes[i]->sWPInNode == lanes[pIdx]->sWPInNode &&
                            lanes[pIdx]->sWPNodeDir >= 0 ){
                        lanes[i]->sWPNodeDir = lanes[pIdx]->sWPNodeDir;
                        break;
                    }
                }
            }
        }

    }


    for(int i=0;i<lanes.size();++i){

        lanes[i]->suspectError = false;

        if( lanes[i]->sWPNodeDir < 0 || lanes[i]->eWPNodeDir < 0 ){
            lanes[i]->suspectError = true;
        }

        if( lanes[i]->sWPInNode == lanes[i]->eWPInNode ){

            if( lanes[i]->sWPNodeDir == lanes[i]->eWPNodeDir ){
                lanes[i]->suspectError = true;
            }

            for(int j=0;j<lanes[i]->nextLanes.size();++j){
                int nIdx = indexOfLane(lanes[i]->nextLanes[j]);
                if( nIdx >= 0 ){
                    if( lanes[nIdx]->eWPInNode != lanes[nIdx]->sWPInNode &&
                            lanes[i]->eWPInNode != lanes[nIdx]->sWPInNode ){
                        lanes[i]->suspectError = true;
                        lanes[nIdx]->suspectError = true;
                    }
                    else if( lanes[nIdx]->eWPInNode != lanes[nIdx]->sWPInNode &&
                            lanes[i]->eWPInNode == lanes[nIdx]->sWPInNode &&
                            lanes[nIdx]->sWPNodeDir != lanes[i]->eWPNodeDir ){
                        lanes[i]->suspectError = true;
                        lanes[nIdx]->suspectError = true;
                    }
                    else if( lanes[nIdx]->eWPInNode == lanes[nIdx]->sWPInNode &&
                             lanes[nIdx]->sWPInNode == lanes[i]->sWPInNode &&
                             lanes[nIdx]->sWPNodeDir != lanes[i]->sWPNodeDir ){
                        lanes[i]->suspectError = true;
                        lanes[nIdx]->suspectError = true;
                    }
                }
            }

            for(int j=0;j<lanes[i]->previousLanes.size();++j){
                int pIdx = indexOfLane(lanes[i]->previousLanes[j]);
                if( pIdx >= 0 ){
                    if( lanes[pIdx]->eWPInNode != lanes[pIdx]->sWPInNode &&
                            lanes[i]->sWPInNode != lanes[pIdx]->eWPInNode ){
                        lanes[i]->suspectError = true;
                        lanes[pIdx]->suspectError = true;
                    }
                    else if( lanes[pIdx]->eWPInNode != lanes[pIdx]->sWPInNode &&
                            lanes[i]->sWPInNode == lanes[pIdx]->eWPInNode &&
                            lanes[pIdx]->eWPNodeDir != lanes[i]->sWPNodeDir ){
                        lanes[i]->suspectError = true;
                        lanes[pIdx]->suspectError = true;
                    }
                    else if( lanes[pIdx]->eWPInNode == lanes[pIdx]->sWPInNode &&
                             lanes[pIdx]->sWPInNode == lanes[i]->sWPInNode &&
                             lanes[pIdx]->eWPNodeDir != lanes[i]->eWPNodeDir ){
                        lanes[i]->suspectError = true;
                        lanes[pIdx]->suspectError = true;
                    }
                    else if( lanes[i]->sWPBoundary == true && lanes[pIdx]->eWPBoundary == false ){
                        lanes[i]->suspectError = true;
                    }
                }
            }
        }
        else{

            for(int j=0;j<lanes[i]->nextLanes.size();++j){
                int nIdx = indexOfLane(lanes[i]->nextLanes[j]);
                if( nIdx >= 0 ){
                    if( lanes[nIdx]->eWPInNode == lanes[i]->eWPInNode &&
                            lanes[nIdx]->sWPInNode == lanes[i]->sWPInNode &&
                            ( lanes[nIdx]->eWPNodeDir != lanes[i]->eWPNodeDir ||
                              lanes[nIdx]->sWPNodeDir != lanes[i]->sWPNodeDir ) ){
                        lanes[i]->suspectError = true;
                        lanes[nIdx]->suspectError = true;
                    }
                    else if( lanes[nIdx]->eWPInNode != lanes[i]->eWPInNode &&
                             lanes[nIdx]->sWPInNode != lanes[i]->eWPInNode ){
                        lanes[i]->suspectError = true;
                        lanes[nIdx]->suspectError = true;
                    }
                    else if( lanes[nIdx]->eWPInNode != lanes[i]->eWPInNode &&
                             lanes[nIdx]->sWPInNode == lanes[i]->eWPInNode &&
                             lanes[nIdx]->sWPNodeDir != lanes[i]->eWPNodeDir ){
                        lanes[i]->suspectError = true;
                        lanes[nIdx]->suspectError = true;
                    }
                }
            }

            for(int j=0;j<lanes[i]->previousLanes.size();++j){
                int pIdx = indexOfLane(lanes[i]->previousLanes[j]);
                if( pIdx >= 0 ){
                    if( lanes[pIdx]->eWPInNode == lanes[i]->eWPInNode &&
                            lanes[pIdx]->sWPInNode == lanes[i]->sWPInNode &&
                            ( lanes[pIdx]->eWPNodeDir != lanes[i]->eWPNodeDir ||
                              lanes[pIdx]->sWPNodeDir != lanes[i]->sWPNodeDir ) ){
                        lanes[i]->suspectError = true;
                        lanes[pIdx]->suspectError = true;
                    }
                    else if( lanes[pIdx]->eWPInNode != lanes[i]->eWPInNode &&
                             lanes[pIdx]->eWPInNode != lanes[i]->sWPInNode ){
                        lanes[i]->suspectError = true;
                        lanes[pIdx]->suspectError = true;
                    }
                    else if( lanes[pIdx]->eWPInNode != lanes[i]->eWPInNode &&
                             lanes[pIdx]->eWPInNode == lanes[i]->sWPInNode &&
                             lanes[pIdx]->eWPNodeDir != lanes[i]->sWPNodeDir ){
                        lanes[i]->suspectError = true;
                        lanes[pIdx]->suspectError = true;
                    }
                    else if( lanes[i]->sWPBoundary == true && lanes[pIdx]->eWPBoundary == false ){
                        lanes[i]->suspectError = true;
                    }
                }
            }

        }
    }
}
