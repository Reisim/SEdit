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


#ifndef RESIMFILESOUTPUT_H
#define RESIMFILESOUTPUT_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QCheckBox>

#include "roadinfo.h"


class ResimFilesOutput : public QWidget
{
    Q_OBJECT
public:
    explicit ResimFilesOutput(QWidget *parent = nullptr);

    void Clear();
    RoadInfo *road;


signals:

public slots:
    void SelectOutputFolder();
    void SelectOutputFilename();
    void SelectScenarioFilename();
    void OutputFiles();


private:

    QLabel *outputFolderStr;
    QLineEdit *outputFilename;
    QPushButton *outputData;
    QPushButton *selectFilename;
    QPushButton *selectFolder;
    QSpinBox *maxAgent;
    QCheckBox *assignScenarioData;
    QLabel *scenarioFilename;
    QPushButton *selectScenarioFile;
    QCheckBox *onlyFilename;

};

#endif // RESIMFILESOUTPUT_H
