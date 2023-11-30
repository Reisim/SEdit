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


#include "objectproperty.h"
#include <QInputDialog>

void RoadObjectProperty::ChangeTrafficSignalInfo(int id)
{
    qDebug() << "[RoadObjectProperty::ChangeTrafficSignalInfo] id = " << id;

    if( !tsInfo ){
        return;
    }

    tsInfo->clear();

    if( !road ){
        return;
    }

    int nodeID = road->indexOfTS( id, -1 );
    if( nodeID < 0){
        return;
    }
    int ndIdx  = road->indexOfNode( nodeID );
    int tsIdx  = road->indexOfTS( id, nodeID );
    if( tsIdx < 0 ){
        return;
    }

    QString infoStr = QString();

    infoStr += QString("\n");
    infoStr += QString("[Type]\n");
    if( road->nodes[ndIdx]->trafficSignals[tsIdx]->type == 0 ){
        infoStr += QString("  For Vehicles\n");
    }
    else if( road->nodes[ndIdx]->trafficSignals[tsIdx]->type == 1 ){
        infoStr += QString("  For Pedestrians\n");
    }
    infoStr += QString("\n");
    infoStr += QString("[Control Info]\n");
    infoStr += QString("  Related Node : %1\n").arg( road->nodes[ndIdx]->trafficSignals[tsIdx]->relatedNode );
    infoStr += QString("  Control Leg  : %1\n").arg( road->nodes[ndIdx]->trafficSignals[tsIdx]->controlNodeDirection );
    infoStr += QString("  Control Lane : %1\n").arg( road->nodes[ndIdx]->trafficSignals[tsIdx]->controlNodeLane );
    infoStr += QString("\n");
    infoStr += QString("[Signal Pattern]\n");

    int totalCycle = 0;
    for(int i=0;i<road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.size();++i){
        infoStr += QString("  Pattern[%1]: Val=%2 Dur=%3\n").arg(i)
                .arg( road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i]->signal )
                .arg( road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i]->duration );

        totalCycle += road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i]->duration;
    }
    infoStr += QString("Total Cycle = %1[s]\n").arg( totalCycle );
    infoStr += QString("\n");
    infoStr += QString("\n");

    tsInfo->setText( infoStr );
    tsInfo->setAlignment( Qt::AlignTop );
    tsInfo->setFixedSize( tsInfo->sizeHint() );

    tsStartOffset->setValue( road->nodes[ndIdx]->trafficSignals[tsIdx]->startOffset );

    if( road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.size() == 0 ){
        // Set Default
        SetDefaultTSPattern();
    }
    else{
        SetTSPattern( ndIdx, tsIdx );
    }

    if( road->nodes[ndIdx]->trafficSignals[tsIdx]->isSensorType == true ){
        tsIsSensorType->setChecked(true);
        tsChangeTimeBySensor->setValue( road->nodes[ndIdx]->trafficSignals[tsIdx]->timeToChangeBySensor );
    }
    else{
        tsIsSensorType->setChecked(false);
        tsChangeTimeBySensor->setValue( 0 );
    }

    if( cbChangeSelectionBySpinbox->isChecked() == true ){
        emit ChangeSelectionRequest(5,id);
    }
}


void RoadObjectProperty::AdjustRowCountTSPatternTable(int n)
{
    if( n < 1 ){
        return;
    }

    int nRow = tsDisplayPattern->rowCount();
    if( nRow < n ){
        int nAdd = n - nRow;
        for(int i=0;i<nAdd;++i){
            tsDisplayPattern->insertRow(0);
            for(int j=0;j<7;++j){
                QTableWidgetItem *item = new QTableWidgetItem();
                tsDisplayPattern->setItem(0,j,item);
            }
        }
    }
    else if( nRow > n ){
        int nDel = nRow - n;
        for(int i=0;i<nDel;++i){
            tsDisplayPattern->removeRow( nRow - i - 1 );
        }
    }
}


void RoadObjectProperty::SetTSPattern(int ndIdx,int tsIdx)
{
    int nRow = road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.size();

    AdjustRowCountTSPatternTable( nRow );

    for(int i=0;i<nRow;++i){

        tsDisplayPattern->item(i,0)->setCheckState( Qt::Unchecked );
        tsDisplayPattern->item(i,1)->setCheckState( Qt::Unchecked );
        tsDisplayPattern->item(i,2)->setCheckState( Qt::Unchecked );
        tsDisplayPattern->item(i,3)->setCheckState( Qt::Unchecked );
        tsDisplayPattern->item(i,4)->setCheckState( Qt::Unchecked );
        tsDisplayPattern->item(i,5)->setCheckState( Qt::Unchecked );

        if( (road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i]->signal & 0x01) == 0x01 ){  // Green
            tsDisplayPattern->item(i,0)->setCheckState( Qt::Checked );
        }

        if( (road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i]->signal & 0x02) == 0x02 ){  // Amber/Yellow
            tsDisplayPattern->item(i,1)->setCheckState( Qt::Checked );
        }

        if( (road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i]->signal & 0x04) == 0x04 ){  // Red
            tsDisplayPattern->item(i,2)->setCheckState( Qt::Checked );
        }

        if( (road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i]->signal & 0x08) == 0x08 ){  // Left Arrow
            tsDisplayPattern->item(i,3)->setCheckState( Qt::Checked );
        }

        if( (road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i]->signal & 0x10) == 0x10 ){  // Straight Arrow
            tsDisplayPattern->item(i,4)->setCheckState( Qt::Checked );
        }

        if( (road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i]->signal & 0x20) == 0x20 ){  // Right Arrow
            tsDisplayPattern->item(i,5)->setCheckState( Qt::Checked );
        }

        tsDisplayPattern->item(i,6)->setText( QString("%1").arg( road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i]->duration ) );
    }
}


void RoadObjectProperty::SetDefaultTSPattern()
{
    AdjustRowCountTSPatternTable(3);

    for(int i=0;i<3;++i){

        tsDisplayPattern->item(i,0)->setCheckState( i == 0 ? Qt::Checked : Qt::Unchecked );
        tsDisplayPattern->item(i,1)->setCheckState( i == 1 ? Qt::Checked : Qt::Unchecked );
        tsDisplayPattern->item(i,2)->setCheckState( i == 2 ? Qt::Checked : Qt::Unchecked );
        tsDisplayPattern->item(i,3)->setCheckState( Qt::Unchecked );
        tsDisplayPattern->item(i,4)->setCheckState( Qt::Unchecked );
        tsDisplayPattern->item(i,5)->setCheckState( Qt::Unchecked );

        int dur = 55;
        if(i == 1 ){
            dur = 3;
        }
        else if(i == 2){
            dur = 62;
        }
        tsDisplayPattern->item(i,6)->setText( QString("%1").arg(dur) );
    }
}

void RoadObjectProperty::TSPatternAddRowClicked()
{
    int selectedRow = tsDisplayPattern->currentRow();
    if( selectedRow < 0 ){
        selectedRow = tsDisplayPattern->rowCount(); // If no row is selected, add row to last position
    }

    tsDisplayPattern->insertRow( selectedRow );
    for(int j=0;j<7;++j){
        QTableWidgetItem *item = new QTableWidgetItem();
        tsDisplayPattern->setItem(selectedRow,j,item);
        if( j < 6 ){
            tsDisplayPattern->item(selectedRow,j)->setCheckState( Qt::Unchecked );
        }
    }
}


void RoadObjectProperty::TSPatternDelRowClicked()
{
    int selectedRow = tsDisplayPattern->currentRow();
    if( selectedRow < 0 ){
        selectedRow = tsDisplayPattern->rowCount() - 1;
    }

    tsDisplayPattern->removeRow( selectedRow );
    tsDisplayPattern->setCurrentIndex( QModelIndex() );
}


void RoadObjectProperty::TSPatternApplyClicked()
{
    int id = tsIDSB->value();
    int nodeID = road->indexOfTS( id, -1 );
    if( nodeID < 0){
        return;
    }
    int ndIdx  = road->indexOfNode( nodeID );
    int tsIdx  = road->indexOfTS( id, nodeID );
    if( tsIdx < 0 ){
        return;
    }

    if( tsIsSensorType->isChecked() == true ){
        road->nodes[ndIdx]->trafficSignals[tsIdx]->isSensorType = true;
        road->nodes[ndIdx]->trafficSignals[tsIdx]->timeToChangeBySensor = tsChangeTimeBySensor->value();
    }
    else{
        road->nodes[ndIdx]->trafficSignals[tsIdx]->isSensorType = false;
        road->nodes[ndIdx]->trafficSignals[tsIdx]->timeToChangeBySensor = 0;
    }

    for(int i=0;i<road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.size();++i){
        delete road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i];
    }
    road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.clear();

    for(int i=0;i<tsDisplayPattern->rowCount();++i){

        struct SignalPatternData *sp = new struct SignalPatternData;

        int sigVal = 0;
        if( tsDisplayPattern->item(i,0)->checkState() == Qt::Checked ){
            sigVal += 1;
        }
        if( tsDisplayPattern->item(i,1)->checkState() == Qt::Checked ){
            sigVal += 2;
        }
        if( tsDisplayPattern->item(i,2)->checkState() == Qt::Checked ){
            sigVal += 4;
        }
        if( tsDisplayPattern->item(i,3)->checkState() == Qt::Checked ){
            sigVal += 8;
        }
        if( tsDisplayPattern->item(i,4)->checkState() == Qt::Checked ){
            sigVal += 16;
        }
        if( tsDisplayPattern->item(i,5)->checkState() == Qt::Checked ){
            sigVal += 32;
        }

        sp->signal = sigVal;
        sp->duration = tsDisplayPattern->item(i,6)->text().toInt();

        road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.append( sp );
    }

    road->nodes[ndIdx]->trafficSignals[tsIdx]->startOffset = tsStartOffset->value();

    ChangeTrafficSignalInfo( road->nodes[ndIdx]->trafficSignals[tsIdx]->id );
}


void RoadObjectProperty::TSPatternCopyClicked()
{
    int id = tsIDSB->value();
    int nodeID = road->indexOfTS( id, -1 );
    if( nodeID < 0){
        return;
    }
    int ndIdx  = road->indexOfNode( nodeID );
    int tsIdx  = road->indexOfTS( id, nodeID );
    if( tsIdx < 0 ){
        return;
    }

    int copyFromTSID = QInputDialog::getInt( NULL, "Copy From Other TS", "TS ID");
    int cftsNodeID = road->indexOfTS( copyFromTSID, -1 );
    if( cftsNodeID < 0){
        return;
    }
    int cfndIdx  = road->indexOfNode( cftsNodeID );
    int cftsIdx = road->indexOfTS( copyFromTSID, cftsNodeID );
    if( cftsIdx < 0 ){
        return;
    }

    road->nodes[ndIdx]->trafficSignals[tsIdx]->startOffset = road->nodes[cfndIdx]->trafficSignals[cftsIdx]->startOffset;
    road->nodes[ndIdx]->trafficSignals[tsIdx]->isSensorType = road->nodes[cfndIdx]->trafficSignals[cftsIdx]->isSensorType;
    road->nodes[ndIdx]->trafficSignals[tsIdx]->timeToChangeBySensor = road->nodes[cfndIdx]->trafficSignals[cftsIdx]->timeToChangeBySensor;

    for(int i=0;i<road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.size();++i){
        delete road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern[i];
    }
    road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.clear();

    for(int i=0;i<road->nodes[cfndIdx]->trafficSignals[cftsIdx]->sigPattern.size();++i){

        struct SignalPatternData* spd = new struct SignalPatternData;

        spd->signal = road->nodes[cfndIdx]->trafficSignals[cftsIdx]->sigPattern[i]->signal;
        spd->duration = road->nodes[cfndIdx]->trafficSignals[cftsIdx]->sigPattern[i]->duration;

        road->nodes[ndIdx]->trafficSignals[tsIdx]->sigPattern.append( spd );
    }

    tsStartOffset->setValue( road->nodes[ndIdx]->trafficSignals[tsIdx]->startOffset );
    tsIsSensorType->setChecked( road->nodes[ndIdx]->trafficSignals[tsIdx]->isSensorType );
    tsChangeTimeBySensor->setValue( road->nodes[ndIdx]->trafficSignals[tsIdx]->timeToChangeBySensor );

    SetTSPattern( ndIdx, tsIdx );

    ChangeTrafficSignalInfo( road->nodes[ndIdx]->trafficSignals[tsIdx]->id );
}


