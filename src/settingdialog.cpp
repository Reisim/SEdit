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

    QStringList trafficDirectionsStr;
    trafficDirectionsStr << "Left-Hand" << "Right-Hand";
    LeftRight->addItems( trafficDirectionsStr );
    connect( LeftRight, SIGNAL(currentIndexChanged(int)), this, SLOT(cbLeftRightChanged(int)) );

    QGridLayout *tableLayout = new QGridLayout();

    int row = 0;

    tableLayout->addWidget( new QLabel("Traffic Direction"), row, 0 );
    tableLayout->addWidget( LeftRight, row, 1 );
    row++;

    vehicleKindTable = new QTableWidget();

    vehicleKindTable->setColumnCount(5);

    QStringList tableLabels;
    tableLabels << "Category";
    tableLabels << "Subcategory";
    tableLabels << "Length";
    tableLabels << "Width";
    tableLabels << "Height";

    vehicleKindTable->setColumnWidth(0,200);
    vehicleKindTable->setColumnWidth(1,200);
    vehicleKindTable->setColumnWidth(2,80);
    vehicleKindTable->setColumnWidth(3,80);
    vehicleKindTable->setColumnWidth(4,80);

    vehicleKindTable->setHorizontalHeaderLabels( tableLabels );

    vehicleKindTable->setMinimumWidth( 650 );

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

    pedestrianKindTable->setColumnCount(5);

    pedestrianKindTable->setColumnWidth(0,200);
    pedestrianKindTable->setColumnWidth(1,200);
    pedestrianKindTable->setColumnWidth(2,80);
    pedestrianKindTable->setColumnWidth(3,80);
    pedestrianKindTable->setColumnWidth(4,80);

    pedestrianKindTable->setHorizontalHeaderLabels( tableLabels );

    pedestrianKindTable->setMinimumWidth( 650 );


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
        else if( tagStr == QString("Vehicle Kind") ){

            QStringList divVal = QString(divLine[1]).trimmed().split(",");
            if( divVal.size() == 5 ){

                int nRow = vehicleKindTable->rowCount();
                vehicleKindTable->insertRow( nRow );
                for(int i=0;i<5;++i){
                    QTableWidgetItem *item = new QTableWidgetItem();
                    item->setText( QString(divVal[i]).trimmed() );
                    vehicleKindTable->setItem( nRow, i, item);
                }
            }

        }
        else if( tagStr == QString("Pedestrian Kind") ){

            QStringList divVal = QString(divLine[1]).trimmed().split(",");
            if( divVal.size() == 5 ){

                int nRow = pedestrianKindTable->rowCount();
                pedestrianKindTable->insertRow( nRow );
                for(int i=0;i<5;++i){
                    QTableWidgetItem *item = new QTableWidgetItem();
                    item->setText( QString(divVal[i]).trimmed() );
                    pedestrianKindTable->setItem( nRow, i, item);
                }
            }
        }

    }

    if( vehicleKindTable->rowCount() == 0 ){
        SetDefaultVehicleKind();
    }

    if( pedestrianKindTable->rowCount() == 0 ){
        SetDefaultPedestrianKind();
    }

    file.close();
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

    for(int i=0;i<vehicleKindTable->rowCount();++i){
        out << "Vehicle Kind ; ";
        for(int j=0;j<5;++j){
            out << vehicleKindTable->item(i,j)->text();
            if( j < 4 ){
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
        for(int j=0;j<5;++j){
            out << pedestrianKindTable->item(i,j)->text();
            if( j < 4 ){
                out << " , ";
            }
            else{
                out << "\n";
            }
        }
    }

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

        return vehicleKindTable->item(row,col)->text();
    }
    else{
        return QString();
    }
}

QString SettingDialog::GetPedestKindTableStr(int row,int col)
{
    if( row >= 0 && row < pedestrianKindTable->rowCount() &&
            col >= 0 && col < pedestrianKindTable->columnCount() ){

        return pedestrianKindTable->item(row,col)->text();
    }
    else{
        return QString();
    }
}


QStringList SettingDialog::GetVehicleKindSubcategory()
{
    QStringList subcategory;

    for(int i=0;i<vehicleKindTable->rowCount();++i){
        subcategory << vehicleKindTable->item(i,1)->text();
    }

    return subcategory;
}


void SettingDialog::SetDefaultVehicleKind()
{
    vehicleKindTable->insertRow(0);
    for(int i=0;i<5;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Private Casr"); break;
        case 1: item->setText("Sedan 3Number"); break;
        case 2: item->setText("5.210"); break;
        case 3: item->setText("1.875"); break;
        case 4: item->setText("1.475"); break;
        }
        vehicleKindTable->setItem( 0, i, item);
    }

    vehicleKindTable->insertRow(1);
    for(int i=0;i<5;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Private Cars"); break;
        case 1: item->setText("Sedan 5Number"); break;
        case 2: item->setText("3.955"); break;
        case 3: item->setText("1.695"); break;
        case 4: item->setText("1.500"); break;
        }
        vehicleKindTable->setItem( 1, i, item);
    }

    vehicleKindTable->insertRow(2);
    for(int i=0;i<5;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Private Cars"); break;
        case 1: item->setText("Small Car"); break;
        case 2: item->setText("3.395"); break;
        case 3: item->setText("1.475"); break;
        case 4: item->setText("1.650"); break;
        }
        vehicleKindTable->setItem( 2, i, item);
    }

    vehicleKindTable->insertRow(3);
    for(int i=0;i<5;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Private Cars"); break;
        case 1: item->setText("Bike"); break;
        case 2: item->setText("2.250"); break;
        case 3: item->setText("0.755"); break;
        case 4: item->setText("1.220"); break;
        }
        vehicleKindTable->setItem( 3, i, item);
    }

    vehicleKindTable->insertRow(4);
    for(int i=0;i<5;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Commercial Cars"); break;
        case 1: item->setText("Bus"); break;
        case 2: item->setText("8.990"); break;
        case 3: item->setText("2.300"); break;
        case 4: item->setText("3.045"); break;
        }
        vehicleKindTable->setItem( 4, i, item);
    }

    vehicleKindTable->insertRow(5);
    for(int i=0;i<5;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Commercial Cars"); break;
        case 1: item->setText("Truck"); break;
        case 2: item->setText("6.360"); break;
        case 3: item->setText("2.200"); break;
        case 4: item->setText("3.180"); break;
        }
        vehicleKindTable->setItem( 5, i, item);
    }

}


void SettingDialog::SetDefaultPedestrianKind()
{
    pedestrianKindTable->insertRow(0);
    for(int i=0;i<5;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Pedestrian"); break;
        case 1: item->setText("Adult"); break;
        case 2: item->setText("0.30"); break;
        case 3: item->setText("0.45"); break;
        case 4: item->setText("1.74"); break;
        }
        pedestrianKindTable->setItem( 0, i, item);
    }

    pedestrianKindTable->insertRow(1);
    for(int i=0;i<5;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Pedestrian"); break;
        case 1: item->setText("Child"); break;
        case 2: item->setText("0.20"); break;
        case 3: item->setText("0.30"); break;
        case 4: item->setText("1.20"); break;
        }
        pedestrianKindTable->setItem( 1, i, item);
    }

    pedestrianKindTable->insertRow(2);
    for(int i=0;i<5;++i){
        QTableWidgetItem *item = new QTableWidgetItem();
        switch(i){
        case 0: item->setText("Bicycle"); break;
        case 1: item->setText("Citybike"); break;
        case 2: item->setText("1.850"); break;
        case 3: item->setText("0.580"); break;
        case 4: item->setText("1.200"); break;
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
    for(int j=0;j<5;++j){
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
    for(int j=0;j<5;++j){
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



