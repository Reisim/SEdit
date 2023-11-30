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


#include "objectproperty.h"
#include <QDebug>
#include <QDialog>


void RoadObjectProperty::ChangeStaticObjInfo(int id)
{
    if( !soInfo ){
        return;
    }

    soInfo->clear();

    if( !road ){
        return;
    }

    int idx = road->indexOfStaticObject( id );
    if( idx < 0 ){
        return;
    }

    QString infoStr = QString();

    infoStr += QString("\n");
    infoStr += QString("[Position]\n");
    infoStr += QString("  X: %1, Y=%2, Z=%3\n").arg( road->staticObj[idx]->xc ).arg( road->staticObj[idx]->yc ).arg( road->staticObj[idx]->zc );
    infoStr += QString("[Orientation]\n");
    infoStr += QString("  direction =%1[deg]\n").arg( road->staticObj[idx]->direction );
    infoStr += QString("[Size]\n");
    infoStr += QString("  Lx =%1 , Ly = %2\n").arg( road->staticObj[idx]->lenx ).arg( road->staticObj[idx]->leny );
    infoStr += QString("[Height]\n");
    infoStr += QString("  height =%1\n").arg( road->staticObj[idx]->height );

    soInfo->setText( infoStr );
    soInfo->setAlignment( Qt::AlignTop );
    soInfo->setFixedSize( soInfo->sizeHint() );

    if( cbChangeSelectionBySpinbox->isChecked() == true ){
        emit ChangeSelectionRequest(8,id);
    }
}


void RoadObjectProperty::EditStaticObjectData()
{
    qDebug() << "[RoadObjectProperty::EditStaticObjectData]";

    QDialog *dialog = new QDialog();

    QGridLayout *gLay = new QGridLayout();
    gLay->addWidget( new QLabel("Xc[m]")   , 0, 0 );
    gLay->addWidget( new QLabel("Yc[m]")  , 1, 0 );
    gLay->addWidget( new QLabel("Zc[m]") , 2, 0 );
    gLay->addWidget( new QLabel("Direction[deg]")   , 3, 0 );
    gLay->addWidget( new QLabel("Lx[m]")  , 4, 0 );
    gLay->addWidget( new QLabel("Ly[m]") , 5, 0 );
    gLay->addWidget( new QLabel("Height[m]") , 6, 0 );

    QDoubleSpinBox *xcSB = new QDoubleSpinBox();
    xcSB->setMinimum(-100000.0 );
    xcSB->setMaximum( 100000.0 );

    QDoubleSpinBox *ycSB = new QDoubleSpinBox();
    ycSB->setMinimum(-100000.0 );
    ycSB->setMaximum( 100000.0 );

    QDoubleSpinBox *zcSB = new QDoubleSpinBox();
    zcSB->setMinimum(-100000.0 );
    zcSB->setMaximum( 100000.0 );

    QDoubleSpinBox *dirSB = new QDoubleSpinBox();
    dirSB->setMinimum(-180.0 );
    dirSB->setMaximum( 180.0 );

    QDoubleSpinBox *lxSB = new QDoubleSpinBox();
    lxSB->setMinimum( 0.0 );
    lxSB->setMaximum( 100000.0 );

    QDoubleSpinBox *lySB = new QDoubleSpinBox();
    lySB->setMinimum( 0.0 );
    lySB->setMaximum( 100000.0 );

    QDoubleSpinBox *hSB = new QDoubleSpinBox();
    hSB->setMinimum(-100000.0 );
    hSB->setMaximum( 100000.0 );

    gLay->addWidget( xcSB   , 0, 1 );
    gLay->addWidget( ycSB  , 1, 1 );
    gLay->addWidget( zcSB , 2, 1 );
    gLay->addWidget( dirSB   , 3, 1 );
    gLay->addWidget( lxSB  , 4, 1 );
    gLay->addWidget( lySB , 5, 1 );
    gLay->addWidget( hSB , 6, 1 );

    QPushButton *okBtn = new QPushButton("Accept");
    okBtn->setIcon(QIcon(":/images/accept.png"));
    QPushButton *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setIcon(QIcon(":/images/delete.png"));

    connect( okBtn, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect( cancelBtn, SIGNAL(clicked()), dialog, SLOT(reject()));
    connect( okBtn, SIGNAL(clicked()), dialog, SLOT(close()));
    connect( cancelBtn, SIGNAL(clicked()), dialog, SLOT(close()));

    QHBoxLayout *btnLay = new QHBoxLayout();
    btnLay->addStretch(1);
    btnLay->addWidget( okBtn );
    btnLay->addSpacing(50);
    btnLay->addWidget( cancelBtn );
    btnLay->addStretch(1);

    QVBoxLayout *mLay = new QVBoxLayout();
    mLay->addLayout( gLay );
    mLay->addLayout( btnLay );

    dialog->setLayout( mLay );
    dialog->setFixedSize( dialog->sizeHint() );

    int soID = soIDSB->value();
    int idx = road->indexOfStaticObject( soID );
    if( idx >= 0 ){

        xcSB->setValue( road->staticObj[idx]->xc );
        ycSB->setValue( road->staticObj[idx]->yc );
        zcSB->setValue( road->staticObj[idx]->zc );
        dirSB->setValue( road->staticObj[idx]->direction );
        lxSB->setValue( road->staticObj[idx]->lenx );
        lySB->setValue( road->staticObj[idx]->leny );
        hSB->setValue( road->staticObj[idx]->height );
    }

    dialog->exec();

    if( dialog->result() == QDialog::Accepted ){

        if( idx >= 0 ){

            road->staticObj[idx]->xc = xcSB->value();
            road->staticObj[idx]->yc = ycSB->value();
            road->staticObj[idx]->zc = zcSB->value();
            road->staticObj[idx]->direction = dirSB->value();
            road->staticObj[idx]->lenx = lxSB->value();
            road->staticObj[idx]->leny = lySB->value();
            road->staticObj[idx]->height = hSB->value();

            ChangeStaticObjInfo( soID );
        }
    }

    emit UpdateGraphic();
}



