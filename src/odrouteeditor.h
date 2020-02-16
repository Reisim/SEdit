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


#ifndef ODROUTEEDITOR_H
#define ODROUTEEDITOR_H

#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>
#include <QStringList>
#include <QList>
#include <QResizeEvent>

#include "roadinfo.h"
#include "objectproperty.h"
#include "settingdialog.h"


class ODRouteEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ODRouteEditor(QWidget *parent = nullptr);

    RoadInfo *road;
    RoadObjectProperty *propRO;
    SettingDialog *setDlg;

    int currentOriginNode;

    void SetHeaderTrafficVolumeTable();
    QList<int> destinationNodeList;

protected:
    void resizeEvent(QResizeEvent *event);


signals:
    void ShowMessageStatusBar(QString);
    void SetNodePickMode(int,int);
    void ResetNodePickMode();

public slots:
    void SetCurrentODRouteData(int oNode,bool routTableSetFlag);
    void SetCurrentODRouteDataForDestination(QString dNodeStr);
    void ApplyData();

    void AddRoute();
    void DelRoute();
    void GetNodeListForRoute(QList<int>);

private:

    QComboBox *cbDestnation;

    QTableWidget *trafficVolumeTable;
    QTableWidget *routeTable;

    QPushButton *addRouteBtn;
    QPushButton *delRouteBtn;
    QPushButton *applyDataBtn;

};

#endif // ODROUTEEDITOR_H
