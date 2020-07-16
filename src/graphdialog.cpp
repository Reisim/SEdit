#include "graphdialog.h"

#include <QDebug>


GraphDialog::GraphDialog(QWidget *parent) : QWidget(parent)
{
    type = -1;

    gScene = new QGraphicsScene( QRect(0,0,430,410) );
    gView = new QGraphicsView( gScene );


    QHBoxLayout *btnLay = new QHBoxLayout();

    updateBtn = new QPushButton("Update");
    connect( updateBtn, SIGNAL(clicked()), this, SLOT(UpdateGraph()) );

    closeBtn = new QPushButton("Close");
    connect( closeBtn, SIGNAL(clicked()), this, SLOT(close()) );

    btnLay->addWidget(updateBtn);
    btnLay->addStretch(1);
    btnLay->addWidget(closeBtn);


    QVBoxLayout *mainLay = new QVBoxLayout();

    mainLay->addLayout( btnLay );
    mainLay->addWidget( gView );
    mainLay->addStretch( 1 );

    setLayout( mainLay );
}


void GraphDialog::Initialize(int _type)
{
    type = _type;
    DrawAxis();
}


void GraphDialog::DrawAxis()
{
    gScene->clear();

    if( type == 0 ){

        gScene->addLine( 40, 180, 380, 180 );
        gScene->addLine( 40, 180,  40,  20 );
        QGraphicsTextItem *lbl1 = gScene->addText( QString("t[s]") );
        lbl1->setPos( 380, 180 );
        QGraphicsTextItem *lbl2 = gScene->addText( QString("V[km/h]") );
        lbl2->setPos(   0,   0 );

        gScene->addLine( 40, 380, 380, 380 );
        gScene->addLine( 40, 380,  40, 220 );
        QGraphicsTextItem *lbl3 = gScene->addText( QString("s[m]") );
        lbl3->setPos( 380, 380 );
        QGraphicsTextItem *lbl4 = gScene->addText( QString("V[km/h]") );
        lbl4->setPos(   0, 200 );
    }
    else if( type == 1 ){

        gScene->addLine( 40, 200, 380, 200 );
        gScene->addLine( 40, 380,  40,  20 );
        QGraphicsTextItem *lbl1 = gScene->addText( QString("t[s]") );
        lbl1->setPos( 380, 200 );
        QGraphicsTextItem *lbl2 = gScene->addText( QString("Accel/Decel[G]") );
        lbl2->setPos(   0,   0 );
        QGraphicsTextItem *lbl3 = gScene->addText( QString("0") );
        lbl3->setPos(   0, 188 );
        QGraphicsTextItem *lbl4 = gScene->addText( QString("1.0") );
        lbl4->setPos(   0, 20 );
        QGraphicsTextItem *lbl5 = gScene->addText( QString("-1.0") );
        lbl5->setPos(   0, 368 );
    }
    else if( type == 2 ){

        gScene->addLine( 40, 200, 380, 200 );
        gScene->addLine( 40, 380,  40,  20 );
        QGraphicsTextItem *lbl1 = gScene->addText( QString("t[s]") );
        lbl1->setPos( 380, 200 );
        QGraphicsTextItem *lbl2 = gScene->addText( QString("Steer[deg]") );
        lbl2->setPos(   0,   0 );
        QGraphicsTextItem *lbl3 = gScene->addText( QString("0") );
        lbl3->setPos(   0, 188 );
        QGraphicsTextItem *lbl4 = gScene->addText( QString("180") );
        lbl4->setPos(   0, 20 );
        QGraphicsTextItem *lbl5 = gScene->addText( QString("-180") );
        lbl5->setPos(   0, 368 );
    }
    else if( type == 3 ){

        gScene->addLine( 40, 180, 380, 180 );
        gScene->addLine( 40, 180,  40,  20 );
        QGraphicsTextItem *lbl1 = gScene->addText( QString("t[s]") );
        lbl1->setPos( 380, 180 );
        QGraphicsTextItem *lbl2 = gScene->addText( QString("V[m/s]") );
        lbl2->setPos(   0,   0 );

        gScene->addLine( 40, 380, 380, 380 );
        gScene->addLine( 40, 380,  40, 220 );
        QGraphicsTextItem *lbl3 = gScene->addText( QString("s[m]") );
        lbl3->setPos( 380, 380 );
        QGraphicsTextItem *lbl4 = gScene->addText( QString("V[m/s]") );
        lbl4->setPos(   0, 200 );
    }

    gScene->update();
}


void GraphDialog::ResetData()
{
    time.clear();
    val.clear();
    DrawAxis();
}


void GraphDialog::SetData(QList<float> t, QList<float> v)
{
    ResetData();

    time = t;
    val = v;

    Draw();
}


void GraphDialog::UpdateGraph()
{
    if( type < 0 ){
        return;
    }

    emit requestDataAndUpdate( type );
}


void GraphDialog::Draw()
{
    if( time.size() == 0 || val.size() == 0 ){
        return;
    }

    float maxTime = 0.0;
    for(int i=0;i<time.size();++i){
        if( maxTime < time[i] ){
            maxTime = time[i];
        }
    }
    if( maxTime < 0.1 ){
        return;
    }

    float dx = 340.0 / maxTime;

    QPen sPen;
    sPen.setColor( QColor( Qt::blue) );
    sPen.setStyle( Qt::SolidLine );

    QBrush sB;
    sB.setColor( QColor( Qt::blue) );
    sB.setStyle( Qt::SolidPattern );

    if( type == 0 || type == 3 ){

        float maxV = 0.0;
        for(int i=0;i<val.size();++i){
            if( maxV < val[i] ){
                maxV = val[i];
            }
        }
        if( maxV < 0.1 ){
            return;
        }

        float dy = 160.0 / maxV;
        for(int i=0;i<time.size();++i){

            float x = 40.0 + dx * time[i];
            float y = 180.0 - dy * val[i];
            if( y < 20.0 ){
                y = 20.0;
            }
            else if( y > 180.0 ){
                y = 180.0;
            }

            gScene->addEllipse( x-3, y-3, 6, 6, sPen, sB );
        }

        QList<float> dist;
        dist.append( 0.0 );

        for(int i=1;i<time.size();++i){

            float x1 = 40.0 + dx * time[i-1];
            float y1 = 180.0 - dy * val[i-1];
            if( y1 < 20.0 ){
                y1 = 20.0;
            }
            else if( y1 > 180.0 ){
                y1 = 180.0;
            }

            float x2 = 40.0 + dx * time[i];
            float y2 = 180.0 - dy * val[i];
            if( y2 < 20.0 ){
                y2 = 20.0;
            }
            else if( y2 > 180.0 ){
                y2 = 180.0;
            }

            gScene->addLine( x1, y1, x2, y2, sPen );

            float xm = (x1 + x2) / 2;
            float ym = (y1 + y2) / 2;
            if( time[i] - time[i-1] > 0.1 ){
                float a = (val[i] - val[i-1]) / 3.6 / (time[i] - time[i-1]) / 9.81;
                QGraphicsTextItem *lbl = gScene->addText( QString("%1[G]").arg(a,5,'g',2) );
                lbl->setPos( xm, ym );

                float dt = time[i] - time[i-1];
                float s = dist.last();
                s += val[i-1] / 3.6 * dt + 0.5 * a * 9.81 * dt * dt;
                dist.append( s );
            }
            else{
                dist.append( dist.last() );
            }
        }

        float maxS = 0.0;
        for(int i=0;i<dist.size();++i){
            if( maxS < dist[i] ){
                maxS = dist[i];
            }
        }

        if( maxS < 0.1 ){
            return;
        }

        dx = 340.0 / maxS;

        for(int i=0;i<dist.size();++i){

            float x = 40.0 + dx * dist[i];
            float y = 380.0 - dy * val[i];
            if( y < 220.0 ){
                y = 220.0;
            }
            else if( y > 380.0 ){
                y = 380.0;
            }

            gScene->addEllipse( x-3, y-3, 6, 6, sPen, sB );
        }

        for(int i=1;i<dist.size();++i){

            float x1 = 40.0 + dx * dist[i-1];
            float y1 = 380.0 - dy * val[i-1];
            if( y1 < 220.0 ){
                y1 = 220.0;
            }
            else if( y1 > 380.0 ){
                y1 = 380.0;
            }

            float x2 = 40.0 + dx * dist[i];
            float y2 = 380.0 - dy * val[i];
            if( y2 < 220.0 ){
                y2 = 220.0;
            }
            else if( y2 > 380.0 ){
                y2 = 380.0;
            }

            gScene->addLine( x1, y1, x2, y2, sPen );

            if( i == dist.size() - 1 ){
                QGraphicsTextItem *lbl = gScene->addText( QString("%1[m]").arg(maxS,5,'g',4) );
                lbl->setPos( x2-50, y2-24 );
            }
        }

    }
    else if( type == 1 ){

        float dy = 180.0;
        for(int i=0;i<time.size();++i){

            float x = 40.0 + dx * time[i];
            float y = 200.0 - dy * val[i];
            if( y < 20.0 ){
                y = 20.0;
            }
            else if( y > 380.0 ){
                y = 380.0;
            }

            gScene->addEllipse( x-3, y-3, 6, 6, sPen, sB );
        }

        for(int i=1;i<time.size();++i){

            float x1 = 40.0 + dx * time[i-1];
            float y1 = 200.0 - dy * val[i-1];
            if( y1 < 20.0 ){
                y1 = 20.0;
            }
            else if( y1 > 380.0 ){
                y1 = 380.0;
            }

            float x2 = 40.0 + dx * time[i];
            float y2 = 200.0 - dy * val[i];
            if( y2 < 20.0 ){
                y2 = 20.0;
            }
            else if( y2 > 380.0 ){
                y2 = 380.0;
            }

            gScene->addLine( x1, y1, x2, y2, sPen );
        }

    }
    else if( type == 2 ){

        float dy = 180.0 / 180.0;
        for(int i=0;i<time.size();++i){

            float x = 40.0 + dx * time[i];
            float y = 200.0 - dy * val[i];
            if( y < 20.0 ){
                y = 20.0;
            }
            else if( y > 380.0 ){
                y = 380.0;
            }

            gScene->addEllipse( x-3, y-3, 6, 6, sPen, sB );
        }

        for(int i=1;i<time.size();++i){

            float x1 = 40.0 + dx * time[i-1];
            float y1 = 200.0 - dy * val[i-1];
            if( y1 < 20.0 ){
                y1 = 20.0;
            }
            else if( y1 > 380.0 ){
                y1 = 380.0;
            }

            float x2 = 40.0 + dx * time[i];
            float y2 = 200.0 - dy * val[i];
            if( y2 < 20.0 ){
                y2 = 20.0;
            }
            else if( y2 > 380.0 ){
                y2 = 380.0;
            }

            gScene->addLine( x1, y1, x2, y2, sPen );
        }
    }
}
