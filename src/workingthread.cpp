#include "workingthread.h"
#include <QDebug>


WorkingThread::WorkingThread(QObject *parent) : QThread(parent)
{
    mode = -1;
    road = NULL;
    stopFlag = false;
}


void WorkingThread::run()
{
    if( road == NULL ){
        mode = -1;
        params.clear();
        return;
    }

    if( params.size() == 0 ){
        mode = -1;
        params.clear();
        return;
    }

    if( mode == 1 ){
        for(int i=0;i<params.size();++i){

            //qDebug() << "Check SL" << params[i];
            road->CheckStopLineCrossLanes( params[i] );

        }
    }
    else if( mode == 2 ){

        nProcessed = 0;

        for(int i=0;i<params.size();++i){

            if( stopFlag == true ){
                break;
            }

            int ndIdx = params[i];

            for(int j=0;j<road->nodes[ndIdx]->relatedLanes.size();++j){

                nProcessed++;

                int lIdx = road->indexOfLane( road->nodes[ndIdx]->relatedLanes.at(j) );
                if( lIdx < 0 ){
                    continue;
                }
                if( road->lanes[lIdx]->sWPInNode != road->lanes[lIdx]->eWPInNode ){
                    continue;
                }

                for(int k=0;k<road->nodes[ndIdx]->relatedLanes.size();++k){

                    if( j == k ){
                        continue;
                    }

                    int clIdx = road->indexOfLane( road->nodes[ndIdx]->relatedLanes.at(k) );
                    if( clIdx < 0 ){
                        continue;
                    }
                    if( road->lanes[clIdx]->sWPInNode != road->lanes[clIdx]->eWPInNode ){
                        continue;
                    }

                    road->CheckIfTwoLanesCross( lIdx, clIdx );
                }
            }
        }

    }
    else if( mode == 3 ){

        nProcessed = 0;

        for(int i=0;i<params.size();++i){

            if( stopFlag == true ){
                break;
            }

            nProcessed++;

            int ndIdx = params[i];

            qDebug() << "[Start] Check Node " << road->nodes[ndIdx]->id;

            road->SetLaneLists( road->nodes[ndIdx]->id, wtID );

            qDebug() << "[End] Check Node " << road->nodes[ndIdx]->id;

        }

    }
    else if( mode == 4 ){

        nProcessed = 0;

        for(int i=0;i<params.size();++i){

            if( stopFlag == true ){
                break;
            }

            nProcessed++;

            int ndIdx = params[i];
            int origNodeID = road->nodes[ndIdx]->id;
            for(int j=0;j<road->nodes[ndIdx]->odData.size();++j){
                int destNodeID = road->nodes[ndIdx]->odData[j]->destinationNode;

                road->CheckRouteInOutDirectionGivenODNode( origNodeID, destNodeID );

                road->GetLaneListForRoute( origNodeID, destNodeID, wtID );

                if( stopFlag == true ){
                    break;
                }
            }
        }
    }
    else if( mode == 5 ){

        nProcessed = 0;
        int ndIdx = -1;
        QList<int> checkLanesList;

        for(int i=0;i<params.size();++i){

            if( stopFlag == true ){
                break;
            }

            int lIdx = params[i];

            if( i == 0 ){
                ndIdx = road->indexOfNode( road->lanes[lIdx]->eWPInNode );
                if( ndIdx >= 0 ){
                    for(int j=0;j<road->nodes[ndIdx]->relatedLanes.size();++j){
                        int clIdx = road->indexOfLane( road->nodes[ndIdx]->relatedLanes[j] );
                        if( clIdx >= 0 ){
                            if( road->lanes[clIdx]->sWPInNode != road->lanes[clIdx]->eWPInNode ){
                                continue;
                            }
                            if( checkLanesList.indexOf( clIdx ) < 0 ){
                                checkLanesList.append( clIdx );
                            }
                        }
                    }
                }
            }

            nProcessed++;

            for(int j=0;j<checkLanesList.size();++j){

                int tlIdx = checkLanesList.at(j);
                if( lIdx == tlIdx ){
                    continue;
                }

                road->CheckIfTwoLanesCross( lIdx, tlIdx );
            }
        }
    }
    else if( mode == 6 ){

        nProcessed = 0;

        for(int i=0;i<params.size();++i){

            if( stopFlag == true ){
                break;
            }

            int lIdx = params[i];

            for(int k=0;k<road->pedestLanes.size();++k){

                road->CheckLaneCrossWithPedestLane( road->lanes[lIdx]->id, road->pedestLanes[k]->id );
            }

            nProcessed++;
        }
    }

    mode = -1;
    params.clear();
}


