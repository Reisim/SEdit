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


int RoadInfo::CreateNode(int assignId, float x, float y, QList<int> inLanes, QList<int> outLanes)
{
    // Id check
    int cId = -1;
    if( assignId < 0 ){
        cId = 0;
        for(int i=0;i<nodes.size();++i){
            if( cId <= nodes[i]->id ){
                cId = nodes[i]->id + 1;
            }
        }
    }
    else{
        if( indexOfNode(assignId) < 0 ){
            cId = assignId;
        }
        else{
            qDebug() << "[CreateNode] assigned ID already exists.";
            return -1;
        }
    }

    struct NodeInfo *node = new struct NodeInfo;

    node->id = cId;

    node->pos.setX( x );
    node->pos.setY( y );

    node->nLeg = inLanes.size();
    for(int i=0;i<node->nLeg;++i){

        struct LegInfo *leg = new struct LegInfo;

        leg->legID = i;

        if( node->nLeg <= 4 ){
            leg->angle = 360.0 / 4.0 * (float)i;
        }
        else{
            leg->angle = 360.0 / (node->nLeg) * (float)i;
        }


        leg->nLaneIn = inLanes[i];
        leg->nLaneOut = outLanes[i];

        leg->connectedNode = -1;
        leg->connectingNode = -1;
        leg->connectedNodeOutDirect = -1;
        leg->connectingNodeInDirect = -1;

        node->legInfo.append( leg );
    }

    node->hasTS = false;
    node->isOriginNode = false;
    node->isDestinationNode = false;

    nodes.append( node );

//    qDebug() << "[CreateNode] add data with id = " << cId;

    return cId;
}


void RoadInfo::DeleteNode(int id)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[DeleteNode] cannot find index of id = " << id;
        return;
    }

    for(int i=0;i<nodes[index]->legInfo.size();++i){
        delete nodes[index]->legInfo[i];
    }
    nodes[index]->legInfo.clear();

    for(int i=0;i<nodes[index]->laneList.size();++i){
        for(int j=0;j<nodes[index]->laneList[i]->lanes.size();++j){
            nodes[index]->laneList[i]->lanes[j].clear();
        }
        nodes[index]->laneList[i]->lanes.clear();
        delete nodes[index]->laneList[i];
    }
    nodes[index]->laneList.clear();


    for(int i=0;i<nodes[index]->trafficSignals.size();++i){
        delete nodes[index]->trafficSignals[i];
    }
    nodes[index]->trafficSignals.clear();

    for(int i=0;i<nodes[index]->stopLines.size();++i){
        nodes[index]->stopLines[i]->crossLanes.clear();
        delete nodes[index]->stopLines[i];
    }
    nodes[index]->stopLines.clear();

    delete nodes[index];

    qDebug() << "[DeleteNode] delete node; id = " << id << ", size of nodes = " << nodes.size();

    nodes.removeAt(index);
}


void RoadInfo::DeleteNodeLeg(int id, int legID)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[DeleteNodeLeg] cannot find index of id = " << id;
        return;
    }

    int leg = -1;
    for(int i=0;i<nodes[index]->legInfo.size();++i){
        if( nodes[index]->legInfo[i]->legID == legID ){
            leg = i;
            break;
        }
    }
    if( leg < 0 ){
        qDebug() << "[DeleteNodeLeg] invalid legID: legID = " << legID << ", can not file the legID";
        return;
    }

    for(int i=nodes[index]->trafficSignals.size()-1;i>=0;--i){
        if( nodes[index]->trafficSignals[i]->controlNodeDirection == legID ){
            delete nodes[index]->trafficSignals[i];
            nodes[index]->trafficSignals.removeAt(i);
        }
    }

    for(int i=nodes[index]->stopLines.size()-1;i>=0;--i){
        if( nodes[index]->stopLines[i]->relatedNodeDir == legID ){
            nodes[index]->stopLines[i]->crossLanes.clear();
            delete nodes[index]->stopLines[i];
            nodes[index]->stopLines.removeAt(i);
        }
    }

    delete nodes[index]->legInfo[leg];

    qDebug() << "[DeleteNodeLeg] delete leg; id = " << legID << " of node; id = " << id;

    nodes[index]->legInfo.removeAt(leg);

    nodes[index]->nLeg = nodes[index]->legInfo.size();
}


void RoadInfo::SetNodePosition(int id, float atX, float atY)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[MoveNode] cannot find index of id = " << id;
        return;
    }

    float moveX = atX - nodes[index]->pos.x();
    float moveY = atY - nodes[index]->pos.y();

    MoveNode(id, moveX, moveY);

    qDebug() << "MoveNode: id = " << id << " moveX = " << moveX << " moveY = " << moveY;

    QList<int> moveLaneBothEdge;
    QList<int> moveLaneStart;
    QList<int> moveLaneEnd;

    for(int j=0;j<nodes[index]->relatedLanes.size();++j){

        int lIdx = indexOfLane( nodes[index]->relatedLanes[j] );
        if( lanes[lIdx]->sWPInNode != id && lanes[lIdx]->sWPBoundary == true ){
            if( moveLaneEnd.indexOf( nodes[index]->relatedLanes[j] ) < 0 ){
                moveLaneEnd.append( nodes[index]->relatedLanes[j] );
            }
        }
        else if( lanes[lIdx]->eWPInNode != id && lanes[lIdx]->eWPBoundary == true ){
            if( moveLaneStart.indexOf( nodes[index]->relatedLanes[j] ) < 0 ){
                moveLaneStart.append( nodes[index]->relatedLanes[j] );
            }
        }
        else{
            if( moveLaneBothEdge.indexOf( nodes[index]->relatedLanes[j] ) < 0 ){
                moveLaneBothEdge.append( nodes[index]->relatedLanes[j] );
            }
        }
    }

    for(int i=0;i<moveLaneBothEdge.size();++i){
        MoveLane( moveLaneBothEdge[i], moveX, moveY, true);
    }
    for(int i=0;i<moveLaneStart.size();++i){
        MoveLaneEdge( moveLaneStart[i], moveX, moveY, 0 );  // Lane Start Point
    }
    for(int i=0;i<moveLaneEnd.size();++i){
        MoveLaneEdge( moveLaneEnd[i], moveX, moveY, 1 );  // Lane End Point
    }
}


void RoadInfo::MoveNode(int id,float moveX,float moveY,bool moveNonEdge)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[MoveNode] cannot find index of id = " << id;
        return;
    }

    nodes[index]->pos.setX( nodes[index]->pos.x() + moveX );
    nodes[index]->pos.setY( nodes[index]->pos.y() + moveY );

    for(int i=0;i<nodes[index]->trafficSignals.size();++i){
        MoveTrafficSignal( nodes[index]->trafficSignals[i]->id, moveX, moveY );
    }

    for(int i=0;i<nodes[index]->stopLines.size();++i){
        MoveStopLine( nodes[index]->stopLines[i]->id, moveX, moveY );
    }
}


void RoadInfo::RotateNode(int id,float rotate)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[RotateNode] cannot find index of id = " << id;
        return;
    }

    nodes[index]->nLeg = nodes[index]->legInfo.size();

    for(int i=0;i<nodes[index]->nLeg;++i){

        int legId = nodes[index]->legInfo[i]->legID;

        RotateNodeLeg(id, legId, rotate);
    }
}


void RoadInfo::RotateNodeLeg(int id,int legID,float rotate)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[RotateNodeLeg] cannot find index of id = " << id;
        return;
    }

    int leg = -1;
    for(int i=0;i<nodes[index]->legInfo.size();++i){
        if( nodes[index]->legInfo[i]->legID == legID ){
            leg = i;
            break;
        }
    }
    if( leg < 0 ){
        qDebug() << "[RotateNodeLeg] invalid legID: legID = " << legID << ", can not file the legID";
        return;
    }

    nodes[index]->legInfo[leg]->angle += rotate;
    if( nodes[index]->legInfo[leg]->angle > 180.0 ){
        nodes[index]->legInfo[leg]->angle -= 360.0;
    }
    else if( nodes[index]->legInfo[leg]->angle < -180.0 ){
        nodes[index]->legInfo[leg]->angle += 360.0;
    }

    float cp = cos(rotate * 0.017452);
    float sp = sin(rotate * 0.017452);

    for(int i=0;i<nodes[index]->trafficSignals.size();++i){

        if( nodes[index]->trafficSignals[i]->controlNodeDirection != legID ){
            continue;
        }

        float xsl = nodes[index]->trafficSignals[i]->pos.x();
        float ysl = nodes[index]->trafficSignals[i]->pos.y();

        float dx = xsl - nodes[index]->pos.x();
        float dy = ysl - nodes[index]->pos.y();

        float rx = dx * cp + dy * (-sp);
        float ry = dx * sp + dy * cp;

        nodes[index]->trafficSignals[i]->pos.setX( nodes[index]->pos.x() + rx );
        nodes[index]->trafficSignals[i]->pos.setY( nodes[index]->pos.y() + ry );

        nodes[index]->trafficSignals[i]->facingDirect = nodes[index]->legInfo[leg]->angle;
        if( nodes[index]->trafficSignals[i]->type == 1 ){
            nodes[index]->trafficSignals[i]->facingDirect += 90.0;
        }
    }

    for(int i=0;i<nodes[index]->stopLines.size();++i){

        if( nodes[index]->stopLines[i]->relatedNodeDir != legID ){
            continue;
        }

        {
            float xsl = nodes[index]->stopLines[i]->leftEdge.x();
            float ysl = nodes[index]->stopLines[i]->leftEdge.y();

            float dx = xsl - nodes[index]->pos.x();
            float dy = ysl - nodes[index]->pos.y();

            float rx = dx * cp + dy * (-sp);
            float ry = dx * sp + dy * cp;

            nodes[index]->stopLines[i]->leftEdge.setX( nodes[index]->pos.x() + rx );
            nodes[index]->stopLines[i]->leftEdge.setY( nodes[index]->pos.y() + ry );
        }

        {
            float xsl = nodes[index]->stopLines[i]->rightEdge.x();
            float ysl = nodes[index]->stopLines[i]->rightEdge.y();

            float dx = xsl - nodes[index]->pos.x();
            float dy = ysl - nodes[index]->pos.y();

            float rx = dx * cp + dy * (-sp);
            float ry = dx * sp + dy * cp;

            nodes[index]->stopLines[i]->rightEdge.setX( nodes[index]->pos.x() + rx );
            nodes[index]->stopLines[i]->rightEdge.setY( nodes[index]->pos.y() + ry );
        }
    }

    //qDebug() << "[RotateNodeLeg:node" << id << "] angle[" << leg << "] = " << nodes[index]->angles[leg];
}


void RoadInfo::AddStopLineToNode(int nodeId,int assignStopLintId,int relatedDirection)
{
    int index = indexOfNode(nodeId);
    if( index < 0 ){
        qDebug() << "[AddStopLineToNode] cannot find index of id = " << nodeId;
        return;
    }

    int sl = CreateStopLine( -1, nodeId, relatedDirection, _STOPLINE_KIND::STOPLINE_SIGNAL );
    if( sl >= 0 ){
        CheckStopLineCrossLanes( sl );
    }
}


void RoadInfo::AddTrafficSignalToNode(int nodeId,int assignTSId,int type, int relatedDirection)
{
    int index = indexOfNode(nodeId);
    if( index < 0 ){
        qDebug() << "[AddTrafficSignalToNode] cannot find index of id = " << nodeId;
        return;
    }

    if( type == 0 ){

        int vtsID = CreateTrafficSignal( assignTSId, nodeId, relatedDirection , 0 );
        int vtsIdx = indexOfTS( vtsID, nodeId );
        if( vtsIdx >= 0 ){
            if(relatedDirection % 2 == 0 ){
                nodes[index]->trafficSignals[vtsIdx]->startOffset = 0;
            }
            else{
                nodes[index]->trafficSignals[vtsIdx]->startOffset = 60;
            }
        }
    }
    else{

        int ptsId = CreateTrafficSignal( assignTSId, nodeId, relatedDirection, 1 );
        int ptsIdx = indexOfTS( ptsId, nodeId );
        if( ptsIdx >= 0 ){
            if(relatedDirection % 2 == 1 ){
                nodes[index]->trafficSignals[ptsIdx]->startOffset = 0;
            }
            else{
                nodes[index]->trafficSignals[ptsIdx]->startOffset = 60;
            }
        }
    }
}


void RoadInfo::SetNodeConnectInfo(int id, int legID, int connectInfo, QString type)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[SetNodeConnectInfo] cannot find index of id = " << id;
        return;
    }

    nodes[index]->nLeg = nodes[index]->legInfo.size();

    int leg = -1;
    for(int i=0;i<nodes[index]->legInfo.size();++i){
        if( nodes[index]->legInfo[i]->legID == legID ){
            leg = i;
            break;
        }
    }
    if( leg < 0 ){
        qDebug() << "[SetNodeConnectInfo] invalid legID: legID = " << legID << ", can not file the legID";
        return;
    }

    if( type == QString("OutNode") ){

        nodes[index]->legInfo[leg]->connectingNode = connectInfo;
        qDebug() << "[SetNodeConnectInfo:node" << id << "] connectingNode[" << leg << "] = " << nodes[index]->legInfo[leg]->connectingNode;
    }
    else if( type == QString("OutNodeInDirect") ){

        nodes[index]->legInfo[leg]->connectingNodeInDirect = connectInfo;
        qDebug() << "[SetNodeConnectInfo:node" << id << "] connectingNodeInDirect[" << leg << "] = " << nodes[index]->legInfo[leg]->connectingNodeInDirect;
    }
    else if( type == QString("InNode") ){

        nodes[index]->legInfo[leg]->connectedNode = connectInfo;
        qDebug() << "[SetNodeConnectInfo:node" << id << "] connectedNode[" << leg << "] = " << nodes[index]->legInfo[leg]->connectedNode;
    }
    else if( type == QString("InNodeOutDirect") ){

        nodes[index]->legInfo[leg]->connectedNodeOutDirect = connectInfo;
        qDebug() << "[SetNodeConnectInfo:node" << id << "] connectedNodeOutDirect[" << leg << "] = " << nodes[index]->legInfo[leg]->connectedNodeOutDirect;
    }
}

void RoadInfo::SetNodeRelatedLane(int id, int laneID)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[SetNodeRelatedLane] cannot find index of id = " << id;
        return;
    }
    if( nodes[index]->relatedLanes.contains( laneID ) == false ){
        nodes[index]->relatedLanes.append( laneID );
    }
}


void RoadInfo::ClearNodeRelatedLane(int id, int laneID)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[ClearNodeRelatedLane] cannot find index of id = " << id;
        return;
    }
    if( laneID < 0 ){
        nodes[index]->relatedLanes.clear();
    }
    else{
        int idx = nodes[index]->relatedLanes.indexOf( laneID );
        if( idx >= 0 && idx < nodes[index]->relatedLanes.size() ){
            nodes[index]->relatedLanes.removeAt(idx);
        }
    }
}


QVector2D RoadInfo::GetNodePosition(int id)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[GetNodePosition] cannot find index of id = " << id;
        return QVector2D();
    }
    return nodes[index]->pos;
}


int RoadInfo::GetNodeNumLeg(int id)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[GetNodeNumLeg] cannot find index of id = " << id;
        return 0;
    }
    nodes[index]->nLeg = nodes[index]->legInfo.size();
    return nodes[index]->nLeg;
}


float RoadInfo::GetNodeLegAngle(int id, int legID)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[GetNodeLegAngle] cannot find index of id = " << id;
        return 360.0;
    }

    int leg = -1;
    for(int i=0;i<nodes[index]->legInfo.size();++i){
        if( nodes[index]->legInfo[i]->legID == legID ){
            leg = i;
            break;
        }
    }
    if( leg < 0 ){
        qDebug() << "[GetNodeLegAngle] invalid legID: legID = " << legID << ", can not file the legID";
        return 360.0;
    }

    float angle = nodes[index]->legInfo[leg]->angle;
    if( angle > 180.0 ){
        angle -= 360.0;
    }
    if( angle < -180.0 ){
        angle += 360.0;
    }
    return angle;
}


int RoadInfo::GetNodeLegIDByConnectedNode(int id, int connectedNode)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[GetNodeLegIDByConnectedNode] cannot find index of id = " << id;
        return -1;
    }

    int legID = -1;
    for(int i=0;i<nodes[index]->legInfo.size();++i){
        if( nodes[index]->legInfo[i]->connectedNode == connectedNode ){
            legID = nodes[index]->legInfo[i]->legID;
            break;
        }
    }
    return legID;
}


int RoadInfo::GetNodeLegIDByConnectingNode(int id, int connectingNode)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[GetNodeLegIDByConnectingNode] cannot find index of id = " << id;
        return -1;
    }

    int legID = -1;
    for(int i=0;i<nodes[index]->legInfo.size();++i){
        if( nodes[index]->legInfo[i]->connectingNode == connectingNode ){
            legID = nodes[index]->legInfo[i]->legID;
            break;
        }
    }
    return legID;
}


QString RoadInfo::GetNodeProperty(int id)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[GetNodeProperty] cannot find index of id = " << id;
        return QString();
    }

    QString propertyStr = QString("Node: ID = %1\n").arg(id);

    propertyStr += QString("Center: (%1,%2)\n").arg( nodes[index]->pos.x() ).arg( nodes[index]->pos.y() );

    for(int i=0;i<nodes[index]->legInfo.size();++i){

        propertyStr += QString("Leg : ID = %1\n").arg( nodes[index]->legInfo[i]->legID );
        propertyStr += QString("  Angle = %1 [deg]\n").arg( nodes[index]->legInfo[i]->angle );
        propertyStr += QString("   <--  [L%1] Nd%2\n").arg( nodes[index]->legInfo[i]->connectedNodeOutDirect ).arg( nodes[index]->legInfo[i]->connectedNode );
        propertyStr += QString("   -->  [L%1] Nd%2\n").arg( nodes[index]->legInfo[i]->connectingNodeInDirect ).arg( nodes[index]->legInfo[i]->connectingNode );
    }

    propertyStr += QString("Related Lanes : n = %1\n").arg( nodes[index]->relatedLanes.size() );
    for(int i=0;i<nodes[index]->relatedLanes.size();++i){
        propertyStr += QString("  Lane %1\n").arg( nodes[index]->relatedLanes[i] );
    }

    return propertyStr;
}


void RoadInfo::SetODFlagOfTerminalNode()
{
    qDebug() << "[RoadInfo::SetODFlagOfTerminalNode]";

    for(int i=0;i<nodes.size();++i){

        qDebug() << "Node " << nodes[i]->id << " : legInfo.size = " << nodes[i]->legInfo.size();

        if(nodes[i]->legInfo.size() == 1 ){

            qDebug() << "  -> Set Origin and Destination";

            nodes[i]->isOriginNode = true;
            nodes[i]->isDestinationNode = true;
        }
    }
}


void RoadInfo::SetTurnDirectionInfo(QList<int> nodeList, bool verbose)
{
    qDebug() << "[RoadInfo::SetTurnDirectionInfo]";
    if( nodeList.size() == 0 ){
        qDebug() << " no nodeList supplied.";
        return;
    }

    // Clear Data
    for(int n=0;n<nodeList.size();++n){
        int i = nodeList.at(n);
        for(int j=0;j<nodes[i]->legInfo.size();++j){
            nodes[i]->legInfo[j]->oncomingLegID = -1;
            nodes[i]->legInfo[j]->leftTurnLegID.clear();
            nodes[i]->legInfo[j]->rightTurnLegID.clear();
        }
    }


    QProgressDialog *pd = new QProgressDialog("SetTurnDirectionInfo", "Cancel", 0, nodeList.size(), 0);
    pd->setWindowModality(Qt::WindowModal);
    pd->setAttribute( Qt::WA_DeleteOnClose );
    pd->setWindowIcon(QIcon(":images/SEdit-icon.png"));
    pd->show();

    pd->setValue(0);
    QApplication::processEvents();


    // Set Data
    int nProcessed = 0;
    for(int n=0;n<nodeList.size();++n){
        int i = nodeList.at(n);

        if( nodes[i]->hasTS == true ){


            // Check Traffic Signal Data
            for(int k=nodes[i]->trafficSignals.size()-1;k>=0;k--){
                int tdID = nodes[i]->trafficSignals[k]->id;
                if( indexOfTS( tdID, nodes[i]->id) < 0 ){
                    delete nodes[i]->trafficSignals[k];
                    nodes[i]->trafficSignals.removeAt(k);
                }
            }


            for(int j=0;j<nodes[i]->legInfo.size();++j){

                if( nodes[i]->legInfo[j]->nLaneIn == 0 || nodes[i]->legInfo[j]->inWPs.size() == 0 ){
                    continue;
                }


                float ct = cos( (nodes[i]->legInfo[j]->angle + 180.0) * 0.017452 );
                float st = sin( (nodes[i]->legInfo[j]->angle + 180.0) * 0.017452 );

                // Set oncoming Direction
                int myTS = -1;
                for(int k=0;k<nodes[i]->trafficSignals.size();k++){
                    if( nodes[i]->trafficSignals[k]->type != 0 ){
                        continue;
                    }
                    if( nodes[i]->trafficSignals[k]->controlNodeDirection == nodes[i]->legInfo[j]->legID ){
                        myTS = k;
                        break;
                    }
                }
                if( myTS >= 0 ){

                    struct DurationPair
                    {
                        int startTime;
                        int endTime;
                    };

                    QList<struct DurationPair> myMoveTime;

                    int Duration = 0;
                    for(int l=0;l<nodes[i]->trafficSignals[myTS]->sigPattern.size();++l){
                        Duration += nodes[i]->trafficSignals[myTS]->sigPattern[l]->duration;
                    }

                    int ST = (Duration - nodes[i]->trafficSignals[myTS]->startOffset) % Duration;

                    for(int k=0;k<nodes[i]->trafficSignals[myTS]->sigPattern.size();++k){
                        if( (nodes[i]->trafficSignals[myTS]->sigPattern[k]->signal & 0x01) == 0x01 ||
                                (nodes[i]->trafficSignals[myTS]->sigPattern[k]->signal & 0x10) == 0x10  ){
                            int tST = ST;
                            int tET = ST + nodes[i]->trafficSignals[myTS]->sigPattern[k]->duration;

                            struct DurationPair p;
                            p.startTime = tST;
                            p.endTime   = tET;
                            myMoveTime.append( p );
                        }
                        ST += nodes[i]->trafficSignals[myTS]->sigPattern[k]->duration;
                    }

                    for(int k=0;k<myMoveTime.size();++k){
                        myMoveTime[k].startTime = myMoveTime[k].startTime % Duration;
                        myMoveTime[k].endTime   = myMoveTime[k].endTime % Duration;
                        if( myMoveTime[k].endTime < myMoveTime[k].startTime ){
                            myMoveTime[k].startTime -= Duration;
                        }
                    }

                    if( verbose == true ){
                        qDebug() << "Node " << nodes[i]->id << " Dir=" << nodes[i]->legInfo[j]->legID;
                        for(int k=0;k<myMoveTime.size();++k){
                            qDebug() << "myMoveTime[" << k << "]: start=" << myMoveTime[k].startTime << " end=" << myMoveTime[k].endTime;
                        }
                    }


                    // Find other direction allowed to go into the intersection with my direction
                    for(int k=0;k<nodes[i]->legInfo.size();++k){

                        if( j == k ){
                            continue;
                        }

                        int cTS = -1;
                        for(int l=0;l<nodes[i]->trafficSignals.size();l++){
                            if( nodes[i]->trafficSignals[l]->type != 0 ){
                                continue;
                            }
                            if( nodes[i]->trafficSignals[l]->controlNodeDirection == nodes[i]->legInfo[k]->legID ){
                                cTS = l;
                                break;
                            }
                        }
                        if( cTS >= 0 ){
                            QList<struct DurationPair> cMoveTime;

                            int Duration = 0;
                            for(int l=0;l<nodes[i]->trafficSignals[cTS]->sigPattern.size();++l){
                                Duration += nodes[i]->trafficSignals[cTS]->sigPattern[l]->duration;
                            }

                            ST = (Duration - nodes[i]->trafficSignals[cTS]->startOffset) % Duration;

                            for(int l=0;l<nodes[i]->trafficSignals[cTS]->sigPattern.size();++l){
                                if( (nodes[i]->trafficSignals[cTS]->sigPattern[l]->signal & 0x01) == 0x01 ||
                                        (nodes[i]->trafficSignals[cTS]->sigPattern[l]->signal & 0x10) == 0x10  ){
                                    int tST = ST;
                                    int tET = ST + nodes[i]->trafficSignals[cTS]->sigPattern[l]->duration;

                                    struct DurationPair p;
                                    p.startTime = tST;
                                    p.endTime   = tET;
                                    cMoveTime.append( p );
                                }
                                ST += nodes[i]->trafficSignals[cTS]->sigPattern[l]->duration;
                            }

                            for(int l=0;l<cMoveTime.size();++l){
                                cMoveTime[l].startTime = cMoveTime[l].startTime % Duration;
                                cMoveTime[l].endTime   = cMoveTime[l].endTime % Duration;
                                if( cMoveTime[l].endTime < cMoveTime[l].startTime ){
                                    cMoveTime[l].startTime -= Duration;
                                }
                            }

                            if( verbose == true ){
                                qDebug() << "Node " << nodes[i]->id << " : check Dir=" << nodes[i]->legInfo[k]->legID;
                                for(int k=0;k<cMoveTime.size();++k){
                                    qDebug() << "cMoveTime[" << k << "]: start=" << cMoveTime[k].startTime << " end=" << cMoveTime[k].endTime;
                                }
                            }


                            bool enterIntersectionSameTime = false;
                            for(int m=0;m<myMoveTime.size();++m){
                                for(int n=0;n<cMoveTime.size();++n){
                                    if( myMoveTime[m].startTime <= cMoveTime[n].startTime &&
                                            myMoveTime[m].endTime <= cMoveTime[n].endTime &&
                                            cMoveTime[n].startTime < myMoveTime[m].endTime ){
                                        enterIntersectionSameTime = true;
                                        break;
                                    }
                                    else if( cMoveTime[n].startTime <= myMoveTime[m].startTime &&
                                             cMoveTime[n].endTime <= myMoveTime[m].endTime &&
                                             myMoveTime[m].startTime < cMoveTime[n].endTime ){
                                         enterIntersectionSameTime = true;
                                         break;
                                     }
                                }
                                if( enterIntersectionSameTime == true ){
                                    break;
                                }
                            }

                            if( verbose == true ){
                                for(int k=0;k<cMoveTime.size();++k){
                                    qDebug() << "enterIntersectionSameTime" << enterIntersectionSameTime;
                                }
                            }

                            if( enterIntersectionSameTime == true ){
                                nodes[i]->legInfo[j]->oncomingLegID = nodes[i]->legInfo[k]->legID;

                                if( verbose == true ){
                                    for(int k=0;k<cMoveTime.size();++k){
                                        qDebug() << "set oncomingLegID[" << nodes[i]->legInfo[j] << "] = " << nodes[i]->legInfo[j]->oncomingLegID;
                                    }
                                }

                                break;
                            }
                            else{
                                nodes[i]->legInfo[j]->oncomingLegID = -1;
                            }
                        }
                    }
                }

                // Set left-turn and right-turn direction
                for(int k=0;k<nodes[i]->legInfo.size();++k){

                    if( j == k ){
                        continue;
                    }

                    if( nodes[i]->legInfo[k]->legID == nodes[i]->legInfo[j]->oncomingLegID ){
                        continue;
                    }

                    float cp = cos( nodes[i]->legInfo[k]->angle * 0.017452 );
                    float sp = sin( nodes[i]->legInfo[k]->angle * 0.017452 );
                    float y = (-st) * cp + ct * sp;

                    if( verbose == true ){
                        qDebug() << "Cross Product of Dir[" << nodes[i]->legInfo[k]->legID << "] = " << y;
                    }

                    if( y > 0.5 ){
                        nodes[i]->legInfo[j]->leftTurnLegID.append( nodes[i]->legInfo[k]->legID );

                        if( verbose == true ){
                            qDebug() << " -> Left";
                        }
                    }
                    else if( y < -0.5 ){
                        nodes[i]->legInfo[j]->rightTurnLegID.append( nodes[i]->legInfo[k]->legID );

                        if( verbose == true ){
                            qDebug() << " -> Right";
                        }
                    }
                }


                if( nodes[i]->legInfo[j]->oncomingLegID == -1 ){

                    for(int k=0;k<nodes[i]->relatedLanes.size();++k){
                        int lIdx = indexOfLane( nodes[i]->relatedLanes[k] );
                        if( lIdx >= 0 ){
                            if( lanes[lIdx]->eWPInNode == nodes[i]->id && lanes[lIdx]->sWPInNode == nodes[i]->id ){
                                if( lanes[lIdx]->sWPNodeDir == nodes[i]->legInfo[j]->legID ){
                                    if( nodes[i]->legInfo[j]->leftTurnLegID.indexOf( lanes[lIdx]->eWPNodeDir ) < 0 &&
                                            nodes[i]->legInfo[j]->rightTurnLegID.indexOf( lanes[lIdx]->eWPNodeDir ) < 0 ){

                                        nodes[i]->legInfo[j]->oncomingLegID = lanes[lIdx]->eWPNodeDir;

                                        if( verbose == true ){
                                            qDebug() << " Set oncoming direction, as is not Left nor Right = " << nodes[i]->legInfo[j]->oncomingLegID;
                                        }

                                        break;
                                    }
                                }
                            }
                        }
                    }

                }
            }
        }
        else{


            for(int j=0;j<nodes[i]->legInfo.size();++j){

                if( nodes[i]->legInfo[j]->nLaneIn == 0 || nodes[i]->legInfo[j]->inWPs.size() == 0 ){
                    continue;
                }


                float ct = cos( (nodes[i]->legInfo[j]->angle + 180.0) * 0.017452 );
                float st = sin( (nodes[i]->legInfo[j]->angle + 180.0) * 0.017452 );


                // Set oncoming Direction
                QList<int> yieldDir;
                for(int k=0;k<nodes[i]->stopLines.size();++k){
                    yieldDir.append( nodes[i]->stopLines[k]->relatedNodeDir );
                }

                int idx = yieldDir.indexOf( nodes[i]->legInfo[j]->legID );
                if( idx >= 0 ){
                    yieldDir.removeAt( idx );
                    if( yieldDir.size() > 0 ){
                        nodes[i]->legInfo[j]->oncomingLegID = yieldDir.first();
                    }
                }
                else{
                    int maxDir = -1;
                    float maxIP = 0.0;
                    for(int k=0;k<nodes[i]->legInfo.size();++k){
                        if( j == k ){
                            continue;
                        }
                        if( yieldDir.indexOf(nodes[i]->legInfo[k]->legID) < 0 ){

                            float cp = cos( nodes[i]->legInfo[k]->angle * 0.017452 );
                            float sp = sin( nodes[i]->legInfo[k]->angle * 0.017452 );
                            float ip = cp * ct + sp * st;
                            if( maxDir < 0 || maxIP < ip ){
                                maxDir = k;
                                maxIP = ip;
                            }
                        }
                    }
                    if( maxDir >= 0 ){
                        nodes[i]->legInfo[j]->oncomingLegID = nodes[i]->legInfo[maxDir]->legID;
                    }
                }


                // Set left-turn and right-turn direction
                for(int k=0;k<nodes[i]->legInfo.size();++k){

                    if( j == k ){
                        continue;
                    }

                    if( nodes[i]->legInfo[k]->legID == nodes[i]->legInfo[j]->oncomingLegID ){
                        continue;
                    }

                    float cp = cos( nodes[i]->legInfo[k]->angle * 0.017452 );
                    float sp = sin( nodes[i]->legInfo[k]->angle * 0.017452 );
                    float y = (-st) * cp + ct * sp;
                    if( y > 0.0 ){
                        nodes[i]->legInfo[j]->leftTurnLegID.append( nodes[i]->legInfo[k]->legID );
                    }
                    else{
                        nodes[i]->legInfo[j]->rightTurnLegID.append( nodes[i]->legInfo[k]->legID );
                    }

                }
            }
        }

        nProcessed++;
        pd->setValue(nProcessed);
        QApplication::processEvents();

        if( pd->wasCanceled() ){
            qDebug() << "Canceled.";
            break;
        }
    }

    pd->close();
}


void RoadInfo::ClearNodes()
{
    QList<int> allNodeIDs;
    for(int i=0;i<nodes.size();++i){
        allNodeIDs.append( nodes[i]->id );
    }

    for(int i=0;i<allNodeIDs.size();++i){
        DeleteNode( allNodeIDs[i] );
    }
}


int RoadInfo::GetNearestNode(QVector2D pos)
{
    int ret = -1;
    float nearDist = 0.0;

    for(int i=0;i<nodes.size();++i){

        float dist = (nodes[i]->pos - pos).length();
        if( ret < 0 || nearDist > dist ){
            ret = nodes[i]->id;
            nearDist = dist;
        }
    }

    qDebug() << "[GetNearestNode] ret = " << ret << " nearDist = " << nearDist;

    return ret;
}


