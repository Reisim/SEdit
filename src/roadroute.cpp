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


void RoadInfo::CheckRouteInOutDirectionGivenODNode(int origNodeId,int destNodeID)
{
    qDebug() << "[RoadInfo::CheckRouteInOutDirectionGivenODNode] origNodeId = " << origNodeId << " destNodeID = " << destNodeID;

    int i = indexOfNode( origNodeId );
    int j = -1;
    for(int m=0;m<nodes[i]->odData.size();++m){
        if( nodes[i]->odData[m]->destinationNode == destNodeID ){
            j = m;
            break;
        }
    }
    if( j < 0 ){
        return;
    }


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
                    int prevNdOut = nodes[i]->odData[j]->route[k]->nodeList[l-1]->outDir;

                    for(int m=0;m<nodes[ndIdx]->legInfo.size();++m){
                        if( nodes[ndIdx]->legInfo[m]->connectedNode == prevNd &&
                              nodes[ndIdx]->legInfo[m]->connectedNodeOutDirect == prevNdOut  ){
                            nodes[i]->odData[j]->route[k]->nodeList[l]->inDir = nodes[ndIdx]->legInfo[m]->legID;
                            break;
                        }
                    }

                    int nextNd = nodes[i]->odData[j]->route[k]->nodeList[l+1]->node;
                    for(int m=0;m<nodes[ndIdx]->legInfo.size();++m){
                        if( nodes[ndIdx]->legInfo[m]->connectingNode == nextNd &&
                                nodes[ndIdx]->legInfo[m]->outWPs.size() > 0 ){
                            nodes[i]->odData[j]->route[k]->nodeList[l]->outDir = nodes[ndIdx]->legInfo[m]->legID;
                            break;
                        }
                    }
                }

            }
        }
    }
}


void RoadInfo::SetAllRouteLaneList()
{
    qDebug() << "[RoadInfo::SetAllRouteLaneList]";

    int nThread = 8;
    WorkingThread *wt = new WorkingThread[nThread];
    for(int i=0;i<nThread;++i){
        wt[i].mode = 4;
        wt[i].road = this;
        wt[i].wtID = i;
    }

    int nSet = 0;
    int thrIdx = 0;
    for(int i=0;i<nodes.size();++i){
        if( nodes[i]->isOriginNode == false ){
            continue;
        }
        if( nodes[i]->odData.size() == 0 ){
            continue;
        }
        wt[thrIdx].params.append( i );
        thrIdx++;
        if( thrIdx == nThread ){
            thrIdx = 0;
        }
        nSet++;
    }

    if( nSet == 0 ){
        delete [] wt;
        return;
    }

    for(int i=0;i<nThread;++i){
        wt[i].start();
    }


    QProgressDialog *pd = new QProgressDialog("SetAllRouteLaneList", "Cancel", 0, nThread, 0);
    pd->setWindowModality(Qt::WindowModal);
    pd->setAttribute( Qt::WA_DeleteOnClose );
    pd->show();

    pd->setValue(0);
    QApplication::processEvents();


    while(1){

        int nFinish = 0;
        int nProcessed = 0;
        for(int i=0;i<nThread;++i){
            nProcessed += wt[i].nProcessed;
            if( wt[i].mode < 0 ){
                nFinish++;
            }
        }

        Sleep(200);

        qDebug() << "nFinish = " << nFinish << " nThread = " << nThread;

        pd->setValue(nProcessed);
        QApplication::processEvents();
        if( pd->wasCanceled() ){
            qDebug() << "Canceled.  nFinish = " << nFinish;
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

    pd->setValue( nThread );

    pd->close();


    delete [] wt;
}


void RoadInfo::SetAllLaneLists()
{
    qDebug() << "[RoadInfo::SetAllLaneLists]";

    int nThread = 8;
    WorkingThread *wt = new WorkingThread[nThread];
    for(int i=0;i<nThread;++i){
        wt[i].mode = 3;
        wt[i].road = this;
        wt[i].wtID = i;
    }

    int thrIdx = 0;
    for(int i=0;i<nodes.size();++i){
        wt[thrIdx].params.append( i );
        thrIdx++;
        if( thrIdx == nThread ){
            thrIdx = 0;
        }
    }

    for(int i=0;i<nThread;++i){
        wt[i].start();
    }


    QProgressDialog *pd = new QProgressDialog("SetAllLaneLists", "Cancel", 0, nodes.size(), 0);
    pd->setWindowModality(Qt::WindowModal);
    pd->setAttribute( Qt::WA_DeleteOnClose );
    pd->show();

    pd->setValue(0);
    QApplication::processEvents();


    while(1){

        int nFinish = 0;
        int nProcessed = 0;
        for(int i=0;i<nThread;++i){
            nProcessed += wt[i].nProcessed;
            if( wt[i].mode < 0 ){
                nFinish++;
            }
        }

        Sleep(200);

        qDebug() << "nFinish = " << nFinish << " nThread = " << nThread;

        pd->setValue(nProcessed);
        QApplication::processEvents();
        if( pd->wasCanceled() ){
            qDebug() << "Canceled.  nFinish = " << nFinish;
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

    pd->setValue( nodes.size() );

    pd->close();


    delete [] wt;
}


void RoadInfo::SetLaneLists(int id, int hIdx,bool showConsoleOutput)
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
        qDebug() << "[RoadInfo::SetLaneLists] no in and out WPs found.  Node = " << id;
        return;
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

                    ClearSearchHelper(hIdx);
                    ForwardTreeSearch(id, -1, topLanes[k],hIdx);

                    bool foundAll = false;
                    while( 1 ){

                        QList<int> extractedLanes;

                        for(int l=0;l<treeSeachHelper[hIdx].size();++l){
                            if( treeSeachHelper[hIdx][l]->isEnd == false ){
                                extractedLanes.append( treeSeachHelper[hIdx][l]->currentLane );
                            }
                            else if( treeSeachHelper[hIdx][l]->isEnd == true ){
                                extractedLanes.append( treeSeachHelper[hIdx][l]->currentLane );

                                LLs->lanes.append( extractedLanes );

                                if( l < treeSeachHelper[hIdx].size() - 1 ){
                                    int branch = treeSeachHelper[hIdx][l+1]->nextLane;
                                    for(int m=l;m>0;m--){
                                        if( treeSeachHelper[hIdx][m]->currentLane != branch ){
                                            delete treeSeachHelper[hIdx][m];
                                            treeSeachHelper[hIdx].removeAt(m);
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

                ClearSearchHelper(hIdx);
                ForwardTreeSearch(id, -1, topLanes[k], hIdx);

                if( showConsoleOutput == true ){
                    qDebug() << "Extracted:";
                    for(int l=0;l<treeSeachHelper[hIdx].size();++l){
                        qDebug() << treeSeachHelper[hIdx][l]->nextLane << " , " << treeSeachHelper[hIdx][l]->currentLane;
                    }
                }

                bool foundAll = false;
                while( 1 ){

                    QList<int> extractedLanes;

                    for(int l=0;l<treeSeachHelper[hIdx].size();++l){
                        if( treeSeachHelper[hIdx][l]->isEnd == false ){
                            extractedLanes.append( treeSeachHelper[hIdx][l]->currentLane );
                        }
                        else if( treeSeachHelper[hIdx][l]->isEnd == true ){
                            extractedLanes.append( treeSeachHelper[hIdx][l]->currentLane );

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

                            if( l < treeSeachHelper[hIdx].size() - 1 ){
                                int branch = treeSeachHelper[hIdx][l+1]->nextLane;
                                for(int m=l;m>0;m--){
                                    if( treeSeachHelper[hIdx][m]->currentLane != branch ){
                                        delete treeSeachHelper[hIdx][m];
                                        treeSeachHelper[hIdx].removeAt(m);
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


void RoadInfo::ForwardTreeSearch(int nodeId,int nextLane,int currentLane,int hIdx)
{
    struct TreeSearchElem *e = new struct TreeSearchElem;

    e->nextLane = nextLane;
    e->currentLane = currentLane;
    e->isEnd = false;

    treeSeachHelper[hIdx].append( e );

    int cIdx = indexOfLane( currentLane );
    if( lanes[cIdx]->previousLanes.size() == 0 ){
        e->isEnd = true;
        return;
    }

    int nIdx = indexOfNode( nodeId );

    for(int i=0;i<lanes[cIdx]->previousLanes.size();++i){

        int prevLane = lanes[cIdx]->previousLanes[i];

        // check if prevLane is relatedLane of nodeId
        if( nodes[nIdx]->relatedLanes.contains( prevLane ) == false ){
            e->isEnd = true;
            return;
        }

        ForwardTreeSearch( nodeId, currentLane, prevLane, hIdx );
    }
}


void RoadInfo::ClearSearchHelper(int hIdx)
{
    for(int l=0;l<treeSeachHelper[hIdx].size();++l){
        delete treeSeachHelper[hIdx][l];
    }
    treeSeachHelper[hIdx].clear();
}


void RoadInfo::GetLaneListForRoute(int origNodeId,int destNodeId,int hIdx)
{
    int ndIdx = indexOfNode(origNodeId);  // Origin Node Index
    if( nodes[ndIdx]->odData.size() == 0 ){
        return;
    }

    int n = -1;
    for(int i=0;i<nodes[ndIdx]->odData.size();++i){
        if( nodes[ndIdx]->odData[i]->destinationNode == destNodeId ){
            n = i;
            break;
        }
    }
    if( n < 0 ){
        return;
    }

    bool showDetail = false;
    if( hIdx == -1 ){
        hIdx = 0;
        showDetail = true;
    }


    qDebug() << "[" << hIdx << "]" << "[RoadInfo::GetLaneListForRoute]";
    qDebug() << "[" << hIdx << "]" << "  origNodeId = " << origNodeId << " destNodeId = " << destNodeId;
    qDebug() << "[" << hIdx << "]" << "  route.size() = " << nodes[ndIdx]->odData[n]->route.size();


    for(int m=0;m<nodes[ndIdx]->odData[n]->route.size();++m){

        // Clear Existing Data
        for(int i=0;i<nodes[ndIdx]->odData[n]->route[m]->routeLaneLists.size();++i){

            for(int j=0;j<nodes[ndIdx]->odData[n]->route[m]->routeLaneLists.at(i)->laneList.size();++j){
                nodes[ndIdx]->odData[n]->route[m]->routeLaneLists.at(i)->laneList[j].clear();
            }
            nodes[ndIdx]->odData[n]->route[m]->routeLaneLists.at(i)->laneList.clear();

            delete nodes[ndIdx]->odData[n]->route[m]->routeLaneLists.at(i);
        }
        nodes[ndIdx]->odData[n]->route[m]->routeLaneLists.clear();



        for(int i=0;i<nodes[ndIdx]->odData[n]->route[m]->laneList.size();++i){
            nodes[ndIdx]->odData[n]->route[m]->laneList[i].clear();
        }
        nodes[ndIdx]->odData[n]->route[m]->laneList.clear();



        if( showDetail == true ){
            qDebug() << " checking route " << (m+1) << " / " << nodes[ndIdx]->odData[n]->route.size();
            for(int i=0;i<nodes[ndIdx]->odData[n]->route[m]->nodeList.size();++i){
                qDebug() << "[" << i << "]: "
                         << " In:" << nodes[ndIdx]->odData[n]->route[m]->nodeList[i]->inDir
                         << " Nd:" << nodes[ndIdx]->odData[n]->route[m]->nodeList[i]->node
                         << " Out:" << nodes[ndIdx]->odData[n]->route[m]->nodeList[i]->outDir;
            }
        }


        int currentDestNodeIndex = nodes[ndIdx]->odData[n]->route[m]->nodeList.size() - 1;


        bool reachOrigin = false;

        while( reachOrigin == false ){

            int tmpDestNodeId = nodes[ndIdx]->odData[n]->route[m]->nodeList.at( currentDestNodeIndex )->node;
            int destNdIdx = indexOfNode( tmpDestNodeId );
            int destInDir = nodes[ndIdx]->odData[n]->route[m]->nodeList.at( currentDestNodeIndex )->inDir;

            if( showDetail == true ){
                qDebug() << " tmpDestNodeId = " << tmpDestNodeId << " destInDir = " << destInDir;
            }

            QList<int> topLanes;
            for(int i=0;i<nodes[destNdIdx]->relatedLanes.size();++i ){

                int lIdx = indexOfLane( nodes[destNdIdx]->relatedLanes[i] );

    //            qDebug() << " eWPInNode   = " << lanes[lIdx]->eWPInNode;
    //            qDebug() << " sWPInNode   = " << lanes[lIdx]->sWPInNode;
    //            qDebug() << " eWPNodeDir  = " << lanes[lIdx]->eWPNodeDir;
    //            qDebug() << " eWPBoundary = " << lanes[lIdx]->eWPBoundary;


                if( lanes[lIdx]->eWPInNode != lanes[lIdx]->sWPInNode &&
                        lanes[lIdx]->eWPInNode == tmpDestNodeId &&
                        lanes[lIdx]->eWPBoundary == true &&
                        lanes[lIdx]->eWPNodeDir == destInDir ){

                    if( topLanes.indexOf(nodes[destNdIdx]->relatedLanes[i]) < 0 ){
                        topLanes.append( nodes[destNdIdx]->relatedLanes[i] );
    //                    qDebug() << "Add this lane to topLanes";
                    }
                }
            }

            if( showDetail == true ){
                qDebug() << "  topLanes = " << topLanes;
            }


            struct RouteData *rt = new struct RouteData;
            for(int i=0;i<=currentDestNodeIndex;++i){
                struct RouteElem *re = new struct RouteElem;
                re->node = nodes[ndIdx]->odData[n]->route[m]->nodeList[i]->node;
                re->inDir = nodes[ndIdx]->odData[n]->route[m]->nodeList[i]->inDir;
                re->outDir = nodes[ndIdx]->odData[n]->route[m]->nodeList[i]->outDir;
                rt->nodeList.append( re );
            }

            if( showDetail == true ){
                qDebug() << "[Search]: ";
                for(int i=0;i<rt->nodeList.size();++i){
                    qDebug() << "[" << i << "]: "
                             << " In:" << rt->nodeList[i]->inDir
                             << " Nd:" << rt->nodeList[i]->node
                             << " Out:" << rt->nodeList[i]->outDir;
                }
            }


            QList<struct RouteLaneData*> tmpRouteLanes;


            for(int k=0;k<topLanes.size();++k){

                if( showDetail == true ){
                    qDebug() << " checking topLane = " << topLanes[k];
                }

                ClearSearchHelper(hIdx);

                if( showDetail == true ){
                    qDebug() << " start search";
                }

                ForwardTreeSearchForRouteLaneList( rt, -1, topLanes[k], hIdx);

                if( showDetail == true ){
                    qDebug() << " end search";
                }


                struct RouteLaneData *rLanes = new struct RouteLaneData;

                rLanes->goalNode = tmpDestNodeId;
                rLanes->startNode = -1;
                rLanes->gIndexInNodeList = currentDestNodeIndex;
                rLanes->sIndexInNodeList = -1;


                bool foundAll = false;
                while( 1 ){

                    QList<int> extractedLanes;

                    if( showDetail == true ){
                        qDebug() << " treeSeachHelper size = " << treeSeachHelper[hIdx].size();
                    }

                    for(int l=0;l<treeSeachHelper[hIdx].size();++l){

                        if( treeSeachHelper[hIdx][l]->isEnd == false ){
                            extractedLanes.append( treeSeachHelper[hIdx][l]->currentLane );
                        }
                        else if( treeSeachHelper[hIdx][l]->isEnd == true ){
                            extractedLanes.append( treeSeachHelper[hIdx][l]->currentLane );

                            // Check this list reached origin

                            if( showDetail == true ){
                                qDebug() << "Last Lane = " << extractedLanes.last();
                            }

                            int lastLaneIdx = indexOfLane( extractedLanes.last() );

                            if( showDetail == true ){
                                qDebug() << "sWPInNode = " << lanes[lastLaneIdx]->sWPInNode;
                                qDebug() << "origNodeId = " << origNodeId;
                            }

                            if( lanes[lastLaneIdx]->sWPInNode == origNodeId ){
                                reachOrigin = true;
                                qDebug() << "Reached Origin Node";
                            }

                            if( rLanes->startNode < 0 || rLanes->startNode == lanes[lastLaneIdx]->sWPInNode ){

                                rLanes->startNode = lanes[lastLaneIdx]->sWPInNode;
                                rLanes->laneList.append( extractedLanes );

                                for(int i=nodes[ndIdx]->odData[n]->route[m]->nodeList.size()-1;i>0;--i){
                                    int dnd = nodes[ndIdx]->odData[n]->route[m]->nodeList[i]->node;
                                    int dni = nodes[ndIdx]->odData[n]->route[m]->nodeList[i]->inDir;
                                    int ond = nodes[ndIdx]->odData[n]->route[m]->nodeList[i-1]->node;
                                    int ono = nodes[ndIdx]->odData[n]->route[m]->nodeList[i-1]->outDir;
                                    if( dnd == lanes[lastLaneIdx]->eWPInNode &&
                                            dni == lanes[lastLaneIdx]->eWPNodeDir &&
                                            ond == lanes[lastLaneIdx]->sWPInNode &&
                                            ono == lanes[lastLaneIdx]->sWPNodeDir ){

                                        rLanes->sIndexInNodeList = i - 1;
                                        break;
                                    }
                                }
                            }

                            if( l < treeSeachHelper[hIdx].size() - 1 ){
                                int branch = treeSeachHelper[hIdx][l+1]->nextLane;
                                for(int m=l;m>0;m--){
                                    if( treeSeachHelper[hIdx][m]->currentLane != branch ){
                                        delete treeSeachHelper[hIdx][m];
                                        treeSeachHelper[hIdx].removeAt(m);
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

                tmpRouteLanes.append( rLanes );
            }


            int farON = -1;
            int farONIndex = -1;
            for(int i=0;i<tmpRouteLanes.size();++i){
                if( farON < 0 || farONIndex > tmpRouteLanes[i]->sIndexInNodeList ){
                    farON = tmpRouteLanes[i]->startNode;
                    farONIndex = tmpRouteLanes[i]->sIndexInNodeList;
                }
            }

            currentDestNodeIndex = farONIndex + 1;

            if( showDetail == true ){
                qDebug() << " farON = " << farON
                         << " farONIndex = " << farONIndex
                         << " currentDestNodeIndex = " << currentDestNodeIndex;
            }


            for(int i=0;i<tmpRouteLanes.size();++i){
                if( tmpRouteLanes[i]->sIndexInNodeList == farONIndex ){
                    if( nodes[ndIdx]->odData[n]->route[m]->routeLaneLists.size() > 0 ){
                        //
                        // Extend lane list forward
                        for(int j=0;j<tmpRouteLanes.at(i)->laneList.size();++j){
                            while(1){
                                int tL = tmpRouteLanes[i]->laneList[j].first();
                                int tLIdx = indexOfLane( tL );
                                if( lanes[tLIdx]->nextLanes.size() == 1 ){
                                    tmpRouteLanes[i]->laneList[j].prepend( lanes[tLIdx]->nextLanes[0] );
                                }
                                else if( lanes[tLIdx]->nextLanes.size() > 1 ){
                                    bool hasNL = false;
                                    for(int z=0;z<lanes[tLIdx]->nextLanes.size();++z){
                                        int nLIdx = indexOfLane( lanes[tLIdx]->nextLanes[z] );
                                        if( lanes[nLIdx]->eWPInNode == lanes[nLIdx]->sWPInNode ){
                                            int inDir = lanes[nLIdx]->sWPNodeDir;
                                            int outDir = lanes[nLIdx]->eWPNodeDir;
                                            int ndIdx = indexOfNode( lanes[nLIdx]->eWPInNode );
                                            if( ndIdx >= 0 ){
                                                for(int l=0;l<nodes[ndIdx]->legInfo.size();++l){
                                                    if( nodes[ndIdx]->legInfo[l]->legID == inDir &&
                                                            nodes[ndIdx]->legInfo[l]->oncomingLegID == outDir ){
                                                        tmpRouteLanes[i]->laneList[j].prepend( lanes[tLIdx]->nextLanes[z] );
                                                        hasNL = true;
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                        if( hasNL == true ){
                                            break;
                                        }
                                    }
                                    if( hasNL == false ){
                                        break;
                                    }
                                }
                                else{
                                    break;
                                }
                            }
                        }
                    }

                    if( reachOrigin == false ){
                        //
                        // Extend lane list backward
                        for(int j=0;j<tmpRouteLanes.at(i)->laneList.size();++j){
                            while(1){
                                int tL = tmpRouteLanes[i]->laneList[j].last();
                                int tLIdx = indexOfLane( tL );
                                if( lanes[tLIdx]->previousLanes.size() == 1 ){
                                    tmpRouteLanes[i]->laneList[j].append( lanes[tLIdx]->previousLanes[0] );
                                }
                                else if( lanes[tLIdx]->previousLanes.size() > 1 ){
                                    bool hasPL = false;
                                    for(int z=0;z<lanes[tLIdx]->previousLanes.size();++z){
                                        int nLIdx = indexOfLane( lanes[tLIdx]->previousLanes[z] );
                                        if( lanes[nLIdx]->eWPInNode == lanes[nLIdx]->sWPInNode ){
                                            int inDir = lanes[nLIdx]->sWPNodeDir;
                                            int outDir = lanes[nLIdx]->eWPNodeDir;
                                            int ndIdx = indexOfNode( lanes[nLIdx]->eWPInNode );
                                            if( ndIdx >= 0 ){
                                                for(int l=0;l<nodes[ndIdx]->legInfo.size();++l){
                                                    if( nodes[ndIdx]->legInfo[l]->legID == inDir &&
                                                            nodes[ndIdx]->legInfo[l]->oncomingLegID == outDir ){
                                                        tmpRouteLanes[i]->laneList[j].append( lanes[tLIdx]->previousLanes[z] );
                                                        hasPL = true;
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                        if( hasPL == true ){
                                            break;
                                        }
                                    }
                                    if( hasPL == false ){
                                        break;
                                    }
                                }
                                else{
                                    break;
                                }
                            }
                        }
                    }

                    bool addNew = true;
                    for(int j=0;j<nodes[ndIdx]->odData[n]->route[m]->routeLaneLists.size();++j){

                        if( nodes[ndIdx]->odData[n]->route[m]->routeLaneLists[j]->startNode == tmpRouteLanes[i]->startNode &&
                                nodes[ndIdx]->odData[n]->route[m]->routeLaneLists[j]->goalNode == tmpRouteLanes[i]->goalNode &&
                                nodes[ndIdx]->odData[n]->route[m]->routeLaneLists[j]->sIndexInNodeList == tmpRouteLanes[i]->sIndexInNodeList &&
                                nodes[ndIdx]->odData[n]->route[m]->routeLaneLists[j]->gIndexInNodeList == tmpRouteLanes[i]->gIndexInNodeList ){

                            addNew = false;
                            for(int k=0;k<tmpRouteLanes[i]->laneList.size();++k){
                                nodes[ndIdx]->odData[n]->route[m]->routeLaneLists[j]->laneList.append( tmpRouteLanes[i]->laneList[k] );
                            }
                            break;
                        }

                    }

                    if( addNew == true ){
                        nodes[ndIdx]->odData[n]->route[m]->routeLaneLists.append( tmpRouteLanes.at(i) );
                    }

                }
                else{
                    delete tmpRouteLanes.at(i);
                }
            }

            tmpRouteLanes.clear();



            for(int i=0;i<rt->nodeList.size();++i){
                delete rt->nodeList[i];
            }
            rt->nodeList.clear();
            delete rt;

        }


        qDebug() << "[" << hIdx << "]" << " Route Lane List from ON = "
                 << nodes[ndIdx]->odData[n]->route[m]->nodeList.first()->node << " to DN = "
                 << nodes[ndIdx]->odData[n]->route[m]->nodeList.last()->node;

        for(int i=0;i<nodes[ndIdx]->odData[n]->route[m]->routeLaneLists.size();i++){

            qDebug() << "[" << hIdx << "]" << "  ND: "
                     << nodes[ndIdx]->odData[n]->route[m]->routeLaneLists[i]->startNode << " -> "
                     << nodes[ndIdx]->odData[n]->route[m]->routeLaneLists[i]->goalNode;

            for(int j=0;j<nodes[ndIdx]->odData[n]->route[m]->routeLaneLists[i]->laneList.size();++j){
                qDebug() << "[" << hIdx << "]" << "    List[" << j << "]: "
                         << nodes[ndIdx]->odData[n]->route[m]->routeLaneLists[i]->laneList[j];
            }
        }
    }
}


void RoadInfo::ForwardTreeSearchForRouteLaneList(struct RouteData* route,int nextLane,int currentLane,int hIdx)
{
    struct TreeSearchElem *e = new struct TreeSearchElem;

    e->nextLane = nextLane;
    e->currentLane = currentLane;
    e->isEnd = false;

    treeSeachHelper[hIdx].append( e );

    int cIdx = indexOfLane( currentLane );
    if( lanes[cIdx]->previousLanes.size() == 0 ){
        e->isEnd = true;
        return;
    }

    int hasPrev = false;
    for(int i=0;i<lanes[cIdx]->previousLanes.size();++i){

        int prevLane = lanes[cIdx]->previousLanes[i];

        int pIdx = indexOfLane( prevLane );

        bool isInRoute = false;
        if( lanes[pIdx]->eWPInNode == lanes[pIdx]->sWPInNode ){

            for(int j=0;j<route->nodeList.size();++j){
                if( route->nodeList[j]->node == lanes[pIdx]->eWPInNode &&
                        route->nodeList[j]->outDir == lanes[pIdx]->eWPNodeDir &&
                        route->nodeList[j]->inDir  == lanes[pIdx]->sWPNodeDir ){

                    isInRoute = true;
                    break;
                }
            }
        }
        else{
            isInRoute = true;
        }

        if( isInRoute == true ){
            hasPrev = true;
            ForwardTreeSearchForRouteLaneList( route, currentLane, prevLane, hIdx );
        }
    }

    if( hasPrev == false ){
        e->isEnd = true;
        return;
    }
}

