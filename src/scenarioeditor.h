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


#ifndef SCENARIOEDITOR_H
#define SCENARIOEDITOR_H

#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QToolBar>
#include <QMenu>
#include <QAction>
#include <QTabBar>
#include <QListWidget>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

#include "roadinfo.h"
#include "settingdialog.h"
#include "graphdialog.h"


class RoadInfo;


struct ScenarioItemCondition
{
    int combination;

    bool noTrigger;

    bool timeTrigger;
    int ttMin;
    float ttSec;
    int ttAbsOrRel;

    bool positionTrigger;
    float ptX;
    float ptY;
    float ptPassAngle;
    float ptWidth;
    int ptTargetObjID;

    bool velocityTrigger;
    float vtSpeed;
    int vtLowOrHigh;
    int vtTargetObjID;

    bool TTCTrigger;
    float ttcVal;
    int ttcCalType;
    float ttcCalPosX;
    float ttcCalPosY;
    int ttcCalObjectID;
    int ttcCalTargetObjID;

    bool FETrigger;

    bool externalTrigger;
    int funcKey;
};


struct ScenarioItemBehavior
{
    int actionType;
    QList<float> fParams;
    QList<int> iParams;
    QList<bool> bParams;

    QList<struct LaneShapeInfo*> laneShape;
    QList<struct PedestrianLaneShapeElement*> pedestLaneshape;
    struct RouteData *route;
};


struct ScenarioItem
{
    struct ScenarioItemCondition cond;
    struct ScenarioItemBehavior act;
};


struct ScenarioVehicle
{
    int ID;
    QList<struct ScenarioItem *> sItem;
};


struct ScenarioPedestrian
{
    int ID;
    QList<struct ScenarioItem *> sItem;
};


struct ScenarioSystem
{
    int ID;
    struct ScenarioItem sItem;
};


class ScenarioEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ScenarioEditor(SettingDialog *s, RoadInfo *r, QWidget *parent = nullptr);

    void SaveDataWithFilename(QString filename);
    void LoadDataWithFilename(QString filename);
    void ClearData();

    void SetPathRouteLaneShape(int sIdx, int rIdx);
    void SetNodeRouteLaneLists(int sIdx, int rIdx);
    void SetPedestRouteLaneShape(int sIdx, int rIdx);

    bool isScenarioVehicleSelected(int id);
    bool isScenarioPedestrianSelected(int id);
    void CloseDialogs();


signals:
    void SetScenarioPickMode(int);
    void UpdateGraphic();



public slots:
    void NewData();
    void LoadData();
    void SaveData();
    void SaveAsData();

    void AddScenarioVehicle();
    void AddScenarioPedestrian();
    void AddScenarioSystem();

    void AddScenarioVehicleSlot();
    void AddScenarioPedestSlot();

    void DelScenarioVehicle(int ID);
    void DelScenarioPedestrian(int ID);
    void DelScenarioSystem(int ID);
    void DelScenarioClicked();

    void DelScenarioVehicleSlot();
    void DelScenarioPedestSlot();

    void AddRowTable();
    void DelRowTable();
    void SetCtrlDataToTable(int tblNo,QList<float> t,QList<float> val);
    void ClearCtrlTable(int tblNo);

    void ApplyDataClicked();

    void ChangeSystemActSeletion(int);

    void ChangeActDataContents(int);
    void ChangeVehicleActSeletion(int);
    void ChangePedestActSeletion(int);

    void UpdateScenarioSystemList();
    void UpdateScenarioVehicleList();
    void UpdateScenarioVehicleSlotList();
    void UpdateScenarioPedestList();
    void UpdateScenarioPedestSlotList();
    void SetScenarioDataToGUI();

    void SetPickMode();
    void SetToNearestLane();
    void GetPointsPicked(int,float,float,float,float);
    void GetPointListPicked(int, QList<QPointF>);
    void ClearRouteData();

    void SetDataToGraph(int);

    void EmitUpdateGraphic();


public:
    QList<struct ScenarioSystem *> sSys;
    QList<struct ScenarioVehicle *> sVehicle;
    QList<struct ScenarioPedestrian *> sPedest;

    SettingDialog *setDlg;
    RoadInfo *road;


private:

    QString currentScenarioFile;

    QToolBar *toolbar;
    QCheckBox *confirmToDelete;

    QTabWidget *tabW;
    QListWidget *sysList;
    QListWidget *vehicleList;
    QListWidget *vehicleSlot;
    QPushButton *vehicleAddSlot;
    QPushButton *vehicleDelSlot;
    QListWidget *pedestList;
    QListWidget *pedestSlot;
    QPushButton *pedestAddSlot;
    QPushButton *pedestDelSlot;

    QGroupBox *condDataGB;
    QGroupBox *actDataGB;

    //----------- Trigger
    QComboBox *AND_OR;

    QCheckBox *noTrigger;

    QCheckBox *timeTrigger;
    QSpinBox *ttMinSB;
    QDoubleSpinBox *ttSecSB;
    QRadioButton *ttRelative;
    QRadioButton *ttAbsolute;
    QButtonGroup *ttRadioGroup;

    QCheckBox *positionTrigger;
    QDoubleSpinBox *ptX;
    QDoubleSpinBox *ptY;
    QDoubleSpinBox *ptPsi;
    QSpinBox *ptTargetID;
    QPushButton *ptSelPosition;
    QDoubleSpinBox *ptWidth;

    QCheckBox *velocityTrigger;
    QDoubleSpinBox *vtSpeed;
    QRadioButton *vtSlower;
    QRadioButton *vtHigher;
    QButtonGroup *vtRadioGroup;
    QSpinBox *vtTargetID;

    QCheckBox *TTCTrigger;
    QDoubleSpinBox *ttcX;
    QDoubleSpinBox *ttcY;
    QPushButton *ttcSelPosition;
    QSpinBox *ttcTargetID;
    QSpinBox *ttcCalObjectID;
    QDoubleSpinBox *ttcVal;
    QRadioButton *ttcPoint;
    QRadioButton *ttcObject;
    QButtonGroup *ttcRadioGroup;

    QCheckBox *FETrigger;

    QCheckBox *externalTrigger;
    QComboBox *etKeys;

    QWidget *sysActDataWidget;
    QWidget *vehicleActDataWidget;
    QWidget *pedestActDataWidget;

    //---------- system actios
    QComboBox *sysActSelCB;

    QWidget *saTeleportWidget;
    QDoubleSpinBox *saMoveToX;
    QDoubleSpinBox *saMoveToY;
    QDoubleSpinBox *saMoveToPsi;
    QSpinBox *saMoveToTargetID;
    QPushButton *saMoveToPick;
    QPushButton *saMoveToSetToNearestLane;
    QCheckBox *saMoveToWithSurroundingVehicles;
    QCheckBox *saMoveToOnlyOnce;
    QCheckBox *saMoveToClearLateralOffset;
    QSpinBox *saMoveToNearLaneID;

    QWidget *saChangeTSWidget;
    QSpinBox *saChangeTSTargetTSID;
    QSpinBox *saChangeTSChangeToIndex;
    QPushButton *saChangeTSPickTS;
    QCheckBox *saChangeTSSystemDown;
    QCheckBox *saChangeTSApplyAll;

    QWidget *saChangeSpeedInfoWidget;
    QCheckBox *saChangeSpeedInfoSpeedLimit;
    QCheckBox *saChangeSpeedInfoActualSpeed;
    QDoubleSpinBox *saChangeSpeedInfoSpeedVal;
    QLineEdit *saChangeSpeedInfoTargetLanes;
    QPushButton *saChangeSpeedInfoPickLane;

    QWidget *saSendUDPWidget;
    QLineEdit *saUDPIPAddr;
    QSpinBox *saUDPPortSB;
    QLineEdit *saUDPSendData;
    QCheckBox *saRepeatSend;


    //---------- vehicle actions
    QComboBox *vehicleActSelCB;

    QWidget *vaAppearWidget;
    QDoubleSpinBox *vaAppearX;
    QDoubleSpinBox *vaAppearY;
    QDoubleSpinBox *vaAppearPsi;
    QDoubleSpinBox *vaAppearSpeed;
    QPushButton *vaAppearPick;
    QPushButton *vaAppearSetToNearestLane;
    QCheckBox *vaAppearAllowRepeat;
    QRadioButton *vaAppearNodeRoute;
    QPushButton *vaAppearSetNodeRoute;
    QPushButton *vaAppearClearNodeRoute;
    QRadioButton *vaAppearPathRoute;
    QPushButton *vaAppearSetPathRoute;
    QPushButton *vaAppearClearPathRoute;
    QLabel *vaAppearRouteInfo;
    QButtonGroup *vaAppearRadioGroup;
    QSpinBox *vaVehicleModelID;
    QSpinBox *vaAppearNearLaneID;

    //--

    QWidget *vaControlWidget;
    QCheckBox *vaCtrlChangeControlMode;
    QComboBox *vaCtrlSelControlMode;
    QSpinBox *vaCtrlTargetID;
    QDoubleSpinBox *vaCtrlTargetSpeed;
    QDoubleSpinBox *vaCtrlTargetHeadwayTime;
    QDoubleSpinBox *vaCtrlTargetHeadwayDistance;
    QDoubleSpinBox *vaCtrlStopX;
    QDoubleSpinBox *vaCtrlStopY;
    QPushButton *vaCtrlSelPosition;

    QTableWidget *vaCtrlSpeedProfile;
    QPushButton *addRowSpeedProfileTable;
    QPushButton *delRowSpeedProfileTable;
    QPushButton *showSpeedProfileTableAsGraph;

    QCheckBox *vaCtrlSteer;
    QTableWidget *vaCtrlSteerTable;
    QPushButton *addRowSteerTable;
    QPushButton *delRowSteerTable;
    QPushButton *showSteerTableAsGraph;

    QCheckBox *vaCtrlAccelDecel;
    QTableWidget *vaCtrlAccelDecelTable;
    QPushButton *addRowAccelDecelTable;
    QPushButton *delRowAccelDecelTable;
    QPushButton *showAccelDecelTableAsGraph;

    QCheckBox *vaCtrlAllowRepeat;

    //--

    QWidget *vaUDPWidget;
    QLineEdit *vaUDPIPAddr;
    QSpinBox *vaUDPPortSB;
    QLineEdit *vaUDPSendData;
    QCheckBox *vaRepeatSend;

    //--

    QWidget *vaDisappearWidget;
    QCheckBox *vaDisappearAppearSoon;


    //----------- pedestrian actions
    QComboBox *pedestActSelCB;

    QWidget *paAppearWidget;
    QDoubleSpinBox *paAppearX;
    QDoubleSpinBox *paAppearY;
    QDoubleSpinBox *paAppearPsi;
    QDoubleSpinBox *paAppearSpeed;
    QPushButton *paAppearPick;
    QCheckBox *paAppearAllowRepeat;
    QPushButton *paAppearSetPathRoute;
    QPushButton *paAppearClearPathRoute;
    QLabel *paAppearRouteInfo;
    QSpinBox *paPedestModelID;
    QSpinBox *paAppearNearLaneID;

    //--

    QWidget *paControlWidget;

    QComboBox *paCtrlSelControlMode;
    QDoubleSpinBox *paCtrlTargetSpeed;
    QTableWidget *paCtrlSpeedProfile;
    QPushButton *addRowPedestSpeedProfileTable;
    QPushButton *delRowPedestSpeedProfileTable;
    QPushButton *showPedestSpeedProfileTableAsGraph;
    QDoubleSpinBox *paCtrlRunOutDirection;

    QCheckBox *paCtrlAllowRepeat;

    //--

    QWidget *paUDPWidget;
    QLineEdit *paUDPIPAddr;
    QSpinBox *paUDPPortSB;
    QLineEdit *paUDPSendData;
    QCheckBox *paRepeatSend;

    //--

    QWidget *paDisappearWidget;
    QCheckBox *paDisappearAppearSoon;


    QPushButton *applyBtn;


    //------------ Graph Dialog
    GraphDialog *speedProfileGraph;
    GraphDialog *accelDecelGraph;
    GraphDialog *steerGraph;
    GraphDialog *pedestSpeedProfileGraph;

};

#endif // SCENARIOEDITOR_H
