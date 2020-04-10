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


#include "odrouteeditor.h"

ODRouteEditor::ODRouteEditor(QWidget *parent) : QWidget(parent)
{
    currentOriginNode = -1;

    cbDestnation = new QComboBox();
    connect( cbDestnation, SIGNAL(currentIndexChanged(QString)), this, SLOT(SetCurrentODRouteDataForDestination(QString)) );

    QHBoxLayout *destLay = new QHBoxLayout();
    destLay->addWidget( new QLabel("Destination Node : ") );
    destLay->addWidget( cbDestnation );
    destLay->addStretch(1);

    QHBoxLayout *TVLabelLay = new QHBoxLayout();
    TVLabelLay->addWidget( new QLabel("Traffic Volumn : ") );
    TVLabelLay->addStretch(1);

    trafficVolumeTable = new QTableWidget();

    QHBoxLayout *TVTableLay = new QHBoxLayout();
    TVTableLay->addWidget( trafficVolumeTable );
    //TVTableLay->addStretch(1);

    addRouteBtn = new QPushButton("Add");
    addRouteBtn->setIcon( QIcon(":/images/Add.png") );
    connect( addRouteBtn, SIGNAL(clicked()), this, SLOT(AddRoute()) );

    delRouteBtn = new QPushButton("Remove");
    delRouteBtn->setIcon( QIcon(":/images/Remove.png") );
    connect( delRouteBtn, SIGNAL(clicked()), this, SLOT(DelRoute()) );

    QHBoxLayout *routeLabelLay = new QHBoxLayout();
    routeLabelLay->addWidget( new QLabel("Routes : ") );
    routeLabelLay->addWidget( addRouteBtn );
    routeLabelLay->addWidget( delRouteBtn );
    routeLabelLay->addStretch(1);

    routeTable = new QTableWidget();

    routeTable->setColumnCount( 1 );
    routeTable->setColumnWidth(0,1000);

    QStringList tableLabels;
    tableLabels << QString("Route");

    routeTable->setHorizontalHeaderLabels( tableLabels );
    routeTable->setMinimumWidth( 1010 );
    routeTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | (Qt::Alignment)Qt::TextWordWrap);
    routeTable->horizontalHeader()->setMinimumHeight(50);

    QHBoxLayout *routeTableLay = new QHBoxLayout();
    routeTableLay->addWidget( routeTable );


    applyDataBtn = new QPushButton("Apply Changes");
    applyDataBtn->setIcon( QIcon(":/images/accept.png") );
    connect( applyDataBtn, SIGNAL(clicked()), this, SLOT(ApplyData()) );

    QHBoxLayout *applyLay = new QHBoxLayout();
    applyLay->addStretch(1);
    applyLay->addWidget( applyDataBtn );
    applyLay->addStretch(1);


    QVBoxLayout *mainLayout = new QVBoxLayout();

    mainLayout->addLayout( destLay );
    mainLayout->addLayout( routeLabelLay );
    mainLayout->addLayout( routeTableLay );
    mainLayout->addLayout( TVLabelLay );
    mainLayout->addLayout( TVTableLay );
    mainLayout->addLayout( applyLay );

    setLayout( mainLayout );

    setWindowTitle("OD & Route Editor");
}


void ODRouteEditor::SetHeaderTrafficVolumeTable()
{
    if( !setDlg ){
        return;
    }

    trafficVolumeTable->clear();

    int N = setDlg->GetVehicleKindNum();
    trafficVolumeTable->setColumnCount( 1 + N );

    trafficVolumeTable->setColumnWidth(0,100);
    int width = 100;
    for(int i=0;i<N;++i){
        trafficVolumeTable->setColumnWidth(1+i,100);
        width += 100;
    }
    width += 10;

    QStringList tableLabels = setDlg->GetVehicleKindSubcategory();
    tableLabels.prepend("Route");

    trafficVolumeTable->setHorizontalHeaderLabels( tableLabels );

    if( width > 1000 ){
        width = 1000;
    }
    trafficVolumeTable->setMinimumWidth( width );
    trafficVolumeTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | (Qt::Alignment)Qt::TextWordWrap);
    trafficVolumeTable->horizontalHeader()->setMinimumHeight(50);
}


void ODRouteEditor::SetCurrentODRouteData(int oNode,bool routTableSetFlag)
{
    qDebug() << "[ODRouteEditor::SetCurrentODRouteData] oNode = " << oNode;

    int ndIdx = road->indexOfNode( oNode );
    if( ndIdx < 0 ){
        return;
    }

    currentOriginNode = oNode;

    disconnect( cbDestnation, SIGNAL(currentIndexChanged(QString)), this, SLOT(SetCurrentODRouteDataForDestination(QString)) );

    cbDestnation->clear();
    QStringList destNodeList;
    for(int i=0;i<road->nodes.size();++i){
        if( road->nodes[i]->isDestinationNode == true && road->nodes[i]->id != currentOriginNode ){
            destNodeList.append( QString("Node %1").arg( road->nodes[i]->id ) );
        }
    }
    cbDestnation->addItems( destNodeList );

    connect( cbDestnation, SIGNAL(currentIndexChanged(QString)), this, SLOT(SetCurrentODRouteDataForDestination(QString)) );


    if( routTableSetFlag == true ){

        if( destNodeList.size() > 0 ){

            SetCurrentODRouteDataForDestination( destNodeList.first() );
        }
    }
}


void ODRouteEditor::SetCurrentODRouteDataForDestination(QString dNodeStr)
{
    qDebug() << "[ODRouteEditor::SetCurrentODRouteDataForDestination] dNodeStr = " << dNodeStr;

    int dNode = dNodeStr.remove("Node ").toInt();
    int ndIdx = road->indexOfNode( currentOriginNode );
    if( ndIdx < 0 ){
        return;
    }

    for(int i=routeTable->rowCount()-1;i>=0;i--){
        routeTable->removeRow(i);
    }
    for(int i=trafficVolumeTable->rowCount()-1;i>=0;i--){
        trafficVolumeTable->removeRow(i);
    }

    qDebug() << "odData.size = " << road->nodes[ndIdx]->odData.size();

    for(int i=0;i<road->nodes[ndIdx]->odData.size();++i){

        if( road->nodes[ndIdx]->odData[i]->destinationNode != dNode ){
            continue;
        }

        qDebug() << "destinationNode = " << road->nodes[ndIdx]->odData[i]->destinationNode;
        qDebug() << "route.size() = " << road->nodes[ndIdx]->odData[i]->route.size();

        for(int j=0;j<road->nodes[ndIdx]->odData[i]->route.size();++j){

            int atRow = routeTable->rowCount();
            routeTable->insertRow( atRow );

            QTableWidgetItem *item1 = new QTableWidgetItem();
            QString routeStr = QString("Node List: ");
            for(int k=0;k<road->nodes[ndIdx]->odData[i]->route[j]->nodeList.size();++k){
                routeStr += QString("%1").arg( road->nodes[ndIdx]->odData[i]->route[j]->nodeList[k]->node );
                if( k < road->nodes[ndIdx]->odData[i]->route[j]->nodeList.size() - 1 ){
                    routeStr += QString(" -> ");
                }
            }
            item1->setText( routeStr );
            routeTable->setItem(atRow,0,item1);


            atRow = trafficVolumeTable->rowCount() ;
            trafficVolumeTable->insertRow( atRow );

            QTableWidgetItem *item3 = new QTableWidgetItem();
            item3->setText( QString("%1").arg( j + 1 ) );
            item3->setFlags( Qt::NoItemFlags );
            trafficVolumeTable->setItem(atRow,0,item3);

            qDebug() << "volume.size() = " << road->nodes[ndIdx]->odData[i]->route[j]->volume.size();

            for(int k=0;k<road->nodes[ndIdx]->odData[i]->route[j]->volume.size();++k){
                int kind = road->nodes[ndIdx]->odData[i]->route[j]->volume[k]->vehicleKind;

                qDebug() << "[" << k << "] kind = " << kind;
                qDebug() << "[" << k << "] trafficVolume = " << road->nodes[ndIdx]->odData[i]->route[j]->volume[k]->trafficVolume;

                if( kind >= 0 && kind < trafficVolumeTable->columnCount()-1 ){
                    QTableWidgetItem *item = new QTableWidgetItem();
                    item->setText( QString("%1").arg(road->nodes[ndIdx]->odData[i]->route[j]->volume[k]->trafficVolume) );
                    trafficVolumeTable->setItem(atRow,kind+1,item);
                }
            }

        }

        break;
    }

    emit UpdateGraphic();
}


int ODRouteEditor::GetCurrentDestinationNode()
{
    int destNode = cbDestnation->currentText().remove("Node ").toInt();
    return destNode;
}

int ODRouteEditor::GetCurrentRouteIndex()
{
    return routeTable->currentIndex().row();
}

void ODRouteEditor::ApplyData()
{
    if( currentOriginNode < 0 ){
        return;
    }

    int ndIdx = road->indexOfNode( currentOriginNode );
    if( ndIdx < 0 ){
        return;
    }

    int destNode = GetCurrentDestinationNode();
    qDebug() << "ApplyData : OriginNode = " << currentOriginNode << " DestinationNode = " << destNode;

    int odIdx = -1;
    for(int i=0;i<road->nodes[ndIdx]->odData.size();++i){
        if( road->nodes[ndIdx]->odData[i]->destinationNode == destNode ){
            odIdx = i;
        }
    }

    qDebug() << "odIdx = " << odIdx;

    if( odIdx >= 0 ){

        // Clear Data
        for(int i=0;i<road->nodes[ndIdx]->odData[odIdx]->route.size();++i){

            for(int j=0;j<road->nodes[ndIdx]->odData[odIdx]->route[i]->nodeList.size();++j){
                delete road->nodes[ndIdx]->odData[odIdx]->route[i]->nodeList[j];
            }
            road->nodes[ndIdx]->odData[odIdx]->route[i]->nodeList.clear();

            for(int j=0;j<road->nodes[ndIdx]->odData[odIdx]->route[i]->volume.size();++j){
                delete road->nodes[ndIdx]->odData[odIdx]->route[i]->volume[j];
            }
            road->nodes[ndIdx]->odData[odIdx]->route[i]->volume.clear();

            delete road->nodes[ndIdx]->odData[odIdx]->route[i];
        }
        road->nodes[ndIdx]->odData[odIdx]->route.clear();

        qDebug() << "Data cleared.";
    }
    else{

        // Add New
        struct ODData *od = new struct ODData;

        od->destinationNode = destNode;

        road->nodes[ndIdx]->odData.append( od );

        odIdx = road->nodes[ndIdx]->odData.size() - 1;

        qDebug() << "Add New Data. odIdx = " << odIdx;

    }

    for(int i=0;i<routeTable->rowCount();++i){

        qDebug() << "Route Table : row = " << i;

        struct RouteData *rd = new struct RouteData;

        QString nodeListStr = routeTable->item(i,0)->text();
        QStringList nodesStr = nodeListStr.split("->");

        qDebug() << "nodesStr = " << nodesStr;

        for(int j=0;j<nodesStr.size();++j){

            struct RouteElem *re = new struct RouteElem;

            re->node = QString( nodesStr[j] ).remove("Node List:").trimmed().toInt();

            rd->nodeList.append( re );

            qDebug() << " add nodelist : node = " << re->node;
        }

        for(int j=0;j<trafficVolumeTable->rowCount();++j){
            if( trafficVolumeTable->item(j,0)->text().toInt() != i + 1 ){
                continue;
            }

            qDebug() << "trafficVolumeTable : row = " << j;

            int N = trafficVolumeTable->columnCount();
            for(int k=1;k<N;++k){

                struct TrafficVolumeData *tv = new struct TrafficVolumeData;

                tv->vehicleKind = k - 1;

                QString valStr = trafficVolumeTable->item(j,k)->text();
                if( valStr.isNull() == true || valStr.isEmpty() == true ){
                    tv->trafficVolume = 0;
                }
                else{
                    tv->trafficVolume = valStr.toInt();
                }

                qDebug() << "vehicleKind =" << tv->vehicleKind << " trafficVolume = " << tv->trafficVolume;

                rd->volume.append( tv );
            }

            break;
        }

        qDebug() << "Append route data";

        road->nodes[ndIdx]->odData[odIdx]->route.append( rd );

        road->CheckRouteInOutDirectionGivenODNode( currentOriginNode, destNode );

        for(int j=0;j<road->nodes[ndIdx]->odData[odIdx]->route.last()->nodeList.size();++j){
            road->CheckLaneConnectionOfNode( road->nodes[ndIdx]->odData[odIdx]->route.last()->nodeList[j]->node );
        }

        road->GetLaneListForRoute( currentOriginNode, destNode, 0 );

        emit SetNodeSelected( currentOriginNode );
        qDebug() << "Object Selection : set to SEL_NODE, node = " << currentOriginNode;
    }

    qDebug() << "Data Applied.";

    emit ShowMessageStatusBar( QString("Route and Traffic Volume Data applied.") );
}


void ODRouteEditor::AddRoute()
{
    if( currentOriginNode < 0 ){
        return;
    }

    if( cbDestnation->count() == 0 || cbDestnation->currentIndex() < 0 ){
        return;
    }

    int destNode = cbDestnation->currentText().remove("Node ").toInt();
    qDebug() << "AddRoute : OriginNode = " << currentOriginNode << " DestinationNode = " << destNode;

    emit SetNodePickMode(currentOriginNode, destNode);
}


void ODRouteEditor::DelRoute()
{
    int selectedRow = routeTable->currentRow();
    if( selectedRow < 0 ){
        selectedRow = routeTable->rowCount() - 1;
    }

    routeTable->removeRow( selectedRow );
    routeTable->setCurrentIndex( QModelIndex() );

    trafficVolumeTable->removeRow( selectedRow );
    trafficVolumeTable->setCurrentIndex( QModelIndex() );
}

void ODRouteEditor::GetNodeListForRoute(QList<int> nodeList)
{
    int atRow = routeTable->rowCount();

    routeTable->insertRow( atRow );
    QTableWidgetItem *item = new QTableWidgetItem();
    routeTable->setItem( atRow, 0, item );

    QString routeStr = QString("Node List: ");
    for(int k=0;k<nodeList.size();++k){
        routeStr += QString("%1").arg( nodeList[k] );
        if( k < nodeList.size() - 1 ){
            routeStr += QString(" -> ");
        }
    }
    item->setText( routeStr );

    trafficVolumeTable->insertRow( atRow );
    int N = trafficVolumeTable->columnCount();
    for(int i=0;i<N;++i){
        item = new QTableWidgetItem();
        trafficVolumeTable->setItem( atRow, i, item );
        if( i == 0 ){
            item->setText( QString("%1").arg( atRow+1 ) );
            item->setFlags( Qt::NoItemFlags );
        }
    }

    emit ResetNodePickMode();
}


void ODRouteEditor::resizeEvent(QResizeEvent *event)
{
    routeTable->setColumnWidth( 0, event->size().width() );
}

