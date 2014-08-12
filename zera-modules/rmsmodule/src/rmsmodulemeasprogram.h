#ifndef RMSMODULEMEASPROGRAM_H
#define RMSMODULEMEASPROGRAM_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QStateMachine>
#include <QState>
#include <QFinalState>

#include "basemeasprogram.h"
#include "measchannelinfo.h"


namespace RMSMEASPROGRAM
{

enum rmsmoduleCmds
{
    resourcemanagerconnect,
    sendrmident,
    readresourcetypes,
    readresource,
    readresourceinfos,
    readresourceinfo,
    pcbserverconnect,
    readsamplenr,
    readalias,
    readdspchannel,
    claimpgrmem,
    claimusermem,
    varlist2dsp,
    cmdlist2dsp,
    activatedsp,
    deactivatedsp,
    dataaquistion,
    writeparameter,
    freepgrmem,
    freeusermem
};

enum replies
{
    ack,
    nack,
    error,
    debug,
    ident
};

}


namespace Zera {
namespace Proxy {
    class cProxy;
}
}


class cModuleSignal;
class cModuleParameter;
class VeinPeer;
class VeinEntity;
class cBaseModule;
class cDspMeasData;
class cDspIFace;
class cRmsModuleConfigData;
class QStateMachine;
class QState;
class QFinalState;


class cRmsModuleMeasProgram: public cBaseMeasProgram
{
    Q_OBJECT

public:
    cRmsModuleMeasProgram(Zera::Proxy::cProxy* proxy, VeinPeer* peer, Zera::Server::cDSPInterface* iface, cRmsModuleConfigData& configdata);
    virtual ~cRmsModuleMeasProgram();
    virtual void generateInterface(); // here we export our interface (entities)
    virtual void deleteInterface(); // we delete interface in case of reconfiguration


public slots:
    virtual void start(); // difference between start and stop is that actual values
    virtual void stop(); // in interface are not updated when stop

protected:
    virtual void setDspVarList(); // dsp related stuff
    virtual void deleteDspVarList();
    virtual void setDspCmdList();
    virtual void deleteDspCmdList();

protected slots:
    virtual void catchInterfaceAnswer(quint32 msgnr, quint8 reply, QVariant answer);

private:
    cRmsModuleConfigData& m_ConfigData;
    QStringList m_ActValueList; // the list of actual values we work on
    QList<VeinEntity*> m_EntityList;
    QHash<QString, cMeasChannelInfo> m_measChannelInfoHash;
    QList<QString> channelInfoReadList; // a list of all channel info we have to read
    QString channelInfoRead; // the actual channel info we are working on
    quint32 m_nSamples; // number of samples / signal period

    cModuleSignal* m_pMeasureSignal;
    cModuleParameter* m_pIntegrationTimeParameter;
    cDspMeasData* m_pTmpDataDsp;
    cDspMeasData* m_pParameterDSP;
    cDspMeasData* m_pActualValuesDSP;

    // statemachine for activating gets the following states
    QState m_resourceManagerConnectState;
    QState m_IdentifyState;
    QState m_readResourceTypesState;
    QState m_readResourceState;
    QState m_readResourceInfosState;
    QState m_readResourceInfoState;
    QState m_readResourceInfoDoneState;
    QState m_pcbserverConnectState;
    QState m_readSamplenrState;
    QState m_readChannelInformationState;
    QState m_readChannelAliasState;
    QState m_readDspChannelState;
    QState m_readDspChannelDoneState;

    QState m_claimPGRMemState;
    QState m_claimUSERMemState;
    QState m_var2DSPState;
    QState m_cmd2DSPState;
    QState m_activateDSPState;
    QFinalState m_loadDSPDoneState;

    // statemachine for deactivating
    QState m_deactivateDSPState;
    QState m_freePGRMemState;
    QState m_freeUSERMemState;
    QFinalState m_unloadDSPDoneState;

    // statemachine for reading actual values
    QStateMachine m_dataAcquisitionMachine;
    QState m_dataAcquisitionState;
    QFinalState m_dataAcquisitionDoneState;

    void setActualValuesNames();

private slots:
    void setInterfaceActualValues(QVector<float> *actualValues);

    void resourceManagerConnect();
    void sendRMIdent();
    void readResourceTypes();
    void readResource();
    void readResourceInfos();
    void readResourceInfo();
    void readResourceInfoDone();

    void pcbserverConnect();
    void readSamplenr();
    void readChannelInformation();
    void readChannelAlias();
    void readDspChannel();
    void readDspChannelDone();

    void claimPGRMem();
    void claimUSERMem();
    void varList2DSP();
    void cmdList2DSP();
    void activateDSP();
    void activateDSPdone();

    void deactivateDSP();
    void freePGRMem();
    void freeUSERMem();
    void deactivateDSPdone();

    void dataAcquisitionDSP();
    void dataReadDSP();

    void newIntegrationtime(QVariant ti);

};

#endif // RMSMODULEMEASPROGRAM_H