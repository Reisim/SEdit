#ifndef GRAPHDIALOG_H
#define GRAPHDIALOG_H

#include <QObject>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QPen>
#include <QColor>
#include <QBrush>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QList>


class GraphDialog : public QWidget
{
    Q_OBJECT
public:
    explicit GraphDialog(QWidget *parent = nullptr);

    void Initialize(int _type);
    void ResetData();
    void SetData(QList<float> t, QList<float> v);
    void DrawAxis();
    void Draw();

signals:
    void requestDataAndUpdate(int);

public slots:
    void UpdateGraph();


private:

    int type;

    QList<float> time;
    QList<float> val;

    QPushButton *updateBtn;
    QPushButton *closeBtn;

    QGraphicsScene *gScene;
    QGraphicsView *gView;
};

#endif // GRAPHDIALOG_H
