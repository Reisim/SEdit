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
#include <QDebug>

int RoadInfo::CreateLane(int assignId,
                         QVector4D startPoint, int sWPInNode, int sWPNodeDir, int departureNode, bool sWPBoundary,
                         QVector4D endPoint, int eWPInNode, int eWPNodeDir,int connectedNode , bool eWPBoundary)
{
    // Id check
    int cId = -1;
    if( assignId < 0 ){
        cId = 0;
        for(int i=0;i<lanes.size();++i){
            if( cId <= lanes[i]->id ){
                cId = lanes[i]->id + 1;
            }
        }
    }
    else{
        if( indexOfLane(assignId) < 0 ){
            cId = assignId;
        }
        else{
            qDebug() << "[CreateLane] assigned ID already exists.";
            return -1;
        }
    }

//    qDebug() << "[CreateLane] cId = " << cId;
//    qDebug() << "sWPInNode = " << sWPInNode << " sWPNodeDir = " << sWPNodeDir << " departureNode = " << departureNode << " isBound = " << sWPBoundary;
//    qDebug() << "eWPInNode = " << eWPInNode << " eWPNodeDir = " << eWPNodeDir << " connectedNode = " << connectedNode << " isBound = " << eWPBoundary;


    struct LaneInfo *lane = new struct LaneInfo;

    lane->id = cId;

    lane->shape.pos.append( new QVector3D( startPoint ) );
    lane->shape.pos.append( new QVector3D( endPoint ) );
    lane->shape.derivative.append( new QVector2D(cos(startPoint.w()), sin(startPoint.w())) );
    lane->shape.derivative.append( new QVector2D(cos(endPoint.w()), sin(endPoint.w())) );

    lane->sWPInNode   = sWPInNode;
    lane->sWPNodeDir  = sWPNodeDir;
    lane->sWPBoundary = sWPBoundary;

    lane->eWPInNode   = eWPInNode;
    lane->eWPNodeDir  = eWPNodeDir;
    lane->eWPBoundary = eWPBoundary;

    lane->connectedNode = connectedNode;
    lane->departureNode = departureNode;

    lane->connectedNodeOutDirect = -1;
    lane->connectedNodeInDirect  = -1;
    lane->departureNodeOutDirect = -1;


    // inside intersection
    if( connectedNode == departureNode ){
        if( sWPInNode == connectedNode && eWPInNode == connectedNode ){
            lane->connectedNodeOutDirect = eWPNodeDir;
            lane->connectedNodeInDirect = sWPNodeDir;
        }
        else if( sWPInNode == connectedNode && eWPInNode == -2 ){
            lane->connectedNodeInDirect = sWPNodeDir;
        }
        else if( sWPInNode == -2 && eWPInNode == connectedNode ){
            lane->connectedNodeOutDirect = eWPNodeDir;
        }
    }

    // outside intersection
    if( connectedNode != departureNode ){
        if( sWPInNode == departureNode && eWPInNode == connectedNode ){
            lane->connectedNodeInDirect = eWPNodeDir;
            lane->departureNodeOutDirect = sWPNodeDir;
        }
        else if( sWPInNode == departureNode && eWPInNode == -1 ){
            lane->departureNodeOutDirect = sWPNodeDir;
        }
        else if( sWPInNode == -1 && eWPInNode == connectedNode ){
            lane->connectedNodeInDirect = eWPNodeDir;
        }
    }

    lane->speedInfo = 60.0;

//    qDebug() << "[CreateLane] Call CalculateShape";

    CalculateShape( &(lane->shape) );

    lanes.append( lane );

//    qDebug() << "[CreateLane] add data with id = " << cId;

    return cId;
}


void RoadInfo::DeleteLane(int id)
{
    int index = indexOfLane(id);
    if( index < 0 ){
        qDebug() << "[DeleteLane] cannot find index of id = " << id;
        return;
    }

    // shape
    for(int i=0;i<lanes[index]->shape.pos.size();++i){
        delete lanes[index]->shape.pos[i];
    }
    lanes[index]->shape.pos.clear();

    for(int i=0;i<lanes[index]->shape.derivative.size();++i){
        delete lanes[index]->shape.derivative[i];
    }
    lanes[index]->shape.derivative.clear();

    for(int i=0;i<lanes[index]->shape.diff.size();++i){
        delete lanes[index]->shape.diff[i];
    }
    lanes[index]->shape.diff.clear();

    lanes[index]->shape.angles.clear();

    lanes[index]->shape.curvature.clear();
    lanes[index]->shape.length.clear();
    lanes[index]->shape.segmentLength.clear();

    // next lanes, previous lanes
    lanes[index]->nextLanes.clear();
    lanes[index]->previousLanes.clear();

    // stop points
    for(int i=0;i<lanes[index]->stopPoints.size();++i){
        delete lanes[index]->stopPoints[i];
    }
    lanes[index]->stopPoints.clear();

    // lane
    delete lanes[index];

    lanes.removeAt(index);

    qDebug() << "[DeleteLane] delete lane; id = " << id << ", size of lanes = " << lanes.size();
}


void RoadInfo::SetLaneNumber(int id, int laneNumner)
{
    int index = indexOfLane(id);
    if( index < 0 ){
        qDebug() << "[SetLaneNumber] cannot find index of id = " << id;
        return;
    }
    lanes[index]->laneNum = laneNumner;
}


void RoadInfo::MoveLane(int id, float moveX, float moveY, bool onlyThisLane)
{
    int index = indexOfLane(id);
    if( index < 0 ){
        qDebug() << "[MoveLane] cannot find index of id = " << id;
        return;
    }

    struct LaneShapeInfo *s = &(lanes[index]->shape);

    for(int i=0;i<s->pos.size();++i){
        float x = s->pos[i]->x() + moveX;
        float y = s->pos[i]->y() + moveY;
        s->pos[i]->setX( x );
        s->pos[i]->setY( y );
    }

    if( onlyThisLane == true ){
        return;
    }

    for(int i=0;i<lanes[index]->nextLanes.size();++i){
        MoveLaneEdge( lanes[index]->nextLanes[i], moveX, moveY, 1 );
    }

    for(int i=0;i<lanes[index]->previousLanes.size();++i){
        MoveLaneEdge( lanes[index]->previousLanes[i], moveX, moveY, 0 );
    }
}


void RoadInfo::MoveLaneEdge(int id, float moveX, float moveY, int edgeFlag)
{
    int index = indexOfLane(id);
    if( index < 0 ){
        qDebug() << "[MoveLaneEdge] cannot find index of id = " << id;
        return;
    }

    struct LaneShapeInfo *s = &(lanes[index]->shape);

    if( edgeFlag == 0 ){
        float x = s->pos[0]->x() + moveX;
        float y = s->pos[0]->y() + moveY;
        s->pos[0]->setX( x );
        s->pos[0]->setY( y );
    }
    else{
        float x = s->pos.last()->x() + moveX;
        float y = s->pos.last()->y() + moveY;
        s->pos.last()->setX( x );
        s->pos.last()->setY( y );
    }

    CalculateShape( s );
}


void RoadInfo::RotateLane(int id, float rotate, QVector2D rotCenter)
{
    int index = indexOfLane(id);
    if( index < 0 ){
        qDebug() << "[RotateLane] cannot find index of id = " << id;
        return;
    }

    float cRot = cos(rotate * 0.017452);
    float sRot = sin(rotate * 0.017452);
    float cx = rotCenter.x();
    float cy = rotCenter.y();

    struct LaneShapeInfo *s = &(lanes[index]->shape);

    for(int i=0;i<s->pos.size();++i){

        float rx = s->pos[i]->x() - cx;
        float ry = s->pos[i]->y() - cy;
        float rxd = rx * cRot - ry * sRot;
        float ryd = rx * sRot + ry * cRot;
        s->pos[i]->setX( cx + rxd );
        s->pos[i]->setY( cy + ryd );
    }

    for(int i=0;i<s->derivative.size();++i){

        float dx = s->derivative[i]->x();
        float dy = s->derivative[i]->y();
        float dxd = dx * cRot - dy * sRot;
        float dyd = dx * sRot + dy * cRot;
        s->derivative[i]->setX( dxd );
        s->derivative[i]->setY( dyd );
    }
}


void RoadInfo::RotateLaneEdge(int id, float rotate, int edgeFlag)
{
    int index = indexOfLane(id);
    if( index < 0 ){
        qDebug() << "[RotateLaneEdge] cannot find index of id = " << id;
        return;
    }

    float cRot = cos(rotate * 0.017452);
    float sRot = sin(rotate * 0.017452);

    struct LaneShapeInfo *s = &(lanes[index]->shape);

    if( edgeFlag == 0 ){

        float dx = s->derivative[0]->x();
        float dy = s->derivative[0]->y();
        float dxd = dx * cRot - dy * sRot;
        float dyd = dx * sRot + dy * cRot;
        s->derivative[0]->setX( dxd );
        s->derivative[0]->setY( dyd );
    }
    else{

        float dx = s->derivative.last()->x();
        float dy = s->derivative.last()->y();
        float dxd = dx * cRot - dy * sRot;
        float dyd = dx * sRot + dy * cRot;
        s->derivative.last()->setX( dxd );
        s->derivative.last()->setY( dyd );
    }

    CalculateShape( s );
}


void RoadInfo::CalculateShape(struct LaneShapeInfo *shape)
{
    int lIdx = shape->pos.size() - 1;

    float fwpX = shape->pos[0]->x();
    float fwpY = shape->pos[0]->y();
    float fwpZ = shape->pos[0]->z();

    float twpX = shape->pos[lIdx]->x();
    float twpY = shape->pos[lIdx]->y();
    float twpZ = shape->pos[lIdx]->z();


    float dx = twpX - fwpX;
    float dy = twpY - fwpY;
    float D = sqrt(dx*dx+dy*dy);
    if( D < 1.0 )
        D = 1.0;

    D -= 1.0;
    D *= PATH_ADJUST_PARAM;
    D += 1.0;

    float fct = shape->derivative[0]->x();
    float fst = shape->derivative[0]->y();
    float tct = shape->derivative[lIdx]->x();
    float tst = shape->derivative[lIdx]->y();

    float p1x = -(tct) * D;
    float p1y = -(tst) * D;
    float p2x =  (fct) * D;
    float p2y =  (fst) * D;
    float p3x = -p1x;
    float p3y = -p1y;
    float p4x =  dx - p2x;
    float p4y =  dy - p2y;

    for(int i=1;i<DEFAULT_LANE_SHAPE_POINTS;++i){

        float ct1,st1;
        float ct2,st2;

        ct1 = tbl_cos[i];
        st1 = tbl_sin[i];
        ct2 = ct1 * ct1;
        st2 = st1 * st1;

        float xp = fwpX + (p1x * ct1 + p2x * st1 + p3x * ct2 + p4x * st2);
        float yp = fwpY + (p1y * ct1 + p2y * st1 + p3y * ct2 + p4y * st2);
        float zp = (twpZ - fwpZ) * i / (float)(DEFAULT_LANE_SHAPE_POINTS) + fwpZ;

        float dx = -p1x * st1 + p2x * ct1 + 2.0 * (p4x - p3x) * ct1 * st1;
        float dy = -p1y * st1 + p2y * ct1 + 2.0 * (p4y - p3y) * ct1 * st1;
        float D = 1.0 / sqrt( dx * dx + dy * dy );
        dx *= D;
        dy *= D;

        if( lIdx == 1 ){
            shape->pos.insert(i, new QVector3D(xp,yp,zp) );
            shape->derivative.insert(i, new QVector2D(dx,dy) );
        }
        else if( lIdx == DEFAULT_LANE_SHAPE_POINTS ){
            shape->pos[i]->setX(xp);
            shape->pos[i]->setY(yp);
            shape->pos[i]->setZ(zp);
            shape->derivative[i]->setX(dx);
            shape->derivative[i]->setY(dy);
        }
    }


    if( shape->diff.size() == shape->pos.size() - 1 ){
        for(int i=1;i<shape->pos.size();++i){
            float dx = shape->pos[i]->x() - shape->pos[i-1]->x();
            float dy = shape->pos[i]->y() - shape->pos[i-1]->y();
            float L = sqrt( dx * dx + dy * dy );
            dx /= L;
            dy /= L;
            shape->diff[i-1]->setX( dx );
            shape->diff[i-1]->setY( dy );
            shape->segmentLength[i-1] = L;
            shape->angles[i-1] = atan2(dy,dx);
        }
    }
    else{
        shape->diff.clear();
        shape->segmentLength.clear();
        for(int i=1;i<shape->pos.size();++i){
            float dx = shape->pos[i]->x() - shape->pos[i-1]->x();
            float dy = shape->pos[i]->y() - shape->pos[i-1]->y();
            float L = sqrt( dx * dx + dy * dy );
            dx /= L;
            dy /= L;
            shape->diff.append( new QVector2D(dx, dy) );
            shape->segmentLength.append( L );
            shape->angles.append( atan2(dy,dx) );
        }
    }



    shape->searchHelper.xmax = shape->searchHelper.xmin = shape->pos[0]->x();
    shape->searchHelper.ymax = shape->searchHelper.ymin = shape->pos[0]->y();

    for(int i=1;i<shape->pos.size();++i){

        shape->searchHelper.xmax = shape->searchHelper.xmax < shape->pos[i]->x() ? shape->pos[i]->x() : shape->searchHelper.xmax;
        shape->searchHelper.xmin = shape->searchHelper.xmin > shape->pos[i]->x() ? shape->pos[i]->x() : shape->searchHelper.xmin;

        shape->searchHelper.ymax = shape->searchHelper.ymax < shape->pos[i]->y() ? shape->pos[i]->y() : shape->searchHelper.ymax;
        shape->searchHelper.ymin = shape->searchHelper.ymin > shape->pos[i]->y() ? shape->pos[i]->y() : shape->searchHelper.ymin;
    }

    shape->searchHelper.xmax += 10.0;
    shape->searchHelper.xmin -= 10.0;
    shape->searchHelper.ymax += 10.0;
    shape->searchHelper.ymin -= 10.0;



    int lMd = 0;
    shape->pathLength = 0.0;
    if( shape->length.size() != shape->pos.size() ){
        lMd = 1;
        shape->length.clear();
        shape->length.append(0.0);
    }
    else{
        shape->length[0] = 0.0;
    }

    for(int i=1;i<shape->pos.size();++i){
        float dx = shape->pos[i]->x() - shape->pos[i-1]->x();
        float dy = shape->pos[i]->y() - shape->pos[i-1]->y();
        float L = sqrt(dx * dx + dy * dy);
        shape->pathLength += L;
        if(lMd == 1 ){
            shape->length.append( shape->pathLength );
        }
        else{
            shape->length[i] = shape->pathLength;
        }
    }


    int cMd = 0;
    if( shape->curvature.size() != shape->pos.size() ){
        cMd = 1;
        shape->curvature.clear();
    }

    for(int i=0;i<=DEFAULT_LANE_SHAPE_POINTS;++i){

        if( i == 0 || i == DEFAULT_LANE_SHAPE_POINTS ){
            if( cMd == 1 ){
                shape->curvature.append( 0.0 );
            }
            else{
                shape->curvature[i] = 0.0;
            }
            continue;
        }

        float x1 = shape->pos[i-1]->x();
        float y1 = shape->pos[i-1]->y();
        float x2 = shape->pos[i]->x();
        float y2 = shape->pos[i]->y();
        float x3 = shape->pos[i+1]->x();
        float y3 = shape->pos[i+1]->y();


        float xm1 = (x2 + x1) * 0.5;
        float ym1 = (y2 + y1) * 0.5;
        float xm2 = (x3 + x2) * 0.5;
        float ym2 = (y3 + y2) * 0.5;

        float dx1 = (x2 - x1);
        float dy1 = (y2 - y1);
        float dx2 = (x3 - x2);
        float dy2 = (y3 - y2);

        float det = dx2 * dy1 - dx1 * dy2;
        if( fabs(det) < 0.01 ){
            if( cMd == 1 ){
                shape->curvature.append( 0.0 );
            }
            else{
                shape->curvature[i] = 0.0;
            }
            continue;
        }

        float l = dx2 * (xm1 - xm2) + dy2 * (ym1 - ym2);
        l /= det;

        float xc = xm1 - dy1 * l;
        float yc = ym1 + dx1 * l;

        float rx = x2 - xc;
        float ry = y2 - yc;
        float R = sqrt( rx * rx + ry * ry );
        if( det > 0.0 ){
            R *= (-1.0);
        }

        if( cMd == 1 ){
            shape->curvature.append( 1.0 / R );
        }
        else{
            shape->curvature[i] = 1.0 / R;
        }
    }

    shape->curvature[0] = 2.0 * shape->curvature[1] - shape->curvature[2];
    if( shape->curvature[0] * shape->curvature[1] < 0.0 ){
        shape->curvature[0] = 0.0;
    }

    shape->curvature[DEFAULT_LANE_SHAPE_POINTS] = 2.0 * shape->curvature[DEFAULT_LANE_SHAPE_POINTS-1] - shape->curvature[DEFAULT_LANE_SHAPE_POINTS-2];
    if( shape->curvature[DEFAULT_LANE_SHAPE_POINTS] * shape->curvature[DEFAULT_LANE_SHAPE_POINTS-1] < 0.0 ){
        shape->curvature[DEFAULT_LANE_SHAPE_POINTS] = 0.0;
    }
}


void RoadInfo::DivideLaneHalf(int id)
{
    int index = indexOfLane(id);
    if( index < 0 ){
        qDebug() << "[DivideLaneHalf] cannot find index of id = " << id;
        return;
    }

    struct LaneShapeInfo *s = &(lanes[index]->shape);
    int Np = s->pos.size() / 2;

    int sWPInNode  = lanes[index]->sWPInNode;
    int eWPInNode  = lanes[index]->eWPInNode;
    int eWPNodeDir = lanes[index]->eWPNodeDir;

    int departureNode = lanes[index]->departureNode;
    int connectedNode = lanes[index]->connectedNode;

    QVector4D sP, eP;

    sP.setX( s->pos[Np]->x() );
    sP.setY( s->pos[Np]->y() );
    sP.setZ( s->pos[Np]->z() );
    sP.setW( atan2( s->derivative[Np]->y(), s->derivative[Np]->x() ) );

    eP.setX( s->pos.last()->x() );
    eP.setY( s->pos.last()->y() );
    eP.setZ( s->pos.last()->z() );
    eP.setW( atan2( s->derivative.last()->y(), s->derivative.last()->x() ) );

    int tWPInNode = -1;
    if( sWPInNode == eWPInNode ){
        tWPInNode = eWPInNode;
    }
    CreateLane( -1, sP, tWPInNode, -1, departureNode, -1, eP, eWPInNode, eWPNodeDir, connectedNode, lanes[index]->eWPBoundary );

    s->pos.last()->setX( s->pos[Np]->x() );
    s->pos.last()->setY( s->pos[Np]->y() );
    s->pos.last()->setZ( s->pos[Np]->z() );

    s->derivative.last()->setX( s->derivative[Np]->x() );
    s->derivative.last()->setY( s->derivative[Np]->y() );

    lanes[index]->eWPInNode = tWPInNode;
    lanes[index]->eWPNodeDir = -1;
    lanes[index]->eWPBoundary = false;

    CalculateShape(s);
}


void RoadInfo::DivideLaneAtPos(int id, QVector4D atPoint)
{
    int index = indexOfLane(id);
    if( index < 0 ){
        qDebug() << "[DivideLaneHalf] cannot find index of id = " << id;
        return;
    }

    struct LaneShapeInfo *s = &(lanes[index]->shape);
    int Np = s->pos.size() / 2;

    int sWPInNode  = lanes[index]->sWPInNode;
    int eWPInNode  = lanes[index]->eWPInNode;
    int eWPNodeDir = lanes[index]->eWPNodeDir;

    int departureNode = lanes[index]->departureNode;
    int connectedNode = lanes[index]->connectedNode;

    QVector4D eP;

    eP.setX( s->pos.last()->x() );
    eP.setY( s->pos.last()->y() );
    eP.setZ( s->pos.last()->z() );
    eP.setW( atan2( s->derivative.last()->y(), s->derivative.last()->x() ) );

    int tWPInNode = -1;
    if( sWPInNode == eWPInNode ){
        tWPInNode = eWPInNode;
    }

    CreateLane(-1, atPoint, tWPInNode, -1, departureNode, -1, eP, eWPInNode, eWPNodeDir, connectedNode, lanes[index]->eWPBoundary );

    s->pos.last()->setX( atPoint.x() );
    s->pos.last()->setY( atPoint.y() );
    s->pos.last()->setZ( atPoint.z() );

    s->derivative.last()->setX( cos(atPoint.w()) );
    s->derivative.last()->setY( sin(atPoint.w()) );

    lanes[index]->eWPInNode = tWPInNode;
    lanes[index]->eWPNodeDir = -1;
    lanes[index]->eWPBoundary = false;

    CalculateShape(s);
}


int RoadInfo::GetNearestLane(QVector2D pos)
{
    int ret = -1;
    float nearDist = 0.0;

    for(int i=0;i<lanes.size();++i){
        if( lanes[i]->shape.searchHelper.xmax < pos.x() ||
                lanes[i]->shape.searchHelper.xmin > pos.x() ||
                lanes[i]->shape.searchHelper.ymax < pos.y() ||
                lanes[i]->shape.searchHelper.ymin > pos.y() ){
            continue;
        }

        float dist = 0.0;
        int isEdge = -1;
        int result = GetDistanceLaneFromPoint(lanes[i]->id, pos, dist, isEdge);
        if( result >= 0 ){
            if( ret < 0 || nearDist > dist ){
                ret = lanes[i]->id;
                nearDist = dist;
            }
        }
    }

    qDebug() << "[GetNearestLane] ret = " << ret << " nearDist = " << nearDist;

    return ret;
}


void RoadInfo::CheckLaneCrossPoints()
{
    for(int i=0;i<lanes.size();++i){

        for(int i=0;i<lanes[i]->crossPoints.size();++i){
            delete lanes[i]->crossPoints[i];
        }
        lanes[i]->crossPoints.clear();
    }

    for(int i=0;i<lanes.size();++i){

        for(int j=0;j<lanes.size();++j){

            if( i == j ){
                continue;
            }

            if( lanes[i]->shape.searchHelper.xmax < lanes[j]->shape.searchHelper.xmin ||
                lanes[i]->shape.searchHelper.xmin > lanes[j]->shape.searchHelper.xmax ||
                lanes[i]->shape.searchHelper.ymax < lanes[j]->shape.searchHelper.ymin ||
                lanes[i]->shape.searchHelper.ymin > lanes[j]->shape.searchHelper.ymax  ){
                continue;
            }

            if( lanes[i]->sWPInNode == lanes[j]->sWPInNode && lanes[i]->sWPNodeDir == lanes[j]->sWPNodeDir ){
                continue;
            }

            if( lanes[i]->sWPInNode == lanes[j]->eWPInNode && lanes[i]->eWPInNode != lanes[j]->sWPInNode ){
                continue;
            }

            if( lanes[i]->eWPInNode == lanes[j]->sWPInNode && lanes[i]->sWPInNode != lanes[j]->eWPInNode ){
                continue;
            }

            // Check the lanes merge
            float dx = lanes[i]->shape.pos.last()->x() - lanes[j]->shape.pos.last()->x();
            float dy = lanes[i]->shape.pos.last()->y() - lanes[j]->shape.pos.last()->y();
            float L = dx * dx + dy * dy;
            if( L < 1.0 ){
                continue;
            }


            QPointF p1;
            p1.setX( lanes[j]->shape.pos[0]->x() );
            p1.setY( lanes[j]->shape.pos[0]->y() );

            for(int k=1;k<lanes[j]->shape.pos.size();++k){

                QPointF p2;

                p2.setX( lanes[j]->shape.pos[k]->x() );
                p2.setY( lanes[j]->shape.pos[k]->y() );

                struct CrossPointInfo* cp = CheckLaneCrossPoint( lanes[i]->id, p1, p2 );

                if( cp != NULL ){
                    cp->crossLaneID = lanes[j]->id;
                    lanes[i]->crossPoints.append( cp );
                    break;
                }

                p1 = p2;
            }
        }
    }
}


int RoadInfo::GetDistanceLaneFromPoint(int id,QVector2D pos,float &dist,int &isEdge)
{
    int ret = -1;

    isEdge = -1;

    int index = indexOfLane(id);
    if( index < 0 ){
        qDebug() << "[GetDistanceLaneFromPoint] cannot find index of id = " << id;
        return ret;
    }

    struct LaneShapeInfo *s = &(lanes[index]->shape);

    for(int i=0;i<s->diff.size();++i){

        float rx = pos.x() - s->pos[i]->x();
        float ry = pos.y() - s->pos[i]->y();
        float X = rx * s->diff[i]->x() + ry * s->diff[i]->y();
        if( X < 0.0 || X > s->segmentLength[i] ){
            continue;
        }

        float Y = rx * (s->diff[i]->y()) * (-1.0) + ry * s->diff[i]->x();
        dist = Y;
        if( dist < 0.0 ){
            dist *= -1.0;
        }
        ret = id;

        if( i == 0 ){
            isEdge = 0;
        }
        else if( i == s->diff.size() - 1 ){
            isEdge = 1;
        }
    }

    return ret;
}


struct CrossPointInfo* RoadInfo::CheckLaneCrossPoint(int id, QPointF p1, QPointF p2, bool debugFlag)
{
    struct CrossPointInfo* ret = NULL;

    int index = indexOfLane(id);
    if( index < 0 ){
        qDebug() << "[CheckLaneCrossPoint] cannot find index of id = " << id;
        return ret;
    }

    struct LaneShapeInfo *s = &(lanes[index]->shape);

    for(int i=0;i<s->diff.size();++i){

        float rx1 = p1.x() - s->pos[i]->x();
        float ry1 = p1.y() - s->pos[i]->y();


        float rx2 = p2.x() - s->pos[i]->x();
        float ry2 = p2.y() - s->pos[i]->y();

        float L1 = rx1 * s->derivative[i]->x() + ry1 * s->derivative[i]->y();
        float L2 = rx2 * s->derivative[i]->x() + ry2 * s->derivative[i]->y();

        float a = rx1 * (s->derivative[i]->y()) * (-1.0) + ry1 * s->derivative[i]->x();
        float b = rx2 * (s->derivative[i]->y()) * (-1.0) + ry2 * s->derivative[i]->x();

        if( a * b > 0.0 ){
            continue;
        }

        a = fabs(a);
        b = fabs(b);

        float A = a / ( a + b );
        float B = b / ( a + b );

        float L = A * L2 + B * L1;

        if( L < -0.08 ){  // Arrow small error at boundary
            continue;
        }
        else if( L + 0.08 > s->segmentLength[i] ){
            continue;
        }

        if( L < 0.0 ){
            L = 0.0;
        }
        else if( L > s->segmentLength[i] ){
            L = s->segmentLength[i];
        }

        ret = new struct CrossPointInfo;

        ret->crossLaneID = id;

        ret->pos.setX( s->pos[i]->x() + s->derivative[i]->x() * L );
        ret->pos.setY( s->pos[i]->y() + s->derivative[i]->y() * L );
        ret->pos.setZ( s->pos[i]->z() + (s->pos[i+1]->z() - s->pos[i]->z() ) * ( L / s->segmentLength[i] ) );

        ret->derivative.setX( s->derivative[i]->x() );
        ret->derivative.setY( s->derivative[i]->y() );

        ret->distanceFromLaneStartPoint = s->length[i] + L;

        break;
    }

    return ret;
}

QString RoadInfo::GetLaneProperty(int id)
{
    int index = indexOfLane(id);
    if( index < 0 ){
        qDebug() << "[GetLaneProperty] cannot find index of id = " << id;
        return QString();
    }

    QString propertyStr = QString("Lane: ID = %1\n").arg(id);

    propertyStr += QString("sWPInNode = %1\n").arg( lanes[index]->sWPInNode );
    propertyStr += QString("sWPNodeDir = %1\n").arg( lanes[index]->sWPNodeDir );
    propertyStr += QString("eWPInNode = %1\n").arg( lanes[index]->eWPInNode );
    propertyStr += QString("eWPNodeDir = %1\n").arg( lanes[index]->eWPNodeDir );

    propertyStr += QString("connectedNodeOutDirect = %1\n").arg( lanes[index]->connectedNodeOutDirect );
    propertyStr += QString("connectedNode = %1\n").arg( lanes[index]->connectedNode );
    propertyStr += QString("connectedNodeInDirect = %1\n").arg( lanes[index]->connectedNodeInDirect );
    propertyStr += QString("departureNodeOutDirect = %1\n").arg( lanes[index]->departureNodeOutDirect );
    propertyStr += QString("departureNode = %1\n").arg( lanes[index]->departureNode );
    propertyStr += QString("nextLanes = [");
    for(int i=0;i<lanes[index]->nextLanes.size();++i){
        propertyStr += QString(" %1 ").arg( lanes[index]->nextLanes[i] );
    }
    propertyStr += QString("]\n");
    propertyStr += QString("previousLanes = [");
    for(int i=0;i<lanes[index]->previousLanes.size();++i){
        propertyStr += QString(" %1 ").arg( lanes[index]->previousLanes[i] );
    }
    propertyStr += QString("]\n");

    return propertyStr;
}


void RoadInfo::CheckLaneConnection()
{
    for(int i=0;i<lanes.size();++i){
        lanes[i]->nextLanes.clear();
        lanes[i]->previousLanes.clear();
    }

    for(int i=0;i<lanes.size();++i){

        float lx = lanes[i]->shape.pos.last()->x();
        float ly = lanes[i]->shape.pos.last()->y();

        for(int j=0;j<lanes.size();++j){

            if( i == j ){
                continue;
            }

            float dx = lanes[j]->shape.pos.first()->x() - lx;
            float dy = lanes[j]->shape.pos.first()->y() - ly;

            if( dx < -0.5 || dx > 0.5 ){
                continue;
            }

            if( dy < -0.5 || dy > 0.5 ){
                continue;
            }

            if( lanes[i]->nextLanes.contains( lanes[j]->id ) == false ){
                lanes[i]->nextLanes.append( lanes[j]->id );
            }
            if( lanes[j]->previousLanes.contains( lanes[i]->id ) == false ){
                lanes[j]->previousLanes.append( lanes[i]->id );
            }
        }
    }
}
