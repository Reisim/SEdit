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

        leg->angle = 360.0 / 4.0 * (float)i;

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
}


void RoadInfo::MoveNode(int id,float moveX,float moveY)
{
    int index = indexOfNode(id);
    if( index < 0 ){
        qDebug() << "[MoveNode] cannot find index of id = " << id;
        return;
    }

    nodes[index]->pos.setX( nodes[index]->pos.x() + moveX );
    nodes[index]->pos.setY( nodes[index]->pos.y() + moveY );

    for(int i=0;i<nodes[index]->relatedLanes.size();++i){
        int lidx = indexOfLane( nodes[index]->relatedLanes[i] );
        if( lidx < 0 ){
            continue;
        }
        bool isMoved = false;
        if( lanes[lidx]->sWPInNode == id ){
            lanes[lidx]->shape.pos.first()->setX( lanes[lidx]->shape.pos.first()->x() + moveX );
            lanes[lidx]->shape.pos.first()->setY( lanes[lidx]->shape.pos.first()->y() + moveY );
            isMoved = true;
        }
        if( lanes[lidx]->eWPInNode == id ){
            lanes[lidx]->shape.pos.last()->setX( lanes[lidx]->shape.pos.last()->x() + moveX );
            lanes[lidx]->shape.pos.last()->setY( lanes[lidx]->shape.pos.last()->y() + moveY );
            isMoved = true;
        }
        if( isMoved == true ){
            CalculateShape( &(lanes[lidx]->shape) );
        }
    }

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


    for(int i=0;i<nodes[index]->relatedLanes.size();++i){
        int lidx = indexOfLane( nodes[index]->relatedLanes[i] );
        if( lidx < 0 ){
            continue;
        }
        bool isRot = false;
        if( lanes[lidx]->sWPInNode == id && lanes[lidx]->sWPNodeDir == leg ){
            float xwp = lanes[lidx]->shape.pos.first()->x();
            float ywp = lanes[lidx]->shape.pos.first()->y();

            float dx = xwp - nodes[index]->pos.x();
            float dy = ywp - nodes[index]->pos.y();

            float rx = dx * cp + dy * (-sp);
            float ry = dx * sp + dy * cp;

            lanes[lidx]->shape.pos.first()->setX( nodes[index]->pos.x() + rx );
            lanes[lidx]->shape.pos.first()->setY( nodes[index]->pos.y() + ry );

            dx = lanes[lidx]->shape.derivative.first()->x();
            dy = lanes[lidx]->shape.derivative.first()->y();

            rx = dx * cp + dy * (-sp);
            ry = dx * sp + dy * cp;

            lanes[lidx]->shape.derivative.first()->setX( rx );
            lanes[lidx]->shape.derivative.first()->setY( ry );
            isRot = true;
        }
        if( lanes[lidx]->eWPInNode == id && lanes[lidx]->eWPNodeDir == leg ){
            float xwp = lanes[lidx]->shape.pos.last()->x();
            float ywp = lanes[lidx]->shape.pos.last()->y();

            float dx = xwp - nodes[index]->pos.x();
            float dy = ywp - nodes[index]->pos.y();

            float rx = dx * cp + dy * (-sp);
            float ry = dx * sp + dy * cp;

            lanes[lidx]->shape.pos.last()->setX( nodes[index]->pos.x() + rx );
            lanes[lidx]->shape.pos.last()->setY( nodes[index]->pos.y() + ry );

            dx = lanes[lidx]->shape.derivative.last()->x();
            dy = lanes[lidx]->shape.derivative.last()->y();

            rx = dx * cp + dy * (-sp);
            ry = dx * sp + dy * cp;

            lanes[lidx]->shape.derivative.last()->setX( rx );
            lanes[lidx]->shape.derivative.last()->setY( ry );
            isRot = true;
        }

        if( isRot == true ){
            CalculateShape( &(lanes[lidx]->shape) );
        }
    }

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


void RoadInfo::AddStopLineToNode(int nodeId,int assignStopLintId,int type,int relatedDirection)
{
    int index = indexOfNode(nodeId);
    if( index < 0 ){
        qDebug() << "[AddStopLineToNode] cannot find index of id = " << nodeId;
        return;
    }



}


void RoadInfo::AddTrafficSignalToNode(int nodeId,int assignTSId,int type, int relatedDirection)
{
    int index = indexOfNode(nodeId);
    if( index < 0 ){
        qDebug() << "[AddTrafficSignalToNode] cannot find index of id = " << nodeId;
        return;
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


void RoadInfo::SetTurnDirectionInfo()
{
    qDebug() << "[RoadInfo::SetTurnDirectionInfo]";

    // Clear Data
    for(int i=0;i<nodes.size();++i){
        for(int j=0;j<nodes[i]->legInfo.size();++j){
            nodes[i]->legInfo[j]->oncomingLegID = -1;
            nodes[i]->legInfo[j]->leftTurnLegID.clear();
            nodes[i]->legInfo[j]->rightTurnLegID.clear();
        }
    }


    QProgressDialog *pd = new QProgressDialog("SetTurnDirectionInfo", "Cancel", 0, nodes.size(), 0);
    pd->setWindowModality(Qt::WindowModal);
    pd->setAttribute( Qt::WA_DeleteOnClose );
    pd->show();

    pd->setValue(0);
    QApplication::processEvents();


    // Set Data
    for(int i=0;i<nodes.size();++i){

        if( nodes[i]->hasTS == true ){


            for(int j=0;j<nodes[i]->legInfo.size();++j){

                float ct = cos( (nodes[i]->legInfo[j]->angle + 180.0) * 0.017452 );
                float st = sin( (nodes[i]->legInfo[j]->angle + 180.0) * 0.017452 );

                // Set oncoming Direction
                int myTS = -1;
                for(int k=0;k<nodes[i]->trafficSignals.size();k++){
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

                    int ST = nodes[i]->trafficSignals[myTS]->startOffset;
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
                    ST -= nodes[i]->trafficSignals[myTS]->startOffset;
                    for(int k=0;k<myMoveTime.size();++k){
                        myMoveTime[k].startTime = myMoveTime[k].startTime % ST;
                        myMoveTime[k].endTime   = myMoveTime[k].endTime % ST;
                        if( myMoveTime[k].endTime < myMoveTime[k].startTime ){
                            myMoveTime[k].startTime -= ST;
                        }
                    }


                    // Find other direction allowed to go into the intersection with my direction
                    for(int k=0;k<nodes[i]->legInfo.size();++k){

                        if( j == k ){
                            continue;
                        }

                        int cTS = -1;
                        for(int l=0;l<nodes[i]->trafficSignals.size();l++){
                            if( nodes[i]->trafficSignals[l]->controlNodeDirection == nodes[i]->legInfo[k]->legID ){
                                cTS = l;
                                break;
                            }
                        }
                        if( cTS >= 0 ){
                            QList<struct DurationPair> cMoveTime;

                            ST = nodes[i]->trafficSignals[cTS]->startOffset;
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
                            ST -= nodes[i]->trafficSignals[cTS]->startOffset;
                            for(int l=0;l<cMoveTime.size();++l){
                                cMoveTime[l].startTime = cMoveTime[l].startTime % ST;
                                cMoveTime[l].endTime   = cMoveTime[l].endTime % ST;
                                if( cMoveTime[l].endTime < cMoveTime[l].startTime ){
                                    cMoveTime[l].startTime -= ST;
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
                            if( enterIntersectionSameTime == true ){
                                nodes[i]->legInfo[j]->oncomingLegID = nodes[i]->legInfo[k]->legID;
                                break;
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
                    if( y > 0.0 ){
                        nodes[i]->legInfo[j]->leftTurnLegID.append( nodes[i]->legInfo[k]->legID );
                    }
                    else{
                        nodes[i]->legInfo[j]->rightTurnLegID.append( nodes[i]->legInfo[k]->legID );
                    }
                }
            }
        }
        else{


            for(int j=0;j<nodes[i]->legInfo.size();++j){

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
                    for(int k=0;k<nodes[i]->legInfo.size();++k){
                        if( j == k ){
                            continue;
                        }
                        if( yieldDir.indexOf(nodes[i]->legInfo[k]->legID) < 0 ){
                            nodes[i]->legInfo[j]->oncomingLegID = nodes[i]->legInfo[k]->legID;
                            break;
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
                    if( y > 0.0 ){
                        nodes[i]->legInfo[j]->leftTurnLegID.append( nodes[i]->legInfo[k]->legID );
                    }
                    else{
                        nodes[i]->legInfo[j]->rightTurnLegID.append( nodes[i]->legInfo[k]->legID );
                    }

                }
            }
        }

        pd->setValue(i+1);
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


