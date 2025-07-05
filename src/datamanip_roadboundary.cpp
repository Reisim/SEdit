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

void DataManipulator::StartCreateRoadBoundary()
{
    canvas->SetRoadBoundaryPointPickMode();
}


int DataManipulator::CreateRoadBoundary()
{
    qDebug() << "[DataManipulator::CreateRoadBoundary]";

    QList<float> height;
    for(int i=0;i<canvas->roadBoundaryPoints.size()-1;++i){
        height.append( 0.20 );
    }

    int id = road->CreateRoadBoundaryInfo( -1, canvas->roadBoundaryPoints, height );

    height.clear();

    return id;
}
