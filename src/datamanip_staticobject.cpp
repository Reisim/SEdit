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


#include "datamanipulator.h"


int DataManipulator::CreateStaticObject()
{
    qDebug() << "[DataManipulator::CreateStaticObject]";

    float x = 0.0;
    float y = 0.0;
    if( canvas ){
        QVector2D clickPos = canvas->GetMouseClickPosition();
        canvas->Get3DPhysCoordFromPickPoint( clickPos.x(), clickPos.y(), x, y );
        //qDebug() << "Cursor: x = " << clickPos.x() << ", y = " << clickPos.y() << ",  World: x = " << x << " , y = " << y;
    }

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

    xcSB->setValue( x );
    ycSB->setValue( y );
    zcSB->setValue( 0.0 );
    dirSB->setValue( 0.0 );
    lxSB->setValue( 10.0 );
    lySB->setValue( 10.0 );
    hSB->setValue( 10.0 );

    dialog->exec();

    if( dialog->result() == QDialog::Accepted ){

        int soId = road->CreateStaticObject( -1 );

        road->MoveStaticObject( soId, xcSB->value(), ycSB->value(), zcSB->value() );
        road->RotateStaticObject( soId, dirSB->value() );
        road->SetSizeStaticObject( soId, lxSB->value(), lySB->value(), hSB->value() );

        if( canvas ){
            canvas->update();
        }

        qDebug() << "Static Object ID = " << soId << " created.";

        return soId;
    }

    qDebug() << "canceled.";

    return -1;
}
