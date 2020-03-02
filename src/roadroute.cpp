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


void RoadInfo::CheckRouteInOutDirection()
{
    qDebug() << "[RoadInfo::CheckRouteInOutDirection]";

    for(int i=0;i<nodes.size();++i){

        for(int j=0;j<nodes[i]->odData.size();++j){

            for(int k=0;k<nodes[i]->odData[j]->route.size();++k){

                for(int l=0;l<nodes[i]->odData[j]->route[k]->nodeList.size();++l){
                    nodes[i]->odData[j]->route[k]->nodeList[l]->inDir = -1;
                    nodes[i]->odData[j]->route[k]->nodeList[l]->outDir = -1;
                }

                for(int l=0;l<nodes[i]->odData[j]->route[k]->nodeList.size();++l){

                    if( l == 0 ){

                        int nd = nodes[i]->odData[j]->route[k]->nodeList[l]->node;
                        int ndIdx = indexOfNode( nd );
                        if( ndIdx >= 0 ){
                            if( nodes[ndIdx]->legInfo.size() == 1 ){
                                nodes[i]->odData[j]->route[k]->nodeList[l]->inDir  = -1;
                            }
                            else if( nodes[ndIdx]->legInfo.size() > 1 ){
                                nodes[i]->odData[j]->route[k]->nodeList[l]->inDir  = -2;
                            }

                            int nextNd = nodes[i]->odData[j]->route[k]->nodeList[l+1]->node;
                            for(int m=0;m<nodes[ndIdx]->legInfo.size();++m){
                                if( nodes[ndIdx]->legInfo[m]->connectingNode == nextNd ){
                                    nodes[i]->odData[j]->route[k]->nodeList[l]->outDir = nodes[ndIdx]->legInfo[m]->legID;
                                    break;
                                }
                            }
                        }
                    }
                    else if( l == nodes[i]->odData[j]->route[k]->nodeList.size() - 1 ){

                        int nd = nodes[i]->odData[j]->route[k]->nodeList[l]->node;
                        int ndIdx = indexOfNode( nd );
                        if( ndIdx >= 0 ){

                            if( nodes[ndIdx]->legInfo.size() == 1 ){
                                nodes[i]->odData[j]->route[k]->nodeList[l]->outDir  = -1;
                            }
                            else if( nodes[ndIdx]->legInfo.size() > 1 ){
                                nodes[i]->odData[j]->route[k]->nodeList[l]->outDir  = -2;
                            }

                            int prevNd = nodes[i]->odData[j]->route[k]->nodeList[l-1]->node;
                            for(int m=0;m<nodes[ndIdx]->legInfo.size();++m){
                                if( nodes[ndIdx]->legInfo[m]->connectedNode == prevNd ){
                                    nodes[i]->odData[j]->route[k]->nodeList[l]->inDir = nodes[ndIdx]->legInfo[m]->legID;
                                    break;
                                }
                            }
                        }
                    }
                    else{

                        int nd = nodes[i]->odData[j]->route[k]->nodeList[l]->node;
                        int ndIdx = indexOfNode( nd );
                        if( ndIdx >= 0 ){

                            int prevNd = nodes[i]->odData[j]->route[k]->nodeList[l-1]->node;
                            for(int m=0;m<nodes[ndIdx]->legInfo.size();++m){
                                if( nodes[ndIdx]->legInfo[m]->connectedNode == prevNd ){
                                    nodes[i]->odData[j]->route[k]->nodeList[l]->inDir = nodes[ndIdx]->legInfo[m]->legID;
                                    break;
                                }
                            }

                            int nextNd = nodes[i]->odData[j]->route[k]->nodeList[l+1]->node;
                            for(int m=0;m<nodes[ndIdx]->legInfo.size();++m){
                                if( nodes[ndIdx]->legInfo[m]->connectingNode == nextNd ){
                                    nodes[i]->odData[j]->route[k]->nodeList[l]->outDir = nodes[ndIdx]->legInfo[m]->legID;
                                    break;
                                }
                            }
                        }

                    }
                }
            }
        }
    }
}


void RoadInfo::SetAllLaneLists()
{
    QProgressDialog *pd = new QProgressDialog("SetAllLaneLists", "Cancel", 0, nodes.size(), 0);
    pd->setWindowModality(Qt::WindowModal);
    pd->setAttribute( Qt::WA_DeleteOnClose );
    pd->show();

    pd->setValue(0);
    QApplication::processEvents();

    for(int i=0;i<nodes.size();++i){
        SetLaneLists( nodes[i]->id );

        pd->setValue(i+1);
        QApplication::processEvents();

        if( pd->wasCanceled() ){
            qDebug() << "Canceled.";
            break;
        }
    }

    pd->close();
}


void RoadInfo::SetLaneLists(int id, bool showConsoleOutput)
{
    int ndIdx = indexOfNode( id );
    if( ndIdx < 0 ){
        return;
    }

    // Check WP Data have been created
    int NWPs = 0;
    for(int i=0;i<nodes[ndIdx]->legInfo.size();++i){
        NWPs += nodes[ndIdx]->legInfo[i]->outWPs.size();
        NWPs += nodes[ndIdx]->legInfo[i]->inWPs.size();
    }

    if( NWPs == 0){
        CreateWPData();
    }

    if( showConsoleOutput == true ){
        qDebug() << "[RoadInfo::SetLaneLists] id = " << id;
    }

    // Clear old data
//    qDebug() << "Clear Data.";
    for(int i=0;i<nodes[ndIdx]->laneList.size();++i){

        for(int j=0;j<nodes[ndIdx]->laneList[i]->lanes.size();++j){
            nodes[ndIdx]->laneList[i]->lanes[j].clear();
        }
        nodes[ndIdx]->laneList[i]->lanes.clear();

        delete nodes[ndIdx]->laneList[i];
    }
    nodes[ndIdx]->laneList.clear();


//    qDebug() << "Start analize tree-structure of Lanes.";

    // Special case for terminal node
    if( nodes[ndIdx]->legInfo.size() == 1 ){

        if( showConsoleOutput == true ){
            qDebug() << "This is terminal node.";
        }

        for(int i=0;i<nodes[ndIdx]->legInfo[0]->inWPs.size();++i){

            struct LaneLists* LLs = new struct LaneLists;

            LLs->relatedNodeOutDirection = -1;
            LLs->relatedNodeInDirection = nodes[ndIdx]->legInfo[0]->legID;

            for(int j=0;j<nodes[ndIdx]->legInfo[0]->inWPs.size();++j){

                int topWP = nodes[ndIdx]->legInfo[0]->inWPs[j];
                int tWPIdx = indexOfWP( topWP );
                if( tWPIdx < 0 ){
                    continue;
                }

                QList<int> topLanes;
                for(int k=0;k<wps[tWPIdx]->relatedLanes.size();++k){

                    int tmpLane = wps[tWPIdx]->relatedLanes[k];
                    int tlIdx = indexOfLane( tmpLane );
                    if(tlIdx < 0 ){
                        continue;
                    }
                    if( lanes[tlIdx]->endWPID == topWP ){
                        topLanes.append( lanes[tlIdx]->id );
                    }
                }

                for(int k=0;k<topLanes.size();++k){

                    ClearSearchHelper();
                    ForwardTreeSearch(id, -1, topLanes[k]);

                    bool foundAll = false;
                    while( 1 ){

                        QList<int> extractedLanes;

                        for(int l=0;l<treeSeachHelper.size();++l){
                            if( treeSeachHelper[l]->isEnd == false ){
                                extractedLanes.append( treeSeachHelper[l]->currentLane );
                            }
                            else if( treeSeachHelper[l]->isEnd == true ){
                                extractedLanes.append( treeSeachHelper[l]->currentLane );

                                LLs->lanes.append( extractedLanes );

                                if( l < treeSeachHelper.size() - 1 ){
                                    int branch = treeSeachHelper[l+1]->nextLane;
                                    for(int m=l;m>0;m--){
                                        if( treeSeachHelper[m]->currentLane != branch ){
                                            delete treeSeachHelper[m];
                                            treeSeachHelper.removeAt(m);
                                        }
                                        else{
                                            break;
                                        }
                                    }
                                }
                                else{
                                    foundAll = true;
                                }
                                break;
                            }
                        }
                        if( foundAll == true ){
                            break;
                        }
                    }

                }
            }

            nodes[ndIdx]->laneList.append( LLs );
        }

        if( showConsoleOutput == true ){
            qDebug() << "[SetLaneLists] id = " << id;
            for(int i=0;i<nodes[ndIdx]->laneList.size();++i){
                qDebug() << "List : Out = " << nodes[ndIdx]->laneList[i]->relatedNodeOutDirection
                         << " In = " << nodes[ndIdx]->laneList[i]->relatedNodeInDirection;
                for(int j=0;j<nodes[ndIdx]->laneList[i]->lanes.size();++j){
                    QString LaneListStr = QString();
                    for(int k=0;k<nodes[ndIdx]->laneList[i]->lanes[j].size();++k){
                        LaneListStr += QString("%1").arg( nodes[ndIdx]->laneList[i]->lanes[j].at(k) );
                        if( k < nodes[ndIdx]->laneList[i]->lanes[j].size() - 1 ){
                            LaneListStr += QString(" <- ");
                        }
                    }
                    qDebug() << "  [" << j << "]: " << LaneListStr;
                }
            }
        }
        return;
    }

    if( showConsoleOutput == true ){
        qDebug() << "This is normal node: nLeg = " << nodes[ndIdx]->legInfo.size();
    }

    for(int i=0;i<nodes[ndIdx]->legInfo.size();++i){

        if( showConsoleOutput == true ){
            qDebug() << "[Leg " << nodes[ndIdx]->legInfo[i]->legID << "]";
        }

        for(int j=0;j<nodes[ndIdx]->legInfo.size();++j){

            struct LaneLists* LLs = new struct LaneLists;

            LLs->relatedNodeOutDirection = nodes[ndIdx]->legInfo[i]->legID;
            LLs->relatedNodeInDirection = nodes[ndIdx]->legInfo[j]->legID;

            nodes[ndIdx]->laneList.append( LLs );
        }

        for(int j=0;j<nodes[ndIdx]->legInfo[i]->outWPs.size();++j){

            int topWP = nodes[ndIdx]->legInfo[i]->outWPs[j];
            int tWPIdx = indexOfWP( topWP );
            if( tWPIdx < 0 ){
                continue;
            }

            if( showConsoleOutput == true ){
                qDebug() << "topWP = " << topWP;
                qDebug() << "relatedLanes = " << wps[tWPIdx]->relatedLanes;
            }

            QList<int> topLanes;
            for(int k=0;k<wps[tWPIdx]->relatedLanes.size();++k){

                int tmpLane = wps[tWPIdx]->relatedLanes[k];
                int tlIdx = indexOfLane( tmpLane );
                if(tlIdx < 0 ){
                    continue;
                }
                if( lanes[tlIdx]->endWPID == topWP ){
                    topLanes.append( lanes[tlIdx]->id );
                }
            }

            if( showConsoleOutput == true ){
                qDebug() << "topLanes = " << topLanes;
            }

            for(int k=0;k<topLanes.size();++k){

                ClearSearchHelper();
                ForwardTreeSearch(id, -1, topLanes[k]);

                if( showConsoleOutput == true ){
                    qDebug() << "Extracted:";
                    for(int l=0;l<treeSeachHelper.size();++l){
                        qDebug() << treeSeachHelper[l]->nextLane << " , " << treeSeachHelper[l]->currentLane;
                    }
                }

                bool foundAll = false;
                while( 1 ){

                    QList<int> extractedLanes;

                    for(int l=0;l<treeSeachHelper.size();++l){
                        if( treeSeachHelper[l]->isEnd == false ){
                            extractedLanes.append( treeSeachHelper[l]->currentLane );
                        }
                        else if( treeSeachHelper[l]->isEnd == true ){
                            extractedLanes.append( treeSeachHelper[l]->currentLane );

                            for(int m=0;m<extractedLanes.size();++m){
                                int tlIdx = indexOfLane(extractedLanes[m]);
                                if( tlIdx >= 0 ){
                                    if( lanes[tlIdx]->sWPBoundary == true && lanes[tlIdx]->sWPInNode == id ){
                                        int inDir = lanes[tlIdx]->sWPNodeDir;
                                        for(int n=0;n<nodes[ndIdx]->laneList.size();++n){
                                            if( nodes[ndIdx]->laneList[n]->relatedNodeOutDirection == nodes[ndIdx]->legInfo[i]->legID &&
                                                    nodes[ndIdx]->laneList[n]->relatedNodeInDirection == inDir ){
                                                nodes[ndIdx]->laneList[n]->lanes.append( extractedLanes );
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                }
                            }

                            if( l < treeSeachHelper.size() - 1 ){
                                int branch = treeSeachHelper[l+1]->nextLane;
                                for(int m=l;m>0;m--){
                                    if( treeSeachHelper[m]->currentLane != branch ){
                                        delete treeSeachHelper[m];
                                        treeSeachHelper.removeAt(m);
                                    }
                                    else{
                                        break;
                                    }
                                }
                            }
                            else{
                                foundAll = true;
                            }
                            break;
                        }
                    }
                    if( foundAll == true ){
                        break;
                    }
                }
            }
        }
    }

    if( showConsoleOutput == true ){
        qDebug() << "[SetLaneLists] id = " << id;
        for(int i=0;i<nodes[ndIdx]->laneList.size();++i){
            qDebug() << "List : Out = " << nodes[ndIdx]->laneList[i]->relatedNodeOutDirection
                     << " In = " << nodes[ndIdx]->laneList[i]->relatedNodeInDirection;
            for(int j=0;j<nodes[ndIdx]->laneList[i]->lanes.size();++j){
                QString LaneListStr = QString();
                for(int k=0;k<nodes[ndIdx]->laneList[i]->lanes[j].size();++k){
                    LaneListStr += QString("%1").arg( nodes[ndIdx]->laneList[i]->lanes[j].at(k) );
                    if( k < nodes[ndIdx]->laneList[i]->lanes[j].size() - 1 ){
                        LaneListStr += QString(" <- ");
                    }
                }
                qDebug() << "  [" << j << "]: " << LaneListStr;
            }
        }
    }

}


void RoadInfo::ForwardTreeSearch(int nodeId,int nextLane,int currentLane)
{
    struct TreeSearchElem *e = new struct TreeSearchElem;

    e->nextLane = nextLane;
    e->currentLane = currentLane;
    e->isEnd = false;

    treeSeachHelper.append( e );

    int cIdx = indexOfLane( currentLane );
    if( lanes[cIdx]->sWPBoundary == true && lanes[cIdx]->sWPInNode != nodeId ){
        e->isEnd = true;
        return;
    }
    else{
        for(int i=0;i<lanes[cIdx]->previousLanes.size();++i){
            int prevLane = lanes[cIdx]->previousLanes[i];
            ForwardTreeSearch( nodeId, currentLane, prevLane );
        }
    }
}


void RoadInfo::ClearSearchHelper()
{
    for(int l=0;l<treeSeachHelper.size();++l){
        delete treeSeachHelper[l];
    }
    treeSeachHelper.clear();
}

