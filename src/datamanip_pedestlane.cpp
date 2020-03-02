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


void DataManipulator::StartCreatePedestPath()
{
    canvas->SetPedestLanePointPickMode();
}


int DataManipulator::CreatePedestPath()
{
    int id = road->CreatePedestLane( -1, canvas->pedestLanePoints );

    int plIdx = road->indexOfPedestLane( id );
    if( plIdx >= 0 ){
        int nPedKind = setDlg->GetPedestrianKindNum();
        road->pedestLanes[plIdx]->trafficVolume.clear();
        for(int i=0;i<nPedKind;++i){
            road->pedestLanes[plIdx]->trafficVolume.append( 0 );
        }
    }

    return id;
}



