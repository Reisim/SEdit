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


#include <QApplication>
#include <QtGui>
#include <QStyleFactory>
#include <QTextCodec>
#include <QString>
#include <QDebug>

#include <windows.h>


#include "mainwindow.h"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    AllocConsole();

    qDebug() << "+--- Start Application";

    qDebug() << " Avaiable styles are : " << QStyleFactory::keys();

    qDebug() << "+--- setCodecForLocale";
    QTextCodec::setCodecForLocale( QTextCodec::codecForLocale() );

    qDebug() << "+--- setStyle -> Fusion";
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    MainWindow w;
    w.setWindowTitle("MDS02-Canopus | S-Edit");
    w.show();

    return a.exec();
}
