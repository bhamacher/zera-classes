#ifndef SEM1MODULEMEASPROGRAM_H
#define SEM1MODULEMEASPROGRAM_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QTimer>
#include <QStateMachine>
#include <QState>
#include <QFinalState>

#include "basemeasprogram.h"
#include "secinputinfo.h"
#include "ecalcinfo.h"

namespace Zera
{
namespace Proxy
{
    class cProxyClient;
}
}


class cVeinModuleParameter;
class cVeinModuleActvalue;
class cStringValidator;

class cDspMeasData;
class cDspIFace;
class QStateMachine;
class QState;
class QFinalState;


namespace SEM1MODULE
{

enum sem1moduleCmds
{
    sendrmident,
    testsec1resource,
    setecresource,
    readresourcetypes,
    readresource,
    fetchecalcunits,
    readrefInputalias,
    setpcbrefconstantnotifier,
    setsecintnotifier,

    freeecalcunits,
    freeecresource,
    freeusermem,

    actualizestatus,
    actualizeMTCnt,
    actualizeenergy,
    actualizepower,

    setsync,
    setmeaspulses,
    setmastermux,
    setslavemux,
    setmastermeasmode,
    setslavemeasmode,
    enableinterrupt,
    startmeasurement,

    stopmeas,
    fetchrefconstant,

    readintregister,
    resetintregister,
    readvicount,
    readtcount

};

#define irqPCBNotifier 16

class cSem1Module;
class cSem1ModuleConfigData;


class cSem1ModuleMeasProgram: public cBaseMeasProgram
{
    Q_OBJECT

public:
    cSem1ModuleMeasProgram(cSem1Module* module, Zera::Proxy::cProxy* proxy, cSem1ModuleConfigData& configData);
    virtual ~cSem1ModuleMeasProgram();
    virtual void generateInterface(); // here we export our interface (entities)
    virtual void deleteInterface(); // we delete interface in case of reconfiguration

public slots:
    virtual void start(); // difference between start and stop is that actual values
    virtual void stop(); // in interface are not updated when stop


protected slots:
    virtual void catchInterfaceAnswer(quint32 msgnr, quint8 reply, QVariant answer);

private:
    cSem1Module* m_pModule; // the module we live in
    cSem1ModuleConfigData& m_ConfigData;

    Zera::Server::cSECInterface* m_pSECInterface;
    Zera::Server::cPCBInterface* m_pPCBInterface;

    // statemachine for activating gets the following states
    QState resourceManagerConnectState; // connect to resource manager
    QState m_IdentifyState; // identify to resource manager
    QState m_testSEC1ResourceState; // test for our configured error calculator units
    QState m_setECResourceState; // here we try to set 3 ecalcunits at rm
    QState m_readResourceTypesState; // read all resource types ...at the moment we set a list of predefined resource types
    QState m_readResourcesState; // init to read all resource information for each type
    QState m_readResourceState; // read for 1 type
    QState m_testSemInputsState; // here we test if all our configured Inputs are present, we don't set them because we only get information from here
    QState m_ecalcServerConnectState; // connect to ecalculator server
    QState m_fetchECalcUnitsState; // we try to fetch 3 error calc units from sec server
    QState m_pcbServerConnectState; // connect to pcb server
    QState m_readREFInputsState; // init to read all ref Input informations
    QState m_readREFInputAliasState; // read for 1 Input
    QState m_readREFInputDoneState;
    QState m_setpcbREFConstantNotifierState; // we get notified on refconstant changes
    QState m_setsecINTNotifierState; // we get notified on sec interrupts
    QFinalState m_activationDoneState; // and then we have finished

    // statemachine for deactivating
    QState m_freeECalculatorState; // we give back our ecalcunits to sec server
    QState m_freeECResource; // and also give them back to the resource manager
    QFinalState m_deactivationDoneState;

    // statemachine for starting error measurement
    QStateMachine m_startMeasurementMachine;
    QState m_setsyncState; // our 2 ecalc units must be synchronized
    QState m_setsync2State;
    QState m_setMeaspulsesState; // we set the desired measpulses
    QState m_setMasterMuxState; // we set the Input selectors
    QState m_setSlaveMuxState;
    QState m_setSlave2MuxState;
    QState m_setMasterMeasModeState; // and the meas modes
    QState m_setSlaveMeasModeState;
    QState m_setSlave2MeasModeState;
    QState m_enableInterruptState;
    QState m_startMeasurementState;
    QFinalState m_startMeasurementDoneState; // here we start it

    // statemachine for interrupthandling;
    QStateMachine m_InterrupthandlingStateMachine;
    QState m_readIntRegisterState;
    QState m_resetIntRegisterState;
    QState m_readVICountactState;
    QState m_readTCountactState;
    QFinalState m_setEMResultState;

    Zera::Proxy::cProxyClient* m_pRMClient;
    Zera::Proxy::cProxyClient* m_pSECClient;
    Zera::Proxy::cProxyClient* m_pPCBClient;

    QStringList m_ResourceTypeList;
    QHash<QString,QString> m_ResourceHash; // resourcetype, resourcelist ; seperated
    QHash<QString,cSecInputInfo*> mREFSemInputInfoHash; // we hold a list of all our Input properties
    QHash<QString,cSecInputInfo*> mREFSemInputSelectionHash; // systemname from configfile->alias, csecInputinfo
    QHash<QString, double> mEnergyUnitFactorHash; //

    QStringList m_REFAliasList; // we want to have an ordered list with Input alias
    qint32 m_nIt;
    QList<QString> m_sItList; // for interation over x Input hash
    cSecInputInfo* siInfo;
    QString m_sIt;

    cECalcChannelInfo m_MasterEcalculator;
    cECalcChannelInfo m_SlaveEcalculator;
    cECalcChannelInfo m_Slave2Ecalculator;

    cVeinModuleParameter* m_pRefInputPar;
    cVeinModuleParameter* m_pRefConstantPar;
    cVeinModuleParameter* m_pTargetedPar;
    cVeinModuleParameter* m_pMeasTimePar;

    cVeinModuleParameter* m_pT0InputPar;
    cVeinModuleParameter* m_pT1InputPar;
    cVeinModuleParameter* m_pInputUnitPar;
    cVeinModuleParameter* m_pStartStopPar;

    cVeinModuleParameter* m_pStatusAct;
    cVeinModuleParameter* m_pTimeAct;
    cVeinModuleParameter* m_pEnergyAct;
    cVeinModuleParameter* m_pPowerAct;
    cVeinModuleParameter* m_pResultAct;

    void setInterfaceComponents();
    void setValidators();
    void setUnits();

    QStringList getEnergyUnitValidator();
    QString getEnergyUnit();
    QStringList getPowerUnitValidator();
    QString getPowerUnit();

    void handleChangedREFConst();
    void handleSECInterrupt();

    // vars dealing with error measurement
    bool m_brunning;
    QTimer m_ActualizeTimer; // after timed out we actualize progressvalue
    quint32 m_nStatus; // idle, started, running, finished
    double m_fResult; // error value in %
    quint32 m_nMTCNTStart;
    quint32 m_nMTCNTact;
    quint32 m_nVIfin;
    quint32 m_nVIAct;
    quint32 m_nTfin;
    quint32 m_nTAct;
    quint32 m_nTCountAct;
    double m_fEnergy;
    double m_fPower;
    quint32 m_nIntReg;

private slots:
    void resourceManagerConnect();
    void sendRMIdent();
    void testSEC1Resource();
    void setECResource();
    void readResourceTypes();
    void readResources();
    void readResource();
    void testSemInputs();
    void ecalcServerConnect();
    void fetchECalcUnits();
    void pcbServerConnect();
    void readREFInputs();
    void readREFInputAlias();
    void readREFInputDone();
    void setpcbREFConstantNotifier();
    void setsecINTNotifier();
    void activationDone();

    void freeECalculator();
    void freeECResource();
    void deactivationDone();

    void setSync();
    void setSync2();
    void setMeaspulses();
    void setMasterMux();
    void setSlaveMux();
    void setSlave2Mux();
    void setMasterMeasMode();
    void setSlaveMeasMode();
    void setSlave2MeasMode();
    void enableInterrupt();
    void startMeasurement();
    void startMeasurementDone();

    void readIntRegister();
    void resetIntRegister();
    void readVICountact();
    void readTCountact();
    void setEMResult();

    void newStartStop(QVariant startstop);
    void newRefConstant(QVariant refconst);
    void newRefInput(QVariant refinput);
    void newTargeted(QVariant targeted);
    void newMeasTime(QVariant meastime);
    void newT0Input(QVariant t0input);
    void newT1Input(QVariant t1input);
    void newUnit(QVariant unit);

    void Actualize();
    bool found(QList<QString>& list, QString searched);

};
}

#endif // SEM1MODULEMEASPROGRAM_H
