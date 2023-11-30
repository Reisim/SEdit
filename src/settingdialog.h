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


#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QFileDialog>
#include <QSpinBox>


class SettingDialog : public QWidget
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);
    void LoadSetting();
    void SaveSetting();

    int GetCurrentLeftRightIndex();
    bool GetUseRelativePath() { return useRelativePath->isChecked(); }
    int GetVehicleKindNum();
    int GetPedestrianKindNum();
    int GetNumberActorForUE4Model(){ return numActorForUE4Models->value(); }
    int GetMaxActorsInUE4(){ return maxActorForUE4->value(); }
    QStringList GetVehicleKindCategory();
    QStringList GetPedestianKindCategory();
    QStringList GetVehicleKindSubcategory();
    QStringList GetPedestianKindSubcategory();

    QString GetVehicleKindTableStr(int row,int col);
    QString GetPedestKindTableStr(int row,int col);

    void SetDefaultVehicleKind();
    void SetDefaultPedestrianKind();

signals:
    void TrafficDirectionSettingChanged(int);


protected:
    void closeEvent(QCloseEvent *event);


public slots:
    void cbLeftRightChanged(int);
    void AddRowVehicleKind();
    void DelRowVehicleKind();
    void AddRowPedestKind();
    void DelRowPedestKind();

    void LoadVehiclePedestrianSetting();
    void SaveVehiclePedestrianSetting();


private:
    QComboBox *LeftRight;
    QCheckBox *useRelativePath;

    QPushButton *saveVehiclePedestrianKindBtn;
    QPushButton *loadVehiclePedestrianKindBtn;

    QTableWidget *vehicleKindTable;
    QPushButton *addVehicleKindBtn;
    QPushButton *delVehicleKindBtn;

    QTableWidget *pedestrianKindTable;
    QPushButton *addPedestKindBtn;
    QPushButton *delPedestKindBtn;

    QSpinBox *numActorForUE4Models;
    QSpinBox *maxActorForUE4;

    QPushButton *closeButton;
};

#endif // SETTINGDIALOG_H
