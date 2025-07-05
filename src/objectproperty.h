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


#ifndef OBJECTPROPERTY_H
#define OBJECTPROPERTY_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTabWidget>
#include <QSpinBox>
#include <QTableWidget>
#include <QCheckBox>
#include <QScrollArea>
#include <QDebug>

#include <QUdpSocket>

#include "roadinfo.h"
#include "settingdialog.h"


class RoadObjectProperty : public QWidget
{
    Q_OBJECT
public:
    explicit RoadObjectProperty(QWidget *parent = nullptr);

    RoadInfo *road;
    QSpinBox *nodeIDSB;
    QSpinBox *laneIDSB;
    QSpinBox *tsIDSB;
    QSpinBox *slIDSB;
    QSpinBox *pedestLaneIDSB;
    QSpinBox *pedestLaneSectionSB;
    QSpinBox *soIDSB;
    QSpinBox *roadBoundaryIDSB;
    QSpinBox *roadBoundarySectionSB;

    SettingDialog *setDlg;



signals:
    void ShowODRouteEdit();
    void HideODRouteEdit();
    void DestinationNodeChanged(int,bool);
    void OriginNodeChanged(int,bool);
    void ResetLaneListIndex();
    void UpdateGraphic();
    void ChangeSelectionRequest(int,int);


public slots:
    void ChangeNodeInfo(int);
    void ChangeLaneInfo(int);
    void ChangeTrafficSignalInfo(int);
    void ChangeStopLineInfo(int);
    void ChangeTabPage(int);
    void ChangeVisibilityODRoute(int);
    void ChangeStaticObjInfo(int);
    void ChangeRoadBoundaryInfo(int);
    void ChangeRoadBoundaryInfo(int,int);

    void CBOriginNodeChanged(bool);
    void CBDestinationNodeChanged(bool);

    void SpeedLimitChanged(int);
    void ActualSpeedChanged(int);
    void AutomaticDrivingEnableFlagChanged(bool);
    void DriverErrorProbChanged(double);
    void LaneWidthChanged(double);
    void EditLaneData();
    void CheckRelatedNode();
    void LaneEdgePosChanged();
    void GetLaneHeightFromUE();

    void SetDefaultTSPattern();
    void SetTSPattern(int ndIdx,int tsIdx);
    void AdjustRowCountTSPatternTable(int n);
    void TSPatternAddRowClicked();
    void TSPatternDelRowClicked();
    void TSPatternApplyClicked();
    void TSPatternCopyClicked();

    void ChangePedestLaneInfo(int);
    void ChangePedestLaneInfo(int,int);
    void PedestLaneApplyClicked();
    void SetPedestLaneTrafficVolume(int);
    void GetPedestLaneHeightFromUE();

    void EditStaticObjectData();

    void RoadBoundaryApplyClicked();

private:

    QTabWidget *tab;

    QWidget *nodePage;
    QLabel *nodeInfo;
    QCheckBox *cbONode;
    QCheckBox *cbDNode;
    QScrollArea *nodeInfoScrollArea;

    QWidget *lanePage;
    QSpinBox *laneSpeed;
    QSpinBox *laneActualSpeed;
    QCheckBox *laneAutomaticDrivingEnabled;
    QDoubleSpinBox *laneDriverErrorProb;
    QDoubleSpinBox *laneWidth;

    QLabel *laneInfo;
    QPushButton *editLaneData;
    QPushButton *checkRelatedNode;
    QScrollArea *laneInfoScrollArea;
    QPushButton *updateLaneShapeBtn;
    QPushButton *getHeightFromUEBtn;

    QDoubleSpinBox *laneStartX;
    QDoubleSpinBox *laneStartY;
    QDoubleSpinBox *laneStartZ;
    QDoubleSpinBox *laneStartDir;
    QDoubleSpinBox *laneEndX;
    QDoubleSpinBox *laneEndY;
    QDoubleSpinBox *laneEndZ;
    QDoubleSpinBox *laneEndDir;

    QWidget *trafficSignalPage;
    QLabel *tsInfo;
    QScrollArea *tsInfoScrollArea;
    QTableWidget *tsDisplayPattern;
    QPushButton *addRowDisplayPattern;
    QPushButton *delRowDisplayPattern;
    QPushButton *applyDisplayPattern;
    QSpinBox *tsStartOffset;
    QCheckBox *tsIsSensorType;
    QSpinBox *tsChangeTimeBySensor;
    QPushButton *copyDisplayPattern;

    QWidget *stopLinePage;
    QLabel *slInfo;

    QWidget *pedestLanePage;
    QLabel *pedestLaneInfo;
    QCheckBox *cbIsCrossWalk;
    QDoubleSpinBox *pedestLaneWidth;
    QDoubleSpinBox *pedestRunOutProb;
    QDoubleSpinBox *pedestMarginToRoad;
    QComboBox *pedestRunOutDirect;
    QTableWidget *pedestLaneTrafficVolume;
    QPushButton *applyPedestLaneDataChange;
    QPushButton *getPedestHeightFromUEBtn;
    QCheckBox *cbCanWaitTaxi;
    QDoubleSpinBox *pedestLaneTaxiTakeProbability;

    QWidget *staticObjPage;
    QLabel *soInfo;
    QPushButton *editStaticObject;

    QWidget *roadBoundaryPage;
    QLabel *roadBoundaryInfo;
    QComboBox *roadBoundaryRoadSide;
    QDoubleSpinBox *roadBoundaryHeight;
    QCheckBox *rbHeightAll;
    QPushButton *applyRoadBoundaryDataChange;

    QCheckBox *cbChangeSelectionBySpinbox;
};

#endif // OBJECTPROPERTY_H
