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


#ifndef DISPLAYCONTROL_H
#define DISPLAYCONTROL_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QSpinBox>


class DisplayControl : public QWidget
{
    Q_OBJECT
public:
    explicit DisplayControl(QWidget *parent = nullptr);

signals:
    void ViewMoveTo(float x,float y);


public slots:
    void moveToClicked();

public:
    QPushButton *resetRotate;
    QPushButton *moveTo;
    QCheckBox *OrthogonalView;
    QCheckBox *showNodes;
    QCheckBox *showNodeLaneList;
    QCheckBox *showRelatedLanes;
    QCheckBox *showLanes;
    QSpinBox *laneWidth;
    QCheckBox *showTrafficSignals;
    QCheckBox *showStopLines;
    QCheckBox *showPedestLanes;
    QCheckBox *showMapImage;
    QCheckBox *backMapImage;

    QCheckBox *showNodeLabels;
    QCheckBox *showLaneLabels;
    QCheckBox *showTrafficSignalLabels;
    QCheckBox *showStopLineLabels;
    QCheckBox *showPedestLaneLabels;
    QCheckBox *showLabels;

    QCheckBox *selectNode;
    QCheckBox *selectLane;
    QCheckBox *selectTrafficSignal;
    QCheckBox *selectStopLine;
    QCheckBox *selectPedestLane;

    QPushButton *prevLaneList;
    QPushButton *nextLaneList;

private:
    QLineEdit *moveToX;
    QLineEdit *moveToY;
    QDoubleValidator *moveToXVal;
    QDoubleValidator *moveToYVal;

};

#endif // DISPLAYCONTROL_H
