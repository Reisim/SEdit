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


#include "resimfilesoutput.h"
#include <QDebug>


ResimFilesOutput::ResimFilesOutput(QWidget *parent) : QWidget(parent)
{
    outputFilename = new QLineEdit();
    outputFilename->setFixedWidth(400);

    outputFolderStr = new QLabel("Not selected.");
    outputFolderStr->setMinimumWidth(800);

    selectFolder = new QPushButton();
    selectFolder->setIcon( QIcon(":/images/Select.png") );
    connect( selectFolder, SIGNAL(clicked()), this, SLOT(SelectOutputFolder()) );

    maxAgent = new QSpinBox();
    maxAgent->setFixedWidth(140);
    maxAgent->setMaximum(10000);
    maxAgent->setValue(1000);


    QGridLayout *gridLayout = new QGridLayout();

    gridLayout->addWidget( new QLabel("Output Filename : "), 0, 0 );
    gridLayout->addWidget( outputFilename, 0, 1 );

    gridLayout->addWidget( new QLabel("Output Folder : "), 1, 0 );
    gridLayout->addWidget( outputFolderStr, 1, 1 );
    gridLayout->addWidget( selectFolder, 1, 2 );

    gridLayout->addWidget( new QLabel("Max Agent Number : "), 2, 0 );
    gridLayout->addWidget( maxAgent, 2, 1 );


    outputData = new QPushButton("Output");
    outputData->setIcon( QIcon(":/images/save.png") );
    connect( outputData, SIGNAL(clicked()), this, SLOT(OutputFiles()) );

    QHBoxLayout *outputBtnLayout = new QHBoxLayout();
    outputBtnLayout->addStretch(1);
    outputBtnLayout->addWidget( outputData );
    outputBtnLayout->addStretch(1);

    //----------------------------------------------

    QVBoxLayout *mainLayout = new QVBoxLayout();

    mainLayout->addLayout( gridLayout );
    mainLayout->addLayout( outputBtnLayout );

    setLayout( mainLayout );

    setWindowTitle("Re:sim Files Output");
}


void ResimFilesOutput::SelectOutputFolder()
{
    QString folderName = QFileDialog::getExistingDirectory(this,"Output Folder");
    if( folderName.isNull() == true || folderName.isEmpty() == true ){
        return;
    }

    outputFolderStr->setText( folderName );
    update();
}


void ResimFilesOutput::OutputFiles()
{
    QString filename = outputFilename->text();
    if( filename.isNull() == true || filename.isEmpty() == true ){
        qDebug() << "[ResimFilesOutput::OutputFiles] filename is null or empty";
        return;
    }

    QString foldername = outputFolderStr->text();
    if( foldername.isNull() == true || foldername.isEmpty() == true || foldername.contains("Not selected.") == true ){
        qDebug() << "[ResimFilesOutput::OutputFiles] foldername is null or empty";
        return;
    }

    qDebug() << "call road->outputResilFiles.";
    qDebug() << "    folder = " << foldername;
    qDebug() << "    filename = " << filename;


    // Check Cross Points
    road->CheckLaneCrossPoints();

    // Create WP Data
    road->CreateWPData();

    // Check Route Data
    road->CheckRouteInOutDirection();

    // Set Lane List
    road->SetAllLaneLists();

    // Set Turn Direction Info
    road->SetTurnDirectionInfo();


    // Output Files
    bool ret1 = road->outputResimRoadFiles( foldername, filename );
    qDebug() << "[outputResimRoadFiles] ret = " << ret1;

    if( ret1 == false ){
        QMessageBox::warning(this,"Error","Failed to make Re:sim Road Files.");
        return;
    }


    bool ret2 = road->outputResimTrafficSignalFiles( foldername, filename );
    qDebug() << "[outputResimTrafficSignalFiles] ret = " << ret2;

    if( ret2 == false ){
        QMessageBox::warning(this,"Error","Failed to make Re:sim Traffic Signal Files.");
        return;
    }


    bool ret3 = road->outputResimScenarioFiles( foldername, filename, maxAgent->value() );
    qDebug() << "[outputResimScenarioFiles] ret = " << ret3;

    if( ret3 == false ){
        QMessageBox::warning(this,"Error","Failed to make Re:sim Scenario Files.");
        return;
    }


    QMessageBox::information(this,"Success","Re:sim Files created.");
}


