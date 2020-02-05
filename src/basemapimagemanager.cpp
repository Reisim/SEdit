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
    upButtonLayout->addWidget( deleteImageBtn );
    upButtonLayout->addWidget( editImageBtn );
    upButtonLayout->addSpacing( 20 );
    upButtonLayout->addWidget( fromFileBtn );
    upButtonLayout->addStretch();

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
    for(int i=0;i<baseMapImages.size();++i){
        delete baseMapImages[i];
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

    AddMapImageToList(map);
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

    item = new QTableWidgetItem( QString("%1").arg(map->x,0,'g',4) );
    item->setFlags( Qt::ItemIsEnabled );
    mapImageList->setItem( nRow, 2, item );

    item = new QTableWidgetItem( QString("%1").arg(map->y,0,'g',4) );
    item->setFlags( Qt::ItemIsEnabled );
    mapImageList->setItem( nRow, 3, item );

    item = new QTableWidgetItem( QString("%1").arg(map->scale,0,'g',4) );
    item->setFlags( Qt::ItemIsEnabled );
    mapImageList->setItem( nRow, 4, item );

    item = new QTableWidgetItem( QString("%1").arg(map->rotate,0,'g',4) );
    item->setFlags( Qt::ItemIsEnabled );
    mapImageList->setItem( nRow, 5, item );
}


void BaseMapImageManager::DeleteMapImage()
{

}


void BaseMapImageManager::EditMapImageProperty()
{

}


void BaseMapImageManager::InsertImageFromFile()
{

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



