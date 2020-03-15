#ifndef WORKINGTHREAD_H
#define WORKINGTHREAD_H


#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QList>

#include "roadinfo.h"


class WorkingThread : public QThread
{
    Q_OBJECT
public:
    explicit WorkingThread(QObject *parent = 0);

    void run();
    void SetStopFlag(){ stopFlag = true; }

    QList<int> params;
    int mode;
    int wtID;

    volatile bool stopFlag;
    volatile int  nProcessed;


    RoadInfo *road;
};

#endif // WORKINGTHREAD_H
