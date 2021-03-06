#include <QRegExp>
#include <rminterface.h>
#include <pcbinterface.h>
#include <proxy.h>
#include <proxyclient.h>

#include "debug.h"
#include "errormessages.h"
#include "pllmeaschannel.h"

namespace SAMPLEMODULE
{

cPllMeasChannel::cPllMeasChannel(Zera::Proxy::cProxy* proxy, cSocket* rmsocket, cSocket* pcbsocket, QString name, quint8 chnnr)
    :cBaseMeasChannel(proxy, rmsocket, pcbsocket, name, chnnr)
{
    m_pRMInterface = new Zera::Server::cRMInterface();
    m_pPCBInterface = new Zera::Server::cPCBInterface();

    // setting up statemachine for "activating" pll meas channel
    // m_rmConnectState.addTransition is done in rmConnect
    m_IdentifyState.addTransition(this, SIGNAL(activationContinue()), &m_readResourceTypesState);
    m_readResourceTypesState.addTransition(this, SIGNAL(activationContinue()), &m_readResourceState);
    m_readResourceState.addTransition(this, SIGNAL(activationContinue()), &m_readResourceInfoState);
    m_readResourceInfoState.addTransition(this, SIGNAL(activationContinue()), &m_pcbConnectionState);
    // m_pcbConnectionState.addTransition is done in pcbConnection
    m_readDspChannelState.addTransition(this, SIGNAL(activationContinue()), &m_readChnAliasState);
    m_readChnAliasState.addTransition(this, SIGNAL(activationContinue()), &m_readSampleRateState);
    m_readSampleRateState.addTransition(this, SIGNAL(activationContinue()), &m_readUnitState);
    m_readUnitState.addTransition(this, SIGNAL(activationContinue()), &m_readRangelistState);
    m_readRangelistState.addTransition(this, SIGNAL(activationContinue()), &m_readRangeProperties1State);
    m_readRangeProperties1State.addTransition(this, SIGNAL(activationContinue()), &m_readRangeProperties2State);
    m_readRangeProperties2State.addTransition(&m_rangeQueryMachine, SIGNAL(finished()), &m_readRangeProperties3State);
    m_readRangeProperties3State.addTransition(this, SIGNAL(activationLoop()), &m_readRangeProperties1State);
    m_readRangeProperties3State.addTransition(this, SIGNAL(activationContinue()), &m_setSenseChannelRangeNotifierState);
    m_setSenseChannelRangeNotifierState.addTransition(this, SIGNAL(activationContinue()), &m_activationDoneState);
    m_activationMachine.addState(&m_rmConnectState);
    m_activationMachine.addState(&m_IdentifyState);
    m_activationMachine.addState(&m_readResourceTypesState);
    m_activationMachine.addState(&m_readResourceState);
    m_activationMachine.addState(&m_readResourceInfoState);
    m_activationMachine.addState(&m_pcbConnectionState);
    m_activationMachine.addState(&m_readDspChannelState);
    m_activationMachine.addState(&m_readChnAliasState);
    m_activationMachine.addState(&m_readSampleRateState);
    m_activationMachine.addState(&m_readUnitState);
    m_activationMachine.addState(&m_readRangelistState);
    m_activationMachine.addState(&m_readRangeProperties1State);
    m_activationMachine.addState(&m_readRangeProperties2State);
    m_activationMachine.addState(&m_readRangeProperties3State);
    m_activationMachine.addState((&m_setSenseChannelRangeNotifierState));
    m_activationMachine.addState(&m_activationDoneState);
    m_activationMachine.setInitialState(&m_rmConnectState);
    connect(&m_rmConnectState, SIGNAL(entered()), SLOT(rmConnect()));
    connect(&m_IdentifyState, SIGNAL(entered()), SLOT(sendRMIdent()));
    connect(&m_readResourceTypesState, SIGNAL(entered()), SLOT(readResourceTypes()));
    connect(&m_readResourceState, SIGNAL(entered()), SLOT(readResource()));
    connect(&m_readResourceInfoState, SIGNAL(entered()), SLOT(readResourceInfo()));
    connect(&m_pcbConnectionState, SIGNAL(entered()), SLOT(pcbConnection()));
    connect(&m_readDspChannelState, SIGNAL(entered()), SLOT(readDspChannel()));
    connect(&m_readChnAliasState, SIGNAL(entered()), SLOT(readChnAlias()));
    connect(&m_readSampleRateState, SIGNAL(entered()), SLOT(readSampleRate()));
    connect(&m_readUnitState, SIGNAL(entered()), SLOT(readUnit()));
    connect(&m_readRangelistState, SIGNAL(entered()), SLOT(readRangelist()));
    connect(&m_readRangeProperties1State, SIGNAL(entered()), SLOT(readRangeProperties1()));
    connect(&m_readRangeProperties3State, SIGNAL(entered()), SLOT(readRangeProperties3()));
    connect(&m_setSenseChannelRangeNotifierState, SIGNAL(entered()), SLOT(setSenseChannelRangeNotifier()));
    connect(&m_activationDoneState, SIGNAL(entered()), SLOT(activationDone()));

    // setting up statemachine for "deactivating" pll meas channel
    m_deactivationInitState.addTransition(this, SIGNAL(deactivationContinue()), &m_deactivationDoneState);
    m_deactivationMachine.addState(&m_deactivationInitState);
    m_deactivationMachine.addState(&m_deactivationDoneState);
    m_deactivationMachine.setInitialState(&m_deactivationInitState);
    connect(&m_deactivationInitState, SIGNAL(entered()), SLOT(deactivationInit()));
    connect(&m_deactivationDoneState, SIGNAL(entered()), SLOT(deactivationDone()));

    // setting up statemachine for querying the pll meas channels ranges properties
    m_readRngAliasState.addTransition(this, SIGNAL(activationContinue()), &m_readTypeState);
    m_readTypeState.addTransition(this, SIGNAL(activationContinue()), &m_readUrvalueState);
    m_readUrvalueState.addTransition(this, SIGNAL(activationContinue()), &m_readRejectionState);
    m_readRejectionState.addTransition(this, SIGNAL(activationContinue()), &m_readOVRejectionState);
    m_readOVRejectionState.addTransition(this, SIGNAL(activationContinue()), &m_readisAvailState);
    m_readisAvailState.addTransition(this, SIGNAL(activationContinue()), &m_rangeQueryDoneState);


    m_rangeQueryMachine.addState(&m_readRngAliasState);
    m_rangeQueryMachine.addState(&m_readTypeState);
    m_rangeQueryMachine.addState(&m_readUrvalueState);
    m_rangeQueryMachine.addState(&m_readRejectionState);
    m_rangeQueryMachine.addState(&m_readOVRejectionState);
    m_rangeQueryMachine.addState(&m_readisAvailState);
    m_rangeQueryMachine.addState(&m_rangeQueryDoneState);

    m_rangeQueryMachine.setInitialState(&m_readRngAliasState);

    connect(&m_readRngAliasState, SIGNAL(entered()), SLOT(readRngAlias()));
    connect(&m_readTypeState, SIGNAL(entered()), SLOT(readType()));
    connect(&m_readUrvalueState, SIGNAL(entered()), SLOT(readUrvalue()));
    connect(&m_readRejectionState, SIGNAL(entered()), SLOT(readRejection()));
    connect(&m_readOVRejectionState, SIGNAL(entered()), SLOT(readOVRejection()));
    connect(&m_readisAvailState, SIGNAL(entered()), SLOT(readisAvail()));
    connect(&m_rangeQueryDoneState, SIGNAL(entered()), SLOT(rangeQueryDone()));

}


cPllMeasChannel::~cPllMeasChannel()
{
    m_pProxy->releaseConnection(m_pRMClient);
    m_pProxy->releaseConnection(m_pPCBClient);
    delete m_pRMInterface;
    delete m_pPCBInterface;
}


double cPllMeasChannel::getUrValue()
{
    return m_RangeInfoHash[m_sActRange].urvalue;
}


void cPllMeasChannel::generateInterface()
{
    // we don't generate any interface
}


void cPllMeasChannel::deleteInterface()
{
    // so we have nothing to delete
}


quint32 cPllMeasChannel::setyourself4PLL(QString samplesysname)
{
    if (m_bActive)
    {
        quint32 msgnr = m_pPCBInterface->setPLLChannel(samplesysname, m_sName);
        m_MsgNrCmdList[msgnr] = set4PLL;
        return msgnr;
    }
    else
        return 1;
}


void cPllMeasChannel::catchInterfaceAnswer(quint32 msgnr, quint8 reply, QVariant answer)
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
        case notifierNr:
            // if we got notification for new range we read that information
            readRange();
            break;
        }
    }
    else
    {
        int cmd = m_MsgNrCmdList.take(msgnr);

        switch (cmd)
        {
        case sendpllchannelrmident:
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
            if ((reply == ack) && (answer.toString().contains(m_sName)))
                emit activationContinue();
            else
            {
                emit errMsg((tr(resourceErrMsg)));
#ifdef DEBUG
                qDebug() << resourceErrMsg;
#endif
                emit activationError();
            }
            break;
        case readresourceinfo:
        {
            bool ok1, ok2;
            int max;
            QStringList sl;


            sl = answer.toString().split(';');
            if ((reply ==ack) && (sl.length() >= 4))
            {
                max = sl.at(0).toInt(&ok1); // fixed position
                m_sDescription = sl.at(2);
                m_nPort = sl.at(3).toInt(&ok2);

                if (ok1 && ok2 && (max == 1)) // we need one but it must not be free
                {
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
        case readdspchannel:
            if (reply == ack)
            {
                m_nDspChannel = answer.toInt(&ok);
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
        case readrange:
            if (reply == ack)
            {
                m_sActRange = answer.toString();
            }
            else
            {
                emit errMsg((tr(getRangeErrMsg)));
#ifdef DEBUG
                qDebug() << getRangeErrMsg;
#endif
            }
            break;
        case readchnalias:
            if (reply == ack)
            {
                m_sAlias = answer.toString();
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
        case readsamplerate:
            if (reply == ack)
            {
                m_nSampleRate = answer.toInt(&ok);
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
        case readunit:
            if (reply == ack)
            {
                m_sUnit = answer.toString();
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
        case readrangelist:
            if (reply == ack)
            {
                m_RangeNameList = answer.toStringList();
                emit activationContinue();
            }
            else
            {
                emit errMsg((tr(readrangelistErrMsg)));
#ifdef DEBUG
                qDebug() << readrangelistErrMsg;
#endif
                emit activationError();
            }
            break;
        case readrngalias:
            if (reply == ack)
            {
                ri.alias = answer.toString();
                emit activationContinue();
            }
            else
            {
                emit errMsg((tr(readrangealiasErrMsg)));
#ifdef DEBUG
                qDebug() << readrangealiasErrMsg;
#endif
                emit activationError();
            }
            break;
        case readtype:
            if (reply == ack)
            {
                ri.type = answer.toInt(&ok);
                emit activationContinue();
            }
            else
            {
                emit errMsg((tr(readrangetypeErrMsg)));
#ifdef DEBUG
                qDebug() << readrangetypeErrMsg;
#endif
                emit activationError();
            }
            break;
        case readurvalue:
            if (reply == ack)
            {
                ri.urvalue = answer.toDouble(&ok);
                emit activationContinue();
            }
            else
            {
                emit errMsg((tr(readrangeurvalueErrMsg)));
#ifdef DEBUG
                qDebug() << readrangeurvalueErrMsg;
#endif
                emit activationError();
            }
            break;
        case readrejection:
            if (reply == ack)
            {
                ri.rejection = answer.toDouble(&ok);
                emit activationContinue();
            }
            else
            {
                emit errMsg((tr(readrangerejectionErrMsg)));
#ifdef DEBUG
                qDebug() << readrangerejectionErrMsg;
#endif
                emit activationError();
            }
            break;
        case readovrejection:
            if (reply == ack)
            {
                ri.ovrejection = answer.toDouble(&ok);
                emit activationContinue();
            }
            else
            {
                emit errMsg((tr(readrangeovrejectionErrMsg)));
#ifdef DEBUG
                qDebug() << readrangeovrejectionErrMsg;
#endif
                emit activationError();
            }
            break;
        case readisavail:
            if (reply == ack)
            {
                ri.avail = answer.toBool();
                emit activationContinue();
            }
            else
            {
                emit errMsg((tr(readrangeavailErrMsg)));
#ifdef DEBUG
                qDebug() << readrangeavailErrMsg;
#endif
                emit activationError();
            }
            break;
        case set4PLL:
            if (reply == ack)
            {}
            else
            {
                emit errMsg((tr(setPllErrMsg)));
#ifdef DEBUG
                qDebug() << setPllErrMsg;
#endif
                emit executionError();
            }; // perhaps some error output
            emit cmdDone(msgnr);
            break;
        case setchannelrangenotifier:
            if (reply == ack)
            {
                emit activationContinue();
            }
            else
            {
                emit errMsg((tr(registerpcbnotifierErrMsg)));
#ifdef DEBUG
                qDebug() << registerpcbnotifierErrMsg;
#endif
                emit activationError();
            }
            break;

        }
    }
}


void cPllMeasChannel::rmConnect()
{
    // we instantiate a working resource manager interface first
    // so first we try to get a connection to resource manager over proxy
    m_pRMClient = m_pProxy->getConnection(m_pRMSocket->m_sIP, m_pRMSocket->m_nPort);
    m_rmConnectState.addTransition(m_pRMClient, SIGNAL(connected()), &m_IdentifyState);
    // and then we set connection resource manager interface's connection
    m_pRMInterface->setClient(m_pRMClient); //
    // todo insert timer for timeout

    connect(m_pRMInterface, SIGNAL(serverAnswer(quint32, quint8, QVariant)), this, SLOT(catchInterfaceAnswer(quint32, quint8, QVariant)));
    m_pProxy->startConnection(m_pRMClient);
    // resource manager liste sense abfragen
    // bin ich da drin ?
    // nein -> fehler activierung
    // ja -> socket von rm besorgen
    // resource bei rm belegen
    // beim pcb proxy server interface beantragen

    // quint8 m_nDspChannel; dsp kanal erfragen
    // QString m_sAlias; kanal alias erfragen
    // eine liste aller möglichen bereichen erfragen
    // d.h. (avail = 1 und type =1
    // und von diesen dann
    // alias, urvalue, rejection und ovrejection abfragen
}


void cPllMeasChannel::sendRMIdent()
{
   m_MsgNrCmdList[m_pRMInterface->rmIdent(QString("MeasChannel%1").arg(m_nChannelNr))] = sendpllchannelrmident;
}


void cPllMeasChannel::readResourceTypes()
{
    m_MsgNrCmdList[m_pRMInterface->getResourceTypes()] = readresourcetypes;
}


void cPllMeasChannel::readResource()
{
    m_MsgNrCmdList[m_pRMInterface->getResources("SENSE")] = readresource;
}


void cPllMeasChannel::readResourceInfo()
{
    m_MsgNrCmdList[m_pRMInterface->getResourceInfo("SENSE", m_sName)] = readresourceinfo;
}


void cPllMeasChannel::pcbConnection()
{
    m_pPCBClient = m_pProxy->getConnection(m_pPCBServerSocket->m_sIP, m_nPort);
    m_pcbConnectionState.addTransition(m_pPCBClient, SIGNAL(connected()), &m_readDspChannelState);

    m_pPCBInterface->setClient(m_pPCBClient);
    connect(m_pPCBInterface, SIGNAL(serverAnswer(quint32, quint8, QVariant)), this, SLOT(catchInterfaceAnswer(quint32, quint8, QVariant)));
    m_pProxy->startConnection(m_pPCBClient);
}


void cPllMeasChannel::readDspChannel()
{
   m_MsgNrCmdList[m_pPCBInterface->getDSPChannel(m_sName)] = readdspchannel;
}


void cPllMeasChannel::readChnAlias()
{
    m_MsgNrCmdList[m_pPCBInterface->getAlias(m_sName)] = readchnalias;
}


void cPllMeasChannel::readSampleRate()
{
    m_MsgNrCmdList[m_pPCBInterface->getSampleRate()] = readsamplerate;
}


void cPllMeasChannel::readUnit()
{
    m_MsgNrCmdList[m_pPCBInterface->getUnit(m_sName)] = readunit;
}


void cPllMeasChannel::readRangelist()
{
    m_MsgNrCmdList[m_pPCBInterface->getRangeList(m_sName)] = readrangelist;
    m_RangeQueryIt = 0; // we start with range 0
}


void cPllMeasChannel::readRangeProperties1()
{
    m_rangeQueryMachine.start(); // yes, fill it with information
    emit activationContinue();
}


void cPllMeasChannel::readRangeProperties3()
{
    m_RangeQueryIt++;
    if (m_RangeQueryIt < m_RangeNameList.count()) // another range ?
        emit activationLoop();
    else
        emit activationContinue();
}


void cPllMeasChannel::setSenseChannelRangeNotifier()
{
    m_MsgNrCmdList[m_pPCBInterface->registerNotifier(QString("sens:%1:rang?").arg(m_sName), QString("%1").arg(notifierNr))] = setchannelrangenotifier;
}


void cPllMeasChannel::activationDone()
{
    QHash<QString, cRangeInfo>::iterator it = m_RangeInfoHash.begin();
    while (it != m_RangeInfoHash.end()) // we delete all unused ranges
    {
        ri = it.value();
        if (!ri.avail) // in case range is not avail
            it = m_RangeInfoHash.erase(it);
        else
            ++it;
    }

    readRange(); // we read the actual range once here, afterwards via notifier
    m_bActive = true;
    emit activated();
}


void cPllMeasChannel::deactivationInit()
{
    // deactivation means we have to free our resources
    // m_MsgNrCmdList[m_pRMInterface->freeResource("SENSE", m_sName)] = freeresource;
    // but we didn't claim here so we continue at once
    m_bActive = false;
    emit deactivationContinue();
}


void cPllMeasChannel::deactivationDone()
{
    // and disconnect for our servers afterwards
    disconnect(m_pRMInterface, 0, this, 0);
    disconnect(m_pPCBInterface, 0, this, 0);
    emit deactivated();
}


void cPllMeasChannel::readRngAlias()
{
    m_MsgNrCmdList[m_pPCBInterface->getAlias(m_sName, m_RangeNameList.at(m_RangeQueryIt))] = readrngalias;
}


void cPllMeasChannel::readType()
{
    m_MsgNrCmdList[m_pPCBInterface->getType(m_sName, m_RangeNameList.at(m_RangeQueryIt))] = readtype;
}


void cPllMeasChannel::readUrvalue()
{
    m_MsgNrCmdList[m_pPCBInterface->getUrvalue(m_sName, m_RangeNameList.at(m_RangeQueryIt))] = readurvalue;
}


void cPllMeasChannel::readRejection()
{
   m_MsgNrCmdList[m_pPCBInterface->getRejection(m_sName, m_RangeNameList.at(m_RangeQueryIt))] = readrejection;
}


void cPllMeasChannel::readOVRejection()
{
    m_MsgNrCmdList[m_pPCBInterface->getOVRejection(m_sName, m_RangeNameList.at(m_RangeQueryIt))] = readovrejection;
}


void cPllMeasChannel::readisAvail()
{
    m_MsgNrCmdList[m_pPCBInterface->isAvail(m_sName, m_RangeNameList.at(m_RangeQueryIt))] = readisavail;
}


void cPllMeasChannel::rangeQueryDone()
{
    ri.name = m_RangeNameList.at(m_RangeQueryIt);
    m_RangeInfoHash[ri.name] = ri; // for each range we append cRangeinfo per name
}


void cPllMeasChannel::readRange()
{
    m_MsgNrCmdList[m_pPCBInterface->getRange(m_sName)] = readrange;
}


}



