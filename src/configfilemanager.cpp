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


#include "configfilemanager.h"
#include <QDebug>


ConfigFileManager::ConfigFileManager(QWidget *parent) : QWidget(parent)
{
    QGridLayout *infoLayout = new QGridLayout();

    QLabel *title;

    title = new QLabel("Re:sim Configulation");
    title->setFixedSize( title->sizeHint() );
    infoLayout->addWidget( title, 0, 0 );

    title = new QLabel("Scenario Data File");
    title->setFixedSize( title->sizeHint() );
    infoLayout->addWidget( title, 1, 0 );

    title = new QLabel("DS Mode");
    title->setFixedSize( title->sizeHint() );
    infoLayout->addWidget( title, 2, 0 );

    title = new QLabel("Calculation Time Step");
    title->setFixedSize( title->sizeHint() );
    infoLayout->addWidget( title, 3, 0 );


    title = new QLabel("Re:sim Log Output");
    title->setFixedSize( title->sizeHint() );
    infoLayout->addWidget( title, 4, 0 );

    title = new QLabel("Re:sim Log File Output Folder");
    title->setFixedSize( title->sizeHint() );
    infoLayout->addWidget( title, 5, 0 );

    title = new QLabel("Re:sim Log File Name");
    title->setFixedSize( title->sizeHint() );
    infoLayout->addWidget( title, 6, 0 );

    title = new QLabel("Add Timestamp");
    title->setFixedSize( title->sizeHint() );
    infoLayout->addWidget( title, 7, 0 );

    title = new QLabel("Re:sim Log Output Interval");
    title->setFixedSize( title->sizeHint() );
    infoLayout->addWidget( title, 8, 0 );


    configFilename = new QLabel();
    configFilename->setText("");
    configFilename->setMinimumWidth(800);

    infoLayout->addWidget( configFilename, 0, 1, 1, 1, Qt::AlignLeft );

    selectedScenariofile = new QLabel();
    selectedScenariofile->setText("");
    selectedScenariofile->setMinimumWidth(800);

    selScenarioBtn = new QPushButton("");
    selScenarioBtn->setIcon( QIcon(":/images/Select.png") );
    selScenarioBtn->setFixedSize( selScenarioBtn->sizeHint() );
    connect( selScenarioBtn, SIGNAL(clicked()), this, SLOT(SelectScenarioClicked()) );

    infoLayout->addWidget( selectedScenariofile, 1, 1, 1, 1, Qt::AlignLeft );
    infoLayout->addWidget( selScenarioBtn, 1, 2 );

    isDSMode = new QCheckBox();
    infoLayout->addWidget( isDSMode, 2, 1, 1, 1, Qt::AlignLeft );

    calTimeStep = new QDoubleSpinBox();
    calTimeStep->setMinimum( 0.001 );
    calTimeStep->setMaximum( 0.1 );
    calTimeStep->setValue( 0.02 );
    calTimeStep->setSuffix("[s]");
    infoLayout->addWidget( calTimeStep, 3, 1, 1, 1, Qt::AlignLeft );

    resimLogOutput = new QCheckBox();
    infoLayout->addWidget( resimLogOutput, 4, 1, 1, 1, Qt::AlignLeft );


    logOutputFolder = new QLabel();
    logOutputFolder->setText( "" );

    selLogFolderBtn = new QPushButton("");
    selLogFolderBtn->setIcon( QIcon(":/images/Select.png") );
    selLogFolderBtn->setFixedSize( selLogFolderBtn->sizeHint() );
    connect( selLogFolderBtn, SIGNAL(clicked()), this, SLOT(SelectLogFolderClicked()) );

    infoLayout->addWidget( logOutputFolder, 5, 1, 1, 1, Qt::AlignLeft );
    infoLayout->addWidget( selLogFolderBtn, 5, 2 );


    resimLogFileName = new QLineEdit();
    resimLogFileName->setFixedWidth(300);
    infoLayout->addWidget( resimLogFileName, 6, 1, 1, 1, Qt::AlignLeft );

    resimLogTimeStamp = new QCheckBox();
    infoLayout->addWidget( resimLogTimeStamp, 7, 1, 1, 1, Qt::AlignLeft );

    logOutputInterval = new QSpinBox();
    logOutputInterval->setMinimum(0);
    logOutputInterval->setFixedWidth(90);
    infoLayout->addWidget( logOutputInterval, 8, 1, 1, 1, Qt::AlignLeft );


    clearBtn = new QPushButton("Clear");
    clearBtn->setIcon( QIcon(":/images/Trash.png") );
    connect( clearBtn, SIGNAL(clicked()), this, SLOT(ClearData()) );

    loadBtn = new QPushButton("Load");
    loadBtn->setIcon( QIcon(":/images/Select.png") );
    connect( loadBtn, SIGNAL(clicked()), this, SLOT(LoadData()) );

    saveBtn = new QPushButton("Save");
    saveBtn->setIcon( QIcon(":/images/save.png") );
    connect( saveBtn, SIGNAL(clicked()), this, SLOT(SaveData()) );


    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget( clearBtn );
    buttonLayout->addWidget( loadBtn );
    buttonLayout->addWidget( saveBtn );
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout();

    mainLayout->addLayout( buttonLayout );
    mainLayout->addLayout( infoLayout );

    setLayout( mainLayout );

    setWindowTitle("Config Manager");
}


void ConfigFileManager::ClearData()
{
    if( configFilename->text().isEmpty() == false ){
        if( QMessageBox::question(this,"Sure?","All data clear.", QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Cancel ){
            return;
        }
    }

    ClearDataWithoutConfirm();
}


void ConfigFileManager::ClearDataWithoutConfirm()
{
    configFilename->clear();
    selectedScenariofile->clear();
    logOutputFolder->clear();

    isDSMode->setChecked( false );
    calTimeStep->setValue( 0.02 );

    resimLogOutput->setChecked( false );
    resimLogFileName->clear();
    resimLogTimeStamp->setChecked( false );
    logOutputInterval->setValue(0);
}


void ConfigFileManager::LoadData()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose Re:sim Config File"),
                                                    ".",
                                                    tr("Re:sim Config file(*.rc.txt)"));

    if( fileName.isNull() || fileName.isEmpty() ){
        qDebug() << "[ConfigFileManager::LoadData] canceled.";
        return;
    }

    QFile rcfile( fileName  );
    if( rcfile.open(QIODevice::ReadOnly | QIODevice::Text) ){

        QTextStream in(&rcfile);
        QString line;

        line = in.readLine();
        line = in.readLine();
        if( line.contains("Re:sim Configulation File") == true ){

            line = in.readLine();

            while( in.atEnd() == false ){

                line = in.readLine();
                if( line.startsWith("#") || line.isEmpty() || line.contains(";") == false ){
                    continue;
                }

                QStringList divLine = line.split(";");
                QString tag = QString( divLine[0] ).trimmed();

                if( tag == QString("Output Log File") ){
                    if( QString( divLine[1] ).trimmed() == QString("yes") ){
                        resimLogOutput->setChecked( true );
                    }
                    else{
                        resimLogOutput->setChecked( false );
                    }
                }
                else if( tag == QString("DS Mode") ){
                    if( QString( divLine[1] ).trimmed() == QString("yes") ){
                        isDSMode->setChecked( true );
                    }
                    else{
                        isDSMode->setChecked( false );
                    }
                }
                else if( tag == QString("Scenario File") ){

                    selectedScenariofile->setText( QString(divLine[1]).trimmed() );
                }
                else if( tag == QString("Calculation Time Step") ){

                    calTimeStep->setValue( QString(divLine[1]).trimmed().toDouble() );
                }
                else if( tag == QString("Log Output Folder") ){

                    logOutputFolder->setText( QString(divLine[1]).trimmed() );
                }
                else if( tag == QString("CSV Output File") ){

                    resimLogFileName->setText( QString(divLine[1]).trimmed() );
                }
                else if( tag == QString("Add Timestamp to Filename") ){
                    if( QString( divLine[1] ).trimmed() == QString("yes") ){
                        resimLogTimeStamp->setChecked( true );
                    }
                    else{
                        resimLogTimeStamp->setChecked( false );
                    }
                }
                else if( tag == QString("Log Output Interval") ){

                    int val = QString( divLine[1] ).trimmed().toInt();
                    logOutputInterval->setValue(val);
                }
            }

            configFilename->setText( fileName );

        }
        else{
            qDebug() << "Not Resim Configulation File,";
        }

        rcfile.close();
    }
}


void ConfigFileManager::SaveData()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Re:sim Config File"),
                                                    ".",
                                                    tr("Re:sim Config file(*.rc.txt)"));

    if( fileName.isNull() || fileName.isEmpty() ){
        qDebug() << "[ConfigFileManager::SaveData] canceled.";
        return;
    }

    if( fileName.isNull() == false ){
        QFile rcfile( fileName );
        if( rcfile.open(QIODevice::WriteOnly | QIODevice::Text) ){

            QTextStream out(&rcfile);
            out << "=============================================\n";
            out << "         Re:sim Configulation File\n";
            out << "=============================================\n";
            out << "\n";
            out << "# 'DS Mode' should be ahead of 'Scenario File'\n";
            if( isDSMode->checkState() == Qt::Checked ){
                out << "DS Mode ; yes\n";
            }
            else{
                out << "DS Mode ; no\n";
            }

            out << "\n";
            out << "Scenario File ; " << selectedScenariofile->text() << "\n";
            out << "Calculation Time Step ; " << calTimeStep->value() << "\n";
            out << "\n";
            if( resimLogOutput->checkState() == Qt::Checked ){
                out << "Output Log File ; yes\n";
            }
            else{
                out << "Output Log File ; no\n";
            }
            out << "Log Output Folder ; " << logOutputFolder->text() << "\n";
            out << "CSV Output File ; " << resimLogFileName->text() << "\n";
            if( resimLogTimeStamp->checkState() == Qt::Checked ){
                out << "Add Timestamp to Filename ; yes\n";
            }
            else{
                out << "Add Timestamp to Filename ; no\n";
            }
            out << "Log Output Interval ; " << logOutputInterval->value() << "\n";

            rcfile.close();

            configFilename->setText( fileName );
        }
    }
}


void ConfigFileManager::SelectScenarioClicked()
{
    QString filename = QFileDialog::getOpenFileName(this,"Select","Re:sim Scenario File",QString("*.rs.txt"));
    if( filename.isNull() == true || filename.isEmpty() == true ){
        qDebug() << "[ConfigFileManager::SelectScenarioClicked] canceled.";
        return;
    }

    selectedScenariofile->setText( filename );
}


void ConfigFileManager::SelectLogFolderClicked()
{
    QString foldername = QFileDialog::getExistingDirectory(this,"Select","Log output folder");
    if( foldername.isNull() == true || foldername.isEmpty() == true ){
        qDebug() << "[ConfigFileManager::SelectScenarioClicked] canceled.";
        return;
    }

    logOutputFolder->setText( foldername );
}

