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

