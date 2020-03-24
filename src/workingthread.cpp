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

                for(int k=0;k<road->nodes[ndIdx]->relatedLanes.size();++k){

                    if( j == k ){
                        continue;
                    }

                    road->CheckIfTwoLanesCross( road->nodes[ndIdx]->relatedLanes[j], road->nodes[ndIdx]->relatedLanes[k] );
                }

                for(int k=0;k<road->pedestLanes.size();++k){

                    road->CheckLaneCrossWithPedestLane( road->nodes[ndIdx]->relatedLanes[j], road->pedestLanes[k]->id );

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



    mode = -1;
    params.clear();
}


