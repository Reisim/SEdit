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
#include <QScreen>
#include <QSurfaceFormat>
#include <QtGui>
#include <QStyleFactory>
#include <QTextCodec>
#include <QString>
#include <QDebug>
#include <QPalette>
#include <QProcess>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "mainwindow.h"


static bool isMacDarkMode()
{
#ifdef Q_OS_MAC
    QProcess p;
    p.start("defaults", {"read", "-g", "AppleInterfaceStyle"});
    p.waitForFinished(500);
    return p.readAllStandardOutput().trimmed().toLower().contains("dark");
#else
    return false;
#endif
}


static QPalette darkFusionPalette()
{
    QPalette pal;
    QColor base(35, 35, 38);
    QColor alt(45, 45, 48);
    QColor text(220, 220, 220);
    QColor highlight(42, 130, 218);
    pal.setColor(QPalette::Window, QColor(53,53,53));
    pal.setColor(QPalette::WindowText, text);
    pal.setColor(QPalette::Base, base);
    pal.setColor(QPalette::AlternateBase, alt);
    pal.setColor(QPalette::ToolTipBase, text);
    pal.setColor(QPalette::ToolTipText, text);
    pal.setColor(QPalette::Text, text);
    pal.setColor(QPalette::Button, QColor(53,53,53));
    pal.setColor(QPalette::ButtonText, text);
    pal.setColor(QPalette::BrightText, Qt::red);
    pal.setColor(QPalette::Link, highlight);
    pal.setColor(QPalette::Highlight, highlight);
    pal.setColor(QPalette::HighlightedText, Qt::black);
    pal.setColor(QPalette::Disabled, QPalette::Text, QColor(120,120,120));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120,120,120));
    return pal;
}



int main(int argc, char *argv[])
{
    // Required on macOS so #version 330 GLSL shaders work
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication a(argc, argv);

    qDebug() << "+--- Start Application";

    QSize ScreenSize = QGuiApplication::screens().at(0)->size();
    qDebug() << "   Secreen Size: " << ScreenSize;


#ifdef Q_OS_WIN
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
#endif



    //
    // Show Main Window
    //

    qDebug() << "   Avaiable styles are : " << QStyleFactory::keys();

    qDebug() << "+--- setCodecForLocale";
    QTextCodec::setCodecForLocale( QTextCodec::codecForLocale() );

    qDebug() << "+--- setStyle -> Fusion";
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    if( isMacDarkMode() ){
        qDebug() << "+--- macOS Dark mode detected, apply dark palette";
        QApplication::setPalette( darkFusionPalette() );
    }

    qDebug() << "+--- Create Main Window";
    MainWindow w;
    w.setWindowTitle("MDS02-Canopus | S-Edit[*]");
    w.setWindowIcon( QIcon(":images/SEdit-icon.png"));
    w.setMinimumSize( ScreenSize.width() * 0.6, ScreenSize.height() * 0.6 );
    w.show();


    qDebug() << "Size of Main Window: " << w.size();

    QSize moveMW = (ScreenSize - w.size()) * 0.5;
    qDebug() << "move Main Window to : " << moveMW;
    w.move( moveMW.width(), moveMW.height() );

    return a.exec();
}
