#include <QString>
#include <QStateMachine>
#include <rminterface.h>
#include <pcbinterface.h>
#include <secinterface.h>
#include <proxy.h>
#include <proxyclient.h>
#include <veinpeer.h>
#include <veinentity.h>

#include "debug.h"
#include "reply.h"
#include "errormessages.h"
#include "sec1module.h"
#include "sec1modulemeasprogram.h"
#include "sec1moduleconfigdata.h"

namespace SEC1MODULE
{

cSec1ModuleMeasProgram::cSec1ModuleMeasProgram(cSec1Module* module, Zera::Proxy::cProxy* proxy, VeinPeer* peer, cSec1ModuleConfigData& configData)
    :cBaseMeasProgram(proxy, peer), m_pModule(module), m_ConfigData(configData)
{
    // we have to instantiate a working resource manager and secserver interface
    m_pRMInterface = new Zera::Server::cRMInterface();
    m_pSECInterface = new Zera::Server::cSECInterface();
    m_pPCBInterface = new Zera::Server::cPCBInterface();

    m_IdentifyState.addTransition(this, SIGNAL(activationContinue()), &m_testSEC1ResourceState);
    m_testSEC1ResourceState.addTransition(this, SIGNAL(activationContinue()), &m_setECResourceState); // test presence of sec1 resource
    m_setECResourceState.addTransition(this, SIGNAL(activationContinue()), &m_readResourceTypesState); // claim 2 ecalculator units
    m_readResourceTypesState.addTransition(this, SIGNAL(activationContinue()), &m_readResourcesState); // read all resources types
    m_readResourcesState.addTransition(this, SIGNAL(activationContinue()), &m_readResourceState); // init read resources
    m_readResourceState.addTransition(this, SIGNAL(activationLoop()), &m_readResourceState); // read their resources into list
    m_readResourceState.addTransition(this, SIGNAL(activationContinue()), &m_testSecInputsState); // go on if done
    m_testSecInputsState.addTransition(this, SIGNAL(activationContinue()), &m_ecalcServerConnectState); // test all configured inputs
    //m_ecalcServerConnectState.addTransition(this, SIGNAL(activationContinue()), &m_fetchECalcUnitsState); // connect to ecalc server
    //transition from this state to m_fetch....is done in ecalcServerConnect
    m_fetchECalcUnitsState.addTransition(this, SIGNAL(activationContinue()), &m_pcbServerConnectState); // connect to pcbserver
    //m_pcbServerConnectState.addTransition(this, SIGNAL(activationContinue()), &m_pcbServerConnectState);
    //transition from this state to m_readREFInputsState....is done in pcbServerConnect
    m_readREFInputsState.addTransition(this, SIGNAL(activationContinue()), &m_readREFInputAliasState);
    m_readREFInputAliasState.addTransition(this, SIGNAL(activationContinue()), &m_readREFInputMuxchannelState);
    m_readREFInputMuxchannelState.addTransition(this, SIGNAL(activationLoop()), &m_readREFInputAliasState);
    m_readREFInputMuxchannelState.addTransition(this, SIGNAL(activationContinue()), &m_readDUTInputsState);
    m_readDUTInputsState.addTransition(this, SIGNAL(activationContinue()), &m_readDUTInputAliasState);
    m_readDUTInputAliasState.addTransition(this, SIGNAL(activationContinue()), &m_readDUTInputMuxchannelState);
    m_readDUTInputMuxchannelState.addTransition(this, SIGNAL(activationLoop()), &m_readDUTInputAliasState);
    m_readDUTInputMuxchannelState.addTransition(this, SIGNAL(activationContinue()), &m_setpcbREFConstantNotifierState);
    m_setpcbREFConstantNotifierState.addTransition(this, SIGNAL(activationContinue()), &m_setsecINTNotifierState);
    m_setsecINTNotifierState.addTransition(this, SIGNAL(activationContinue()), &m_activationDoneState);

    m_activationMachine.addState(&resourceManagerConnectState);
    m_activationMachine.addState(&m_IdentifyState);
    m_activationMachine.addState(&m_testSEC1ResourceState);
    m_activationMachine.addState(&m_setECResourceState);
    m_activationMachine.addState(&m_readResourceTypesState);
    m_activationMachine.addState(&m_readResourcesState);
    m_activationMachine.addState(&m_readResourceState);
    m_activationMachine.addState(&m_testSecInputsState);
    m_activationMachine.addState(&m_ecalcServerConnectState);
    m_activationMachine.addState(&m_fetchECalcUnitsState);
    m_activationMachine.addState(&m_pcbServerConnectState);
    m_activationMachine.addState(&m_readREFInputsState);
    m_activationMachine.addState(&m_readREFInputAliasState);
    m_activationMachine.addState(&m_readREFInputMuxchannelState);
    m_activationMachine.addState(&m_readDUTInputsState);
    m_activationMachine.addState(&m_readDUTInputAliasState);
    m_activationMachine.addState(&m_readDUTInputMuxchannelState);
    m_activationMachine.addState(&m_setpcbREFConstantNotifierState);
    m_activationMachine.addState(&m_setsecINTNotifierState);
    m_activationMachine.addState(&m_activationDoneState);

    m_activationMachine.setInitialState(&resourceManagerConnectState);

    connect(&resourceManagerConnectState, SIGNAL(entered()), SLOT(resourceManagerConnect()));
    connect(&m_IdentifyState, SIGNAL(entered()), SLOT(sendRMIdent()));
    connect(&m_testSEC1ResourceState, SIGNAL(entered()), SLOT(testSEC1Resource()));
    connect(&m_setECResourceState, SIGNAL(entered()), SLOT(setECResource()));
    connect(&m_readResourceTypesState, SIGNAL(entered()), SLOT(readResourceTypes()));
    connect(&m_readResourcesState, SIGNAL(entered()), SLOT(readResources()));
    connect(&m_readResourceState, SIGNAL(entered()), SLOT(readResource()));
    connect(&m_testSecInputsState, SIGNAL(entered()), SLOT(testSecInputs()));
    connect(&m_ecalcServerConnectState, SIGNAL(entered()), SLOT(ecalcServerConnect()));
    connect(&m_fetchECalcUnitsState, SIGNAL(entered()), SLOT(fetchECalcUnits()));
    connect(&m_pcbServerConnectState, SIGNAL(entered()), SLOT(pcbServerConnect()));
    connect(&m_readREFInputsState, SIGNAL(entered()), SLOT(readREFInputs()));
    connect(&m_readREFInputAliasState, SIGNAL(entered()), SLOT(readREFInputAlias()));
    connect(&m_readREFInputMuxchannelState, SIGNAL(entered()), SLOT(readREFInputMuxchannel()));
    connect(&m_readDUTInputsState, SIGNAL(entered()), SLOT(readDUTInputs()));
    connect(&m_readDUTInputAliasState, SIGNAL(entered()), SLOT(readDUTInputAlias()));
    connect(&m_readDUTInputMuxchannelState, SIGNAL(entered()), SLOT(readDUTInputMuxchannel()));
    connect(&m_setpcbREFConstantNotifierState, SIGNAL(entered()), SLOT(setpcbREFConstantNotifier()));
    connect(&m_setsecINTNotifierState, SIGNAL(entered()), SLOT(setsecINTNotifier()));
    connect(&m_activationDoneState, SIGNAL(entered()), SLOT(activationDone()));

    // setting up statemachine to free the occupied resources
    m_freeECalculatorState.addTransition(this, SIGNAL(deactivationContinue()), &m_freeECResource);
    m_freeECResource.addTransition(this, SIGNAL(deactivationContinue()), &m_deactivationDoneState);

    m_deactivationMachine.addState(&m_freeECalculatorState);
    m_deactivationMachine.addState(&m_freeECResource);
    m_deactivationMachine.addState(&m_deactivationDoneState);

    m_deactivationMachine.setInitialState(&m_freeECalculatorState);

    connect(&m_freeECalculatorState, SIGNAL(entered()), SLOT(freeECalculator()));
    connect(&m_freeECResource, SIGNAL(entered()), SLOT(freeECResource()));
    connect(&m_deactivationDoneState, SIGNAL(entered()), SLOT(deactivationDone()));

    // setting up statemachine used when starting a measurement

    m_setsyncState.addTransition(this, SIGNAL(setupContinue()), &m_setMeaspulsesState);
    m_setMeaspulsesState.addTransition(this, SIGNAL(setupContinue()), &m_setMasterMuxState);
    m_setMasterMuxState.addTransition(this, SIGNAL(setupContinue()), &m_setSlaveMuxState);
    m_setSlaveMuxState.addTransition(this, SIGNAL(setupContinue()), &m_setMasterMeasModeState);
    m_setMasterMeasModeState.addTransition(this, SIGNAL(setupContinue()), &m_setSlaveMeasModeState);
    m_setSlaveMeasModeState.addTransition(this, SIGNAL(setupContinue()), &m_enableInterruptState);
    m_enableInterruptState.addTransition(this, SIGNAL(setupContinue()), &m_startMeasurementState);
    m_startMeasurementState.addTransition(this, SIGNAL(setupContinue()), &m_startMeasurementDoneState);

    m_startMeasurementMachine.addState(&m_setsyncState);
    m_startMeasurementMachine.addState(&m_setMeaspulsesState);
    m_startMeasurementMachine.addState(&m_setMasterMuxState);
    m_startMeasurementMachine.addState(&m_setSlaveMuxState);
    m_startMeasurementMachine.addState(&m_setMasterMeasModeState);
    m_startMeasurementMachine.addState(&m_setSlaveMeasModeState);
    m_startMeasurementMachine.addState(&m_enableInterruptState);
    m_startMeasurementMachine.addState(&m_startMeasurementState);
    m_startMeasurementMachine.addState(&m_startMeasurementDoneState);

    m_startMeasurementMachine.setInitialState(&m_setsyncState);

    connect(&m_setsyncState, SIGNAL(entered()), SLOT(setSync()));
    connect(&m_setMeaspulsesState, SIGNAL(entered()), SLOT(setMeaspulses()));
    connect(&m_setMasterMuxState, SIGNAL(entered()), SLOT(setMasterMux()));
    connect(&m_setSlaveMuxState, SIGNAL(entered()), SLOT(setSlaveMux()));
    connect(&m_setMasterMeasModeState, SIGNAL(entered()), SLOT(setMasterMeasMode()));
    connect(&m_setSlaveMeasModeState, SIGNAL(entered()), SLOT(setSlaveMeasMode()));
    connect(&m_enableInterruptState, SIGNAL(entered()), SLOT(enableInterrupt()));
    connect(&m_startMeasurementState, SIGNAL(entered()), SLOT(startMeasurement()));
    connect(&m_startMeasurementDoneState, SIGNAL(entered()), SLOT(startMeasurementDone()));

    // setting up statemachine for interrupt handling

    m_readIntRegisterState.addTransition(this, SIGNAL(interruptContinue()), &m_resetIntRegisterState);
    m_resetIntRegisterState.addTransition(this, SIGNAL(interruptContinue()), &m_readMTCountactState);
    m_readMTCountactState.addTransition(this, SIGNAL(interruptContinue()), &m_setECResultState);

    m_InterrupthandlingStateMachine.addState(&m_readIntRegisterState);
    m_InterrupthandlingStateMachine.addState(&m_resetIntRegisterState);
    m_InterrupthandlingStateMachine.addState(&m_readMTCountactState);
    m_InterrupthandlingStateMachine.addState(&m_setECResultState);

    m_InterrupthandlingStateMachine.setInitialState(&m_readIntRegisterState);

    connect(&m_readIntRegisterState, SIGNAL(entered()), SLOT(readIntRegister()));
    connect(&m_resetIntRegisterState, SIGNAL(entered()), SLOT(resetIntRegister()));
    connect(&m_readMTCountactState, SIGNAL(entered()), SLOT(readMTCountact()));
    connect(&m_setECResultState, SIGNAL(entered()), SLOT(setECResult()));
}


cSec1ModuleMeasProgram::~cSec1ModuleMeasProgram()
{
    delete m_pRMInterface;
    m_pProxy->releaseConnection(m_pRMClient);
    delete m_pSECInterface;
    m_pProxy->releaseConnection(m_pSECClient);
    delete m_pPCBInterface;
    m_pProxy->releaseConnection(m_pPCBClient);
}


void cSec1ModuleMeasProgram::start()
{
    //connect(this, SIGNAL(actualValues(QVector<float>*)), this, SLOT(setInterfaceActualValues(QVector<float>*)));
}


void cSec1ModuleMeasProgram::stop()
{
    //disconnect(this, SIGNAL(actualValues(QVector<float>*)), this, SLOT(setInterfaceActualValues(QVector<float>*)));
}


void cSec1ModuleMeasProgram::generateInterface()
{
    QString s;

    m_pDutinputEntity = m_pPeer->dataAdd(QString("PAR_DutInput")); // here is the actual dut input
    m_pDutinputEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pDutinputEntity->setValue(s = "Unknown", m_pPeer);
    m_EntityList.append(m_pDutinputEntity);

    m_pDutinputListEntity = m_pPeer->dataAdd(QString("INF_DutInputList")); // list of dut input sources
    m_pDutinputListEntity ->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pDutinputListEntity ->modifiersAdd(VeinEntity::MOD_READONLY);
    m_pDutinputListEntity ->setValue(s = "Unknown", m_pPeer);
    m_EntityList.append(m_pDutinputListEntity);

    m_pRefinputEntity = m_pPeer->dataAdd(QString("PAR_RefInput")); // here is the actual ref input
    m_pRefinputEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pRefinputEntity->setValue(s = "Unknown", m_pPeer);
    m_EntityList.append(m_pRefinputEntity);

    m_pRefinputListEntity = m_pPeer->dataAdd(QString("INF_RefInputList")); // list of ref input sources
    m_pRefinputListEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pRefinputListEntity->modifiersAdd(VeinEntity::MOD_READONLY);
    m_pRefinputListEntity->setValue(s = "Unknown", m_pPeer);
    m_EntityList.append(m_pRefinputListEntity);

    m_pRefConstantEntity = m_pPeer->dataAdd(QString("PAR_RefConstant")); // actual reference constant
    m_pRefConstantEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pRefConstantEntity->setValue(QVariant((double)0.0));
    m_EntityList.append(m_pRefConstantEntity);

    m_pRefConstantLimitsEntity = m_pPeer->dataAdd(QString("INF_RefConstant_LIMITS"));
    m_pRefConstantLimitsEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pRefConstantLimitsEntity->modifiersAdd(VeinEntity::MOD_READONLY);
    m_pRefConstantLimitsEntity->setValue(QVariant(QString("%1;%2").arg(1.0).arg(1.0e20)));
    m_EntityList.append(m_pRefConstantLimitsEntity);

    m_pDutConstantEntity = m_pPeer->dataAdd(QString("PAR_DutConstant")); // actual dut constant
    m_pDutConstantEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pDutConstantEntity->setValue(QVariant((double)0.0));
    m_EntityList.append(m_pDutConstantEntity);

    m_pDutConstantLimitsEntity = m_pPeer->dataAdd(QString("INF_DutConstant_LIMITS"));
    m_pDutConstantLimitsEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pDutConstantLimitsEntity->modifiersAdd(VeinEntity::MOD_READONLY);
    m_pDutConstantLimitsEntity->setValue(QVariant(QString("%1;%2").arg(1.0).arg(1.0e20)));
    m_EntityList.append(m_pDutConstantLimitsEntity);

    m_pMeasPulsesEntity = m_pPeer->dataAdd(QString("PAR_MeasPulses")); // actual meas pulses
    m_pMeasPulsesEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pMeasPulsesEntity->setValue(QVariant((double)0.0));
    m_EntityList.append(m_pMeasPulsesEntity);

    m_pMeasPulsesLimitsEntity = m_pPeer->dataAdd(QString("INF_MeasPulses_LIMITS"));
    m_pMeasPulsesLimitsEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pMeasPulsesLimitsEntity->modifiersAdd(VeinEntity::MOD_READONLY);
    m_pMeasPulsesLimitsEntity->setValue(QVariant(QString("%1;%2").arg(1).arg(4294967295)));
    m_EntityList.append(m_pMeasPulsesLimitsEntity);

    m_pTargetValueEntity = m_pPeer->dataAdd(QString("PAR_TargetValue")); // actual meas pulses
    m_pTargetValueEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pTargetValueEntity->setValue(QVariant((double)0.0));
    m_EntityList.append(m_pTargetValueEntity);

    m_pTargetValueLimitsEntity = m_pPeer->dataAdd(QString("INF_TargetValue_LIMITS"));
    m_pTargetValueLimitsEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pTargetValueLimitsEntity->modifiersAdd(VeinEntity::MOD_READONLY);
    m_pTargetValueLimitsEntity->setValue(QVariant(QString("%1;%2").arg(1).arg(4294967295)));
    m_EntityList.append(m_pTargetValueLimitsEntity);

    m_pStatusEntity = m_pPeer->dataAdd("ACT_Status");
    m_pStatusEntity->modifiersAdd(VeinEntity::MOD_READONLY);
    m_pStatusEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pStatusEntity->setValue(QVariant((int) ECALCSTATUS::IDLE), m_pPeer);
    m_EntityList.append(m_pStatusEntity);

    m_pProgressEntity = m_pPeer->dataAdd("ACT_Progress");
    m_pProgressEntity->modifiersAdd(VeinEntity::MOD_READONLY);
    m_pProgressEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pProgressEntity->setValue(QVariant((double) 0.0), m_pPeer);
    m_EntityList.append(m_pProgressEntity);

    m_pResultEntity = m_pPeer->dataAdd("ACT_Result");
    m_pResultEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pResultEntity->modifiersAdd(VeinEntity::MOD_READONLY);
    m_pResultEntity->setValue(QVariant((double)99.99), m_pPeer);
    m_EntityList.append(m_pResultEntity);

    m_pStartStopEntity = m_pPeer->dataAdd("PAR_STARTSTOP");
    m_pStartStopEntity->modifiersAdd(VeinEntity::MOD_NOECHO);
    m_pStartStopEntity->setValue(QVariant((int)0), m_pPeer);
    m_EntityList.append(m_pStartStopEntity);
}


void cSec1ModuleMeasProgram::deleteInterface()
{
    for (int i = 0; i < m_EntityList.count(); i++)
        m_pPeer->dataRemove(m_EntityList.at(i));
}


void cSec1ModuleMeasProgram::catchInterfaceAnswer(quint32 msgnr, quint8 reply, QVariant answer)
{
    bool ok;

    if (msgnr == 0) // 0 was reserved for async. messages
    {
        QString sintnr;
        // qDebug() << "meas program interrupt";
        sintnr = answer.toString().section(':', 1, 1);
        int service = sintnr.toInt(&ok);
        switch (service)
        {
        case irqPCBNotifier:
            // we must fetch the ref constant of the selected reference input
            handleChangedREFConst();
            break;
        case irqSECNotifier:
            // we must fetch the measured impuls count, compute the error and set corresponding entity
            handleSECInterrupt();
            break;
        }
    }
    else
    {
        if (m_MsgNrCmdList.contains(msgnr))
        {
            int cmd = m_MsgNrCmdList.take(msgnr);
            switch (cmd)
            {
            case sendrmident:
                if (reply == ack) // we only continue if resource manager acknowledges
                    emit activationContinue();
                else
                {
                    emit errMsg((tr(rmidentErrMSG)));
#ifdef DEBUG
                    qDebug() << rmidentErrMSG;
#endif
                    emit activationError();
                }
                break;

            case testsec1resource:
                if ((reply == ack) && (answer.toString().contains("SEC1")))
                    emit activationContinue();
                else
                {
                    emit errMsg((tr(resourcetypeErrMsg)));
#ifdef DEBUG
                    qDebug() << resourcetypeErrMsg;
#endif
                    emit activationError();
                }
                break;

            case setecresource:
                if (reply == ack)
                    emit activationContinue();
                else
                {
                    emit errMsg((tr(setresourceErrMsg)));
#ifdef DEBUG
                    qDebug() << setresourceErrMsg;
#endif
                    emit activationError();
                }
                break;

            case readresourcetypes:
                if (reply == ack)
                {
                    m_ResourceTypeList = answer.toString().split(';');
                    emit activationContinue();
                }
                else
                {
                    emit errMsg((tr(resourcetypeErrMsg)));
#ifdef DEBUG
                    qDebug() << resourcetypeErrMsg;
#endif
                    emit activationError();
                }
                break;

            case readresource:
            {
                if (reply == ack)
                {
                    m_ResourceHash[m_ResourceTypeList.at(m_nIt)] = answer.toString();
                    m_nIt++;
                    if (m_nIt < m_ResourceTypeList.count())
                        emit activationLoop();
                    else
                        emit activationContinue();
                }
                else
                {
                    emit errMsg((tr(resourceErrMsg)));
#ifdef DEBUG
                    qDebug() << resourceErrMsg;
#endif
                    emit activationError();
                }
                break;
            }

            case fetchecalcunits:
            {
                QStringList sl;
                sl = answer.toString().split(';');
                if ((reply == ack) && (sl.length() >= 2))
                {
                    m_MasterEcalculator.name = sl.at(0);
                    m_MasterEcalculator.secIFace = m_pSECInterface;
                    m_MasterEcalculator.secServersocket = m_ConfigData.m_SECServerSocket;
                    m_SlaveEcalculator.name = sl.at(1);
                    m_SlaveEcalculator.secIFace = m_pSECInterface;
                    m_SlaveEcalculator.secServersocket = m_ConfigData.m_SECServerSocket;
                }
                else
                {
                    emit errMsg((tr(fetchsececalcunitErrMsg)));
#ifdef DEBUG
                    qDebug() << fetchsececalcunitErrMsg;
#endif
                    emit activationError();
                }
                break;
            }

            case readrefinputalias:
            {
                if (reply == ack)
                {
                    siInfo.resource = answer.toString();
                    emit activationContinue();
                }
                else
                {
                    emit errMsg((tr(readaliasErrMsg)));
#ifdef DEBUG
                    qDebug() << readaliasErrMsg;
#endif
                    emit activationError();
                }
                break;
            }

            case readrefinputmuxchannel:
            {
                if (reply == ack)
                {
                    siInfo.muxchannel = answer.toInt(&ok);
                    mREFSecInputInfoHash[m_sIt] = siInfo;
                    if (m_sItList.isEmpty())
                        emit activationContinue();
                    else
                        emit activationLoop();
                }
                else
                {
                    emit errMsg((tr(readmuxchannelErrMsg)));
#ifdef DEBUG
                    qDebug() << readmuxchannelErrMsg;
#endif
                    emit activationError();
                }
                break;
            }

            case readdutinputalias:
            {
                if (reply == ack)
                {
                    siInfo.resource = answer.toString();
                    emit activationContinue();
                }
                else
                {
                    emit errMsg((tr(readaliasErrMsg)));
#ifdef DEBUG
                    qDebug() << readaliasErrMsg;
#endif
                    emit activationError();
                }
                break;
            }

            case readdutinputmuxchannel:
            {
                if (reply == ack)
                {
                    siInfo.muxchannel = answer.toInt(&ok);
                    mDUTSecInputInfoHash[m_sIt] = siInfo;
                    if (m_sItList.isEmpty())
                        emit activationContinue();
                    else
                        emit activationLoop();
                }
                else
                {
                    emit errMsg((tr(readmuxchannelErrMsg)));
#ifdef DEBUG
                    qDebug() << readmuxchannelErrMsg;
#endif
                    emit activationError();
                }
                break;
            }

            case freeecalcunits:
                if (reply == ack) // we only continue if sec server manager acknowledges
                    emit deactivationContinue();
                else
                {
                    emit errMsg((tr(freesececalcunitErrMsg)));
#ifdef DEBUG
                    qDebug() << freesececalcunitErrMsg;
#endif
                    emit deactivationError();
                }
                break;

            case freeecresource:
                if (reply == ack) // we only continue if resource manager acknowledges
                    emit deactivationContinue();
                else
                {
                    emit errMsg((tr(freeresourceErrMsg)));
#ifdef DEBUG
                    qDebug() << freeresourceErrMsg;
#endif
                    emit deactivationError();
                }
                break;

            case actualizeprogress:
            {
                if (reply == ack)
                {
                    quint32 target = m_pTargetValueEntity->getValue().toUInt(&ok);
                    m_nMTCNTact = answer.toUInt(&ok);
                    m_fProgress = 100.0 - (( 1.0*target - 1.0*m_nMTCNTact)/target)*100.0;
                    if (m_fProgress > 100.0)
                        m_fProgress = 100.0;
                    m_pProgressEntity->setValue(QVariant(m_fProgress), m_pPeer);
                }
                else
                {
                    {
                        emit errMsg((tr(readsecregisterErrMsg)));
#ifdef DEBUG
                        qDebug() << readsecregisterErrMsg;
#endif
                        emit executionError();
                    }
                }
                break;
            }


            case actualizestatus:
            {
                if (reply == ack)
                {
                    m_nStatus = answer.toUInt(&ok) & 7;
                    m_pStatusEntity->setValue(QVariant(m_nStatus), m_pPeer);
                }
                else
                {
                    {
                        emit errMsg((tr(readsecregisterErrMsg)));
#ifdef DEBUG
                        qDebug() << readsecregisterErrMsg;
#endif
                        emit executionError();
                    }
                }
                break;
            }

            case setsync:
                if (reply == ack)
                {
                    emit setupContinue();
                }
                else
                {
                    {
                        emit errMsg((tr(setsyncErrMsg)));
#ifdef DEBUG
                        qDebug() << setsyncErrMsg;
#endif
                        emit executionError();
                    }
                }
                break;

            case enableinterrupt:
            case setmeaspulses:
                if (reply == ack)
                {
                    emit setupContinue();
                }
                else
                {
                    {
                        emit errMsg((tr(writesecregisterErrMsg)));
#ifdef DEBUG
                        qDebug() << writesecregisterErrMsg;
#endif
                        emit executionError();
                    }
                }
                break;

            case setmastermux:
            case setslavemux:
                if (reply == ack)
                {
                    emit setupContinue();
                }
                else
                {
                    {
                        emit errMsg((tr(setmuxErrMsg)));
#ifdef DEBUG
                        qDebug() << setmuxErrMsg;
#endif
                        emit executionError();
                    }
                }
                break;

            case setmastermeasmode:
            case setslavemeasmode:
                if (reply == ack)
                {
                    emit setupContinue();
                }
                else
                {
                    {
                        emit errMsg((tr(setcmdidErrMsg)));
#ifdef DEBUG
                        qDebug() << setcmdidErrMsg;
#endif
                        emit executionError();
                    }
                }
                break;

            case startmeasurement:
                if (reply == ack)
                {
                    emit setupContinue();
                }
                else
                {
                    {
                        emit errMsg((tr(startmeasErrMsg)));
#ifdef DEBUG
                        qDebug() << startmeasErrMsg;
#endif
                        emit executionError();
                    }
                }
                break;

            case stopmeas:
                if (reply == ack)
                {
                }
                else
                {
                    {
                        emit errMsg((tr(stopmeasErrMsg)));
#ifdef DEBUG
                        qDebug() << stopmeasErrMsg;
#endif
                        emit executionError();
                    }
                }
                break;

            case setpcbrefconstantnotifier:
                if (reply == ack) // we only continue if pcb server acknowledges
                    emit activationContinue();
                else
                {
                    emit errMsg((tr(registerpcbnotifierErrMsg)));
#ifdef DEBUG
                    qDebug() << registerpcbnotifierErrMsg;
#endif
                    emit activationError();
                }
                break;

            case setsecintnotifier:
                if (reply == ack) // we only continue if sec server acknowledges
                    emit activationContinue();
                else
                {
                    emit errMsg((tr(registerpcbnotifierErrMsg)));
#ifdef DEBUG
                    qDebug() << registerpcbnotifierErrMsg;
#endif
                    emit activationError();
                }
                break;


            case fetchrefconstant:
            {
                if (reply == ack) // we only continue if sec server acknowledges
                {
                    double constant;
                    constant = answer.toDouble(&ok);
                    m_pRefConstantEntity->setValue(QVariant(constant), m_pPeer);
                }
                else
                {
                    emit errMsg((tr(readrefconstantErrMsg)));
#ifdef DEBUG
                    qDebug() << readrefconstantErrMsg;
#endif
                    emit executionError();
                }
                break;
            }

            case readintregister:
                if (reply == ack) // we only continue if sec server acknowledges
                {
                    m_nIntReg = answer.toInt(&ok) & 7;
                    emit interruptContinue();
                }
                else
                {
                    emit errMsg((tr(readsecregisterErrMsg)));
#ifdef DEBUG
                    qDebug() << readsecregisterErrMsg;
#endif
                    emit executionError();
                }
                break;
            case resetintregister:
                if (reply == ack) // we only continue if sec server acknowledges
                {
                    emit interruptContinue();
                }
                else
                {
                    emit errMsg((tr(writesecregisterErrMsg)));
#ifdef DEBUG
                    qDebug() << writesecregisterErrMsg;
#endif
                    emit executionError();
                }
                break;
            case readmtcount:
                if (reply == ack) // we only continue if sec server acknowledges
                {
                    m_nMTCNTfin = answer.toLongLong(&ok);
                    emit interruptContinue();
                }
                else
                {
                    emit errMsg((tr(readsecregisterErrMsg)));
#ifdef DEBUG
                    qDebug() << readsecregisterErrMsg;
#endif
                    emit executionError();
                }
                break;

            }
        }
    }
}


void cSec1ModuleMeasProgram::setInterfaceEntities()
{
    m_pDutinputEntity->setValue(QVariant(m_ConfigData.m_sDutInput.m_sPar), m_pPeer);
    m_pRefinputEntity->setValue(QVariant(m_ConfigData.m_sRefInput.m_sPar), m_pPeer);
    m_pDutConstantEntity->setValue(QVariant(m_ConfigData.m_fDutConstant.m_fPar), m_pPeer);
    m_pRefConstantEntity->setValue(QVariant(m_ConfigData.m_fRefConstant.m_fPar), m_pPeer);
    m_pMeasPulsesEntity->setValue(QVariant(m_ConfigData.m_nMeasPulses.m_nPar), m_pPeer);
    m_pTargetValueEntity->setValue(QVariant(m_ConfigData.m_nTargetValue.m_nPar), m_pPeer);
    m_pProgressEntity->setValue(QVariant(double(0.0)), m_pPeer);
    m_pDutinputListEntity->setValue(QVariant(m_DUTAliasList), m_pPeer);
    m_pRefinputListEntity->setValue(QVariant(m_REFAliasList), m_pPeer);
}


void cSec1ModuleMeasProgram::handleChangedREFConst()
{
    // we ask for the reference constant of the selected input
    m_MsgNrCmdList[m_pPCBInterface->getConstantSource(mREFSecInputInfoHash[m_ConfigData.m_sRefInput.m_sPar].name)] = fetchrefconstant;
    if ((m_nStatus == ECALCSTATUS::ARMED) || (m_nStatus == ECALCSTATUS::STARTED) )
    {
        m_MsgNrCmdList[m_pSECInterface->stop(m_MasterEcalculator.name)] = stopmeas;
        m_nStatus = ECALCSTATUS::ABORT;
        m_pStatusEntity->setValue(QVariant(m_nStatus),m_pPeer);
        m_pStartStopEntity->setValue(QVariant(0), m_pPeer);
        m_ActualizeTimer.stop();
    }
}


void cSec1ModuleMeasProgram::handleSECInterrupt()
{
    if (!m_InterrupthandlingStateMachine.isRunning())
        m_InterrupthandlingStateMachine.start();
}


void cSec1ModuleMeasProgram::resourceManagerConnect()
{
    // first we try to get a connection to resource manager over proxy
    m_pRMClient = m_pProxy->getConnection(m_ConfigData.m_RMSocket.m_sIP, m_ConfigData.m_RMSocket.m_nPort);
    // and then we set connection resource manager interface's connection
    m_pRMInterface->setClient(m_pRMClient); //
    resourceManagerConnectState.addTransition(m_pRMClient, SIGNAL(connected()), &m_IdentifyState);
    connect(m_pRMInterface, SIGNAL(serverAnswer(quint32, quint8, QVariant)), this, SLOT(catchInterfaceAnswer(quint32, quint8, QVariant)));
    // todo insert timer for timeout and/or connect error conditions
    m_pProxy->startConnection(m_pRMClient);
}


void cSec1ModuleMeasProgram::sendRMIdent()
{
    m_MsgNrCmdList[m_pRMInterface->rmIdent(QString("Sec1Module%1").arg(m_pModule->getModuleNr()))] = sendrmident;
}


void cSec1ModuleMeasProgram::testSEC1Resource()
{
    m_MsgNrCmdList[m_pRMInterface->getResourceTypes()] = testsec1resource;
}


void cSec1ModuleMeasProgram::setECResource()
{
    m_MsgNrCmdList[m_pRMInterface->setResource("SEC1", "ECALCULATOR", 2)] = setecresource;
}



void cSec1ModuleMeasProgram::readResourceTypes()
{
    //m_MsgNrCmdList[m_pRMInterface->getResourceTypes()] = readresourcetypes;
    // instead of taking all resourcetypes we take predefined types
    m_ResourceTypeList.append("FRQINPUT");
    m_ResourceTypeList.append("SOURCE");
    m_ResourceTypeList.append("SCHEAD");
    emit activationContinue();
}


void cSec1ModuleMeasProgram::readResources()
{
    m_nIt = 0; // we want to read all resources from resourcetypelist
    emit activationContinue();
}


void cSec1ModuleMeasProgram::readResource()
{
    QString resourcetype = m_ResourceTypeList.at(m_nIt);
    m_MsgNrCmdList[m_pRMInterface->getResources(resourcetype)] = readresource;
}


void cSec1ModuleMeasProgram::testSecInputs()
{
    // first we build up a list with properties for all configured inputs
    for (int i = 0; i < m_ConfigData.m_refInpList.count(); i++)
        mREFSecInputInfoHash[m_ConfigData.m_refInpList.at(i)] = siInfo;
    for (int i = 0; i < m_ConfigData.m_dutInpList.count(); i++)
        mDUTSecInputInfoHash[m_ConfigData.m_dutInpList.at(i)] = siInfo;

    qint32 nref, ndut;
    QList<QString> inputNameList;

    nref = mREFSecInputInfoHash.count(); // we have n configured ref inputs
    inputNameList = mREFSecInputInfoHash.keys();

    for (int i = 0; i < m_ResourceTypeList.count(); i++)
    {
        QString resourcelist;
        resourcelist = m_ResourceHash[m_ResourceTypeList.at(i)];
        for (int j = 0; j < inputNameList.count(); j++)
        {
            QString name;
            name = inputNameList.at(j);
            if (resourcelist.contains(name))
            {
                nref--;
                siInfo = mREFSecInputInfoHash.take(name);
                siInfo.name = name;
                siInfo.resource = m_ResourceTypeList.at(i);
                mREFSecInputInfoHash[name] = siInfo;
            }
        }
    }

    ndut = mDUTSecInputInfoHash.count(); // we have n configured ref inputs
    inputNameList = mDUTSecInputInfoHash.keys();

    for (int i = 0; i < m_ResourceTypeList.count(); i++)
    {
        QString resourcelist;
        resourcelist = m_ResourceHash[m_ResourceTypeList.at(i)];
        for (int j = 0; j < inputNameList.count(); j++)
        {
            QString name;
            name = inputNameList.at(j);
            if (resourcelist.contains(name))
            {
                ndut--;
                siInfo = mDUTSecInputInfoHash.take(name);
                siInfo.name = name;
                siInfo.resource = m_ResourceTypeList.at(i);
                mDUTSecInputInfoHash[name] = siInfo;
            }
        }
    }

    if ((nref == 0) && (ndut == 0)) // we found all our configured inputs
    {
        emit activationContinue(); // so lets go on
    }
    else
    {
        emit errMsg((tr(resourceErrMsg)));
        #ifdef DEBUG
            qDebug() << resourceErrMsg;
        #endif
        emit activationError();
    }

}


void cSec1ModuleMeasProgram::ecalcServerConnect()
{
    // we try to get a connection to ecalc server over proxy
    m_pSECClient = m_pProxy->getConnection(m_ConfigData.m_SECServerSocket.m_sIP, m_ConfigData.m_SECServerSocket.m_nPort);
    // and then we set ecalcalculator interface's connection
    m_pSECInterface->setClient(m_pSECClient); //
    m_ecalcServerConnectState.addTransition(m_pSECClient, SIGNAL(connected()), &m_fetchECalcUnitsState);
    connect(m_pSECInterface, SIGNAL(serverAnswer(quint32, quint8, QVariant)), this, SLOT(catchInterfaceAnswer(quint32, quint8, QVariant)));
    // todo insert timer for timeout and/or connect error conditions
    m_pProxy->startConnection(m_pSECClient);
}


void cSec1ModuleMeasProgram::fetchECalcUnits()
{
    m_MsgNrCmdList[m_pSECInterface->setECalcUnit(2)] = fetchecalcunits;
}


void cSec1ModuleMeasProgram::pcbServerConnect()
{
    // we try to get a connection to ecalc server over proxy
    m_pPCBClient = m_pProxy->getConnection(m_ConfigData.m_PCBServerSocket.m_sIP, m_ConfigData.m_PCBServerSocket.m_nPort);
    // and then we set ecalcalculator interface's connection
    m_pPCBInterface->setClient(m_pPCBClient); //
    m_pcbServerConnectState.addTransition(m_pPCBClient, SIGNAL(connected()), &m_readREFInputsState);
    connect(m_pPCBInterface, SIGNAL(serverAnswer(quint32, quint8, QVariant)), this, SLOT(catchInterfaceAnswer(quint32, quint8, QVariant)));
    // todo insert timer for timeout and/or connect error conditions
    m_pProxy->startConnection(m_pPCBClient);
}


void cSec1ModuleMeasProgram::readREFInputs()
{
    m_sItList = mREFSecInputInfoHash.keys();
    emit activationContinue();
}


void cSec1ModuleMeasProgram::readREFInputAlias()
{
    m_sIt = m_sItList.takeFirst();
    siInfo = mREFSecInputInfoHash.take(m_sIt); // if set some info that could be useful later
    siInfo.pcbIFace = m_pPCBInterface; // in case that inputs would be provided by several servers
    siInfo.pcbServersocket = m_ConfigData.m_PCBServerSocket;
    m_MsgNrCmdList[siInfo.pcbIFace->resourceAliasQuery(siInfo.resource, m_sIt)] = readrefinputalias;
}


void cSec1ModuleMeasProgram::readREFInputMuxchannel()
{
    m_MsgNrCmdList[siInfo.pcbIFace->resourceMuxChannelQuery(siInfo.resource, m_sIt)] = readrefinputmuxchannel;
}


void cSec1ModuleMeasProgram::readDUTInputs()
{
    m_sItList = mDUTSecInputInfoHash.keys();
    emit activationContinue();
}


void cSec1ModuleMeasProgram::readDUTInputAlias()
{
    m_sIt = m_sItList.takeFirst();
    siInfo = mDUTSecInputInfoHash.take(m_sIt); // if set some info that could be useful later
    siInfo.pcbIFace = m_pPCBInterface; // in case that inputs would be provided by several servers
    siInfo.pcbServersocket = m_ConfigData.m_PCBServerSocket;
    m_MsgNrCmdList[siInfo.pcbIFace->resourceAliasQuery(siInfo.resource, m_sIt)] = readdutinputalias;
}


void cSec1ModuleMeasProgram::readDUTInputMuxchannel()
{
    m_MsgNrCmdList[siInfo.pcbIFace->resourceMuxChannelQuery(siInfo.resource, m_sIt)] = readdutinputmuxchannel;
}


void cSec1ModuleMeasProgram::setpcbREFConstantNotifier()
{
    if (m_ConfigData.m_bwithrefconstantnotifier)
    {
        m_MsgNrCmdList[m_pPCBInterface->registerNotifier(QString("SOURCE:fo0:CONSTANT?"),QString("%1").arg(irqPCBNotifier))] = setpcbrefconstantnotifier;
        // todo also configure the query for setting this notofier .....very flexible
    }
    else
        emit activationContinue(); // if no ref constant notifier (standalone error calc) we directly go on
}


void cSec1ModuleMeasProgram::setsecINTNotifier()
{
    m_MsgNrCmdList[m_pSECInterface->registerNotifier(QString("ECALCULATOR:%1:register? %2;").arg(m_MasterEcalculator.name).arg(ECALCREG::INTREG), QString("%1").arg(irqSECNotifier))] = setsecintnotifier;
}


void cSec1ModuleMeasProgram::activationDone()
{
    for (int i = 0; i < m_ConfigData.m_refInpList.count(); i++)
    {
        m_REFAliasList.append(mREFSecInputInfoHash[m_ConfigData.m_refInpList.at(i)].alias); // build up a fixed sorted list of alias
        siInfo = mREFSecInputInfoHash.take(m_ConfigData.m_refInpList.at(i)); // change the hash for access via alias
        mREFSecInputInfoHash[siInfo.alias] = siInfo;
    }

    for (int i = 0; i < m_ConfigData.m_dutInpList.count(); i++)
    {
        m_DUTAliasList.append(mDUTSecInputInfoHash[m_ConfigData.m_dutInpList.at(i)].alias); // build up a fixed sorted list of alias
        siInfo = mDUTSecInputInfoHash.take(m_ConfigData.m_dutInpList.at(i)); // change the hash for access via alias
        mDUTSecInputInfoHash[siInfo.alias] = siInfo;
    }

    m_bActive = true;
    connect(&m_ActualizeTimer, SIGNAL(timeout()), this, SLOT(Actualize()));
    connect(m_pStartStopEntity, SIGNAL(sigValueChanged(QVariant)), this, SLOT(newStartStop(QVariant)));
    emit activated();
}


void cSec1ModuleMeasProgram::freeECalculator()
{
    m_bActive = false;
    m_MsgNrCmdList[m_pSECInterface->freeECalcUnits()] = freeecalcunits;
}


void cSec1ModuleMeasProgram::freeECResource()
{
    m_MsgNrCmdList[m_pRMInterface->freeResource("SEC1", "ECALCULATOR")] = freeecresource;
}


void cSec1ModuleMeasProgram::deactivationDone()
{
    m_pProxy->releaseConnection(m_pRMClient);
    disconnect(m_pRMInterface, 0, this, 0);
    disconnect(m_pSECInterface, 0, this, 0);
    disconnect(m_pPCBInterface, 0, this, 0);
    emit deactivated();
}


void cSec1ModuleMeasProgram::setSync()
{
    m_MsgNrCmdList[m_pSECInterface->setSync(m_SlaveEcalculator.name, m_MasterEcalculator.name)] = setsync;
}


void cSec1ModuleMeasProgram::setMeaspulses()
{
    m_MsgNrCmdList[m_pSECInterface->writeRegister(m_MasterEcalculator.name, ECALCREG::MTCNTin, m_pMeasPulsesEntity->getValue().toLongLong())] = setmeaspulses;
}


void cSec1ModuleMeasProgram::setMasterMux()
{
    m_MsgNrCmdList[m_pSECInterface->setMux(m_MasterEcalculator.name, mDUTSecInputInfoHash[m_pDutinputEntity->getValue().toString()].muxchannel)] = setmastermux;
}


void cSec1ModuleMeasProgram::setSlaveMux()
{
    m_MsgNrCmdList[m_pSECInterface->setMux(m_SlaveEcalculator.name, mREFSecInputInfoHash[m_pRefinputEntity->getValue().toString()].muxchannel)] = setslavemux;

}


void cSec1ModuleMeasProgram::setMasterMeasMode()
{
    m_MsgNrCmdList[m_pSECInterface->setCmdid(m_MasterEcalculator.name, ECALCCMDID::ERRORMEASMASTER)] = setmastermeasmode;
}


void cSec1ModuleMeasProgram::setSlaveMeasMode()
{
    m_MsgNrCmdList[m_pSECInterface->setCmdid(m_SlaveEcalculator.name, ECALCCMDID::ERRORMEASSLAVE)] = setslavemeasmode;

}


void cSec1ModuleMeasProgram::enableInterrupt()
{
    m_MsgNrCmdList[m_pSECInterface->writeRegister(m_MasterEcalculator.name, ECALCREG::INTMASK, ECALCINT::MTCount0)] = enableinterrupt;
}


void cSec1ModuleMeasProgram::startMeasurement()
{
    m_MsgNrCmdList[m_pSECInterface->start(m_MasterEcalculator.name)] = startmeasurement;
}


void cSec1ModuleMeasProgram::startMeasurementDone()
{
    Actualize(); // we acualize at once after started
    m_ActualizeTimer.start(m_ConfigData.m_fMeasInterval*1000.0); // and after configured interval
}


void cSec1ModuleMeasProgram::readIntRegister()
{
    m_MsgNrCmdList[m_pSECInterface->writeRegister(m_MasterEcalculator.name, ECALCREG::INTREG, ECALCINT::MTCount0)] = readintregister;
}


void cSec1ModuleMeasProgram::resetIntRegister()
{
    m_MsgNrCmdList[m_pSECInterface->writeRegister(m_MasterEcalculator.name, ECALCREG::INTREG, m_nIntReg)] = resetintregister;
}


void cSec1ModuleMeasProgram::readMTCountact()
{
    m_MsgNrCmdList[m_pSECInterface->readRegister(m_SlaveEcalculator.name, ECALCREG::MTCNTfin)] = readmtcount;
}


void cSec1ModuleMeasProgram::setECResult()
{
    m_nStatus = ECALCSTATUS::READY;
    m_fResult = (m_nTargetValue - m_nMTCNTfin) * 100.0 / m_nTargetValue;
    m_pStatusEntity->setValue(QVariant(m_nStatus),m_pPeer);
    m_pTargetValueEntity->setValue(QVariant(m_fResult), m_pPeer);
    m_pStartStopEntity->setValue(QVariant(0), m_pPeer); // restart enable
}


void cSec1ModuleMeasProgram::newStartStop(QVariant startstop)
{
    bool ok;
    int ss = startstop.toInt(&ok);
    if (ss > 0) // we get started
    {
        if (!m_startMeasurementMachine.isRunning())
            m_startMeasurementMachine.start();
        // setsync
        // mtvorwahl setzen
        // master -> input mux setzen
        // slave -> input mux setzen
        // meas mode setzen + arm
        // m_ActualizeTimer.start(m_ConfigData.m_fMeasInterval*1000.0);
    }
    else
    {
        m_MsgNrCmdList[m_pSECInterface->stop(m_MasterEcalculator.name)] = stopmeas;
        m_ActualizeTimer.stop();
    }
}


void cSec1ModuleMeasProgram::Actualize()
{
    m_MsgNrCmdList[m_pSECInterface->readRegister(m_MasterEcalculator.name, ECALCREG::MTCNTact)] = actualizeprogress;
    m_MsgNrCmdList[m_pSECInterface->readRegister(m_MasterEcalculator.name, ECALCREG::STATUS)] = actualizestatus;

}

}














