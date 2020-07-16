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


bool RoadInfo::SaveRoadData(QString filename)
{
    QFile file(filename);
    if( file.open( QIODevice::WriteOnly | QIODevice::Text) == false ){
        qDebug() << "[SaveRoadData] cannot open file: "<< filename << " to write data.";
        return false;
    }

    roadDataFileName = filename;

    QTextStream out(&file);

    //
    //  Output Road Data
    //
    out << "#---------------------------------------\n";
    out << "#   SEdit Data File\n";
    out << "#---------------------------------------\n";
    out << "LeftOrRight: " << LeftOrRight << "\n";
    out << "\n";

    for(int i=0;i<nodes.size();++i){
        out << "[Node] ID : " << nodes[i]->id << "\n";
        out << "ND Center : " << nodes[i]->pos.x() << " , " << nodes[i]->pos.y() << "\n";
        out << "OD Type   : " << (nodes[i]->isOriginNode == true ? 1 : 0 ) << " , " << (nodes[i]->isDestinationNode == true ? 1 : 0) << "\n";
        for(int j=0;j<nodes[i]->legInfo.size();++j){
            out << "[Leg] ID                   : " << nodes[i]->legInfo[j]->legID << "\n";
            out << "Angle[deg]                 : " << nodes[i]->legInfo[j]->angle << "\n";
            out << "Connecting Node and In-Dir : " << nodes[i]->legInfo[j]->connectingNode << " , " << nodes[i]->legInfo[j]->connectingNodeInDirect << "\n";
            out << "Connected Node and Out-Dir : " << nodes[i]->legInfo[j]->connectedNode << " , " << nodes[i]->legInfo[j]->connectedNodeOutDirect << "\n";
            out << "Lane Number(In,Out)        : " << nodes[i]->legInfo[j]->nLaneIn << " , " << nodes[i]->legInfo[j]->nLaneOut << "\n";
        }
        for(int j=0;j<nodes[i]->trafficSignals.size();++j){
            out << "[TrafficSignal] ID : " << nodes[i]->trafficSignals[j]->id << "\n";
            out << "TS Type            : " << nodes[i]->trafficSignals[j]->type << "\n";
            out << "TS Position        : " << nodes[i]->trafficSignals[j]->pos.x() << " , " << nodes[i]->trafficSignals[j]->pos.y() << " , " << nodes[i]->trafficSignals[j]->pos.z() << "\n";
            out << "Facing Dir[deg]    : " << nodes[i]->trafficSignals[j]->facingDirect << "\n";
            out << "TS Node Info       : " << nodes[i]->trafficSignals[j]->relatedNode << " , " << nodes[i]->trafficSignals[j]->controlNodeDirection << " , " << nodes[i]->trafficSignals[j]->controlNodeLane << "\n";
            out << "TS Crosswalk Info  : " << nodes[i]->trafficSignals[j]->controlCrossWalk << "\n";
            for(int k=0;k<nodes[i]->trafficSignals[j]->sigPattern.size();++k){
                out << "TS Display Pattern : " << nodes[i]->trafficSignals[j]->sigPattern[k]->signal << " , " << nodes[i]->trafficSignals[j]->sigPattern[k]->duration << "\n";
            }
            out << "TS Start Offset : " << nodes[i]->trafficSignals[j]->startOffset << "\n";
        }
        for(int j=0;j<nodes[i]->stopLines.size();++j){
            out << "[StopLine] ID : " << nodes[i]->stopLines[j]->id << "\n";
            out << "SL Type       : " << nodes[i]->stopLines[j]->stopLineType << "\n";
            out << "SL Left Edge  : " << nodes[i]->stopLines[j]->leftEdge.x() << " , " << nodes[i]->stopLines[j]->leftEdge.y() << "\n";
            out << "SL Right Edge : " << nodes[i]->stopLines[j]->rightEdge.x() << " , " << nodes[i]->stopLines[j]->rightEdge.y() << "\n";
            out << "SL Cross Lane : ";
            for(int k=0;k<nodes[i]->stopLines[j]->crossLanes.size();++k){
                out << nodes[i]->stopLines[j]->crossLanes[k];
                if( k < nodes[i]->stopLines[j]->crossLanes.size() - 1 ){
                    out << " , ";
                }
            }
            out << "\n";
            out << "SL Node Info : " << nodes[i]->stopLines[j]->relatedNode << " , " << nodes[i]->stopLines[j]->relatedNodeDir << "\n";
        }
        if( nodes[i]->relatedLanes.size() > 0 ){
            out << "[Related Lanes] : ";
            for(int j=0;j<nodes[i]->relatedLanes.size();++j){
                out << nodes[i]->relatedLanes[j];
                if( j < nodes[i]->relatedLanes.size() - 1 ){
                    out << ",";
                }
            }
            out << "\n";
        }

        if( nodes[i]->isOriginNode == true ){
            out << "[Route Data]\n";
            for(int j=0;j<nodes[i]->odData.size();++j){
                out << "Destination Node : " << nodes[i]->odData[j]->destinationNode << "\n";
                for(int k=0;k<nodes[i]->odData[j]->route.size();++k){
                    out << "Route Node List : ";
                    for(int l=0;l<nodes[i]->odData[j]->route[k]->nodeList.size();++l){
                        out << nodes[i]->odData[j]->route[k]->nodeList[l]->node;
                        if( l < nodes[i]->odData[j]->route[k]->nodeList.size() - 1 ){
                            out << " , ";
                        }
                    }
                    out << "\n";
                    out << "Traffic Volume Data : ";
                    for(int l=0;l<nodes[i]->odData[j]->route[k]->volume.size();++l){
                        out << "(" << nodes[i]->odData[j]->route[k]->volume[l]->vehicleKind << "/"
                            << nodes[i]->odData[j]->route[k]->volume[l]->trafficVolume << ")";
                        if( l < nodes[i]->odData[j]->route[k]->volume.size() - 1 ){
                            out << " , ";
                        }
                    }
                    out << "\n";
                }
            }
        }
        out << "\n";
    }

    for(int i=0;i<lanes.size();++i){
        out << "[Lane] ID    : " << lanes[i]->id << "\n";
        out << "Speed Info   : " << lanes[i]->speedInfo << "\n";
        out << "Actual Speed   : " << lanes[i]->actualSpeed << "\n";
        out << "Automatic Driving : " << (lanes[i]->automaticDrivingEnabled == true ? "Enabled" : "Unabled") << "\n";
        out << "Driver Error Tendency : " << lanes[i]->driverErrorProb << "\n";
        out << "LN Node Info : " << lanes[i]->connectedNodeOutDirect << " , " << lanes[i]->connectedNode << " , "
            << lanes[i]->connectedNodeInDirect << " , " << lanes[i]->departureNodeOutDirect << " , " << lanes[i]->departureNode << "\n";
        out << "Lane Number  : " << lanes[i]->laneNum << "\n";
        if( lanes[i]->nextLanes.size() > 0 ){
            out << "Next Lanes   : ";
            for(int j=0;j<lanes[i]->nextLanes.size();++j){
                out << lanes[i]->nextLanes[j];
                if( j < lanes[i]->nextLanes.size() - 1 ){
                    out << " , ";
                }
            }
            out << "\n";
        }
        if( lanes[i]->previousLanes.size() > 0 ){
            out << "Prev Lanes   : ";
            for(int j=0;j<lanes[i]->previousLanes.size();++j){
                out << lanes[i]->previousLanes[j];
                if( j < lanes[i]->previousLanes.size() - 1 ){
                    out << " , ";
                }
            }
            out << "\n";
        }

        out << "LN Boundary : " << (lanes[i]->sWPBoundary == true ? 1 : 0) << " , " << (lanes[i]->eWPBoundary == true ? 1 : 0) << "\n";
        out << "LN Edge Info : " << lanes[i]->sWPInNode << " , " << lanes[i]->sWPNodeDir << " , " << lanes[i]->eWPInNode << " , " << lanes[i]->eWPNodeDir << "\n";
        out << "LN Shape     : " << lanes[i]->shape.pos.first()->x() << "," << lanes[i]->shape.pos.first()->y() << "," << lanes[i]->shape.pos.first()->z() << ","
            << lanes[i]->shape.derivative.first()->x() << "," << lanes[i]->shape.derivative.first()->y() << ","
            << lanes[i]->shape.pos.last()->x() << "," << lanes[i]->shape.pos.last()->y() << "," << lanes[i]->shape.pos.last()->z() << ","
            << lanes[i]->shape.derivative.last()->x() << "," << lanes[i]->shape.derivative.last()->y() << "\n";

        for(int j=0;j<lanes[i]->stopPoints.size();++j){
            out << "[StopPoint] ID  : " << lanes[i]->stopPoints[j]->stoplineID << "\n";
            out << "SP Type         : " << lanes[i]->stopPoints[j]->stoplineType << "\n";
            out << "SP Position     : " << lanes[i]->stopPoints[j]->pos.x() << " , " << lanes[i]->stopPoints[j]->pos.y() << " , " << lanes[i]->stopPoints[j]->pos.z() << "\n";
            out << "SP Derivative   : " << lanes[i]->stopPoints[j]->derivative.x() << " , " << lanes[i]->stopPoints[j]->derivative.y() << "\n";
            out << "SP Dist in Lane : " << lanes[i]->stopPoints[j]->distanceFromLaneStartPoint << "\n";
        }

        for(int j=0;j<lanes[i]->crossPoints.size();++j){
            out << "[CrossPoint] Cross Lane ID : " << lanes[i]->crossPoints[j]->crossLaneID << "\n";
            out << "CP Position                : " << lanes[i]->crossPoints[j]->pos.x() << " , " << lanes[i]->crossPoints[j]->pos.y() << " , " << lanes[i]->crossPoints[j]->pos.z() << "\n";
            out << "CP Derivative              : " << lanes[i]->crossPoints[j]->derivative.x() << " , " << lanes[i]->crossPoints[j]->derivative.y() << "\n";
            out << "CP Dist in Lane            : " << lanes[i]->crossPoints[j]->distanceFromLaneStartPoint << "\n";
        }

        out << "\n";
    }
    out << "\n";

    for(int i=0;i<pedestLanes.size();++i){
        out << "[PedestLane] ID    : " << pedestLanes[i]->id << "\n";
        for(int j=0;j<pedestLanes[i]->shape.size();++j){
            out << "PL Shape : " << pedestLanes[i]->shape[j]->pos.x() << " , " << pedestLanes[i]->shape[j]->pos.y() << " , "
                << pedestLanes[i]->shape[j]->pos.z() << " , " << pedestLanes[i]->shape[j]->width << "\n";
            out << "PL Property : " << (pedestLanes[i]->shape[j]->isCrossWalk == true ? 1 : 0) << " , "
                << pedestLanes[i]->shape[j]->runOutProb << " , " << pedestLanes[i]->shape[j]->runOutDirect << "\n";
        }
        out << "PL Volumne : ";
        for(int j=0;j<pedestLanes[i]->trafficVolume.size();++j){
            out << pedestLanes[i]->trafficVolume[j];
            if( j < pedestLanes[i]->trafficVolume.size() - 1 ){
                out << " , ";
            }
        }
        out << "\n";
        out << "\n";
    }
    out << "\n";


    QString tmpfilename = filename;

    QStringList divFileName = tmpfilename.replace("\\","/").split("/");
    QString pureFileName = QString(divFileName.last());
    QString pathToThisFolder = tmpfilename.remove( pureFileName );

    QStringList divPathToThisPath = pathToThisFolder.split("/");
    int n1 = divPathToThisPath.size() - 1;

    for(int i=0;i<mapImageMng->baseMapImages.size();++i){

        QString pathToImage = mapImageMng->baseMapImages[i]->path;
        if( pathToImage.endsWith("/") == false ){
            pathToImage += QString("/");
        }

        QString reconstStr = pathToImage;
        if( useRelativePath == true ){

            QStringList divPathToImage = pathToImage.replace("\\","/").split("/");
            int n2 = divPathToImage.size() - 1;

            int N = ( n1 < n2 ? n1 : n2 );
            int nb = 0;
            for(int j=0;j<N;++j){
                nb = j;
                if( divPathToThisPath.at(j) != divPathToImage.at(j) ){
                    break;
                }
            }
            nb++;

            if( nb > 1 ){
                int na1 = n1 - nb;
                int na2 = n2 - nb;
                reconstStr = QString();
                for(int j=0;j<na1;++j){
                    reconstStr += QString("../");
                }
                for(int j=nb;j<n2;++j){
                    reconstStr += divPathToImage.at(j) + QString("/");
                }
            }
        }
        pathToImage = reconstStr;


        out << "BASE MAP : "
            << pathToImage << " , "
            << mapImageMng->baseMapImages[i]->filename << " , "
            << mapImageMng->baseMapImages[i]->x << " , "
            << mapImageMng->baseMapImages[i]->y << " , "
            << mapImageMng->baseMapImages[i]->scale << " , "
            << mapImageMng->baseMapImages[i]->rotate << "\n";
    }
    out << "\n";


    file.close();

    return true;
}


bool RoadInfo::LoadRoadData(QString filename)
{
    QFile file(filename);
    if( file.open( QIODevice::ReadOnly | QIODevice::Text) == false ){
        qDebug() << "[LoadRoadData] cannot open file: "<< filename << " to read data.";
        return false;
    }

    roadDataFileName = filename;


    int line_count=0;
    {
        QTextStream in(&file);
        while( !in.atEnd())
        {
            in.readLine();
            line_count++;
        }
    }
    file.close();



    if( file.open( QIODevice::ReadOnly | QIODevice::Text) == false ){
        qDebug() << "[LoadRoadData] cannot open file: "<< filename << " to read data.";
        return false;
    }


    QString tmpfilename = filename;

    QStringList divFileName = tmpfilename.replace("\\","/").split("/");
    QString pureFileName = QString(divFileName.last());
    QString pathToThisFolder = tmpfilename.remove( pureFileName );



    QTextStream in(&file);


    struct ImageFileLoadData
    {
        QString pathToFile;
        float x;
        float y;
        float s;
        float r;
    };

    QList<struct ImageFileLoadData *> imLoadData;


    //
    //  Read Road Data
    //
    bool validFile = false;
    int lineRead = 0;

    QProgressDialog *pd = new QProgressDialog("Loading Data ...", "Cancel", 0, line_count, 0);
    pd->setWindowModality(Qt::WindowModal);
    pd->setWindowIcon(QIcon(":images/SEdit-icon.png"));
    pd->show();

    pd->setValue(0);
    QApplication::processEvents();


    while( in.atEnd() == false ){

        QString Line = in.readLine();

        lineRead++;
        pd->setValue(lineRead);
        QApplication::processEvents();


        if( Line.startsWith("#") == true || Line.isEmpty() == true ){

            if( Line.contains("SEdit Data File") == true ){
                validFile = true;
            }
            continue;
        }

        if( validFile == false ){
            qDebug() << "[LoadRoadData] The file is not SEdit Data File";
            return false;
        }


        QStringList divLine = Line.split(":");
        if( divLine.size() < 2 ){
            continue;
        }

        QString tagStr = QString( divLine[0] ).trimmed();
        QString dataStr = QString( divLine[1] ).trimmed();
        if( divLine.size() > 2 ){
            for(int i=2;i<divLine.size();++i){
                dataStr += QString(":") + QString( divLine[2] ).trimmed();
            }
        }


        if( tagStr.contains("LeftOrRight") == true ){

            LeftOrRight = dataStr.toInt();

        }
        else if( tagStr.contains("[Node] ID") == true ){

            struct NodeInfo *nd = new struct NodeInfo;
            nd->id = dataStr.toInt();
            nd->isOriginNode = false;
            nd->isDestinationNode = false;

            nodes.append( nd );

//            qDebug() << "[Node] ID = " << nd->id;
        }
        else if( tagStr.contains("ND Center") == true ){

            QStringList divDataStr = dataStr.split(",");

            nodes.last()->pos.setX( QString(divDataStr[0]).trimmed().toFloat() );
            nodes.last()->pos.setY( QString(divDataStr[1]).trimmed().toFloat() );
        }
        else if( tagStr.contains("OD Type") == true ){

            QStringList divDataStr = dataStr.split(",");

            nodes.last()->isOriginNode = QString(divDataStr[0]).trimmed().toInt() == 1 ? true : false;
            nodes.last()->isDestinationNode = QString(divDataStr[1]).trimmed().toInt() == 1 ? true : false;
        }
        else if( tagStr.contains("[Leg] ID") == true ){

            struct LegInfo* leg = new struct LegInfo;
            leg->legID = dataStr.toInt();

            nodes.last()->legInfo.append( leg );

//            qDebug() << "[Leg] ID = " << leg->legID;
        }
        else if( tagStr.contains("Angle[deg]") == true ){

            nodes.last()->legInfo.last()->angle = dataStr.toFloat();
        }
        else if( tagStr.contains("Connecting Node and In-Dir") == true ){

            QStringList divDataStr = dataStr.split(",");

            nodes.last()->legInfo.last()->connectingNode = QString(divDataStr[0]).trimmed().toInt();
            nodes.last()->legInfo.last()->connectingNodeInDirect = QString(divDataStr[1]).trimmed().toInt();

        }
        else if( tagStr.contains("Connected Node and Out-Dir") == true ){

            QStringList divDataStr = dataStr.split(",");

            nodes.last()->legInfo.last()->connectedNode = QString(divDataStr[0]).trimmed().toInt();
            nodes.last()->legInfo.last()->connectedNodeOutDirect = QString(divDataStr[1]).trimmed().toInt();

        }
        else if( tagStr.contains("Lane Number(In,Out)") == true ){

            QStringList divDataStr = dataStr.split(",");

            nodes.last()->legInfo.last()->nLaneIn = QString(divDataStr[0]).trimmed().toInt();
            nodes.last()->legInfo.last()->nLaneOut = QString(divDataStr[1]).trimmed().toInt();

        }
        else if( tagStr.contains("[TrafficSignal] ID") == true ){

            struct TrafficSignalInfo *ts = new struct TrafficSignalInfo;
            ts->id = dataStr.toInt();

            nodes.last()->trafficSignals.append( ts );

//            qDebug() << "[TrafficSignal] ID = " << ts->id;
        }
        else if( tagStr.contains("TS Type") == true ){

            nodes.last()->trafficSignals.last()->type = dataStr.toInt();
        }
        else if( tagStr.contains("TS Position") == true ){

            QStringList divDataStr = dataStr.split(",");

            nodes.last()->trafficSignals.last()->pos.setX( QString(divDataStr[0]).trimmed().toFloat() );
            nodes.last()->trafficSignals.last()->pos.setY( QString(divDataStr[1]).trimmed().toFloat() );
            nodes.last()->trafficSignals.last()->pos.setZ( QString(divDataStr[2]).trimmed().toFloat() );
        }
        else if( tagStr.contains("Facing Dir[deg]") == true ){

            nodes.last()->trafficSignals.last()->facingDirect = dataStr.toFloat();

        }
        else if( tagStr.contains("TS Node Info") == true ){

            QStringList divDataStr = dataStr.split(",");

            nodes.last()->trafficSignals.last()->relatedNode = QString(divDataStr[0]).trimmed().toInt();
            nodes.last()->trafficSignals.last()->controlNodeDirection = QString(divDataStr[1]).trimmed().toInt();
            nodes.last()->trafficSignals.last()->controlNodeLane = QString(divDataStr[2]).trimmed().toInt();

            nodes.last()->trafficSignals.last()->controlCrossWalk = -1;

            for(int k=0;k<nodes.last()->legInfo.size();++k){
                if( nodes.last()->legInfo[k]->legID == nodes.last()->trafficSignals.last()->controlNodeDirection ){
                    nodes.last()->trafficSignals.last()->facingDirect = nodes.last()->legInfo[k]->angle;
                    break;
                }
            }
        }
        else if( tagStr.contains("TS Crosswalk Info") == true ){

            nodes.last()->trafficSignals.last()->controlCrossWalk = dataStr.trimmed().toInt();

        }
        else if( tagStr.contains("TS Display Pattern") == true ){

            QStringList divDataStr = dataStr.split(",");

            struct SignalPatternData *sp = new struct SignalPatternData;

            sp->signal = QString(divDataStr[0]).trimmed().toInt();
            sp->duration = QString(divDataStr[1]).trimmed().toInt();

            nodes.last()->trafficSignals.last()->sigPattern.append( sp );

        }
        else if( tagStr.contains("TS Start Offset") == true ){

            nodes.last()->trafficSignals.last()->startOffset = dataStr.toInt();

        }
        else if( tagStr.contains("[StopLine] ID") == true ){

            struct StopLineInfo *sl = new struct StopLineInfo;
            sl->id = dataStr.toInt();

            nodes.last()->stopLines.append( sl );

//            qDebug() << "[StopLine] ID = " << sl->id;
        }
        else if( tagStr.contains("SL Type") == true ){

            nodes.last()->stopLines.last()->stopLineType = dataStr.toInt();;
        }
        else if( tagStr.contains("SL Left Edge") == true ){

            QStringList divDataStr = dataStr.split(",");

            nodes.last()->stopLines.last()->leftEdge.setX( QString(divDataStr[0]).trimmed().toFloat() );
            nodes.last()->stopLines.last()->leftEdge.setY( QString(divDataStr[1]).trimmed().toFloat() );
        }
        else if( tagStr.contains("SL Right Edge") == true ){

            QStringList divDataStr = dataStr.split(",");

            nodes.last()->stopLines.last()->rightEdge.setX( QString(divDataStr[0]).trimmed().toFloat() );
            nodes.last()->stopLines.last()->rightEdge.setY( QString(divDataStr[1]).trimmed().toFloat() );
        }
        else if( tagStr.contains("SL Cross Lane") == true ){

            QStringList divDataStr = dataStr.split(",");

            for(int i=0;i<divDataStr.size();++i){
                nodes.last()->stopLines.last()->crossLanes.append( QString(divDataStr[i]).trimmed().toInt() );
            }
        }
        else if( tagStr.contains("SL Node Info") == true ){

            QStringList divDataStr = dataStr.split(",");

            nodes.last()->stopLines.last()->relatedNode = QString(divDataStr[0]).trimmed().toInt();
            nodes.last()->stopLines.last()->relatedNodeDir = QString(divDataStr[1]).trimmed().toInt();
        }
        else if( tagStr.contains("Related Lanes") == true ){

            QStringList divDataStr = dataStr.split(",");

            for(int i=0;i<divDataStr.size();++i){
                nodes.last()->relatedLanes.append( QString(divDataStr[i]).trimmed().toInt() );
            }

//            qDebug() << "Related Lanes : size = " << nodes.last()->relatedLanes.size();
        }
        else if( tagStr.contains("Destination Node") == true ){

            struct ODData * od = new struct ODData;

            od->destinationNode = dataStr.toInt();

            nodes.last()->odData.append( od );

        }
        else if( tagStr.contains("Route Node List") == true ){

            struct RouteData *r = new struct RouteData;
            nodes.last()->odData.last()->route.append( r );

            QStringList divDataStr = dataStr.split(",");
            for(int i=0;i<divDataStr.size();++i){

                struct RouteElem *e = new struct RouteElem;

                e->node = QString(divDataStr[i]).trimmed().toInt();

                nodes.last()->odData.last()->route.last()->nodeList.append( e );
            }

        }
        else if( tagStr.contains("Traffic Volume Data") == true ){

            QStringList divDataStr = dataStr.split(",");
            for(int i=0;i<divDataStr.size();++i){

                QStringList pairStr = QString( divDataStr[i] ).remove("(").remove(")").split("/");
                if( pairStr.size() == 2 ){
                    int kind = QString( pairStr[0] ).toInt();
                    int val  = QString( pairStr[1] ).toInt();

                    struct TrafficVolumeData *tv = new struct TrafficVolumeData;
                    tv->vehicleKind = kind;
                    tv->trafficVolume = val;

                    nodes.last()->odData.last()->route.last()->volume.append( tv );
                }
                else{
                    qDebug() << "Can not get value: Traffic Volume Data : " << QString( divDataStr[0] );
                }

            }
        }
        else if( tagStr.contains("[Lane] ID") == true ){

            struct LaneInfo* ln = new struct LaneInfo;
            ln->id = dataStr.toInt();

            // default values
            ln->automaticDrivingEnabled = false;
            ln->driverErrorProb = 0.0;

            lanes.append( ln );

//            qDebug() << "[Lane] ID = " << ln->id;
        }
        else if( tagStr.contains("Speed Info") == true ){

            float spInfo = dataStr.toFloat();
            if( spInfo < 0.1 ){
                spInfo = 60.0;
            }
            lanes.last()->speedInfo = spInfo;
            lanes.last()->actualSpeed = spInfo;  // in case "Actual Speed" is missed
        }
        else if( tagStr.contains("Actual Speed") == true ){

            float spInfo = dataStr.toFloat();
            if( spInfo < 0.1 ){
                spInfo = 65.0;
            }
            lanes.last()->actualSpeed = spInfo;
        }
        else if( tagStr.contains("Automatic Driving") == true ){

            if( dataStr.contains("Enabled") == true ){
                lanes.last()->automaticDrivingEnabled = true;
            }
            else{
                lanes.last()->automaticDrivingEnabled = false;
            }
        }
        else if( tagStr.contains("Driver Error Tendency") == true ){

            double val = dataStr.toDouble();
            if( val < 0.0 ){
                val = 0.0;
            }
            else if( val > 1.0 ){
                val = 1.0;
            }
            lanes.last()->driverErrorProb = val;
        }
        else if( tagStr.contains("LN Boundary") == true ){

            QStringList divDataStr = dataStr.split(",");

            if( QString(divDataStr[0]).trimmed().toInt() == 1 ){
                lanes.last()->sWPBoundary = true;
            }
            else{
                lanes.last()->sWPBoundary = false;
            }

            if( QString(divDataStr[1]).trimmed().toInt() == 1 ){
                lanes.last()->eWPBoundary = true;
            }
            else{
                lanes.last()->eWPBoundary = false;
            }

            //qDebug() << "[LN Boundart] Lane:ID=" << lanes.last()->id << " Boundary=" << lanes.last()->sWPBoundary << " , " << lanes.last()->eWPBoundary;
        }
        else if( tagStr.contains("LN Node Info") == true ){

            QStringList divDataStr = dataStr.split(",");

            lanes.last()->connectedNodeOutDirect = QString(divDataStr[0]).trimmed().toInt();
            lanes.last()->connectedNode          = QString(divDataStr[1]).trimmed().toInt();
            lanes.last()->connectedNodeInDirect  = QString(divDataStr[2]).trimmed().toInt();
            lanes.last()->departureNodeOutDirect = QString(divDataStr[3]).trimmed().toInt();
            lanes.last()->departureNode          = QString(divDataStr[4]).trimmed().toInt();
        }
        else if( tagStr.contains("Lane Number") == true ){

            lanes.last()->laneNum = dataStr.toInt();
        }
        else if( tagStr.contains("Next Lanes") == true ){

            QStringList divDataStr = dataStr.split(",");

            for(int i=0;i<divDataStr.size();++i){
                lanes.last()->nextLanes.append( QString(divDataStr[i]).trimmed().toInt() );
            }
        }
        else if( tagStr.contains("Prev Lanes") == true ){

            QStringList divDataStr = dataStr.split(",");

            for(int i=0;i<divDataStr.size();++i){
                lanes.last()->previousLanes.append( QString(divDataStr[i]).trimmed().toInt() );
            }
        }
        else if( tagStr.contains("LN Edge Info") == true ){

            QStringList divDataStr = dataStr.split(",");

            lanes.last()->sWPInNode  = QString(divDataStr[0]).trimmed().toInt();
            lanes.last()->sWPNodeDir = QString(divDataStr[1]).trimmed().toInt();
            lanes.last()->eWPInNode  = QString(divDataStr[2]).trimmed().toInt();
            lanes.last()->eWPNodeDir = QString(divDataStr[3]).trimmed().toInt();
        }
        else if( tagStr.contains("LN Shape") == true ){

            QStringList divDataStr = dataStr.split(",");

            QVector3D *startPoint = new QVector3D;
            startPoint->setX( QString(divDataStr[0]).trimmed().toFloat() );
            startPoint->setY( QString(divDataStr[1]).trimmed().toFloat() );
            startPoint->setZ( QString(divDataStr[2]).trimmed().toFloat() );
            lanes.last()->shape.pos.append( startPoint );

            QVector2D *startDev = new QVector2D;
            startDev->setX( QString(divDataStr[3]).trimmed().toFloat() );
            startDev->setY( QString(divDataStr[4]).trimmed().toFloat() );
            lanes.last()->shape.derivative.append( startDev );

            QVector3D *endPoint = new QVector3D;
            endPoint->setX( QString(divDataStr[5]).trimmed().toFloat() );
            endPoint->setY( QString(divDataStr[6]).trimmed().toFloat() );
            endPoint->setZ( QString(divDataStr[7]).trimmed().toFloat() );
            lanes.last()->shape.pos.append( endPoint );

            QVector2D *endDev = new QVector2D;
            endDev->setX( QString(divDataStr[8]).trimmed().toFloat() );
            endDev->setY( QString(divDataStr[9]).trimmed().toFloat() );
            lanes.last()->shape.derivative.append( endDev );

            CalculateShape( &(lanes.last()->shape) );

//            qDebug() << "LN Shape calculated";
        }
        else if( tagStr.contains("[CrossPoint] Cross Lane ID") == true ){

            struct CrossPointInfo *cp = new struct CrossPointInfo;
            cp->crossLaneID = dataStr.toInt();

            lanes.last()->crossPoints.append( cp );

//            qDebug() << "[CrossPoint] Cross Lane ID = " << cp->crossLaneID;
        }
        else if( tagStr.contains("CP Position") == true ){

            QStringList divDataStr = dataStr.split(",");

            lanes.last()->crossPoints.last()->pos.setX( QString(divDataStr[0]).trimmed().toFloat() );
            lanes.last()->crossPoints.last()->pos.setY( QString(divDataStr[1]).trimmed().toFloat() );
            lanes.last()->crossPoints.last()->pos.setZ( QString(divDataStr[2]).trimmed().toFloat() );
        }
        else if( tagStr.contains("CP Derivative") == true ){

            QStringList divDataStr = dataStr.split(",");

            lanes.last()->crossPoints.last()->derivative.setX( QString(divDataStr[0]).trimmed().toFloat() );
            lanes.last()->crossPoints.last()->derivative.setY( QString(divDataStr[1]).trimmed().toFloat() );
        }
        else if( tagStr.contains("CP Dist in Lane") == true ){

            lanes.last()->crossPoints.last()->distanceFromLaneStartPoint = dataStr.toFloat();
        }
        else if( tagStr.contains("[StopPoint] ID") == true ){

            struct StopPointInfo* sp = new struct StopPointInfo;
            sp->stoplineID = dataStr.toInt();

            lanes.last()->stopPoints.append( sp );

//            qDebug() << "[StopPoint] ID = " << sp->stoplineID;
        }
        else if( tagStr.contains("SP Type") == true ){

            lanes.last()->stopPoints.last()->stoplineType = dataStr.toInt();
        }
        else if( tagStr.contains("SP Position") == true ){

            QStringList divDataStr = dataStr.split(",");

            lanes.last()->stopPoints.last()->pos.setX( QString(divDataStr[0]).trimmed().toFloat() );
            lanes.last()->stopPoints.last()->pos.setY( QString(divDataStr[1]).trimmed().toFloat() );
            lanes.last()->stopPoints.last()->pos.setZ( QString(divDataStr[2]).trimmed().toFloat() );

        }
        else if( tagStr.contains("SP Derivative") == true ){

            QStringList divDataStr = dataStr.split(",");

            lanes.last()->stopPoints.last()->derivative.setX( QString(divDataStr[0]).trimmed().toFloat() );
            lanes.last()->stopPoints.last()->derivative.setY( QString(divDataStr[1]).trimmed().toFloat() );
        }
        else if( tagStr.contains("SP Dist in Lane") == true ){

            lanes.last()->stopPoints.last()->distanceFromLaneStartPoint = dataStr.toFloat();
        }
        else if( tagStr.contains("[PedestLane] ID") == true ){

            struct PedestrianLane *pl = new PedestrianLane;

            pl->id = dataStr.toInt();

            pedestLanes.append( pl );
        }
        else if( tagStr.contains("PL Shape") == true ){

            QStringList divDataStr = dataStr.split(",");

            struct PedestrianLaneShapeElement *ple = new struct PedestrianLaneShapeElement;

            ple->pos.setX( QString(divDataStr[0]).trimmed().toFloat() );
            ple->pos.setY( QString(divDataStr[1]).trimmed().toFloat() );
            ple->pos.setZ( QString(divDataStr[2]).trimmed().toFloat() );
            ple->width = QString(divDataStr[3]).trimmed().toFloat();

            pedestLanes.last()->shape.append( ple );
        }
        else if( tagStr.contains("PL Property") == true ){

            QStringList divDataStr = dataStr.split(",");

            pedestLanes.last()->shape.last()->isCrossWalk  = ( QString(divDataStr[0]).trimmed().toInt() == 1 ? true : false);
            pedestLanes.last()->shape.last()->runOutProb   = QString(divDataStr[1]).trimmed().toFloat();
            pedestLanes.last()->shape.last()->runOutDirect = QString(divDataStr[2]).trimmed().toFloat();

        }
        else if( tagStr.contains("PL Volumne") == true  ){

            QStringList divDataStr = dataStr.split(",");
            for(int i=0;i<divDataStr.size();++i){
                pedestLanes.last()->trafficVolume.append( QString( divDataStr[i] ).trimmed().toInt() );
            }
        }
        else if( tagStr.contains("BASE MAP") == true ){

            QStringList divDataStr = dataStr.split(",");

            QString imagefilename = QString( divDataStr[0] ).trimmed().replace("\\","/");
            if( imagefilename.isEmpty() == false && imagefilename.endsWith("/") == false ){
                imagefilename += QString("/");
            }

            // Check if the path is absolute or relative
            QString reconstFilename = imagefilename;
            if( imagefilename.contains(":") == false ){

                reconstFilename = pathToThisFolder + imagefilename;

            }
            imagefilename = reconstFilename;

            imagefilename += QString( divDataStr[1] ).trimmed();

            float x = QString( divDataStr[2] ).trimmed().toFloat();
            float y = QString( divDataStr[3] ).trimmed().toFloat();
            float scale = QString( divDataStr[4] ).trimmed().toFloat();
            float rot = QString( divDataStr[5] ).trimmed().toFloat();

            struct ImageFileLoadData *im = new struct ImageFileLoadData;

            im->pathToFile = imagefilename;
            im->x = x;
            im->y = y;
            im->s = scale;
            im->r = rot;

            imLoadData.append( im );
        }
    }

    pd->setValue(line_count);
    QApplication::processEvents();


    file.close();

    for(int i=0;i<nodes.size();++i){
        nodes[i]->nLeg  = nodes[i]->legInfo.size();
        nodes[i]->hasTS = nodes[i]->trafficSignals.size() > 0 ? true : false;

        // Check OD Data is valid
        if(nodes[i]->odData.size() == 0){
            continue;
        }

        for(int j=nodes[i]->odData.size()-1;j>=0;--j){

            bool isFoundDestNode = false;
            for(int k=0;k<nodes.size();++k){
                if( k == i ){
                    continue;
                }
                if( nodes[k]->id == nodes[i]->odData[j]->destinationNode ){
                    isFoundDestNode = true;
                    break;
                }
            }
            if( isFoundDestNode == false ){

                ClearODData( nodes[i]->odData[j] );

                delete nodes[i]->odData[j];

                nodes[i]->odData.removeAt( j );
            }
        }
    }

    for(int i=0;i<pedestLanes.size();++i){
        UpdatePedestLaneShapeParams( pedestLanes[i]->id );
    }


    bool ret = true;


    // Check if the data is old format
    for(int i=0;i<lanes.size();++i){
        if( lanes[i]->eWPInNode < 0 && lanes[i]->connectedNode >= 0 ){
            lanes[i]->eWPInNode = lanes[i]->connectedNode;
        }
        if( lanes[i]->sWPInNode < 0 ){
            if( lanes[i]->connectedNodeOutDirect < 0 && lanes[i]->departureNode >= 0 ){
                lanes[i]->sWPInNode  = lanes[i]->departureNode;
                lanes[i]->sWPNodeDir = lanes[i]->departureNodeOutDirect;
                lanes[i]->eWPNodeDir = lanes[i]->connectedNodeInDirect;
            }
            else if( lanes[i]->connectedNodeOutDirect >= 0 ){
                lanes[i]->sWPInNode  = lanes[i]->eWPInNode;
                lanes[i]->sWPNodeDir = lanes[i]->connectedNodeInDirect;
                lanes[i]->eWPNodeDir = lanes[i]->connectedNodeOutDirect;
            }
        }
    }


    // Check Validity of sWPInNode
    for(int i=0;i<lanes.size();++i){

        if( lanes[i]->eWPInNode == lanes[i]->sWPInNode ){
            continue;
        }

        int ndIdx = indexOfNode( lanes[i]->eWPInNode );
        if( ndIdx >= 0 ){
            int connectedNode = -1;
            for(int j=0;j<nodes[ndIdx]->legInfo.size();++j){
                if( nodes[ndIdx]->legInfo[j]->legID == lanes[i]->eWPNodeDir ){
                    connectedNode = nodes[ndIdx]->legInfo[j]->connectedNode;
                    break;
                }
            }
            if( connectedNode >= 0 && lanes[i]->sWPInNode != connectedNode ){
                lanes[i]->sWPInNode = connectedNode;
            }

            CheckLaneRelatedNode( lanes[i]->id );
        }
    }


    // Check Lane Connection
    CheckLaneConnectionFull();


    // Calculate Stop Point Data
    CheckAllStopLineCrossLane();


    // Calculate Lane Cross Points
    ret = CheckLaneCrossPoints();
    if( ret == false ){
        return false;
    }

    // Check Cross Points with pedestLanes
    CheckPedestLaneCrossPoints();

    // Create WP Data
    CreateWPData();


    // Check Route Data
    CheckRouteInOutDirection();


    // Set Lane List
    SetAllLaneLists();


    // Set Turn Direction Info
    {
        QList<int> nodeList;
        for(int i=0;i<nodes.size();++i){
            nodeList.append( i );
        }
        SetTurnDirectionInfo(nodeList);
    }


    // Find PedestSignal
    FindPedestSignalFroCrossWalk();


    // Set Route Lane List
    SetAllRouteLaneList();



    if( imLoadData.size() > 0 ){

        QProgressDialog *pdimg = new QProgressDialog("Loading Image ...", "Cancel", 0, imLoadData.size(), 0);
        pdimg->setWindowIcon(QIcon(":images/SEdit-icon.png"));
        pdimg->setWindowModality(Qt::WindowModal);
        pdimg->show();

        pdimg->setValue(0);
        QApplication::processEvents();

        bool loadImage = true;
        for(int i=0;i<imLoadData.size();++i){

            if( i > 0 ){

                QStringList divPTF1 = imLoadData[i]->pathToFile.split("/");
                QString folderPath1 = QString();
                for(int j=0;j<divPTF1.size()-1;++j){
                    folderPath1 += QString( divPTF1[j] ) + QString("/");
                }

                QStringList divPTF2 = imLoadData[i-1]->pathToFile.split("/");
                QString folderPath2 = QString();
                for(int j=0;j<divPTF2.size()-1;++j){
                    folderPath2 += QString( divPTF2[j] ) + QString("/");
                }

                if( folderPath1 != folderPath2 ){
                    imLoadData[i]->pathToFile = folderPath2 + QString(divPTF1.last());
                }
            }

            mapImageMng->AddMapImageFromFile( imLoadData[i]->pathToFile,
                                              imLoadData[i]->x,
                                              imLoadData[i]->y,
                                              imLoadData[i]->s,
                                              imLoadData[i]->r,
                                              loadImage );

            pdimg->setValue(i+1);
            QApplication::processEvents();

            if( pdimg->wasCanceled() ){
                if( loadImage == true ){
                    qDebug() << "Canceled.";
                }
                loadImage = false;
            }

            if( i + 1 == imLoadData.size() ){
                qDebug() << "All Images loaded.";
            }
        }


        for(int i=0;i<imLoadData.size();++i){
            delete imLoadData[i];
        }
        imLoadData.clear();
    }


    qDebug() << "------ Edn of Load SEdit Data";

    return true;
}


bool RoadInfo::outputResimRoadFiles(QString outputfoldername, QString outputfilename)
{
    if( outputfoldername.endsWith("/") == false ){
        outputfoldername += QString("/");
    }

    QString resimRoadFile = outputfoldername + outputfilename + QString(".rr.txt");

    QFile file_rr(resimRoadFile);
    if( file_rr.open( QIODevice::WriteOnly | QIODevice::Text) == false ){
        qDebug() << "[RoadInfo::outputResilFiles] cannot open file: "<< resimRoadFile << " to write data.";
        return false;
    }

    QTextStream out(&file_rr);

    out << "#-----------------------------------------------------\n";
    out << "#              Re:sim Road Data File\n";
    out << "#-----------------------------------------------------\n";
    out << "\n";

    out << "Left-Hand Or Right-Hand ; " << (LeftOrRight == LEFT_HAND_TRAFFIC ? "LEFT" : "RIGHT")  << "\n";
    out << "\n";

    out << "#-----------------------------------------------------\n";
    out << "# WayPoint ; id , x , y , z , direct[rad]\n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<wps.size();++i){

        out << "WayPoint ; " << wps[i]->id << " , "
            << wps[i]->pos.x() << " , "
            << wps[i]->pos.y() << " , "
            << wps[i]->pos.z() << " , "
            << wps[i]->angle << "\n";

    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# Path ; id , startWP , endWP , Ndiv , Speed Limit[km/h] , Actual Speed[km/h]\n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<lanes.size();++i){

        out << "Path ; " << lanes[i]->id << " , "
            << lanes[i]->startWPID << " , "
            << lanes[i]->endWPID << " , "
            << (lanes[i]->shape.pos.size()-1) << " , "
            << lanes[i]->speedInfo << " , "
            << lanes[i]->actualSpeed << "\n";
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# CrossPoint ; pathID , \n";
    out << "#       ( crossPathID, xcp, ycp, zcp, Dx, Dy, L) * nCP\n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<lanes.size();++i){

        if( lanes[i]->crossPoints.size() == 0 ){
            continue;
        }

        out << "CrossPoint ; " << lanes[i]->id << " , ";

        for(int j=0;j<lanes[i]->crossPoints.size();++j){

            out << lanes[i]->crossPoints[j]->crossLaneID << " / "
                << lanes[i]->crossPoints[j]->pos.x() << " / "
                << lanes[i]->crossPoints[j]->pos.y() << " / "
                << lanes[i]->crossPoints[j]->pos.z() << " / "
                << lanes[i]->crossPoints[j]->derivative.x() << " / "
                << lanes[i]->crossPoints[j]->derivative.y() << " / "
                << lanes[i]->crossPoints[j]->distanceFromLaneStartPoint;

            if( j < lanes[i]->crossPoints.size() - 1 ){
                out << " , ";
            }
            else{
                out << "\n";
            }
        }
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# StopPoint ; pathID , \n";
    out << "#  ( stoplineID, type, xcp, ycp, zcp, Dx, Dy, L, relatedNode, relatedLeg) * nSL\n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<lanes.size();++i){

        if( lanes[i]->stopPoints.size() == 0 ){
            continue;
        }

        out << "StopPoint ; " << lanes[i]->id << " , ";

        for(int j=0;j<lanes[i]->stopPoints.size();++j){

            out << lanes[i]->stopPoints[j]->stoplineID << " / "
                << lanes[i]->stopPoints[j]->stoplineType << " / "
                << lanes[i]->stopPoints[j]->pos.x() << " / "
                << lanes[i]->stopPoints[j]->pos.y() << " / "
                << lanes[i]->stopPoints[j]->pos.z() << " / "
                << lanes[i]->stopPoints[j]->derivative.x() << " / "
                << lanes[i]->stopPoints[j]->derivative.y() << " / "
                << lanes[i]->stopPoints[j]->distanceFromLaneStartPoint << " / ";

            bool foundSL = false;
            for(int k=0;k<nodes.size();++k){
                for(int l=0;l<nodes[k]->stopLines.size();++l){
                    if( nodes[k]->stopLines[l]->id == lanes[i]->stopPoints[j]->stoplineID ){

                        out << nodes[k]->id << " / "
                            << nodes[k]->stopLines[l]->relatedNodeDir;

                        foundSL = true;
                        break;
                    }
                }
                if( foundSL == true ){
                    break;
                }
            }

            if( j < lanes[i]->stopPoints.size() - 1 ){
                out << " , ";
            }
            else{
                out << "\n";
            }
        }
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# PedestCrossPoint ; pathID , \n";
    out << "#   ( crossPedestPathID, sectionIndex, xcp, ycp, zcp, Dx, Dy, L) * nCP\n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<lanes.size();++i){

        if( lanes[i]->pedestCrossPoints.size() == 0 ){
            continue;
        }

        out << "PedestCrossPoint ; " << lanes[i]->id << " , ";

        for(int j=0;j<lanes[i]->pedestCrossPoints.size();++j){

            out << lanes[i]->pedestCrossPoints[j]->crossLaneID << " / "
                << lanes[i]->pedestCrossPoints[j]->crossSectIndex << " / "
                << lanes[i]->pedestCrossPoints[j]->pos.x() << " / "
                << lanes[i]->pedestCrossPoints[j]->pos.y() << " / "
                << lanes[i]->pedestCrossPoints[j]->pos.z() << " / "
                << lanes[i]->pedestCrossPoints[j]->derivative.x() << " / "
                << lanes[i]->pedestCrossPoints[j]->derivative.y() << " / "
                << lanes[i]->pedestCrossPoints[j]->distanceFromLaneStartPoint;

            if( j < lanes[i]->pedestCrossPoints.size() - 1 ){
                out << " , ";
            }
            else{
                out << "\n";
            }
        }
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# Pedest-Path ; pedestPathID, \n";
    out << "# Pedest-Path Shape ; x, y, z, width, length, angle \n";
    out << "# Pedest-Path Property ; isCrossWalk, pedestSignalID, runOutProb, runOutDir \n";
    out << "# Pedest-Path Traffic ; Volume1 , Volume2 , ... , Volume_nPedKind\n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<pedestLanes.size();++i){

        out << "Pedest-Path ; " << pedestLanes[i]->id << "\n";

        for(int j=0;j<pedestLanes[i]->shape.size();++j){

            out << "Pedest-Path Shape ; "
                << pedestLanes[i]->shape[j]->pos.x() << " , "
                << pedestLanes[i]->shape[j]->pos.y() << " , "
                << pedestLanes[i]->shape[j]->pos.z() << " , "
                << pedestLanes[i]->shape[j]->width << " , "
                << pedestLanes[i]->shape[j]->distanceToNextPos << " , "
                << pedestLanes[i]->shape[j]->angleToNextPos << "\n";

            out << "Pedest-Path Property ; "
                << (pedestLanes[i]->shape[j]->isCrossWalk == true ? 1 : 0) << " , "
                << pedestLanes[i]->shape[j]->controlPedestSignalID << " , "
                << pedestLanes[i]->shape[j]->runOutProb << " , "
                << pedestLanes[i]->shape[j]->runOutDirect << "\n";
        }

        out << "Pedest-Path Traffic ; ";
        for(int j=0;j<pedestLanes[i]->trafficVolume.size();++j){

            out << pedestLanes[i]->trafficVolume[j];
            if( j < pedestLanes[i]->trafficVolume.size() - 1 ){
                out << " , ";
            }
            else{
                out << "\n";
            }
        }

        out << "\n";
    }

    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# Node ; id , x , y , nLeg , (legID_0 / ... / legID_nLeg) , hasTS\n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<nodes.size();++i){

        out << "Node ; " << nodes[i]->id << " , "
            << nodes[i]->pos.x() << " , "
            << nodes[i]->pos.y() << " , "
            << nodes[i]->legInfo.size() << " , ";

        for(int j=0;j<nodes[i]->legInfo.size();++j){
            out << nodes[i]->legInfo[j]->legID;
            if(j < nodes[i]->legInfo.size() - 1 ){
                out << " / ";
            }
        }
        out << " , ";

        out << (nodes[i]->trafficSignals.size() > 0 ? 1 : 0) << "\n";
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# Node Connection ; id , (inLegID / FromNode / outLegID) * nLeg\n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<nodes.size();++i){

        out << "Node Connection ; " << nodes[i]->id << " , ";

        for(int j=0;j<nodes[i]->legInfo.size();++j){

            out << nodes[i]->legInfo[j]->legID << " / "
                << nodes[i]->legInfo[j]->connectedNode << " / "
                << nodes[i]->legInfo[j]->connectedNodeOutDirect;

            if( j < nodes[i]->legInfo.size() - 1 ){
                out << " , ";
            }
            else {
                out << "\n";
            }
        }
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# Direction Map ; Node id , \n";
    out << "#    (inLegID / oncoming / left1|left2|... / right1|right2...) * nLeg\n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<nodes.size();++i){

        out << "Direction Map ; " << nodes[i]->id << " , ";

        for(int j=0;j<nodes[i]->legInfo.size();++j){

            out << nodes[i]->legInfo[j]->legID << " / "
                << nodes[i]->legInfo[j]->oncomingLegID << " / ";
            for(int k=0;k<nodes[i]->legInfo[j]->leftTurnLegID.size();++k){
                out << nodes[i]->legInfo[j]->leftTurnLegID[k];
                if( k < nodes[i]->legInfo[j]->leftTurnLegID.size() - 1 ){
                    out << " | ";
                }
            }
            out << " / ";
            for(int k=0;k<nodes[i]->legInfo[j]->rightTurnLegID.size();++k){
                out << nodes[i]->legInfo[j]->rightTurnLegID[k];
                if( k < nodes[i]->legInfo[j]->rightTurnLegID.size() - 1 ){
                    out << " | ";
                }
            }

            if( j < nodes[i]->legInfo.size() - 1 ){
                out << " , ";
            }
            else {
                out << "\n";
            }
        }
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# In-boundary WPs ; id , (LegID / inPWs) * nLeg\n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<nodes.size();++i){

        out << "In-boundary WPs ; " << nodes[i]->id;

        for(int j=0;j<nodes[i]->legInfo.size();++j){

            if( nodes[i]->legInfo[j]->inWPs.size() > 0 ){

                out << " , " << nodes[i]->legInfo[j]->legID << " / ";

                for(int k=0;k<nodes[i]->legInfo[j]->inWPs.size();++k){

                    out << nodes[i]->legInfo[j]->inWPs[k];

                    if( k < nodes[i]->legInfo[j]->inWPs.size() - 1 ){
                        out << " / ";
                    }
                }
            }

            if( j == nodes[i]->legInfo.size() - 1 ){
                out << "\n";
            }

        }
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# Out-boundary WPs ; id , (LegID / outWPs) * nLeg\n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<nodes.size();++i){

        out << "Out-boundary WPs ; " << nodes[i]->id;

        for(int j=0;j<nodes[i]->legInfo.size();++j){

            if( nodes[i]->legInfo[j]->outWPs.size() > 0 ){

                out << " , " << nodes[i]->legInfo[j]->legID << " / ";

                for(int k=0;k<nodes[i]->legInfo[j]->outWPs.size();++k){

                    out << nodes[i]->legInfo[j]->outWPs[k];

                    if( k < nodes[i]->legInfo[j]->outWPs.size() - 1 ){
                        out << " / ";
                    }
                }
            }

            if( j == nodes[i]->legInfo.size() - 1 ){
                out << "\n";
            }
        }
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# Lane List ; nodeId , outLegID, inLegID, \n";
    out << "#                  (lane1 <- lane2 <- ... <- lane_N ) \n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<nodes.size();++i){
        for(int j=0;j<nodes[i]->laneList.size();++j){
            for(int k=0;k<nodes[i]->laneList[j]->lanes.size();++k){
                out << "Lane List ; " << nodes[i]->id << " , "
                    << nodes[i]->laneList[j]->relatedNodeOutDirection << " , "
                    << nodes[i]->laneList[j]->relatedNodeInDirection << " , ";
                for(int l=0;l<nodes[i]->laneList[j]->lanes[k].size();++l){
                    out << nodes[i]->laneList[j]->lanes[k].at(l);
                    if( l < nodes[i]->laneList[j]->lanes[k].size() - 1 ){
                        out << " <- ";
                    }
                }
                out << "\n";
            }
        }
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# Route Data ;  origin node id , destination Node id, \n";
    out << "#        ( in-Leg | Node | out-Leg ) * nNodeList ,    \n";
    out << "#        ( vehicleKind | trafficVolume ) * nVKind     \n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<nodes.size();++i){

        if( nodes[i]->isOriginNode == false ){
            continue;
        }
        if( nodes[i]->odData.size() == 0 ){
            continue;
        }

        for(int j=0;j<nodes[i]->odData.size();++j){

            if( nodes[i]->odData[j]->route.size() == 0 ){
                continue;
            }


            for(int k=0;k<nodes[i]->odData[j]->route.size();++k){

                if( nodes[i]->odData[j]->route[k]->nodeList.size() == 0 ){
                    continue;
                }
                if( nodes[i]->odData[j]->route[k]->volume.size() == 0 ){
                    continue;
                }

                out << "Route Data ; " << nodes[i]->id << " , " << nodes[i]->odData[j]->destinationNode << " , ";

                for(int l=0;l<nodes[i]->odData[j]->route[k]->nodeList.size();++l){

                    out << nodes[i]->odData[j]->route[k]->nodeList[l]->inDir << "|"
                        << nodes[i]->odData[j]->route[k]->nodeList[l]->node << "|"
                        << nodes[i]->odData[j]->route[k]->nodeList[l]->outDir;

                    if( l < nodes[i]->odData[j]->route[k]->nodeList.size() - 1 ){
                        out << " / ";
                    }
                    else{
                        out << " , ";
                    }
                }

                for(int l=0;l<nodes[i]->odData[j]->route[k]->volume.size();++l){

                    out << nodes[i]->odData[j]->route[k]->volume[l]->vehicleKind << "|"
                        << nodes[i]->odData[j]->route[k]->volume[l]->trafficVolume;

                    if( l < nodes[i]->odData[j]->route[k]->volume.size() - 1 ){
                        out << " / ";
                    }
                    else {
                        out << "\n";
                    }
                }
            }
        }
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# Route Multi-Lanes ; 1, node_1 -> node_2 -> ... -> node_M \n";
    out << "# Route Multi-Lanes ; 2, List No, Start, Goal, sIndex, gIndex \n";
    out << "# Route Multi-Lanes ; 3, List No, lane1 , lane2 , ... , lane_n \n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<nodes.size();++i){

        if( nodes[i]->isOriginNode == false ){
            continue;
        }

        if( nodes[i]->odData.size() == 0 ){
            continue;
        }

        for(int j=0;j<nodes[i]->odData.size();++j){

            for(int k=0;k<nodes[i]->odData[j]->route.size();++k){

                out << "Route Multi-Lanes ; 1 , ";

                for(int l=0;l<nodes[i]->odData[j]->route[k]->nodeList.size();++l){

                    out << nodes[i]->odData[j]->route[k]->nodeList[l]->node;

                    if( l < nodes[i]->odData[j]->route[k]->nodeList.size() - 1 ){
                        out << " , ";
                    }
                }

                out << "\n";

                for(int l=0;l<nodes[i]->odData[j]->route[k]->routeLaneLists.size();++l){

                    out << "Route Multi-Lanes ; 2 , "
                        << nodes[i]->odData[j]->route[k]->routeLaneLists[l]->startNode << " , "
                        << nodes[i]->odData[j]->route[k]->routeLaneLists[l]->goalNode << " , "
                        << nodes[i]->odData[j]->route[k]->routeLaneLists[l]->sIndexInNodeList << " , "
                        << nodes[i]->odData[j]->route[k]->routeLaneLists[l]->gIndexInNodeList << "\n";

                    for(int m=0;m<nodes[i]->odData[j]->route[k]->routeLaneLists[l]->laneList.size();++m){

                        out << "Route Multi-Lanes ; 3 , ";

                        for(int n=0;n<nodes[i]->odData[j]->route[k]->routeLaneLists[l]->laneList[m].size();++n){

                            out << nodes[i]->odData[j]->route[k]->routeLaneLists[l]->laneList[m][n];

                            if( n < nodes[i]->odData[j]->route[k]->routeLaneLists[l]->laneList[m].size() - 1 ){
                                out << " , ";
                            }

                        }

                        out << "\n";
                    }
                }

                out << "\n";
            }
        }
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# Vehicle Kind ;  Category, Subcategory,  \n";
    out << "#                          Length, Width, Height     \n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<setDlg->GetVehicleKindNum();++i){
        out << "Vehicle Kind ; ";
        for(int j=0;j<5;++j){
            out << setDlg->GetVehicleKindTableStr(i,j);
            if( j < 4 ){
                out << " , ";
            }
            else{
                out << "\n";
            }
        }
    }
    out << "\n";


    out << "#-----------------------------------------------------\n";
    out << "# Pedestrian Kind ;  Category, Subcategory,  \n";
    out << "#                          Length, Width, Height     \n";
    out << "#-----------------------------------------------------\n";
    for(int i=0;i<setDlg->GetPedestrianKindNum();++i){
        out << "Pedestrian Kind ; ";
        for(int j=0;j<5;++j){
            out << setDlg->GetPedestKindTableStr(i,j);
            if( j < 4 ){
                out << " , ";
            }
            else{
                out << "\n";
            }
        }
    }
    out << "\n";


    file_rr.close();

    return true;
}


bool RoadInfo::outputResimTrafficSignalFiles(QString outputfoldername, QString outputfilename)
{
    if( outputfoldername.endsWith("/") == false ){
        outputfoldername += QString("/");
    }

    QString resimTSFile = outputfoldername + outputfilename + QString(".ts.txt");

    QFile file_ts(resimTSFile);
    if( file_ts.open( QIODevice::WriteOnly | QIODevice::Text) == false ){
        qDebug() << "[RoadInfo::outputResilFiles] cannot open file: "<< resimTSFile << " to write data.";
        return false;
    }

    QTextStream out_ts(&file_ts);

    out_ts << "#-----------------------------------------------------\n";
    out_ts << "#           Re:sim Traffic Signal Data File\n";
    out_ts << "#-----------------------------------------------------\n";
    out_ts << "\n";


    out_ts << "#-----------------------------------------------------\n";
    out_ts << "# Type ; id , (vehicle or pedestrian) \n";
    out_ts << "#-----------------------------------------------------\n";
    for(int i=0;i<nodes.size();++i){
        for(int j=0;j<nodes[i]->trafficSignals.size();++j){

            out_ts << "Type  ; " << nodes[i]->trafficSignals[j]->id << " , ";
            if( nodes[i]->trafficSignals[j]->type == 0 ){
                out_ts << "vehicle\n";
            }
            else{
                out_ts << "pedestrian\n";
            }
        }
    }
    out_ts << "\n";


    out_ts << "#-----------------------------------------------------\n";
    out_ts << "# Location ; id , x, y, direction[deg] \n";
    out_ts << "#-----------------------------------------------------\n";
    for(int i=0;i<nodes.size();++i){
        for(int j=0;j<nodes[i]->trafficSignals.size();++j){

            out_ts << "Location ; " << nodes[i]->trafficSignals[j]->id << " , "
                << nodes[i]->trafficSignals[j]->pos.x() << " , "
                << nodes[i]->trafficSignals[j]->pos.y() << " , "
                << nodes[i]->trafficSignals[j]->facingDirect << "\n";

        }
    }
    out_ts << "\n";


    out_ts << "#-----------------------------------------------------\n";
    out_ts << "# Control ; id , relatedNode, ctrlDir, CtrlLane, ctrlCrossWalk \n";
    out_ts << "#-----------------------------------------------------\n";
    for(int i=0;i<nodes.size();++i){
        for(int j=0;j<nodes[i]->trafficSignals.size();++j){

            out_ts << "Control ; " << nodes[i]->trafficSignals[j]->id << " , "
                << nodes[i]->trafficSignals[j]->relatedNode << " , "
                << nodes[i]->trafficSignals[j]->controlNodeDirection << " , "
                << nodes[i]->trafficSignals[j]->controlNodeLane << " , "
                << nodes[i]->trafficSignals[j]->controlCrossWalk << "\n";
        }
    }
    out_ts << "\n";


    out_ts << "#-----------------------------------------------------\n";
    out_ts << "# Display ; id , startOffset[s]\n";
    out_ts << "#                 (signal / duration) * nPattern \n";
    out_ts << "#-----------------------------------------------------\n";

    for(int i=0;i<nodes.size();++i){
        for(int j=0;j<nodes[i]->trafficSignals.size();++j){

            if( nodes[i]->trafficSignals[j]->sigPattern.size() == 0 ){
                continue;
            }

            out_ts << "Display ; " << nodes[i]->trafficSignals[j]->id << " , ";

            out_ts << nodes[i]->trafficSignals[j]->startOffset << " , ";   // Start Offset; coded later

            for(int k=0;k<nodes[i]->trafficSignals[j]->sigPattern.size();++k){

                out_ts << nodes[i]->trafficSignals[j]->sigPattern[k]->signal << " / "
                    << nodes[i]->trafficSignals[j]->sigPattern[k]->duration;

                if( k < nodes[i]->trafficSignals[j]->sigPattern.size() - 1 ){
                    out_ts << " , ";
                }
                else{
                    out_ts << "\n";
                }
            }
        }
    }
    out_ts << "\n";


    file_ts.close();

    return true;
}


bool RoadInfo::outputResimScenarioFiles(QString outputfoldername, QString outputfilename,int maxAgent, bool onlyFilename,QString scenariofilename)
{
    if( outputfoldername.endsWith("/") == false ){
        outputfoldername += QString("/");
    }

    QString resimScenarioFile = outputfoldername + outputfilename + QString(".rs.txt");

    QFile file_rs(resimScenarioFile);
    if( file_rs.open( QIODevice::WriteOnly | QIODevice::Text) == false ){
        qDebug() << "[RoadInfo::outputResilFiles] cannot open file: "<< resimScenarioFile << " to write data.";
        return false;
    }

    QTextStream out(&file_rs);

    out << "#-----------------------------------------------------\n";
    out << "#                 Re:sim Scenario File  \n";
    out << "#-----------------------------------------------------\n";
    out << "\n";
    if( onlyFilename == true ){
        out << "Road Data File ; " << (outputfilename + QString(".rr.txt")) << "\n";
    }
    else{
        out << "Road Data File ; " << (outputfoldername + outputfilename + QString(".rr.txt")) << "\n";
    }

    out << "\n";
    if( onlyFilename == true ){
        out << "Signal Data File ; " << (outputfilename + QString(".ts.txt")) << "\n";
    }
    else{
        out << "Signal Data File ; " << (outputfoldername + outputfilename + QString(".ts.txt")) << "\n";
    }
    out << "\n";
    out << "Max Number of Agent ; " << maxAgent << "\n";
    out << "\n";


    // Scenario Data
    if( scenariofilename.isNull() == false && scenariofilename.isEmpty() == false && scenariofilename.contains(".ss.txt") == true ){

        ScenarioEditor *sE = new ScenarioEditor( setDlg, this );

        sE->LoadDataWithFilename( scenariofilename );

        out << "#---------------\n";
        out << "Scenario ID ; 0\n";
        out << "End Condition ; 0\n";
        out << "End Target ; -1\n";
        out << "End Trigger Position ; 0 ; 0 ; 0\n";
        out << "End Time ; 0 ; 0 ; 0 ; 0 ; 0 ; 0\n";


        for(int i=0;i<sE->sSys.size();++i){

             out << "#---------------\n";
             out << "Scenario Event ID ; " << sE->sSys[i]->ID << "\n";

             out << "Scenario Event Type ; " << sE->sSys[i]->sItem.act.actionType << "\n";

             if( sE->sSys[i]->sItem.cond.externalTrigger == true ){
                 out << "Scenario Event Trigger External ; "
                     << (sE->sSys[i]->sItem.cond.funcKey + 1)  << " ; "
                     << sE->sSys[i]->sItem.cond.FETrigger << "\n";
             }
             else{
                 out << "Scenario Event Trigger External ; 0 ; "
                     << sE->sSys[i]->sItem.cond.FETrigger << "\n";
             }

             out << "Scenario Event Trigger Internal ; "
                 << sE->sSys[i]->sItem.cond.combination << " ; "
                 << sE->sSys[i]->sItem.cond.noTrigger << " ; "
                 << sE->sSys[i]->sItem.cond.timeTrigger << " ; "
                 << sE->sSys[i]->sItem.cond.positionTrigger << " ; "
                 << sE->sSys[i]->sItem.cond.velocityTrigger << " ; "
                 << sE->sSys[i]->sItem.cond.TTCTrigger << "\n";

             if( sE->sSys[i]->sItem.cond.timeTrigger == true ){
                 out << "Scenario Event Trigger Time ; 0 ; "
                     << (sE->sSys[i]->sItem.cond.ttMin * 60 + sE->sSys[i]->sItem.cond.ttSec) << " ; "
                     << "0.0\n";
             }

             if( sE->sSys[i]->sItem.cond.positionTrigger == true ){
                 out << "Scenario Event Trigger Position ; "
                     << sE->sSys[i]->sItem.cond.ptX << " ; "
                     << sE->sSys[i]->sItem.cond.ptY << " ; "
                     << sE->sSys[i]->sItem.cond.ptPassAngle << " ; "
                     << sE->sSys[i]->sItem.cond.ptTargetObjID << " ; "
                     << sE->sSys[i]->sItem.cond.ptWidth << "\n";
             }

             if( sE->sSys[i]->sItem.cond.velocityTrigger == true ){
                 out << "Scenario Event Trigger Speed ; "
                     << sE->sSys[i]->sItem.cond.vtSpeed << " ; "
                     << sE->sSys[i]->sItem.cond.vtLowOrHigh << " ; "
                     << sE->sSys[i]->sItem.cond.vtTargetObjID << "\n";
             }

             if( sE->sSys[i]->sItem.cond.TTCTrigger == true ){
                 out << "Scenario Event Trigger TTC ; "
                     << sE->sSys[i]->sItem.cond.ttcVal << " ; "
                     << sE->sSys[i]->sItem.cond.ttcCalTargetObjID << " ; "
                     << sE->sSys[i]->sItem.cond.ttcCalType << " ; "
                     << sE->sSys[i]->sItem.cond.ttcCalObjectID << " ; "
                     << sE->sSys[i]->sItem.cond.ttcCalPosX << " ; "
                     << sE->sSys[i]->sItem.cond.ttcCalPosY << "\n";
             }

             if( sE->sSys[i]->sItem.act.fParams.size() > 0 ){

                 out << "Scenario Event Param Float ; ";
                 for(int j=0;j<sE->sSys[i]->sItem.act.fParams.size();++j){
                     out << sE->sSys[i]->sItem.act.fParams[j];
                     if( j < sE->sSys[i]->sItem.act.fParams.size() - 1 ){
                         out << " ; ";
                     }
                 }
                 out << "\n";

             }

             if( sE->sSys[i]->sItem.act.iParams.size() > 0 ){

                 out << "Scenario Event Param Integer ; ";
                 for(int j=0;j<sE->sSys[i]->sItem.act.iParams.size();++j){
                     out << sE->sSys[i]->sItem.act.iParams[j];
                     if( j < sE->sSys[i]->sItem.act.iParams.size() - 1 ){
                         out << " ; ";
                     }
                 }
                 out << "\n";

             }

             if( sE->sSys[i]->sItem.act.bParams.size() > 0 ){

                 out << "Scenario Event Param Boolean ; ";
                 for(int j=0;j<sE->sSys[i]->sItem.act.bParams.size();++j){
                     out << sE->sSys[i]->sItem.act.bParams[j];
                     if( j < sE->sSys[i]->sItem.act.bParams.size() - 1 ){
                         out << " ; ";
                     }
                 }
                 out << "\n";

             }

        }

        for(int i=0;i<sE->sVehicle.size();++i){

            out << "#---------------\n";
            out << "Vehicle ID ; " << sE->sVehicle[i]->ID << "\n";

            for(int j=0;j<sE->sVehicle[i]->sItem.size();++j){

                out << "Vehicle Event Type ; " << sE->sVehicle[i]->sItem[j]->act.actionType << "\n";

                if( sE->sVehicle[i]->sItem[j]->cond.externalTrigger == true ){
                    out << "Vehicle Event Trigger External ; "
                        << (sE->sVehicle[i]->sItem[j]->cond.funcKey + 1)
                        << sE->sVehicle[i]->sItem[j]->cond.FETrigger << "\n";
                }
                else{
                    out << "Vehicle Event Trigger External ; 0 ; "
                        << sE->sVehicle[i]->sItem[j]->cond.FETrigger << "\n";
                }

                out << "Vehicle Event Trigger Internal ; "
                    << sE->sVehicle[i]->sItem[j]->cond.combination << " ; "
                    << sE->sVehicle[i]->sItem[j]->cond.noTrigger << " ; "
                    << sE->sVehicle[i]->sItem[j]->cond.timeTrigger << " ; "
                    << sE->sVehicle[i]->sItem[j]->cond.positionTrigger << " ; "
                    << sE->sVehicle[i]->sItem[j]->cond.velocityTrigger << " ; "
                    << sE->sVehicle[i]->sItem[j]->cond.TTCTrigger << "\n";

                if( sE->sVehicle[i]->sItem[j]->cond.timeTrigger == true ){
                    out << "Vehicle Event Trigger Time ; 0 ; "
                        << (sE->sVehicle[i]->sItem[j]->cond.ttMin * 60 + sE->sVehicle[i]->sItem[j]->cond.ttSec) << " ; "
                        << "0.0\n";
                }

                if( sE->sVehicle[i]->sItem[j]->cond.positionTrigger == true ){
                    out << "Vehicle Event Trigger Position ; "
                        << sE->sVehicle[i]->sItem[j]->cond.ptX << " ; "
                        << sE->sVehicle[i]->sItem[j]->cond.ptY << " ; "
                        << sE->sVehicle[i]->sItem[j]->cond.ptPassAngle << " ; "
                        << sE->sVehicle[i]->sItem[j]->cond.ptTargetObjID << " ; "
                        << sE->sVehicle[i]->sItem[j]->cond.ptWidth << "\n";
                }

                if( sE->sVehicle[i]->sItem[j]->cond.velocityTrigger == true ){
                    out << "Vehicle Event Trigger Speed ; "
                        << sE->sVehicle[i]->sItem[j]->cond.vtSpeed << " ; "
                        << sE->sVehicle[i]->sItem[j]->cond.vtLowOrHigh << " ; "
                        << sE->sVehicle[i]->sItem[j]->cond.vtTargetObjID << "\n";
                }

                if( sE->sVehicle[i]->sItem[j]->cond.TTCTrigger == true ){
                    out << "Vehicle Event Trigger TTC ; "
                        << sE->sVehicle[i]->sItem[j]->cond.ttcVal << " ; "
                        << sE->sVehicle[i]->sItem[j]->cond.ttcCalType << " ; "
                        << sE->sVehicle[i]->sItem[j]->cond.ttcCalTargetObjID << " ; "
                        << sE->sVehicle[i]->sItem[j]->cond.ttcCalPosX << " ; "
                        << sE->sVehicle[i]->sItem[j]->cond.ttcCalPosY << "\n";
                }

                if( sE->sVehicle[i]->sItem[j]->act.fParams.size() > 0 ){

                    out << "Vehicle Event Param Float ; ";
                    for(int k=0;k<sE->sVehicle[i]->sItem[j]->act.fParams.size();++k){
                        out << sE->sVehicle[i]->sItem[j]->act.fParams[k];
                        if( k < sE->sVehicle[i]->sItem[j]->act.fParams.size() - 1 ){
                            out << " ; ";
                        }
                    }
                    out << "\n";
                }

                if( sE->sVehicle[i]->sItem[j]->act.iParams.size() > 0 ){

                    out << "Vehicle Event Param Integer ; ";
                    for(int k=0;k<sE->sVehicle[i]->sItem[j]->act.iParams.size();++k){
                        out << sE->sVehicle[i]->sItem[j]->act.iParams[k];
                        if( k < sE->sVehicle[i]->sItem[j]->act.iParams.size() - 1 ){
                            out << " ; ";
                        }
                    }
                    out << "\n";
                }

                if( sE->sVehicle[i]->sItem[j]->act.bParams.size() > 0 ){

                    out << "Vehicle Event Param Boolean ; ";
                    for(int k=0;k<sE->sVehicle[i]->sItem[j]->act.bParams.size();++k){
                        out << sE->sVehicle[i]->sItem[j]->act.bParams[k];
                        if( k < sE->sVehicle[i]->sItem[j]->act.bParams.size() - 1 ){
                            out << " ; ";
                        }
                    }
                    out << "\n";
                }

                if( sE->sVehicle[i]->sItem[j]->act.route != NULL &&
                        sE->sVehicle[i]->sItem[j]->act.route->routeLaneLists.size() > 0 ){

                    out << "Vehicle Route Multi-Lanes ; 1 ; ";

                    for(int l=0;l<sE->sVehicle[i]->sItem[j]->act.route->nodeList.size();++l){

                        out << sE->sVehicle[i]->sItem[j]->act.route->nodeList[l]->node;

                        if( l < sE->sVehicle[i]->sItem[j]->act.route->nodeList.size() - 1 ){
                            out << " ; ";
                        }
                    }

                    out << "\n";

                    for(int l=0;l<sE->sVehicle[i]->sItem[j]->act.route->routeLaneLists.size();++l){

                       out << "Vehicle Route Multi-Lanes ; 2 ; "
                           << sE->sVehicle[i]->sItem[j]->act.route->routeLaneLists[l]->startNode << " ; "
                           << sE->sVehicle[i]->sItem[j]->act.route->routeLaneLists[l]->goalNode << " ; "
                           << sE->sVehicle[i]->sItem[j]->act.route->routeLaneLists[l]->sIndexInNodeList << " ; "
                           << sE->sVehicle[i]->sItem[j]->act.route->routeLaneLists[l]->gIndexInNodeList << "\n";

                       for(int m=0;m<sE->sVehicle[i]->sItem[j]->act.route->routeLaneLists[l]->laneList.size();++m){

                           out << "Vehicle Route Multi-Lanes ; 3 ; ";

                           for(int n=0;n<sE->sVehicle[i]->sItem[j]->act.route->routeLaneLists[l]->laneList[m].size();++n){

                               out << sE->sVehicle[i]->sItem[j]->act.route->routeLaneLists[l]->laneList[m][n];

                               if( n < sE->sVehicle[i]->sItem[j]->act.route->routeLaneLists[l]->laneList[m].size() - 1 ){
                                   out << " ; ";
                               }
                           }
                           out << "\n";
                       }
                    }
                }
            }
        }

        for(int i=0;i<sE->sPedest.size();++i){

            out << "#---------------\n";

            out << "Pedestrian ID ; " << sE->sPedest[i]->ID << "\n";

            for(int j=0;j<sE->sPedest[i]->sItem.size();++j){

                out << "Pedestrian Event Type ; " << sE->sPedest[i]->sItem[j]->act.actionType << "\n";

                if( sE->sPedest[i]->sItem[j]->cond.externalTrigger == true ){
                    out << "Pedestrian Event Trigger External ; "
                        << (sE->sPedest[i]->sItem[j]->cond.funcKey + 1)
                        << sE->sPedest[i]->sItem[j]->cond.FETrigger << "\n";
                }
                else{
                    out << "Pedestrian Event Trigger External ; 0 ; "
                        << sE->sPedest[i]->sItem[j]->cond.FETrigger << "\n";
                }

                out << "Pedestrian Event Trigger Internal ; "
                    << sE->sPedest[i]->sItem[j]->cond.combination << " ; "
                    << sE->sPedest[i]->sItem[j]->cond.noTrigger << " ; "
                    << sE->sPedest[i]->sItem[j]->cond.timeTrigger << " ; "
                    << sE->sPedest[i]->sItem[j]->cond.positionTrigger << " ; "
                    << sE->sPedest[i]->sItem[j]->cond.velocityTrigger << " ; "
                    << sE->sPedest[i]->sItem[j]->cond.TTCTrigger << "\n";

                if( sE->sPedest[i]->sItem[j]->cond.timeTrigger == true ){
                    out << "Pedestrian Event Trigger Time ; 0 ; "
                        << (sE->sPedest[i]->sItem[j]->cond.ttMin * 60 + sE->sPedest[i]->sItem[j]->cond.ttSec) << " ; "
                        << "0.0\n";
                }

                if( sE->sPedest[i]->sItem[j]->cond.positionTrigger == true ){
                    out << "Pedestrian Event Trigger Position ; "
                        << sE->sPedest[i]->sItem[j]->cond.ptX << " ; "
                        << sE->sPedest[i]->sItem[j]->cond.ptY << " ; "
                        << sE->sPedest[i]->sItem[j]->cond.ptPassAngle << " ; "
                        << sE->sPedest[i]->sItem[j]->cond.ptTargetObjID << " ; "
                        << sE->sPedest[i]->sItem[j]->cond.ptWidth << "\n";
                }

                if( sE->sPedest[i]->sItem[j]->cond.velocityTrigger == true ){
                    out << "Pedestrian Event Trigger Speed ; "
                        << sE->sPedest[i]->sItem[j]->cond.vtSpeed << " ; "
                        << sE->sPedest[i]->sItem[j]->cond.vtLowOrHigh << " ; "
                        << sE->sPedest[i]->sItem[j]->cond.vtTargetObjID << "\n";
                }

                if( sE->sPedest[i]->sItem[j]->cond.TTCTrigger == true ){
                    out << "Vehicle Event Trigger TTC ; "
                        << sE->sPedest[i]->sItem[j]->cond.ttcVal << " ; "
                        << sE->sPedest[i]->sItem[j]->cond.ttcCalType << " ; "
                        << sE->sPedest[i]->sItem[j]->cond.ttcCalTargetObjID << " ; "
                        << sE->sPedest[i]->sItem[j]->cond.ttcCalPosX << " ; "
                        << sE->sPedest[i]->sItem[j]->cond.ttcCalPosY << "\n";
                }

                if( sE->sPedest[i]->sItem[j]->act.fParams.size() > 0 ){

                    out << "Pedestrian Event Param Float ; ";
                    for(int k=0;k<sE->sPedest[i]->sItem[j]->act.fParams.size();++k){
                        out << sE->sPedest[i]->sItem[j]->act.fParams[k];
                        if( k < sE->sPedest[i]->sItem[j]->act.fParams.size() - 1 ){
                            out << " ; ";
                        }
                    }
                    out << "\n";
                }

                if( sE->sPedest[i]->sItem[j]->act.iParams.size() > 0 ){

                    out << "Pedestrian Event Param Integer ; ";
                    for(int k=0;k<sE->sPedest[i]->sItem[j]->act.iParams.size();++k){
                        out << sE->sPedest[i]->sItem[j]->act.iParams[k];
                        if( k < sE->sPedest[i]->sItem[j]->act.iParams.size() - 1 ){
                            out << " ; ";
                        }
                    }
                    out << "\n";
                }

                if( sE->sPedest[i]->sItem[j]->act.bParams.size() > 0 ){

                    out << "Pedestrian Event Param Boolean ; ";
                    for(int k=0;k<sE->sPedest[i]->sItem[j]->act.bParams.size();++k){
                        out << sE->sPedest[i]->sItem[j]->act.bParams[k];
                        if( k < sE->sPedest[i]->sItem[j]->act.bParams.size() - 1 ){
                            out << " ; ";
                        }
                    }
                    out << "\n";
                }
            }
        }

        out << "#---------------\n";

        delete sE;
    }

    file_rs.close();

    return true;
}

