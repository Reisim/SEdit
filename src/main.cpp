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
#include <QDesktopWidget>
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

    qDebug() << "+--- Start Application";

    QSize ScreenSize = QGuiApplication::screens().at(0)->size();
    qDebug() << "   Secreen Size: " << ScreenSize;


    //
    // Show Console for Windows
    //
    AllocConsole();
    SetConsoleTitleA( "S-Edit Console" );

    RECT consoleRec;
    GetClientRect( GetConsoleWindow(),  &consoleRec);
    int consoleWidth = consoleRec.right - consoleRec.left;
    int consoleHeight = consoleRec.bottom - consoleRec.top;
    MoveWindow( GetConsoleWindow(), 50, ScreenSize.height() - 100 - consoleHeight, consoleWidth, consoleHeight, TRUE );



    //
    // Show Main Window
    //

    qDebug() << "   Avaiable styles are : " << QStyleFactory::keys();

    qDebug() << "+--- setCodecForLocale";
    QTextCodec::setCodecForLocale( QTextCodec::codecForLocale() );

    qDebug() << "+--- setStyle -> Fusion";
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    qDebug() << "+--- Create Main Window";
    MainWindow w;
    w.setWindowTitle("MDS02-Canopus | S-Edit[*]");
    w.setWindowIcon( QIcon(":images/SEdit-icon.png"));
    w.show();


    qDebug() << "Size of Main Window: " << w.size();

    QSize moveMW = (ScreenSize - w.size()) * 0.5;
    qDebug() << "move Main Window to : " << moveMW;
    w.move( moveMW.width(), moveMW.height() );

    return a.exec();
}
