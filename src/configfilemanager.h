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


#ifndef CONFIGFILEMANAGER_H
#define CONFIGFILEMANAGER_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QMessageBox>


class ConfigFileManager : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigFileManager(QWidget *parent = nullptr);

    void ClearDataWithoutConfirm();

signals:

public slots:
    void ClearData();
    void LoadData();
    void SaveData();
    void SelectScenarioClicked();
    void SelectLogFolderClicked();
    void SelectRestartFileClicked();
    void ClearRestartFileClicked();


private:

    QPushButton *clearBtn;
    QPushButton *loadBtn;
    QPushButton *saveBtn;
    QPushButton *selScenarioBtn;
    QPushButton *selLogFolderBtn;
    QPushButton *selRestartFileBtn;
    QPushButton *remRestartFileBtn;

    QLabel *configFilename;
    QLabel *selectedScenariofile;
    QLabel *logOutputFolder;
    QLabel *restartFilename;

    QCheckBox *isDSMode;
    QDoubleSpinBox *calTimeStep;
    QCheckBox *resimLogOutput;
    QLineEdit *resimLogFileName;
    QCheckBox *resimLogTimeStamp;
    QSpinBox *logOutputInterval;
    QCheckBox *onlyFilename;
};

#endif // CONFIGFILEMANAGER_H
