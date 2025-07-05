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


#include "settingdialog.h"


SettingDialog::SettingDialog(QWidget *parent) : QWidget(parent)
{
    LeftRight = new QComboBox();
    LeftRight->setFixedWidth(200);

    useRelativePath = new QCheckBox();


    QStringList trafficDirectionsStr;
    trafficDirectionsStr << "Left-Hand" << "Right-Hand";
    LeftRight->addItems( trafficDirectionsStr );
    connect( LeftRight, SIGNAL(currentIndexChanged(int)), this, SLOT(cbLeftRightChanged(int)) );

    QGridLayout *tableLayout = new QGridLayout();

    int row = 0;

    tableLayout->addWidget( new QLabel("Traffic Direction"), row, 0 );
    tableLayout->addWidget( LeftRight, row, 1 );
    row++;

    tableLayout->addWidget( new QLabel("Use Relative Path"), row, 0 );
    tableLayout->addWidget( useRelativePath, row, 1 );
    row++;


    saveVehiclePedestrianKindBtn = new QPushButton("Save");
    saveVehiclePedestrianKindBtn->setIcon( QIcon(":/images/save.png") );
    saveVehiclePedestrianKindBtn->setFixedSize( saveVehiclePedestrianKindBtn->sizeHint() );
    connect(saveVehiclePedestrianKindBtn,SIGNAL(clicked()),this,SLOT(SaveVehiclePedestrianSetting()));

    loadVehiclePedestrianKindBtn = new QPushButton("Load");
    loadVehiclePedestrianKindBtn->setIcon( QIcon(":/images/open.png") );
    loadVehiclePedestrianKindBtn->setFixedSize( loadVehiclePedestrianKindBtn->sizeHint() );
    connect(loadVehiclePedestrianKindBtn,SIGNAL(clicked()),this,SLOT(LoadVehiclePedestrianSetting()));


    QHBoxLayout *SLLayout = new QHBoxLayout();
    SLLayout->addWidget( saveVehiclePedestrianKindBtn );
    SLLayout->addSpacing( 50 );
    SLLayout->addWidget( loadVehiclePedestrianKindBtn );
    SLLayout->addStretch( 1 );

    tableLayout->addLayout( SLLayout, row, 1 );
    row++;

    vehicleKindTable = new QTableWidget();

    vehicleKindTable->setColumnCount(12);

    QStringList tableLabels;
    tableLabels << "Category";
    tableLabels << "Subcategory";
    tableLabels << "Length";
    tableLabels << "Width";
    tableLabels << "Height";
    tableLabels << "WheelBase";
    tableLabels << "Rear-Axis from Rear-End";
    tableLabels << "UE4 Model ID";
    tableLabels << "Number Spawn";
    tableLabels << "CG Kind";
    tableLabels << "Eye X";
    tableLabels << "Eye Y";



    vehicleKindTable->setColumnWidth(0,200);
    vehicleKindTable->setColumnWidth(1,200);
    vehicleKindTable->setColumnWidth(2,80);
    vehicleKindTable->setColumnWidth(3,80);
    vehicleKindTable->setColumnWidth(4,80);
    vehicleKindTable->setColumnWidth(5,80);
    vehicleKindTable->setColumnWidth(6,80);
    vehicleKindTable->setColumnWidth(7,140);
    vehicleKindTable->setColumnWidth(8,90);
    vehicleKindTable->setColumnWidth(9,90);
    vehicleKindTable->setColumnWidth(10,90);
    vehicleKindTable->setColumnWidth(11,90);

    vehicleKindTable->setHorizontalHeaderLabels( tableLabels );

    vehicleKindTable->setMinimumWidth( 800 );

    tableLayout->addWidget( new QLabel("Vehicle Kind"), row, 0 );
    tableLayout->addWidget( vehicleKindTable, row, 1 );
    row++;

    addVehicleKindBtn = new QPushButton("Add Row");
    addVehicleKindBtn->setIcon( QIcon(":/images/Add.png") );
    connect(addVehicleKindBtn,SIGNAL(clicked()),this,SLOT(AddRowVehicleKind()));

    delVehicleKindBtn = new QPushButton("Del Row");
    delVehicleKindBtn->setIcon( QIcon(":/images/Remove.png") );
    connect(delVehicleKindBtn,SIGNAL(clicked()),this,SLOT(DelRowVehicleKind()));

    QHBoxLayout* vehicleKindBtnLayout = new QHBoxLayout();
    vehicleKindBtnLayout->addWidget( addVehicleKindBtn );
    vehicleKindBtnLayout->addWidget( delVehicleKindBtn );
    vehicleKindBtnLayout->addStretch(1);

    tableLayout->addLayout( vehicleKindBtnLayout, row, 1 );
    row++;


    pedestrianKindTable = new QTableWidget();

    pedestrianKindTable->setColumnCount(11);

    pedestrianKindTable->setColumnWidth(0,200);
    pedestrianKindTable->setColumnWidth(1,200);
    pedestrianKindTable->setColumnWidth(2,80);
    pedestrianKindTable->setColumnWidth(3,80);
    pedestrianKindTable->setColumnWidth(4,80);
    pedestrianKindTable->setColumnWidth(5,140);
    pedestrianKindTable->setColumnWidth(6,100);
    pedestrianKindTable->setColumnWidth(7,100);
    pedestrianKindTable->setColumnWidth(8,80);
    pedestrianKindTable->setColumnWidth(9,90);
    pedestrianKindTable->setColumnWidth(10,90);

    tableLabels.clear();
    tableLabels << "Category";
    tableLabels << "Subcategory";
    tableLabels << "Length";
    tableLabels << "Width";
    tableLabels << "Height";
    tableLabels << "UE4 Model ID";
    tableLabels << "Move Speed[m/s]";
    tableLabels << "S.D.[m/s]";
    tableLabels << "Age Info";
    tableLabels << "Number Spawn";
    tableLabels << "CG Kind";

    pedestrianKindTable->setHorizontalHeaderLabels( tableLabels );

    pedestrianKindTable->setMinimumWidth( 1130 );


    tableLayout->addWidget( new QLabel("Pedestrian Kind"), row, 0 );
    tableLayout->addWidget( pedestrianKindTable, row, 1 );
    row++;

    addPedestKindBtn = new QPushButton("Add Row");
    addPedestKindBtn->setIcon( QIcon(":/images/Add.png") );
    connect(addPedestKindBtn,SIGNAL(clicked()),this,SLOT(AddRowPedestKind()));

    delPedestKindBtn = new QPushButton("Del Row");
    delPedestKindBtn->setIcon( QIcon(":/images/Remove.png") );
    connect(delPedestKindBtn,SIGNAL(clicked()),this,SLOT(DelRowPedestKind()));

    QHBoxLayout* pedestKindBtnLayout = new QHBoxLayout();
    pedestKindBtnLayout->addWidget( addPedestKindBtn );
    pedestKindBtnLayout->addWidget( delPedestKindBtn );
    pedestKindBtnLayout->addStretch(1);

    tableLayout->addLayout( pedestKindBtnLayout, row, 1 );
    row++;

    numActorForUE4Models = new QSpinBox();
    numActorForUE4Models->setRange(10,100);
    numActorForUE4Models->setValue(10);
    numActorForUE4Models->setFixedWidth(100);

    QLabel *lNumActor = new QLabel("Number of Actors for UE4 Models");
    lNumActor->setFixedWidth(150);
    lNumActor->setWordWrap( true );

    tableLayout->addWidget( lNumActor, row, 0 );
    tableLayout->addWidget( numActorForUE4Models, row, 1 );
    row++;

    maxActorForUE4 = new QSpinBox();
    maxActorForUE4->setRange(1000,10000);
    maxActorForUE4->setValue(1000);
    maxActorForUE4->setFixedWidth(100);

    QLabel *lMaxActor = new QLabel("Max Number of Actors in UE4");
    lMaxActor->setFixedWidth(150);
    lMaxActor->setWordWrap( true );

    tableLayout->addWidget( lMaxActor, row, 0 );
    tableLayout->addWidget( maxActorForUE4, row, 1 );
    row++;

    closeButton = new QPushButton("Close");
    connect(closeButton,SIGNAL(clicked()),this,SLOT(close()));

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget( closeButton ,0, Qt::AlignHCenter );

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout( tableLayout );
    mainLayout->addLayout( buttonLayout );

    setLayout( mainLayout );

    LoadSetting();
}


void SettingDialog::LoadSetting()
{
    qDebug() << "[SettingDialog::LoadSetting]";

    QFile file("SEdit_Setting.txt");
    if( !file.open(QIODevice::ReadOnly | QIODevice::Text) ){
        qDebug() << "[SettingDialog::LoadSetting] failed to open SEdit_Setting.txt";
        return;
    }

    QTextStream in(&file);
    QString line;
    QStringList divLine;

    while( in.atEnd() == false ){

        line = in.readLine();
        if( line.startsWith("#") == true || line.isEmpty() == true ){
            continue;
        }

        divLine = line.split(";");
        QString tagStr = QString(divLine[0]).trimmed();

        if( tagStr == QString("Traffic Direction") ){
            int val = QString(divLine[1]).trimmed().toInt();
            LeftRight->setCurrentIndex( val );
        }
        else if( tagStr == QString("Use Relative Path") ){
            int val = QString(divLine[1]).trimmed().toInt();
            if( val == 1 ){
                useRelativePath->setChecked( true );
            }
            else{
                useRelativePath->setChecked( false );
            }
        }
        else if( tagStr == QString("Vehicle Kind") ){

            QStringList divVal = QString(divLine[1]).trimmed().split(",");
            if( divVal.size() <= vehicleKindTable->columnCount() ){

                int nRow = vehicleKindTable->rowCount();
                vehicleKindTable->insertRow( nRow );
                float length = 0.0;
                if( divVal.size() == 8 ){
                    int atcol = 0;
                    for(int i=0;i<divVal.size();++i){
                        QTableWidgetItem *item = new QTableWidgetItem();
                        item->setText( QString(divVal[i]).trimmed() );
                        vehicleKindTable->setItem( nRow, atcol, item);
                        atcol++;
                        if( atcol == 3 ){
                            length = QString(divVal[i]).trimmed().toFloat();
                        }
                        if( atcol == 5 ){
                            QTableWidgetItem *item1 = new QTableWidgetItem();
                            item1->setText( QString("%1").arg(length * 0.58) );
                            vehicleKindTable->setItem( nRow, atcol, item1);
                            atcol++;

                            QTableWidgetItem *item2 = new QTableWidgetItem();
                            item2->setText( QString("%1").arg(length * 0.20) );
                            vehicleKindTable->setItem( nRow, atcol, item2);
                            atcol++;
                        }
                    }
                    for(int i=10;i<12;++i){
                        QTableWidgetItem *item = new QTableWidgetItem();
                        item->setText( QString("0.0") );
                        vehicleKindTable->setItem( nRow, i, item);
                    }
                }
                else if( divVal.size() == 10 ){
                    for(int i=0;i<divVal.size();++i){
                        QTableWidgetItem *item = new QTableWidgetItem();
                        item->setText( QString(divVal[i]).trimmed() );
                        vehicleKindTable->setItem( nRow, i, item);
                    }
                    for(int i=10;i<12;++i){
                        QTableWidgetItem *item = new QTableWidgetItem();
                        item->setText( QString("0.0") );
                        vehicleKindTable->setItem( nRow, i, item);
                    }
                }
                else if( divVal.size() == 12 ){
                    for(int i=0;i<divVal.size();++i){
                        QTableWidgetItem *item = new QTableWidgetItem();
                        item->setText( QString(divVal[i]).trimmed() );
                        vehicleKindTable->setItem( nRow, i, item);

                        qDebug() << "vehicle data: [" << nRow << "][" << i << "] = " << item->text();
                    }
                }

            }

        }
        else if( tagStr == QString("Pedestrian Kind") ){

            QStringList divVal = QString(divLine[1]).trimmed().split(",");
            if( divVal.size() <= pedestrianKindTable->columnCount() ){

                int nRow = pedestrianKindTable->rowCount();
                pedestrianKindTable->insertRow( nRow );
                for(int i=0;i<divVal.size();++i){
                    QTableWidgetItem *item = new QTableWidgetItem();
                    item->setText( QString(divVal[i]).trimmed() );
                    pedestrianKindTable->setItem( nRow, i, item);
                }
            }
        }
        else if( tagStr == QString("Number of Actor for UE4 Model") ){
            int val = QString(divLine[1]).trimmed().toInt();
            numActorForUE4Models->setValue( val );
        }
        else if( tagStr == QString("Max Number of Actors of UE4") ){
            int val = QString(divLine[1]).trimmed().toInt();
            maxActorForUE4->setValue( val );
        }

    }

    if( vehicleKindTable->rowCount() == 0 ){
        SetDefaultVehicleKind();
    }

    if( pedestrianKindTable->rowCount() == 0 ){
        SetDefaultPedestrianKind();
    }

    file.close();

    qDebug() << "done.";
}


void SettingDialog::SaveSetting()
{
    QFile file("SEdit_Setting.txt");
    if( !file.open(QIODevice::WriteOnly | QIODevice::Text) ){
        qDebug() << "[SettingDialog::SaveSetting] failed to open SEdit_Setting.txt";
        return;
    }

    QTextStream out(&file);

    out << "# S-Edit Setting File\n";
    out << "\n";
    out << "Traffic Direction ; " << LeftRight->currentIndex() << "\n";
    out << "\n";
    out << "Use Relative Path ; " << (useRelativePath->isChecked() == true ? 1 : 0) << "\n";
    out << "\n";

    for(int i=0;i<vehicleKindTable->rowCount();++i){
        out << "Vehicle Kind ; ";
        for(int j=0;j<vehicleKindTable->columnCount();++j){
            if( vehicleKindTable->item(i,j) != NULL ){
                out << vehicleKindTable->item(i,j)->text();
            }
            if( j < vehicleKindTable->columnCount() - 1 ){
                out << " , ";
            }
            else{
                out << "\n";
            }
        }
    }

    out << "\n";

    for(int i=0;i<pedestrianKindTable->rowCount();++i){
        out << "Pedestrian Kind ; ";
        for(int j=0;j<pedestrianKindTable->columnCount();++j){
            if( pedestrianKindTable->item(i,j) != NULL ){
                out << pedestrianKindTable->item(i,j)->text();
            }
            if( j < pedestrianKindTable->columnCount() - 1 ){
                out << " , ";
            }
            else{
                out << "\n";
            }
        }
    }

    out << "\n";

    out << "Number of Actor for UE4 Model  ; " << numActorForUE4Models->value() << "\n";
    out << "\n";

    out << "Max Number of Actors of UE4  ; " << maxActorForUE4->value() << "\n";
    out << "\n";

    file.close();
}


void SettingDialog::closeEvent(QCloseEvent *event)
{
    qDebug() << "[SettingDialog] closeEvent";
    SaveSetting();
}


void SettingDialog::cbLeftRightChanged(int index)
{
    emit TrafficDirectionSettingChanged(index);
}


int SettingDialog::GetCurrentLeftRightIndex()
{
    if( LeftRight ){
        return LeftRight->currentIndex();
    }
    else{
        return -1;
    }
}


int SettingDialog::GetVehicleKindNum()
{
    return vehicleKindTable->rowCount();
}


int SettingDialog::GetPedestrianKindNum()
{
    return pedestrianKindTable->rowCount();
}

QString SettingDialog::GetVehicleKindTableStr(int row,int col)
{
    if( row >= 0 && row < vehicleKindTable->rowCount() &&
            col >= 0 && col < vehicleKindTable->columnCount() ){

        if( vehicleKindTable->item(row,col) ){
            return vehicleKindTable->item(row,col)->text();
        }
        else{
            qDebug() << "[GetVehicleKindTableStr] row = " << row << " col = " << col << " item is empty";
            return QString();
        }
    }
    else{
        qDebug() << "[GetVehicleKindTableStr] row = " << row << " col = " << col
                 << " invalid: rowCount = " << vehicleKindTable->rowCount()
                 << "  columnCount = " << vehicleKindTable->columnCount();
        return QString();
    }
}

QString SettingDialog::GetPedestKindTableStr(int row,int col)
{
    if( row >= 0 && row < pedestrianKindTable->rowCount() &&
            col >= 0 && col < pedestrianKindTable->columnCount() ){
        if( pedestrianKindTable->item(row,col) ){
            return pedestrianKindTable->item(row,col)->text();
        }
        else{
            return QString();
        }
    }
    else{
        return QString();
    }
}


QStringList SettingDialog::GetVehicleKindCategory()
{
    QStringList category;

    for(int i=0;i<vehicleKindTable->rowCount();++i){
        category << vehicleKindTable->item(i,0)->text();
    }

    return category;
}

QStringList SettingDialog::GetPedestianKindCategory()
{
    QStringList category;

    for(int i=0;i<pedestrianKindTable->rowCount();++i){
        category << pedestrianKindTable->item(i,0)->text();
    }

    return category;
}


QStringList SettingDialog::GetVehicleKindSubcategory()
{
    QStringList subcategory;

    for(int i=0;i<vehicleKindTable->rowCount();++i){
        subcategory << vehicleKindTable->item(i,1)->text();
    }

    return subcategory;
}

QStringList SettingDialog::GetPedestianKindSubcategory()
{
    QStringList subcategory;

    for(int i=0;i<pedestrianKindTable->rowCount();++i){
        subcategory << pedestrianKindTable->item(i,1)->text();
    }

    return subcategory;
}

void SettingDialog::SetVehicleKindByStringList(QStringList vehicleKindStrs)
{
    vehicleKindTable->clearContents();

    int nRow = vehicleKindTable->rowCount();
    for(int i=nRow-1;i>=0;i--){
        vehicleKindTable->removeRow(i);
    }

    qDebug() << "[SetVehicleKindByStringList]";

    for(int i=0;i<vehicleKindStrs.size();++i){

        qDebug() << i << ": " << vehicleKindStrs[i];

        nRow = vehicleKindTable->rowCount();
        vehicleKindTable->insertRow( nRow );

        QStringList vData = QString(vehicleKindStrs[i]).split(",");
        for(int j=0;j<vehicleKindTable->columnCount();++j){
            QTableWidgetItem *item = new QTableWidgetItem();
            if( j < vData.size() ){
                item->setText( QString(vData[j] ).trimmed() );
            }
            vehicleKindTable->setItem( nRow, j, item);
        }

        if( vehicleKindTable->columnCount() < 12 ){
            for(int j=vehicleKindTable->columnCount();j<12;++j){
                QTableWidgetItem *item = new QTableWidgetItem();
                item->setText( QString("0.0") );
                vehicleKindTable->setItem( nRow, j, item);
            }
        }
    }
}

void SettingDialog::SetPedestrianKindByStringList(QStringList pedestrianKindStrs)
{
    pedestrianKindTable->clearContents();

    int nRow = pedestrianKindTable->rowCount();
    for(int i=nRow-1;i>=0;i--){
        pedestrianKindTable->removeRow(i);
    }

    for(int i=0;i<pedestrianKindStrs.size();++i){
        nRow = pedestrianKindTable->rowCount();
        pedestrianKindTable->insertRow( nRow );

        QStringList vData = QString(pedestrianKindStrs[i]).split(",");
        for(int j=0;j<pedestrianKindTable->columnCount();++j){
            QTableWidgetItem *item = new QTableWidgetItem();
            if( j < vData.size() ){
                item->setText( QString(vData[j] ).trimmed() );
            }
            pedestrianKindTable->setItem( nRow, j, item);
        }
    }
}


void SettingDialog::SetDefaultVehicleKind()
{
    vehicleKindTable->insertRow(0);
    for(int i=0;i<6;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Private Casr"); break;
        case 1: item->setText("Sedan 3Number"); break;
        case 2: item->setText("5.210"); break;
        case 3: item->setText("1.875"); break;
        case 4: item->setText("1.475"); break;
        case 5: item->setText("10"); break;
        }
        vehicleKindTable->setItem( 0, i, item);
    }

    vehicleKindTable->insertRow(1);
    for(int i=0;i<6;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Private Cars"); break;
        case 1: item->setText("Sedan 5Number"); break;
        case 2: item->setText("3.955"); break;
        case 3: item->setText("1.695"); break;
        case 4: item->setText("1.500"); break;
        case 5: item->setText("20"); break;
        }
        vehicleKindTable->setItem( 1, i, item);
    }

    vehicleKindTable->insertRow(2);
    for(int i=0;i<6;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Private Cars"); break;
        case 1: item->setText("Small Car"); break;
        case 2: item->setText("3.395"); break;
        case 3: item->setText("1.475"); break;
        case 4: item->setText("1.650"); break;
        case 6: item->setText("30"); break;
        }
        vehicleKindTable->setItem( 2, i, item);
    }

    vehicleKindTable->insertRow(3);
    for(int i=0;i<6;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Private Cars"); break;
        case 1: item->setText("Bike"); break;
        case 2: item->setText("2.250"); break;
        case 3: item->setText("0.755"); break;
        case 4: item->setText("1.220"); break;
        case 5: item->setText("40"); break;
        }
        vehicleKindTable->setItem( 3, i, item);
    }

    vehicleKindTable->insertRow(4);
    for(int i=0;i<6;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Commercial Cars"); break;
        case 1: item->setText("Bus"); break;
        case 2: item->setText("8.990"); break;
        case 3: item->setText("2.300"); break;
        case 4: item->setText("3.045"); break;
        case 5: item->setText("50"); break;
        }
        vehicleKindTable->setItem( 4, i, item);
    }

    vehicleKindTable->insertRow(5);
    for(int i=0;i<6;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Commercial Cars"); break;
        case 1: item->setText("Truck"); break;
        case 2: item->setText("6.360"); break;
        case 3: item->setText("2.200"); break;
        case 4: item->setText("3.180"); break;
        case 5: item->setText("60"); break;
        }
        vehicleKindTable->setItem( 5, i, item);
    }

}


void SettingDialog::SetDefaultPedestrianKind()
{
    pedestrianKindTable->insertRow(0);
    for(int i=0;i<9;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Pedestrian"); break;
        case 1: item->setText("Adult(MAN)"); break;
        case 2: item->setText("0.30"); break;
        case 3: item->setText("0.45"); break;
        case 4: item->setText("1.74"); break;
        case 5: item->setText("500"); break;
        case 6: item->setText("1.339"); break;
        case 7: item->setText("0.107"); break;
        case 8: item->setText("1"); break;
        }
        pedestrianKindTable->setItem( 0, i, item);
    }

    pedestrianKindTable->insertRow(1);
    for(int i=0;i<9;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Pedestrian"); break;
        case 1: item->setText("Adult(WOMAN)"); break;
        case 2: item->setText("0.20"); break;
        case 3: item->setText("0.30"); break;
        case 4: item->setText("1.20"); break;
        case 5: item->setText("550"); break;
        case 6: item->setText("1.339"); break;
        case 7: item->setText("0.107"); break;
        case 8: item->setText("1"); break;
        }
        pedestrianKindTable->setItem( 1, i, item);
    }

    pedestrianKindTable->insertRow(2);
    for(int i=0;i<9;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Bicycle"); break;
        case 1: item->setText("Citybike"); break;
        case 2: item->setText("1.850"); break;
        case 3: item->setText("0.580"); break;
        case 4: item->setText("1.200"); break;
        case 5: item->setText("580"); break;
        case 6: item->setText("5.62"); break;
        case 7: item->setText("0.861"); break;
        case 8: item->setText("1"); break;
        }
        pedestrianKindTable->setItem( 2, i, item);
    }

}


void SettingDialog::AddRowVehicleKind()
{
    int selectedRow = vehicleKindTable->currentRow();
    if( selectedRow < 0 ){
        selectedRow = vehicleKindTable->rowCount(); // If no row is selected, add row to last position
    }

    vehicleKindTable->insertRow( selectedRow );
    for(int j=0;j<vehicleKindTable->columnCount();++j){
        QTableWidgetItem *item = new QTableWidgetItem();
        vehicleKindTable->setItem(selectedRow,j,item);
    }
}


void SettingDialog::DelRowVehicleKind()
{
    int selectedRow = vehicleKindTable->currentRow();
    if( selectedRow < 0 ){
        selectedRow = vehicleKindTable->rowCount() - 1;
    }

    vehicleKindTable->removeRow( selectedRow );
    vehicleKindTable->setCurrentIndex( QModelIndex() );
}


void SettingDialog::AddRowPedestKind()
{
    int selectedRow = pedestrianKindTable->currentRow();
    if( selectedRow < 0 ){
        selectedRow = pedestrianKindTable->rowCount(); // If no row is selected, add row to last position
    }

    pedestrianKindTable->insertRow( selectedRow );
    for(int j=0;j<pedestrianKindTable->columnCount();++j){
        QTableWidgetItem *item = new QTableWidgetItem();
        pedestrianKindTable->setItem(selectedRow,j,item);
    }
}


void SettingDialog::DelRowPedestKind()
{
    int selectedRow = pedestrianKindTable->currentRow();
    if( selectedRow < 0 ){
        selectedRow = pedestrianKindTable->rowCount() - 1;
    }

    pedestrianKindTable->removeRow( selectedRow );
    pedestrianKindTable->setCurrentIndex( QModelIndex() );
}


void SettingDialog::SaveVehiclePedestrianSetting()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Vehicle and Pedestrian Category Data"),
                                                    ".",
                                                    tr("Vehicle and Pedestrian Category file(*.VPcat.txt)"));

    if( fileName.isNull() == false ){

        if( fileName.endsWith(".VPcat.txt") == false ){
            fileName += QString(".VPcat.txt");
        }

        qDebug() << "filename = " << fileName;

    }
    else{
        qDebug() << "SaveVehiclePedestrianSetting canceled.";
        return;
    }


    QFile file(fileName);
    if( !file.open(QIODevice::WriteOnly | QIODevice::Text) ){
        qDebug() << "[SettingDialog::SaveVehiclePedestrianSetting] failed to open file :" << fileName;
        return;
    }


    QTextStream out(&file);

    out << "# Vehicle and Pedestrian Category file\n";
    out << "\n";

    for(int i=0;i<vehicleKindTable->rowCount();++i){
        out << "Vehicle Kind ; ";
        for(int j=0;j<vehicleKindTable->columnCount();++j){
            out << vehicleKindTable->item(i,j)->text();
            if( j < vehicleKindTable->columnCount() - 1 ){
                out << " , ";
            }
            else{
                out << "\n";
            }
        }
    }

    out << "\n";

    for(int i=0;i<pedestrianKindTable->rowCount();++i){
        out << "Pedestrian Kind ; ";
        for(int j=0;j<pedestrianKindTable->columnCount();++j){
            out << pedestrianKindTable->item(i,j)->text();
            if( j < pedestrianKindTable->columnCount() - 1 ){
                out << " , ";
            }
            else{
                out << "\n";
            }
        }
    }

    out << "\n";

    out << "Number of Actor for UE4 Model  ; " << numActorForUE4Models->value() << "\n";
    out << "\n";

    out << "Max Number of Actors of UE4  ; " << maxActorForUE4->value() << "\n";
    out << "\n";

    file.close();
}


void SettingDialog::LoadVehiclePedestrianSetting()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                            tr("Choose Vehicle and Pedestrian Category File"),
                                            ".",
                                            tr("Vehicle and Pedestrian Category file(*.VPcat.txt)"));

    if( fileName.isNull() == false ){
        qDebug() << "filename = " << fileName;
    }
    else{
        qDebug() << "LoadVehiclePedestrianSetting canceled.";
        return;
    }


    QFile file(fileName);
    if( !file.open(QIODevice::ReadOnly | QIODevice::Text) ){
        qDebug() << "[LoadVehiclePedestrianSetting::LoadSetting] failed to open file : " << fileName;
        return;
    }

    QTextStream in(&file);
    QString line;
    QStringList divLine;

    QList<QStringList> vData;
    QList<QStringList> pData;

    while( in.atEnd() == false ){

        line = in.readLine();
        if( line.startsWith("#") == true || line.isEmpty() == true ){
            continue;
        }

        divLine = line.split(";");
        QString tagStr = QString(divLine[0]).trimmed();

        if( tagStr == QString("Vehicle Kind") ){

            QStringList divVal = QString(divLine[1]).trimmed().split(",");
            vData.append( divVal );
        }
        else if( tagStr == QString("Pedestrian Kind") ){

            QStringList divVal = QString(divLine[1]).trimmed().split(",");
            pData.append( divVal );
        }
        else if( tagStr == QString("Number of Actor for UE4 Model") ){
            int val = QString(divLine[1]).trimmed().toInt();
            numActorForUE4Models->setValue( val );
        }
        else if( tagStr == QString("Max Number of Actors of UE4") ){
            int val = QString(divLine[1]).trimmed().toInt();
            maxActorForUE4->setValue( val );
        }
    }

    if( vData.size() > 0 ){

        vehicleKindTable->clearContents();

        int nRow = vehicleKindTable->rowCount();
        for(int i=nRow-1;i>=0;i--){
            vehicleKindTable->removeRow(i);
        }

        for(int i=0;i<vData.size();++i){
            nRow = vehicleKindTable->rowCount();
            vehicleKindTable->insertRow( nRow );
            for(int j=0;j<vehicleKindTable->columnCount();++j){
                QTableWidgetItem *item = new QTableWidgetItem();
                if( j < vData[i].size() ){
                    item->setText( QString(vData[i][j] ).trimmed() );
                }
                vehicleKindTable->setItem( nRow, j, item);
            }
        }
    }

    if( pData.size() > 0 ){

        pedestrianKindTable->clearContents();

        int nRow = pedestrianKindTable->rowCount();
        for(int i=nRow-1;i>=0;i--){
            pedestrianKindTable->removeRow(i);
        }

        for(int i=0;i<pData.size();++i){
            nRow = pedestrianKindTable->rowCount();
            pedestrianKindTable->insertRow( nRow );
            for(int j=0;j<pedestrianKindTable->columnCount();++j){
                QTableWidgetItem *item = new QTableWidgetItem();
                if( j < pData[i].size() ){
                    item->setText( QString(pData[i][j] ).trimmed() );
                }
                pedestrianKindTable->setItem( nRow, j, item);
            }
        }
    }
}


