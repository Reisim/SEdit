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


#ifndef BASEMAPIMAGEMANAGER_H
#define BASEMAPIMAGEMANAGER_H

#include <QWidget>
#include <QList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QInputDialog>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>


struct baseMapImage
{
    QString path;
    QString filename;
    float scale;
    float x;
    float y;
    float rotate;
};


class BaseMapImageManager : public QWidget
{
    Q_OBJECT
public:
    explicit BaseMapImageManager(QWidget *parent = 0);
    ~BaseMapImageManager();

    QList<struct baseMapImage*> baseMapImages;

signals:
    void MapImageAdded();
    void MapImageDeleted();


public slots:
    void AddMapImage();
    void AddMapImageToList(struct baseMapImage*);
    void DeleteMapImage();
    void EditMapImageProperty();
    void AllChangeFilePath();
    void AllChangeScale();
    void ClearAll();
    void InsertImageFromFile();

private:
    QTableWidget *mapImageList;
};

#endif // BASEMAPIMAGEMANAGER_H
