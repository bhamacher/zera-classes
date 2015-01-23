#include <rminterface.h>
#include <dspinterface.h>
#include <proxy.h>
#include <veinentity.h>

#include "referencemodule.h"
#include "referencemoduleconfiguration.h"
#include "referencemoduleconfigdata.h"
#include "referencemeaschannel.h"
#include "referencemodulemeasprogram.h"
#include "referencemoduleobservation.h"
#include "referenceadjustment.h"
#include "referencemoduleinit.h"
#include "moduleinfo.h"
#include "moduleerror.h"
#include "modulesignal.h"

namespace REFERENCEMODULE
{

cReferenceModule::cReferenceModule(quint8 modnr, Zera::Proxy::cProxy *proxy, VeinPeer* peer, QObject *parent)
    :cBaseModule(modnr, proxy, peer, new cReferenceModuleConfiguration(), parent)
{
    m_ModuleActivistList.clear();

    m_ActivationStartState.addTransition(this, SIGNAL(activationContinue()), &m_ActivationExecState);
    m_ActivationExecState.addTransition(this, SIGNAL(activationContinue()), &m_ActivationDoneState);
    m_ActivationDoneState.addTransition(this, SIGNAL(activationNext()), &m_ActivationExecState);
    m_ActivationDoneState.addTransition(this, SIGNAL(activationContinue()), &m_ActivationFinishedState);
    m_ActivationMachine.addState(&m_ActivationStartState);
    m_ActivationMachine.addState(&m_ActivationExecState);
    m_ActivationMachine.addState(&m_ActivationDoneState);
    m_ActivationMachine.addState(&m_ActivationFinishedState);
    m_ActivationMachine.setInitialState(&m_ActivationStartState);
    connect(&m_ActivationStartState, SIGNAL(entered()), SLOT(activationStart()));
    connect(&m_ActivationExecState, SIGNAL(entered()), SLOT(activationExec()));
    connect(&m_ActivationDoneState, SIGNAL(entered()), SLOT(activationDone()));
    connect(&m_ActivationFinishedState, SIGNAL(entered()), SLOT(activationFinished()));

    m_DeactivationStartState.addTransition(this, SIGNAL(deactivationContinue()), &m_DeactivationExecState);
    m_DeactivationExecState.addTransition(this, SIGNAL(deactivationContinue()), &m_DeactivationDoneState);
    m_DeactivationDoneState.addTransition(this, SIGNAL(deactivationNext()), &m_DeactivationExecState);
    m_DeactivationDoneState.addTransition(this, SIGNAL(deactivationContinue()), &m_DeactivationFinishedState);
    m_DeactivationMachine.addState(&m_DeactivationStartState);
    m_DeactivationMachine.addState(&m_DeactivationExecState);
    m_DeactivationMachine.addState(&m_DeactivationDoneState);
    m_DeactivationMachine.addState(&m_DeactivationFinishedState);
    m_DeactivationMachine.setInitialState(&m_DeactivationStartState);
    connect(&m_DeactivationStartState, SIGNAL(entered()), SLOT(deactivationStart()));
    connect(&m_DeactivationExecState, SIGNAL(entered()), SLOT(deactivationExec()));
    connect(&m_DeactivationDoneState, SIGNAL(entered()), SLOT(deactivationDone()));
    connect(&m_DeactivationFinishedState, SIGNAL(entered()), SLOT(deactivationFinished()));

}


cReferenceModule::~cReferenceModule()
{
    delete m_pConfiguration;
    unsetModule();
}


QByteArray cReferenceModule::getConfiguration()
{
    return m_pConfiguration->exportConfiguration();
}


cReferenceMeasChannel *cReferenceModule::getMeasChannel(QString name)
{
    cReferenceMeasChannel* p_rmc = 0;
    for (int i = 0; i < m_ReferenceMeasChannelList.count(); i++)
    {
        p_rmc =  m_ReferenceMeasChannelList.at(i);
        if ((p_rmc->getName()) == name)
            return p_rmc;
    }

    return p_rmc;
}


void cReferenceModule::doConfiguration(QByteArray xmlConfigData)
{
    m_pConfiguration->setConfiguration(xmlConfigData);
}


void cReferenceModule::setupModule()
{
    cReferenceModuleConfigData *pConfData;
    pConfData = qobject_cast<cReferenceModuleConfiguration*>(m_pConfiguration)->getConfigurationData();

    errorMessage = new cModuleError(m_pPeer, "ERR_Message");

    // first we initialize by setting pcb in reference mode
    m_pReferenceModuleInit = new cReferenceModuleInit(this, m_pProxy, m_pPeer, *pConfData);
    m_ModuleActivistList.append(m_pReferenceModuleInit);
    connect(m_pReferenceModuleInit, SIGNAL(activated()), this, SIGNAL(activationContinue()));
    connect(m_pReferenceModuleInit, SIGNAL(deactivated()), this, SIGNAL(deactivationContinue()));
    connect(m_pReferenceModuleInit, SIGNAL(errMsg(QString)), errorMessage, SLOT(appendMsg(QString)));

    // before we build a list of our meas channels
    for (int i = 0; i < pConfData->m_nChannelCount; i ++)
    {
        cReferenceMeasChannel* pchn = new cReferenceMeasChannel(m_pProxy, m_pPeer, &(pConfData->m_RMSocket),
                                                                &(pConfData->m_PCBServerSocket),
                                                                pConfData->m_referenceChannelList.at(i), i+1);
        m_ReferenceMeasChannelList.append(pchn);
        m_ModuleActivistList.append(pchn);
        connect(pchn, SIGNAL(activated()), this, SIGNAL(activationContinue()));
        connect(pchn, SIGNAL(deactivated()), this, SIGNAL(deactivationContinue()));
        connect(pchn, SIGNAL(errMsg(QString)), errorMessage, SLOT(appendMsg(QString)));
    }

    // then we need some program for adjustment
    m_pReferenceAdjustment = new cReferenceAdjustment(this, m_pProxy, m_pPeer, pConfData);
    m_ModuleActivistList.append(m_pReferenceAdjustment);
    connect(m_pReferenceAdjustment, SIGNAL(activated()), SIGNAL(activationContinue()));
    connect(m_pReferenceAdjustment, SIGNAL(deactivated()), this, SIGNAL(deactivationContinue()));
    connect(m_pReferenceAdjustment, SIGNAL(errMsg(QString)), errorMessage, SLOT(appendMsg(QString)));


    // we have to connect all cmddone from our reference meas channels to refernce adjustment
    for (int i = 0; i < m_ReferenceMeasChannelList.count(); i ++)
    {
        cReferenceMeasChannel* pchn = m_ReferenceMeasChannelList.at(i);
        connect(pchn, SIGNAL(cmdDone(quint32)), m_pReferenceAdjustment, SLOT(catchChannelReply(quint32)));
    }

    // at last we need some program that does the measuring job on dsp
    m_pMeasProgram = new cReferenceModuleMeasProgram(this, m_pProxy, m_pPeer, *pConfData);
    m_ModuleActivistList.append(m_pMeasProgram);
    connect(m_pMeasProgram, SIGNAL(activated()), SIGNAL(activationContinue()));
    connect(m_pMeasProgram, SIGNAL(deactivated()), this, SIGNAL(deactivationContinue()));
    connect(m_pMeasProgram, SIGNAL(errMsg(QString)), errorMessage, SLOT(appendMsg(QString)));
    //
    m_pReferenceModuleObservation = new cReferenceModuleObservation(this, m_pProxy, &(pConfData->m_PCBServerSocket));
    m_ModuleActivistList.append(m_pReferenceModuleObservation);
    connect(m_pReferenceModuleObservation, SIGNAL(activated()), SIGNAL(activationContinue()));
    connect(m_pReferenceModuleObservation, SIGNAL(deactivated()), this, SIGNAL(deactivationContinue()));
    connect(m_pReferenceModuleObservation, SIGNAL(errMsg(QString)), errorMessage, SLOT(appendMsg(QString)));

    for (int i = 0; i < m_ModuleActivistList.count(); i++)
        m_ModuleActivistList.at(i)->generateInterface();
}


void cReferenceModule::unsetModule()
{
    if (m_ModuleActivistList.count() > 0)
    {
        for (int i = 0; i < m_ModuleActivistList.count(); i++)
        {
            m_ModuleActivistList.at(i)->deleteInterface();
            delete m_ModuleActivistList.at(i);
        }
        m_ModuleActivistList.clear();
        m_ReferenceMeasChannelList.clear();
        if (errorMessage) delete errorMessage;
    }
}


void cReferenceModule::startMeas()
{
    m_pMeasProgram->start();
}


void cReferenceModule::stopMeas()
{
    m_pMeasProgram->stop();
}


void cReferenceModule::activationStart()
{
    m_nActivationIt = 0; // we start with the first
    emit activationContinue();
}


void cReferenceModule::activationExec()
{
    m_ModuleActivistList.at(m_nActivationIt)->activate();
}


void cReferenceModule::activationDone()
{
    m_nActivationIt++;

    if (m_nActivationIt < m_ModuleActivistList.count())
        emit activationNext(); // and iterate over our list
    else
        emit activationContinue();
}


void cReferenceModule::activationFinished()
{
    // we connect the measurement output to our adjustment module
    connect(m_pMeasProgram, SIGNAL(actualValues(QVector<float>*)), m_pReferenceAdjustment, SLOT(ActionHandler(QVector<float>*)));
    // if we get informed we have to reconfigure
    connect(m_pReferenceModuleObservation, SIGNAL(moduleReconfigure()), this, SLOT(referenceModuleReconfigure()));

    emit activationReady();
}


void cReferenceModule::deactivationStart()
{
    // we first disconnect all what we connected when activation took place
    disconnect(m_pMeasProgram, SIGNAL(actualValues(QVector<float>*)), m_pReferenceAdjustment, SLOT(ActionHandler(QVector<float>*)));
    disconnect(m_pReferenceAdjustment, SIGNAL(moduleReconfigure()), this, SLOT(referenceModuleReconfigure()));

    for (int i = 0; i < m_ReferenceMeasChannelList.count(); i ++)
    {
        cReferenceMeasChannel* pchn = m_ReferenceMeasChannelList.at(i);
        disconnect(pchn, SIGNAL(cmdDone(quint32)), m_pReferenceAdjustment, SLOT(catchChannelReply(quint32)));
    }

    m_nActivationIt = 0; // we start with the first
    emit deactivationContinue();
}


void cReferenceModule::deactivationExec()
{
    m_ModuleActivistList.at(m_nActivationIt)->deactivate();
}


void cReferenceModule::deactivationDone()
{
    m_nActivationIt++;

    if (m_nActivationIt < m_ModuleActivistList.count())
        emit deactivationNext(); // and iterate over our list
    else
        emit deactivationContinue();
}


void cReferenceModule::deactivationFinished()
{
    emit deactivationReady();
}


void cReferenceModule::referenceModuleReconfigure()
{
    emit sigConfiguration(); // we configure after our notifier has detected
}

}
