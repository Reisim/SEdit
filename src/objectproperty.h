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

#include <QDebug>

#include "roadinfo.h"


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


signals:
    void ShowODRouteEdit();
    void HideODRouteEdit();
    void DestinationNodeChanged(int,bool);
    void OriginNodeChanged(int,bool);
    void ResetLaneListIndex();

public slots:
    void ChangeNodeInfo(int);
    void ChangeLaneInfo(int);
    void ChangeTrafficSignalInfo(int);
    void ChangeStopLineInfo(int);
    void ChangeTabPage(int);
    void ChangeVisibilityODRoute(int);

    void CBOriginNodeChanged(bool);
    void CBDestinationNodeChanged(bool);

    void SpeedLimitChanged(int);

    void SetDefaultTSPattern();
    void SetTSPattern(int ndIdx,int tsIdx);
    void AdjustRowCountTSPatternTable(int n);
    void TSPatternAddRowClicked();
    void TSPatternDelRowClicked();
    void TSPatternApplyClicked();

private:

    QTabWidget *tab;

    QWidget *nodePage;
    QLabel *nodeInfo;
    QCheckBox *cbONode;
    QCheckBox *cbDNode;

    QWidget *lanePage;
    QSpinBox *laneSpeed;
    QLabel *laneInfo;

    QWidget *trafficSignalPage;
    QLabel *tsInfo;
    QTableWidget *tsDisplayPattern;
    QPushButton *addRowDisplayPattern;
    QPushButton *delRowDisplayPattern;
    QPushButton *applyDisplayPattern;

    QWidget *stopLinePage;
    QLabel *slInfo;
};

#endif // OBJECTPROPERTY_H
