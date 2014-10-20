#include <QString>
#include <QStateMachine>
#include <rminterface.h>
#include <dspinterface.h>
#include <pcbinterface.h>
#include <proxy.h>
#include <proxyclient.h>
#include <veinpeer.h>
#include <veinentity.h>
#include <QPointF>

#include "debug.h"
#include "errormessages.h"
#include "reply.h"
#include "modulesignal.h"
#include "moduleparameter.h"
#include "moduleinfo.h"
#include "thdnmodule.h"
#include "thdnmoduleconfigdata.h"
#include "thdnmodulemeasprogram.h"

namespace THDNMODULE
{

cThdnModuleMeasProgram::cThdnModuleMeasProgram(cThdnModule* module, Zera::Proxy::cProxy* proxy, VeinPeer* peer, Zera::Server::cDSPInterface* iface, cThdnModuleConfigData& configdata)
    :cBaseMeasProgram(proxy, peer, iface), m_pModule(module), m_ConfigData(configdata)
{
    m_pRMInterface = new Zera::Server::cRMInterface();
    m_ActValueList = m_ConfigData.m_valueChannelList;

    m_IdentifyState.addTransition(this, SIGNAL(activationContinue()), &m_readResourceTypesState);
    m_readResourceTypesState.addTransition(this, SIGNAL(activationContinue()), &m_readResourceState);
    m_readResourceState.addTransition(this, SIGNAL(activationContinue()), &m_readResourceInfosState);
    m_readResourceInfosState.addTransition(this, SIGNAL(activationContinue()), &m_readResourceInfoState);
    m_readResourceInfoState.addTransition(this, SIGNAL(activationContinue()), &m_readResourceInfoDoneState);
    m_readResourceInfoDoneState.addTransition(this, SIGNAL(activationContinue()), &m_pcbserverConnectState);
    m_readResourceInfoDoneState.addTransition(this, SIGNAL(activationLoop()), &m_readResourceInfoState);
    m_pcbserverConnectState.addTransition(this, SIGNAL(activationContinue()), &m_readSampleRateState);
    m_readSampleRateState.addTransition(this, SIGNAL(activationContinue()), &m_readChannelInformationState);
    m_readChannelInformationState.addTransition(this, SIGNAL(activationContinue()), &m_readChannelAliasState);
    m_readChannelAliasState.addTransition(this, SIGNAL(activationContinue()), &m_readChannelUnitState);
    m_readChannelUnitState.addTransition(this, SIGNAL(activationContinue()), &m_readDspChannelState);
    m_readDspChannelState.addTransition(this, SIGNAL(activationContinue()), &m_readDspChannelDoneState);
    m_readDspChannelDoneState.addTransition(this, SIGNAL(activationContinue()), &m_claimPGRMemState);
    m_readDspChannelDoneState.addTransition(this, SIGNAL(activationLoop()), &m_readChannelAliasState);
    m_claimPGRMemState.addTransition(this, SIGNAL(activationContinue()), &m_claimUSERMemState);
    m_claimUSERMemState.addTransition(this, SIGNAL(activationContinue()), &m_var2DSPState);
    m_var2DSPState.addTransition(this, SIGNAL(activationContinue()), &m_cmd2DSPState);
    m_cmd2DSPState.addTransition(this, SIGNAL(activationContinue()), &m_activateDSPState);
    m_activateDSPState.addTransition(this, SIGNAL(activationContinue()), &m_loadDSPDoneState);

    m_activationMachine.addState(&m_resourceManagerConnectState);
    m_activationMachine.addState(&m_IdentifyState);
    m_activationMachine.addState(&m_readResourceTypesState);
    m_activationMachine.addState(&m_readResourceState);
    m_activationMachine.addState(&m_readResourceInfosState);
    m_activationMachine.addState(&m_readResourceInfoState);
    m_activationMachine.addState(&m_readResourceInfoDoneState);
    m_activationMachine.addState(&m_pcbserverConnectState);
    m_activationMachine.addState(&m_readSampleRateState);
    m_activationMachine.addState(&m_readChannelInformationState);
    m_activationMachine.addState(&m_readChannelAliasState);
    m_activationMachine.addState(&m_readChannelUnitState);
    m_activationMachine.addState(&m_readDspChannelState);
    m_activationMachine.addState(&m_readDspChannelDoneState);
    m_activationMachine.addState(&m_claimPGRMemState);
    m_activationMachine.addState(&m_claimUSERMemState);
    m_activationMachine.addState(&m_var2DSPState);
    m_activationMachine.addState(&m_cmd2DSPState);
    m_activationMachine.addState(&m_activateDSPState);
    m_activationMachine.addState(&m_loadDSPDoneState);

    m_activationMachine.setInitialState(&m_resourceManagerConnectState);

    connect(&m_resourceManagerConnectState, SIGNAL(entered()), SLOT(resourceManagerConnect()));
    connect(&m_IdentifyState, SIGNAL(entered()), SLOT(sendRMIdent()));
    connect(&m_readResourceTypesState, SIGNAL(entered()), SLOT(readResourceTypes()));
    connect(&m_readResourceState, SIGNAL(entered()), SLOT(readResource()));
    connect(&m_readResourceInfosState, SIGNAL(entered()), SLOT(readResourceInfos()));
    connect(&m_readResourceInfoState, SIGNAL(entered()), SLOT(readResourceInfo()));
    connect(&m_readResourceInfoDoneState, SIGNAL(entered()), SLOT(readResourceInfoDone()));
    connect(&m_pcbserverConnectState, SIGNAL(entered()), SLOT(pcbserverConnect()));
    connect(&m_readSampleRateState, SIGNAL(entered()), SLOT(readSampleRate()));
    connect(&m_readChannelInformationState, SIGNAL(entered()), SLOT(readChannelInformation()));
    connect(&m_readChannelAliasState, SIGNAL(entered()), SLOT(readChannelAlias()));
    connect(&m_readChannelUnitState, SIGNAL(entered()), SLOT(readChannelUnit()));
    connect(&m_readDspChannelState, SIGNAL(entered()), SLOT(readDspChannel()));
    connect(&m_readDspChannelDoneState, SIGNAL(entered()), SLOT(readDspChannelDone()));
    connect(&m_claimPGRMemState, SIGNAL(entered()), SLOT(claimPGRMem()));
    connect(&m_claimUSERMemState, SIGNAL(entered()), SLOT(claimUSERMem()));
    connect(&m_var2DSPState, SIGNAL(entered()), SLOT(varList2DSP()));
    connect(&m_cmd2DSPState, SIGNAL(entered()), SLOT(cmdList2DSP()));
    connect(&m_activateDSPState, SIGNAL(entered()), SLOT(activateDSP()));
    connect(&m_loadDSPDoneState, SIGNAL(entered()), SLOT(activateDSPdone()));

    // setting up statemachine for unloading dsp and setting resources free
    m_deactivateDSPState.addTransition(this, SIGNAL(deactivationContinue()), &m_freePGRMemState);
    m_freePGRMemState.addTransition(this, SIGNAL(deactivationContinue()), &m_freeUSERMemState);
    m_freeUSERMemState.addTransition(this, SIGNAL(deactivationContinue()), &m_unloadDSPDoneState);
    m_deactivationMachine.addState(&m_deactivateDSPState);
    m_deactivationMachine.addState(&m_freePGRMemState);
    m_deactivationMachine.addState(&m_freeUSERMemState);
    m_deactivationMachine.addState(&m_unloadDSPDoneState);

    m_deactivationMachine.setInitialState(&m_deactivateDSPState);

    connect(&m_deactivateDSPState, SIGNAL(entered()), SLOT(deactivateDSP()));
    connect(&m_freePGRMemState, SIGNAL(entered()), SLOT(freePGRMem()));
    connect(&m_freeUSERMemState, SIGNAL(entered()), SLOT(freeUSERMem()));
    connect(&m_unloadDSPDoneState, SIGNAL(entered()), SLOT(deactivateDSPdone()));

    // setting up statemachine for data acquisition
    m_dataAcquisitionState.addTransition(this, SIGNAL(activationContinue()), &m_dataAcquisitionDoneState);
    m_dataAcquisitionMachine.addState(&m_dataAcquisitionState);
    m_dataAcquisitionMachine.addState(&m_dataAcquisitionDoneState);
    m_dataAcquisitionMachine.setInitialState(&m_dataAcquisitionState);
    connect(&m_dataAcquisitionState, SIGNAL(entered()), SLOT(dataAcquisitionDSP()));
    connect(&m_dataAcquisitionDoneState, SIGNAL(entered()), SLOT(dataReadDSP()));
}


cThdnModuleMeasProgram::~cThdnModuleMeasProgram()
{
    delete m_pRMInterface;
    for (int i = 0; i < m_pcbIFaceList.count(); i++)
    {
        delete m_pcbIFaceList.at(i);
        m_pProxy->releaseConnection(m_pcbClientList.at(i));
    }
    m_pProxy->releaseConnection(m_pRMClient);
}


void cThdnModuleMeasProgram::start()
{
    connect(this, SIGNAL(actualValues(QVector<float>*)), this, SLOT(setInterfaceActualValues(QVector<float>*)));
}


void cThdnModuleMeasProgram::stop()
{
    disconnect(this, SIGNAL(actualValues(QVector<float>*)), this, SLOT(setInterfaceActualValues(QVector<float>*)));
}


void cThdnModuleMeasProgram::generateInterface()
{
    VeinEntity* p_entity;
    QString s;

    // this here is for translation purpose
    s = tr("UL%1;[%]");
    s = tr("IL%1;[%]");
    s = tr("REF%1;[%]");

    int n = m_ActValueList.count();

    for (int i = 0; i < n; i++)
    {

        s = QString("TRA_THDN%1Name").arg(i+1);
        p_entity = m_pPeer->dataAdd(s);
        p_entity->modifiersAdd(VeinEntity::MOD_READONLY);
        p_entity->modifiersAdd(VeinEntity::MOD_NOECHO);
        p_entity->setValue(QVariant("Unknown"), m_pPeer);
        m_EntityNamePNList.append(p_entity);
        m_EntityNameList.append(p_entity);

        s = QString("ACT_THDN%1").arg(i+1);
        p_entity = m_pPeer->dataAdd(s);
        p_entity->modifiersAdd(VeinEntity::MOD_READONLY);
        p_entity->modifiersAdd(VeinEntity::MOD_NOECHO);
        p_entity->setValue(QVariant((double) 0.0), m_pPeer);
        m_EntityActValuePNList.append(p_entity);
        m_EntityActValueList.append(p_entity);
    }

    m_pThdnCountInfo = new cModuleInfo(m_pPeer, "INF_THDNCount", QVariant(n));

    m_pIntegrationTimeParameter = new cModuleParameter(m_pPeer, "PAR_INTEGRATIONTIME", QVariant((double) m_ConfigData.m_fMeasInterval.m_fValue));
    m_pIntegrationTimeLimits = new cModuleInfo(m_pPeer, "PAR_INTEGRATIONTIME_LIMITS", QVariant(QString("%1;%2").arg(0.1).arg(100.0)));
    m_pMeasureSignal = new cModuleSignal(m_pPeer, "SIG_MEASURING", QVariant(0));
}


void cThdnModuleMeasProgram::deleteInterface()
{
    for (int i = 0; i < m_EntityNamePNList.count(); i++)
        m_pPeer->dataRemove(m_EntityNameList.at(i));
    for (int i = 0; i < m_EntityNamePPList.count(); i++)
        m_pPeer->dataRemove(m_EntityNameList.at(i));
    for (int i = 0; i < m_EntityActValuePNList.count(); i++)
        m_pPeer->dataRemove(m_EntityActValueList.at(i));
    for (int i = 0; i < m_EntityActValuePPList.count(); i++)
        m_pPeer->dataRemove(m_EntityActValueList.at(i));

    m_EntityNameList.clear();
    m_EntityActValueList.clear();

    delete m_pThdnCountInfo;
    delete m_pIntegrationTimeParameter;
    delete m_pIntegrationTimeLimits;
    delete m_pMeasureSignal;
}


void cThdnModuleMeasProgram::setDspVarList()
{
    // we fetch a handle for sampled data and other temporary values
    m_pTmpDataDsp = m_pDSPIFace->getMemHandle("TmpData");
    m_pTmpDataDsp->addVarItem( new cDspVar("MEASSIGNAL", m_nSRate, DSPDATA::vDspTemp));
    m_pTmpDataDsp->addVarItem( new cDspVar("VALXTHDN",m_ActValueList.count(), DSPDATA::vDspTemp));
    m_pTmpDataDsp->addVarItem( new cDspVar("FILTER",2*m_ActValueList.count(),DSPDATA::vDspTemp));
    m_pTmpDataDsp->addVarItem( new cDspVar("N",1,DSPDATA::vDspTemp));

    // a handle for parameter
    m_pParameterDSP =  m_pDSPIFace->getMemHandle("Parameter");
    m_pParameterDSP->addVarItem( new cDspVar("TIPAR",1, DSPDATA::vDspParam, DSPDATA::dInt)); // integrationtime res = 1ms
    // we use tistart as parameter, so we can finish actual measuring interval bei setting 0
    m_pParameterDSP->addVarItem( new cDspVar("TISTART",1, DSPDATA::vDspTemp, DSPDATA::dInt));

    // and one for filtered actual values
    m_pActualValuesDSP = m_pDSPIFace->getMemHandle("ActualValues");
    m_pActualValuesDSP->addVarItem( new cDspVar("VALXTHDNF",m_ActValueList.count(), DSPDATA::vDspResult));

    m_ModuleActualValues.resize(m_pActualValuesDSP->getSize()); // we provide a vector for generated actual values
    m_nDspMemUsed = m_pTmpDataDsp->getSize() + m_pParameterDSP->getSize() + m_pActualValuesDSP->getSize();
}


void cThdnModuleMeasProgram::deleteDspVarList()
{
    m_pDSPIFace->deleteMemHandle(m_pTmpDataDsp);
    m_pDSPIFace->deleteMemHandle(m_pParameterDSP);
    m_pDSPIFace->deleteMemHandle(m_pActualValuesDSP);
}


void cThdnModuleMeasProgram::setDspCmdList()
{
    QString s;

    m_pDSPIFace->addCycListItem( s = "STARTCHAIN(1,1,0x0101)"); // aktiv, prozessnr. (dummy),hauptkette 1 subkette 1 start
        m_pDSPIFace->addCycListItem( s = QString("CLEARN(%1,MEASSIGNAL)").arg(m_nSRate) ); // clear meassignal
        m_pDSPIFace->addCycListItem( s = QString("CLEARN(%1,FILTER)").arg(2*m_ActValueList.count()+1) ); // clear the whole filter incl. count
        m_pDSPIFace->addCycListItem( s = QString("SETVAL(TIPAR,%1)").arg(m_ConfigData.m_fMeasInterval.m_fValue*1000.0)); // initial ti time  /* todo variabel */
        m_pDSPIFace->addCycListItem( s = "GETSTIME(TISTART)"); // einmal ti start setzen
        m_pDSPIFace->addCycListItem( s = "DEACTIVATECHAIN(1,0x0101)"); // ende prozessnr., hauptkette 1 subkette 1
    m_pDSPIFace->addCycListItem( s = "STOPCHAIN(1,0x0101)"); // ende prozessnr., hauptkette 1 subkette 1

    // we compute or copy our wanted actual values
    for (int i = 0; i < m_ActValueList.count(); i++)
    {
        m_pDSPIFace->addCycListItem( s = QString("COPYDATA(CH%1,0,MEASSIGNAL)").arg(m_measChannelInfoHash.value(m_ActValueList.at(i)).dspChannelNr));
        m_pDSPIFace->addCycListItem( s = QString("THDN(MEASSIGNAL,VALXTHDN+%1)").arg(i));
    }

    // and filter them
    m_pDSPIFace->addCycListItem( s = QString("AVERAGE1(%1,VALXTHDN,FILTER)").arg(m_ActValueList.count())); // we add results to filter

    m_pDSPIFace->addCycListItem( s = "TESTTIMESKIPNEX(TISTART,TIPAR)");
    m_pDSPIFace->addCycListItem( s = "ACTIVATECHAIN(1,0x0102)");

    m_pDSPIFace->addCycListItem( s = "STARTCHAIN(0,1,0x0102)");
        m_pDSPIFace->addCycListItem( s = "GETSTIME(TISTART)"); // set new system time
        m_pDSPIFace->addCycListItem( s = QString("CMPAVERAGE1(%1,FILTER,VALXTHDNF)").arg(m_ActValueList.count()));
        m_pDSPIFace->addCycListItem( s = QString("CLEARN(%1,FILTER)").arg(2*m_ActValueList.count()+1) );
        m_pDSPIFace->addCycListItem( s = "DSPINTTRIGGER(0x0,0x0001)"); // send interrupt to module
        m_pDSPIFace->addCycListItem( s = "DEACTIVATECHAIN(1,0x0102)");
    m_pDSPIFace->addCycListItem( s = "STOPCHAIN(1,0x0102)"); // end processnr., mainchain 1 subchain 2
}


void cThdnModuleMeasProgram::deleteDspCmdList()
{
    m_pDSPIFace->clearCmdList();
}


void cThdnModuleMeasProgram::catchInterfaceAnswer(quint32 msgnr, quint8 reply, QVariant answer)
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
        case 1:
            // we got an interrupt from our cmd chain and have to fetch our actual values
            // but we synchronize on ranging process
            if (m_bActive && !m_dataAcquisitionMachine.isRunning()) // in case of deactivation in progress, no dataaquisition
                m_dataAcquisitionMachine.start();
            break;
        }
    }
    else
    {
        // maybe other objexts share the same dsp interface
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
                    emit errMsg(tr(rmidentErrMSG));
#ifdef DEBUG
                    qDebug() << rmidentErrMSG;
#endif
                    emit activationError();
                }
                break;
            case claimpgrmem:
                if (reply == ack) // we only continue if resource manager acknowledges
                    emit activationContinue();
                else
                {
                    emit errMsg((tr(claimresourceErrMsg)));
#ifdef DEBUG
                    qDebug() << claimresourceErrMsg;
#endif
                    emit activationError();
                }
                break;
            case claimusermem:
                if (reply == ack) // we only continue if resource manager acknowledges
                    emit activationContinue();
                else
                {
                    emit errMsg((tr(claimresourceErrMsg)));
#ifdef DEBUG
                    qDebug() << claimresourceErrMsg;
#endif
                    emit activationError();
                }
                break;
            case varlist2dsp:
                if (reply == ack) // we only continue if resource manager acknowledges
                    emit activationContinue();
                else
                {
                    emit errMsg((tr(dspvarlistwriteErrMsg)));
#ifdef DEBUG
                    qDebug() << dspvarlistwriteErrMsg;
#endif
                    emit activationError();
                }
                break;
            case cmdlist2dsp:
                if (reply == ack) // we only continue if resource manager acknowledges
                    emit activationContinue();
                else
                {
                    emit errMsg((tr(dspcmdlistwriteErrMsg)));
#ifdef DEBUG
                    qDebug() << dspcmdlistwriteErrMsg;
#endif
                    emit activationError();
                }
                break;
            case activatedsp:
                if (reply == ack) // we only continue if resource manager acknowledges
                    emit activationContinue();
                else
                {
                    emit errMsg((tr(dspactiveErrMsg)));
#ifdef DEBUG
                    qDebug() << dspactiveErrMsg;
#endif
                    emit activationError();
                }
                break;

            case readresourcetypes:
                if ((reply == ack) && (answer.toString().contains("SENSE")))
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

            case readresource:
            {
                if (reply == ack)
                {
                    bool allfound = true;
                    QList<QString> sl = m_measChannelInfoHash.keys();
                    QString s = answer.toString();
                    for (int i = 0; i < sl.count(); i++)
                    {
                        if (!s.contains(sl.at(i)))
                            allfound = false;
                    }

                    if (allfound)
                        emit activationContinue();
                    else
                    {
                        emit errMsg((tr(resourceErrMsg)));
#ifdef DEBUG
                        qDebug() << resourceErrMsg;
#endif
                        emit activationError();
                    }
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

            case readresourceinfo:

            {
                int port;
                QStringList sl;
                cMeasChannelInfo mi;

                sl = answer.toString().split(';');
                if ((reply == ack) && (sl.length() >= 4))
                {
                    port = sl.at(3).toInt(&ok); // we have to set the port where we can find our meas channel
                    if (ok)
                    {
                        mi = m_measChannelInfoHash.take(channelInfoRead);
                        mi.socket.m_nPort = port;
                        m_measChannelInfoHash[channelInfoRead] = mi;
                        emit activationContinue();
                    }
                    else
                    {
                        emit errMsg((tr(resourceInfoErrMsg)));
#ifdef DEBUG
                        qDebug() << resourceInfoErrMsg;
#endif
                        emit activationError();
                    }
                }
                else
                {
                    emit errMsg((tr(resourceInfoErrMsg)));
#ifdef DEBUG
                    qDebug() << resourceInfoErrMsg;
#endif
                    emit activationError();
                }
                break;
            }

            case readsamplerate:
            if (reply == ack)
            {
                m_nSRate = answer.toInt(&ok);
                emit activationContinue();
            }
            else
            {
                emit errMsg((tr(readsamplerateErrMsg)));
    #ifdef DEBUG
                qDebug() << readsamplerateErrMsg;
    #endif
                emit activationError();
            }
            break;

            case readalias:
            {
                QString alias;
                cMeasChannelInfo mi;

                if (reply == ack)
                {
                    alias = answer.toString();
                    mi = m_measChannelInfoHash.take(channelInfoRead);
                    mi.alias = alias;
                    m_measChannelInfoHash[channelInfoRead] = mi;
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

            case readunit:
            {
                QString unit;
                cMeasChannelInfo mi;

                if (reply == ack)
                {
                    unit = answer.toString();
                    mi = m_measChannelInfoHash.take(channelInfoRead);
                    mi.unit = unit;
                    m_measChannelInfoHash[channelInfoRead] = mi;
                    emit activationContinue();
                }
                else
                {
                    emit errMsg((tr(readunitErrMsg)));
#ifdef DEBUG
                    qDebug() << readunitErrMsg;
#endif
                    emit activationError();
                }
                break;
            }

            case readdspchannel:
            {
                int chnnr;
                cMeasChannelInfo mi;

                if (reply == ack)
                {
                    chnnr = answer.toInt(&ok);
                    mi = m_measChannelInfoHash.take(channelInfoRead);
                    mi.dspChannelNr = chnnr;
                    m_measChannelInfoHash[channelInfoRead] = mi;
                    emit activationContinue();
                }
                else
                {
                    emit errMsg((tr(readdspchannelErrMsg)));
#ifdef DEBUG
                    qDebug() << readdspchannelErrMsg;
#endif
                    emit activationError();
                }
                break;
            break;
            }


            case writeparameter:
                if (reply == ack) // we ignore ack
                    ;
                else
                {
                    emit errMsg((tr(writedspmemoryErrMsg)));
#ifdef DEBUG
                    qDebug() << writedspmemoryErrMsg;
#endif
                    emit executionError();
                }
                break;

            case deactivatedsp:
                if (reply == ack) // we only continue if resource manager acknowledges
                    emit deactivationContinue();
                else
                {
                    emit errMsg((tr(dspdeactiveErrMsg)));
#ifdef DEBUG
                    qDebug() << dspdeactiveErrMsg;
#endif
                    emit deactivationError();
                }
                break;
            case freepgrmem:
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
            case freeusermem:
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

            case dataaquistion:
                if (reply == ack)
                    emit activationContinue();
                else
                {
                    m_dataAcquisitionMachine.stop();
                    emit errMsg((tr(dataaquisitionErrMsg)));
#ifdef DEBUG
                    qDebug() << dataaquisitionErrMsg;
#endif
                    emit executionError(); // but we send error message
                }
                break;
            }
        }
    }
}


void cThdnModuleMeasProgram::setActualValuesNames()
{
    for (int i = 0; i < m_ActValueList.count(); i++)
    {
        QString s;
        QString s1,s2;

        s1 = s2 = m_measChannelInfoHash.value(m_ActValueList.at(i)).alias;
        s1.remove(QRegExp("[1-9][0-9]?"));
        s2.remove(s1);

        s = s1 + "%1" + QString(";%1;[%]").arg(s2);
        m_EntityNameList.at(i)->setValue(s, m_pPeer);
    }
}


void cThdnModuleMeasProgram::setInterfaceActualValues(QVector<float> *actualValues)
{
    if (m_bActive) // maybe we are deactivating !!!!
    {
        for (int i = 0; i < m_ActValueList.count(); i++)
            m_EntityActValueList.at(i)->setValue(QVariant((double)actualValues->at(i)), m_pPeer); // and set entities
    }
}


void cThdnModuleMeasProgram::resourceManagerConnect()
{
    // as this is our entry point when activating the module, we do some initialization first
    m_measChannelInfoHash.clear(); // we build up a new channel info hash
    cMeasChannelInfo mi;
    mi.socket = m_ConfigData.m_PCBServerSocket; // the default from configuration file
    for (int i = 0; i < m_ActValueList.count(); i++)
    {
        QStringList sl = m_ActValueList.at(i).split('-');
        for (int j = 0; j < sl.count(); j++)
        {
            QString s = sl.at(j);
            if (!m_measChannelInfoHash.contains(s))
                m_measChannelInfoHash[s] = mi;
        }
    }

    // we have to instantiate a working resource manager interface
    // so first we try to get a connection to resource manager over proxy
    m_pRMClient = m_pProxy->getConnection(m_ConfigData.m_RMSocket.m_sIP, m_ConfigData.m_RMSocket.m_nPort);
    m_resourceManagerConnectState.addTransition(m_pRMClient, SIGNAL(connected()), &m_IdentifyState);
    // and then we set connection resource manager interface's connection
    m_pRMInterface->setClient(m_pRMClient); //
    // todo insert timer for timeout

    connect(m_pRMInterface, SIGNAL(serverAnswer(quint32, quint8, QVariant)), this, SLOT(catchInterfaceAnswer(quint32, quint8, QVariant)));
}


void cThdnModuleMeasProgram::sendRMIdent()
{
    m_MsgNrCmdList[m_pRMInterface->rmIdent(QString("ThdnModule%1").arg(m_pModule->getModuleNr()))] = sendrmident;
}


void cThdnModuleMeasProgram::readResourceTypes()
{
    m_MsgNrCmdList[m_pRMInterface->getResourceTypes()] = readresourcetypes;
}


void cThdnModuleMeasProgram::readResource()
{
    m_MsgNrCmdList[m_pRMInterface->getResources("SENSE")] = readresource;
}


void cThdnModuleMeasProgram::readResourceInfos()
{
    channelInfoReadList = m_measChannelInfoHash.keys(); // we have to read information for all channels in this list
    emit activationContinue();
}


void cThdnModuleMeasProgram::readResourceInfo()
{
    channelInfoRead = channelInfoReadList.takeLast();
    m_MsgNrCmdList[m_pRMInterface->getResourceInfo("SENSE", channelInfoRead)] = readresourceinfo;
}


void cThdnModuleMeasProgram::readResourceInfoDone()
{
    if (channelInfoReadList.isEmpty())
        emit activationContinue();
    else
        emit activationLoop();
}


void cThdnModuleMeasProgram::pcbserverConnect()
{
    // we have to connect to all ports....
    channelInfoReadList = m_measChannelInfoHash.keys(); // so first we look for our different pcb sockets
    m_nConnectionCount = channelInfoReadList.count();
    for (int i = 0; i < m_nConnectionCount; i++)
    {
        QString key = channelInfoReadList.at(i);
        cMeasChannelInfo mi = m_measChannelInfoHash.take(key);
        cSocket sock = mi.socket;
        Zera::Proxy::cProxyClient* pcbClient = m_pProxy->getConnection(sock.m_sIP, sock.m_nPort);
        m_pcbClientList.append(pcbClient);
        Zera::Server::cPCBInterface* pcbIFace = new Zera::Server::cPCBInterface();
        m_pcbIFaceList.append(pcbIFace);
        pcbIFace->setClient(pcbClient);
        mi.pcbIFace = pcbIFace;
        m_measChannelInfoHash[key] = mi;
        connect(pcbClient, SIGNAL(connected()), this, SLOT(monitorConnection())); // here we wait until all connections are established
        connect(pcbIFace, SIGNAL(serverAnswer(quint32, quint8, QVariant)), this, SLOT(catchInterfaceAnswer(quint32, quint8, QVariant)));
    }

    connect(m_pDSPIFace, SIGNAL(serverAnswer(quint32, quint8, QVariant)), this, SLOT(catchInterfaceAnswer(quint32, quint8, QVariant)));
}


void cThdnModuleMeasProgram::readSampleRate()
{
    // we always take the sample count from the first channels pcb server
    m_MsgNrCmdList[m_pcbIFaceList.at(0)->getSampleRate()] = readsamplerate;
}


void cThdnModuleMeasProgram::readChannelInformation()
{
    channelInfoReadList = m_measChannelInfoHash.keys(); // we have to read information for all channels in this list
    emit activationContinue();
}


void cThdnModuleMeasProgram::readChannelAlias()
{
    channelInfoRead = channelInfoReadList.takeFirst();
    m_MsgNrCmdList[m_measChannelInfoHash[channelInfoRead].pcbIFace->getAlias(channelInfoRead)] = readalias;
}


void cThdnModuleMeasProgram::readChannelUnit()
{
    m_MsgNrCmdList[m_measChannelInfoHash[channelInfoRead].pcbIFace->getUnit(channelInfoRead)] = readunit;
}


void cThdnModuleMeasProgram::readDspChannel()
{
    m_MsgNrCmdList[m_measChannelInfoHash[channelInfoRead].pcbIFace->getDSPChannel(channelInfoRead)] = readdspchannel;
}


void cThdnModuleMeasProgram::readDspChannelDone()
{
    if (channelInfoReadList.isEmpty())
    {
        // now we have all information to setup our var and cmd lists
        setDspVarList(); // first we set the var list for our dsp
        setDspCmdList(); // and the cmd list he has to work on
        emit activationContinue();
    }
    else
        emit activationLoop();
}


void cThdnModuleMeasProgram::claimPGRMem()
{
  m_MsgNrCmdList[m_pRMInterface->setResource("DSP1", "PGRMEMC", m_pDSPIFace->cmdListCount())] = claimpgrmem;
}


void cThdnModuleMeasProgram::claimUSERMem()
{
   m_MsgNrCmdList[m_pRMInterface->setResource("DSP1", "USERMEM", m_nDspMemUsed)] = claimusermem;
}


void cThdnModuleMeasProgram::varList2DSP()
{
    m_MsgNrCmdList[m_pDSPIFace->varList2Dsp()] = varlist2dsp;
}


void cThdnModuleMeasProgram::cmdList2DSP()
{
    m_MsgNrCmdList[m_pDSPIFace->cmdList2Dsp()] = cmdlist2dsp;
}


void cThdnModuleMeasProgram::activateDSP()
{
    m_MsgNrCmdList[m_pDSPIFace->activateInterface()] = activatedsp; // aktiviert die var- und cmd-listen im dsp
}


void cThdnModuleMeasProgram::activateDSPdone()
{
    m_bActive = true;

    setActualValuesNames();
    m_pMeasureSignal->m_pParEntity->setValue(QVariant(1), m_pPeer);
    connect(m_pIntegrationTimeParameter, SIGNAL(updated(QVariant)), this, SLOT(newIntegrationtime(QVariant)));

    emit activated();
}


void cThdnModuleMeasProgram::deactivateDSP()
{
    m_bActive = false;
    deleteDspVarList();
    deleteDspCmdList();

    m_MsgNrCmdList[m_pDSPIFace->deactivateInterface()] = deactivatedsp; // wat wohl
}


void cThdnModuleMeasProgram::freePGRMem()
{
    m_MsgNrCmdList[m_pRMInterface->freeResource("DSP1", "PGRMEMC")] = freepgrmem;
}


void cThdnModuleMeasProgram::freeUSERMem()
{
    m_MsgNrCmdList[m_pRMInterface->freeResource("DSP1", "USERMEM")] = freeusermem;
}


void cThdnModuleMeasProgram::deactivateDSPdone()
{
    disconnect(m_pRMInterface, 0, this, 0);
    disconnect(m_pDSPIFace, 0, this, 0);
    for (int i = 0; m_pcbIFaceList.count(); i++)
        disconnect(m_pcbIFaceList.at(i), 0 ,this, 0);
    emit deactivated();
}


void cThdnModuleMeasProgram::dataAcquisitionDSP()
{
    m_pMeasureSignal->m_pParEntity->setValue(QVariant(0), m_pPeer);
    m_MsgNrCmdList[m_pDSPIFace->dataAcquisition(m_pActualValuesDSP)] = dataaquistion; // we start our data aquisition now
}


void cThdnModuleMeasProgram::dataReadDSP()
{
    m_pDSPIFace->getData(m_pActualValuesDSP, m_ModuleActualValues); // we fetch our actual values
    emit actualValues(&m_ModuleActualValues); // and send them
    m_pMeasureSignal->m_pParEntity->setValue(QVariant(1), m_pPeer); // signal measuring

#ifdef DEBUG
    bool ok;
    QString s;
    for (int i = 0; i < m_ActValueList.count(); i++)
    {
        QString ts;
        ts = QString("THDN_%1:%2[%];").arg(m_measChannelInfoHash.value(m_ActValueList.at(i)).alias)
                                          .arg(m_EntityActValueList.at(i)->getValue().toDouble(&ok));
        s += ts;
    }

    qDebug() << s;
#endif
}


void cThdnModuleMeasProgram::newIntegrationtime(QVariant ti)
{
    bool ok;
    m_ConfigData.m_fMeasInterval.m_fValue = ti.toDouble(&ok);
    m_pDSPIFace->setVarData(m_pParameterDSP, QString("TIPAR:%1;TISTART:%2;").arg(m_ConfigData.m_fMeasInterval.m_fValue*1000)
                                                                            .arg(0), DSPDATA::dInt);
    m_MsgNrCmdList[m_pDSPIFace->dspMemoryWrite(m_pParameterDSP)] = writeparameter;
}

}





