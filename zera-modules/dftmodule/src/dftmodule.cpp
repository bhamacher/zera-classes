#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <rminterface.h>
#include <dspinterface.h>
#include <proxy.h>
#include <modulevalidator.h>
#include <veinmodulecomponent.h>
#include <veinmodulemetadata.h>

#include <ve_commandevent.h>
#include <vcmp_entitydata.h>
#include <vcmp_componentdata.h>

#include "debug.h"
#include "dftmodule.h"
#include "dftmoduleconfiguration.h"
#include "dftmoduleconfigdata.h"
#include "dftmodulemeasprogram.h"
#include "dftmoduleobservation.h"
#include "errormessages.h"


namespace DFTMODULE
{

cDftModule::cDftModule(quint8 modnr, Zera::Proxy::cProxy* proxi, int entityId, VeinEvent::StorageSystem* storagesystem, QObject* parent)
    :cBaseMeasModule(modnr, proxi, entityId, storagesystem, new cDftModuleConfiguration(), parent)
{
    m_sModuleName = QString("%1%2").arg(BaseModuleName).arg(modnr);
    m_sModuleDescription = QString("This module measures configured order dft values for configured channels");
    m_sSCPIModuleName = QString("%1%2").arg(BaseSCPIModuleName).arg(modnr);

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


cDftModule::~cDftModule()
{
    delete m_pConfiguration;
}


QByteArray cDftModule::getConfiguration() const
{
    return m_pConfiguration->exportConfiguration();
}



void cDftModule::doConfiguration(QByteArray xmlConfigData)
{
    m_pConfiguration->setConfiguration(xmlConfigData);
}


void cDftModule::setupModule()
{
    emit addEventSystem(m_pModuleValidator);

    VeinComponent::EntityData *eData = new VeinComponent::EntityData();
    eData->setCommand(VeinComponent::EntityData::Command::ECMD_ADD);
    eData->setEntityId(m_nEntityId);
    VeinEvent::CommandEvent *tmpEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, eData);
    m_pModuleValidator->sigSendEvent(tmpEvent);

    VeinComponent::ComponentData *cData;

    cData = new VeinComponent::ComponentData();
    cData->setEntityId(m_nEntityId);
    cData->setCommand(VeinComponent::ComponentData::Command::CCMD_ADD);
    cData->setComponentName("EntityName");
    cData->setNewValue(m_sModuleName);
    tmpEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, cData);
    m_pModuleValidator->sigSendEvent(tmpEvent);

    cDftModuleConfigData* pConfData;
    pConfData = qobject_cast<cDftModuleConfiguration*>(m_pConfiguration)->getConfigurationData();

    m_pModuleErrorComponent = new cVeinModuleComponent(m_nEntityId, m_pModuleValidator,
                                                       QString("ERR_Message"),
                                                       QString("Component forwards the run time errors"),
                                                       QVariant(QString("")));
    veinModuleComponentList.append(m_pModuleErrorComponent);
    m_pModuleInterfaceComponent = new cVeinModuleComponent(m_nEntityId, m_pModuleValidator,
                                                           QString("INF_ModuleInterface"),
                                                           QString("Component forwards the modules interface"),
                                                           QByteArray());
    veinModuleComponentList.append(m_pModuleInterfaceComponent);

    m_pModuleName = new cVeinModuleMetaData(QString("Name"), QVariant(m_sModuleName));
    veinModuleMetaDataList.append(m_pModuleName);

    m_pModuleDescription = new cVeinModuleMetaData(QString("Description"), QVariant(m_sModuleDescription));
    veinModuleMetaDataList.append(m_pModuleDescription);

    // we need some program that does the measuring on dsp
    m_pMeasProgram = new cDftModuleMeasProgram(this, m_pProxy, *pConfData);
    m_ModuleActivistList.append(m_pMeasProgram);
    connect(m_pMeasProgram, SIGNAL(activated()), SIGNAL(activationContinue()));
    connect(m_pMeasProgram, SIGNAL(deactivated()), this, SIGNAL(deactivationContinue()));
    connect(m_pMeasProgram, SIGNAL(errMsg(QVariant)), m_pModuleErrorComponent, SLOT(setValue(QVariant)));

    // and module observation in case we have to react to naming changes
    m_pDftModuleObservation = new cDftModuleObservation(this, m_pProxy, &(pConfData->m_PCBServerSocket));
    m_ModuleActivistList.append(m_pDftModuleObservation);
    connect(m_pDftModuleObservation, SIGNAL(activated()), SIGNAL(activationContinue()));
    connect(m_pDftModuleObservation, SIGNAL(deactivated()), this, SIGNAL(deactivationContinue()));
    connect(m_pDftModuleObservation, SIGNAL(errMsg(QVariant)), m_pModuleErrorComponent, SLOT(setValue(QVariant)));

    for (int i = 0; i < m_ModuleActivistList.count(); i++)
        m_ModuleActivistList.at(i)->generateInterface();
}


void cDftModule::unsetModule()
{
    cBaseModule::unsetModule();
}


void cDftModule::startMeas()
{
    m_pMeasProgram->start();
}


void cDftModule::stopMeas()
{
    m_pMeasProgram->stop();
}


void cDftModule::activationStart()
{
    m_nActivationIt = 0; // we start with the first
    emit activationContinue();
}


void cDftModule::activationExec()
{
    m_ModuleActivistList.at(m_nActivationIt)->activate();
}


void cDftModule::activationDone()
{
    m_nActivationIt++;

    if (m_nActivationIt < m_ModuleActivistList.count())
        emit activationNext(); // and iterate over our list
    else
        emit activationContinue();
}


void cDftModule::activationFinished()
{
    // if we get informed we have to reconfigure
    connect(m_pDftModuleObservation, SIGNAL(moduleReconfigure()), this, SLOT(dftModuleReconfigure()));

    m_pModuleValidator->setParameterHash(veinModuleParameterHash);

    // now we still have to export the json interface information
    exportMetaData();

    emit activationReady();
}


void cDftModule::deactivationStart()
{
    // if we get informed we have to reconfigure
    disconnect(m_pDftModuleObservation, SIGNAL(moduleReconfigure()), this, SLOT(dftModuleReconfigure()));

    m_nActivationIt = 0; // we start with the first
    emit deactivationContinue();
}


void cDftModule::deactivationExec()
{
    m_ModuleActivistList.at(m_nActivationIt)->deactivate();
}


void cDftModule::deactivationDone()
{
    m_nActivationIt++;

    if (m_nActivationIt < m_ModuleActivistList.count())
        emit deactivationNext(); // and iterate over our list
    else
        emit deactivationContinue();
}


void cDftModule::deactivationFinished()
{
    emit deactivationReady();
}


void cDftModule::dftModuleReconfigure()
{
    emit sigConfiguration(); // we configure after our notifier has detected
}

}
