#include <pcbinterface.h>
#include <proxy.h>
#include <proxyclient.h>

#include "rangemoduleobservation.h"
#include "rangemodule.h"

cRangeModuleObservation::cRangeModuleObservation(cRangeModule* module, Zera::Proxy::cProxy *proxy, cSocket *pcbsocket)
    :m_pRangemodule(module), m_pProxy(proxy), m_pPCBServerSocket(pcbsocket)
{
    m_pPCBInterface = new Zera::Server::cPCBInterface();

    // setting up statemachine for "activating" rangemoduleobservation
    // m_pcbConnectionState.addTransition is done in pcbConnection
    m_setNotifierState.addTransition(this, SIGNAL(activationContinue()), &m_activationDoneState);

    m_activationMachine.addState(&m_pcbConnectState);
    m_activationMachine.addState(&m_setNotifierState);
    m_activationMachine.addState(&m_activationDoneState);
    m_activationMachine.setInitialState(&m_pcbConnectState);

    connect(&m_pcbConnectState, SIGNAL(entered()), SLOT(pcbConnect()));
    connect(&m_setNotifierState, SIGNAL(entered()), SLOT(setNotifier()));
    connect(&m_activationDoneState, SIGNAL(entered()), SLOT(activationDone()));

    m_resetNotifierState.addTransition(this, SIGNAL(deactivationContinue()), &m_deactivationDoneState);
    m_deactivationMachine.addState(&m_resetNotifierState);
    m_deactivationMachine.addState(&m_deactivationDoneState);
    m_deactivationMachine.setInitialState((&m_resetNotifierState));

    connect(&m_resetNotifierState, SIGNAL(entered()), SLOT(resetNotifier()));
    connect(&m_deactivationDoneState, SIGNAL(entered()), SLOT(deactivationDone()));
}


cRangeModuleObservation::~cRangeModuleObservation()
{
    m_pProxy->releaseConnection(m_pPCBClient);
    delete m_pPCBInterface;
}


void cRangeModuleObservation::generateInterface()
{
    // we don't any interface for this module
}


void cRangeModuleObservation::deleteInterface()
{
    // so we have nothing to delete
}


void cRangeModuleObservation::catchInterfaceAnswer(quint32 msgnr, quint8 reply, QVariant answer)
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
            // we got a sense:mmode notifier
            // let's look what to do
            emit moduleReconfigure();
            break;
        }
    }
    else
    {
        int cmd = m_MsgNrCmdList.take(msgnr);
        switch (cmd)
        {
        case RANGEMODULEOBSERVATION::registernotifier:
            if (reply == RANGEMODULEOBSERVATION::ack) // we only continue pcb server acknowledges
                emit activationContinue();
            else
                emit activationError();
            break;
        case RANGEMODULEOBSERVATION::unregisternotifiers:
            if (reply == RANGEMODULEOBSERVATION::ack) // we only continue pcb server acknowledges
                emit deactivationContinue();
            else
                emit deactivationError();
            break;
        }
    }
}


void cRangeModuleObservation::pcbConnect()
{
    Zera::Proxy::cProxyClient* m_pPCBClient = m_pProxy->getConnection(m_pPCBServerSocket->m_sIP, m_pPCBServerSocket->m_nPort);
    m_pcbConnectState.addTransition(m_pPCBClient, SIGNAL(connected()), &m_setNotifierState);

    m_pPCBInterface->setClient(m_pPCBClient);
    connect(m_pPCBInterface, SIGNAL(serverAnswer(quint32, quint8, QVariant)), this, SLOT(catchInterfaceAnswer(quint32, quint8, QVariant)));

}


void cRangeModuleObservation::setNotifier()
{
    m_MsgNrCmdList[m_pPCBInterface->registerNotifier("SENSE:MMODE?","1")] = RANGEMODULEOBSERVATION::registernotifier;
}


void cRangeModuleObservation::activationDone()
{
    emit activated();
}


void cRangeModuleObservation::resetNotifier()
{
    m_MsgNrCmdList[m_pPCBInterface->unregisterNotifiers()] = RANGEMODULEOBSERVATION::unregisternotifiers;
}


void cRangeModuleObservation::deactivationDone()
{
    emit deactivated();
}
