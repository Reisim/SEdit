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


#include "basemapimagemanager.h"

#include <QDialog>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QApplication>
#include <QProgressDialog>


BaseMapImageManager::BaseMapImageManager(QWidget *parent)
{
    mapImageList = new QTableWidget();
    mapImageList->setColumnCount(6);

    QStringList tableLabels;
    tableLabels << "File Path";
    tableLabels << "File Name";
    tableLabels << "X";
    tableLabels << "Y";
    tableLabels << "Scale";
    tableLabels << "Ratate";

    mapImageList->setHorizontalHeaderLabels( tableLabels );
    mapImageList->setColumnWidth(0,500);
    mapImageList->setColumnWidth(1,200);
    mapImageList->setColumnWidth(2,120);
    mapImageList->setColumnWidth(3,120);
    mapImageList->setColumnWidth(4,120);
    mapImageList->setColumnWidth(5,120);

    mapImageList->setMinimumWidth(1180);


    //------

    QHBoxLayout *upButtonLayout = new QHBoxLayout();

    QPushButton *addImageBtn = new QPushButton("Add");
    connect(addImageBtn,SIGNAL(clicked()),this,SLOT(AddMapImage()));

    QPushButton *deleteImageBtn = new QPushButton("Delete");
    connect(deleteImageBtn,SIGNAL(clicked()),this,SLOT(DeleteMapImage()));

    QPushButton *editImageBtn = new QPushButton("Edit");
    connect(editImageBtn,SIGNAL(clicked()),this,SLOT(EditMapImageProperty()));

    QPushButton *fromFileBtn = new QPushButton("From File");
    connect(fromFileBtn,SIGNAL(clicked()),this,SLOT(InsertImageFromFile()));

    upButtonLayout->addWidget( addImageBtn );
    upButtonLayout->addWidget( editImageBtn );
    upButtonLayout->addSpacing( 20 );
    upButtonLayout->addWidget( fromFileBtn );
    upButtonLayout->addStretch( 1 );
    upButtonLayout->addWidget( deleteImageBtn );

    //------

    QHBoxLayout *downButtonLayout = new QHBoxLayout();

    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn,SIGNAL(clicked()),this,SLOT(close()));
    downButtonLayout->addWidget(closeBtn ,0, Qt::AlignHCenter );

    //------

    QVBoxLayout *mainLayout = new QVBoxLayout();

    mainLayout->addLayout( upButtonLayout );
    mainLayout->addWidget( mapImageList );
    mainLayout->addLayout( downButtonLayout );

    setLayout( mainLayout );
}


BaseMapImageManager::~BaseMapImageManager()
{
    ClearAll();
}


void BaseMapImageManager::ClearAll()
{
    for(int i=baseMapImages.size()-1;i>=0;i--){
        emit MapImageDeleted( baseMapImages[i] );
        baseMapImages.removeAt( i );
        mapImageList->removeRow( i );
        emit UpdateGraphic();
    }
    baseMapImages.clear();
}


void BaseMapImageManager::AddMapImage()
{
    QString filename = QFileDialog::getOpenFileName(this,"Select Map Image",".",tr("Image file(*.png *jpg *jpeg)"));
    if( filename.isNull() == true || filename.isEmpty() == true ){
        return;
    }

    QStringList strDiv = filename.split("/");
    QString path = QString();
    QString name = QString();
    for(int i=0;i<strDiv.size();++i){
        if( i == strDiv.size() - 1 ){
            name = QString(strDiv[i]).trimmed();
        }
        else{
            path += QString(strDiv[i]) + QString("/");
        }
    }

    struct baseMapImage *map = new struct baseMapImage;

    map->path = path;
    map->filename = name;
    map->x = 0.0;
    map->y = 0.0;
    map->scale = 1.0;
    map->rotate = 0.0;

    baseMapImages.append( map );

    emit MapImageAdded(map);

    AddMapImageToList(map);

    emit UpdateGraphic();
}


void BaseMapImageManager::AddMapImageFromFile(QString &filename, float x, float y, float scale, float rot,bool loadImage)
{
    QStringList strDiv = filename.split("/");
    QString path = QString();
    QString name = QString();
    for(int i=0;i<strDiv.size();++i){
        if( i == strDiv.size() - 1 ){
            name = QString(strDiv[i]).trimmed();
        }
        else{
            path += QString(strDiv[i]) + QString("/");
        }
    }

    struct baseMapImage *map = new struct baseMapImage;

    map->path = path;
    map->filename = name;
    map->x = x;
    map->y = y;
    map->scale = scale;
    map->rotate = rot;

    baseMapImages.append( map );

    if( loadImage == true ){
        emit MapImageAdded(map);
    }

    AddMapImageToList(map);


    if( map->path != path ){

    }

    if( loadImage == true ){
        emit UpdateGraphic();
    }
}


void BaseMapImageManager::AddMapImageToList(struct baseMapImage* map)
{
    int nRow = mapImageList->rowCount();
    mapImageList->insertRow( nRow );

    QTableWidgetItem *item;

    item = new QTableWidgetItem( map->path );
    item->setFlags( Qt::ItemIsEnabled );
    mapImageList->setItem( nRow, 0, item );

    item = new QTableWidgetItem( map->filename );
    item->setFlags( Qt::ItemIsEnabled );
    mapImageList->setItem( nRow, 1, item );

    item = new QTableWidgetItem( QString("%1").arg(map->x,0,'g',6) );
    item->setFlags( Qt::ItemIsEnabled );
    mapImageList->setItem( nRow, 2, item );

    item = new QTableWidgetItem( QString("%1").arg(map->y,0,'g',6) );
    item->setFlags( Qt::ItemIsEnabled );
    mapImageList->setItem( nRow, 3, item );

    item = new QTableWidgetItem( QString("%1").arg(map->scale,0,'g',6) );
    item->setFlags( Qt::ItemIsEnabled );
    mapImageList->setItem( nRow, 4, item );

    item = new QTableWidgetItem( QString("%1").arg(map->rotate,0,'g',6) );
    item->setFlags( Qt::ItemIsEnabled );
    mapImageList->setItem( nRow, 5, item );
}


void BaseMapImageManager::DeleteMapImage()
{
    int selRow = mapImageList->currentRow();
    if( selRow >= 0 && selRow < baseMapImages.size() ){
        emit MapImageDeleted( baseMapImages[selRow] );
        baseMapImages.removeAt( selRow );
        mapImageList->removeRow( selRow );
        emit UpdateGraphic();
    }
}


void BaseMapImageManager::EditMapImageProperty()
{
    int selRow = mapImageList->currentRow();
    if( selRow < 0 || selRow >= baseMapImages.size() ){
        return;
    }

    QDialog *dialog = new QDialog();

    QGridLayout *gLay = new QGridLayout();
    gLay->addWidget( new QLabel("X[m]") , 0, 0 );
    gLay->addWidget( new QLabel("Y[m]") , 1, 0 );
    gLay->addWidget( new QLabel("Scale[-]") , 2, 0 );
    gLay->addWidget( new QLabel("Ratate[deg]") , 3, 0 );

    QDoubleSpinBox *xPos = new QDoubleSpinBox();
    xPos->setFixedWidth(150);
    xPos->setRange(-1.0e8, 1.0e8);
    xPos->setDecimals(4);
    xPos->setValue( baseMapImages[selRow]->x );
    xPos->setObjectName("xPos");

    QDoubleSpinBox *yPos = new QDoubleSpinBox();
    yPos->setFixedWidth(150);
    yPos->setRange(-1.0e8, 1.0e8);
    yPos->setDecimals(4);
    yPos->setValue( baseMapImages[selRow]->y );
    yPos->setObjectName("yPos");

    QDoubleSpinBox *scale = new QDoubleSpinBox();
    scale->setFixedWidth(150);
    scale->setDecimals(4);
    scale->setValue( baseMapImages[selRow]->scale );
    scale->setObjectName("scale");

    QDoubleSpinBox *rot = new QDoubleSpinBox();
    rot->setFixedWidth(150);
    rot->setRange(-180.0, 180.0);
    rot->setDecimals(4);
    rot->setValue( baseMapImages[selRow]->rotate );
    rot->setObjectName("rot");

    gLay->addWidget( xPos  , 0, 1 );
    gLay->addWidget( yPos  , 1, 1 );
    gLay->addWidget( scale , 2, 1 );
    gLay->addWidget( rot   , 3, 1 );

    QPushButton *okBtn = new QPushButton("Accept");
    okBtn->setIcon(QIcon(":/images/accept.png"));

    QPushButton *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setIcon(QIcon(":/images/delete.png"));

    connect( xPos, SIGNAL(valueChanged(double)), this, SLOT(ApplyChange(double)));
    connect( yPos, SIGNAL(valueChanged(double)), this, SLOT(ApplyChange(double)));
    connect( scale, SIGNAL(valueChanged(double)), this, SLOT(ApplyChange(double)));
    connect( rot, SIGNAL(valueChanged(double)), this, SLOT(ApplyChange(double)));

    connect( okBtn, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect( cancelBtn, SIGNAL(clicked()), dialog, SLOT(reject()));
    connect( okBtn, SIGNAL(clicked()), dialog, SLOT(close()));
    connect( cancelBtn, SIGNAL(clicked()), dialog, SLOT(close()));

    QHBoxLayout *btnLay = new QHBoxLayout();
    btnLay->addStretch(1);
    btnLay->addWidget( okBtn );
    btnLay->addSpacing(50);
    btnLay->addWidget( cancelBtn );
    btnLay->addStretch(1);

    QVBoxLayout *mLay = new QVBoxLayout();
    mLay->addLayout( gLay );
    mLay->addLayout( btnLay );

    dialog->setLayout( mLay );
    dialog->exec();

    if( dialog->result() == QDialog::Accepted ){

        baseMapImages[selRow]->x = xPos->value();
        baseMapImages[selRow]->y = yPos->value();
        baseMapImages[selRow]->scale = scale->value();
        baseMapImages[selRow]->rotate = rot->value();

        mapImageList->item( selRow, 2 )->setText( QString("%1").arg(baseMapImages[selRow]->x,0,'g',6) );
        mapImageList->item( selRow, 3 )->setText( QString("%1").arg(baseMapImages[selRow]->y,0,'g',6) );
        mapImageList->item( selRow, 4 )->setText( QString("%1").arg(baseMapImages[selRow]->scale,0,'g',6) );
        mapImageList->item( selRow, 5 )->setText( QString("%1").arg(baseMapImages[selRow]->rotate,0,'g',6) );

        emit UpdateGraphic();
    }
    else{

        baseMapImages[selRow]->x      = mapImageList->item( selRow, 2 )->text().toFloat();
        baseMapImages[selRow]->y      = mapImageList->item( selRow, 3 )->text().toFloat();
        baseMapImages[selRow]->scale  = mapImageList->item( selRow, 4 )->text().toFloat();
        baseMapImages[selRow]->rotate = mapImageList->item( selRow, 5 )->text().toFloat();

        emit UpdateGraphic();
    }
}


void BaseMapImageManager::ApplyChange(double val)
{
    int selRow = mapImageList->currentRow();
    if( selRow < 0 || selRow >= baseMapImages.size() ){
        return;
    }

    if( sender()->objectName() == QString("xPos") ){
        baseMapImages[selRow]->x = val;
    }
    else if( sender()->objectName() == QString("yPos") ){
        baseMapImages[selRow]->y = val;
    }
    else if( sender()->objectName() == QString("scale") ){
        if( val > 0.0 ){
            baseMapImages[selRow]->scale = val;
        }
    }
    else if( sender()->objectName() == QString("rot") ){
        baseMapImages[selRow]->rotate = val;
    }

    emit UpdateGraphic();
}


void BaseMapImageManager::InsertImageFromFile()
{
    QString filename = QFileDialog::getOpenFileName(this,"Select Image-List File",".",tr("Image-List file(*.txt)"));
    if( filename.isNull() == true || filename.isEmpty() == true ){
        return;
    }

    QFile file(filename);
    if( file.open(QIODevice::ReadOnly | QIODevice::Text) == false ){
        return;
    }

    QTextStream in(&file);

    QStringList divFilename = filename.replace("\\","/").split("/");
    QString folderName = filename.remove( divFilename.last() );

    QStringList allLines;

    QString aLine;
    while( in.atEnd() == false ){

        aLine = in.readLine();
        if( aLine.isEmpty() || aLine.contains(",") == false ){
            continue;
        }

        QStringList divLine = aLine.split(",");
        if( divLine.size() < 4 ){
            continue;
        }

        allLines.append( aLine );
    }

    file.close();


    QProgressDialog *pd = new QProgressDialog("InsertImageFromFile", "Cancel", 0, allLines.size(), 0);
    pd->setWindowModality(Qt::WindowModal);
    pd->setAttribute( Qt::WA_DeleteOnClose );
    pd->setWindowIcon(QIcon(":images/SEdit-icon.png"));
    pd->show();

    pd->setValue(0);
    QApplication::processEvents();


    for(int i=0;i<allLines.size();++i){

        QStringList divLine = QString( allLines[i] ).split(",");

        QString imageFilename = folderName + QString( divLine[0] ).trimmed();
        float x = QString( divLine[1] ).trimmed().toFloat();
        float y = QString( divLine[2] ).trimmed().toFloat();
        float s = QString( divLine[3] ).trimmed().toFloat();

        float rot = 0.0;
        if( divLine.size() == 5 ){
            rot = QString( divLine[4] ).trimmed().toFloat();
        }

        AddMapImageFromFile(imageFilename,x,y,s,0.0);

        pd->setValue(i+1);
        QApplication::processEvents();

        if( pd->wasCanceled() ){
            qDebug() << "Canceled.";
            break;
        }
    }

    pd->close();
}


void BaseMapImageManager::AllChangeScale()
{
    double scale = QInputDialog::getDouble(this,"Change Scaling of All Images","Scaling Value");
    if( scale > 0.0 ){
        for(int i=0;i<baseMapImages.size();++i){
            baseMapImages[i]->scale = scale;
        }
        qDebug() << "[BaseMapImageManager::AllChangeScale] scale = " << scale;
    }
}

void BaseMapImageManager::AllChangeFilePath()
{
    QString path = QInputDialog::getText(this,"Change File Path of All Images","Path to image");
    if( path.isNull() == false && path.isEmpty() == false ){
        path = path.replace("\\","/").trimmed();
        for(int i=0;i<baseMapImages.size();++i){
            baseMapImages[i]->path = path;
        }
        qDebug() << "[BaseMapImageManager::AllChangeFilePath] path = " << path;
    }
}



