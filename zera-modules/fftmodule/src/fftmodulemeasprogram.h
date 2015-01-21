#ifndef FFTMODULEMEASPROGRAM_H
#define FFTMODULEMEASPROGRAM_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QStateMachine>
#include <QState>
#include <QFinalState>

#include "basemeasprogram.h"
#include "measchannelinfo.h"

namespace Zera {
namespace Proxy {
    class cProxy;
}
}

class cDspMeasData;
class VeinPeer;
class VeinEntity;
class QStateMachine;
class QState;
class QFinalState;


class cModuleSignal;
class cModuleParameter;
class cModuleInfo;
class cBaseModule;
class cMovingwindowFilter;

namespace FFTMODULE
{

enum fftmoduleCmds
{
    resourcemanagerconnect,
    sendrmident,
    readresourcetypes,
    readresource,
    readresourceinfos,
    readresourceinfo,
    pcbserverconnect,
    readsamplerate,
    readalias,
    readunit,
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

#define irqNr 7

class cFftModuleConfigData;
class cFftModule;


class cFftModuleMeasProgram: public cBaseMeasProgram
{
    Q_OBJECT

public:
    cFftModuleMeasProgram(cFftModule* module, Zera::Proxy::cProxy* proxy, VeinPeer* peer, cFftModuleConfigData& configdata);
    virtual ~cFftModuleMeasProgram();
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
    cFftModule* m_pModule;
    cFftModuleConfigData& m_ConfigData;
    QStringList m_ActValueList; // the list of actual values we work on
    quint16 m_nfftLen;

    QList<VeinEntity*> m_EntityNamePNList; // we have a list for all rms names for phase neutral
    QList<VeinEntity*> m_EntityNamePPList; // and a list for all rms names for phase phase
    QList<VeinEntity*> m_EntityActValuePNList; // we have a list for all rms values for phase neutral
    QList<VeinEntity*> m_EntityActValuePPList; // and a list for all rms values for phase phase

    QList<VeinEntity*> m_EntityNameList;
    QList<VeinEntity*> m_EntityActValueList;

    QHash<QString, cMeasChannelInfo> m_measChannelInfoHash;
    QList<QString> channelInfoReadList; // a list of all channel info we have to read
    QString channelInfoRead; // the actual channel info we are working on
    quint32 m_nSRate; // number of samples / signal period

    cModuleParameter* m_pRefChannelParameter;
    cModuleInfo* m_pRefChannelInfo;
    cModuleSignal* m_pMeasureSignal;
    cModuleParameter* m_pIntegrationTimeParameter;
    cModuleInfo* m_pInfIntegrationTimeLimits;
    cModuleInfo* m_pFftOrderInfo;

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
    QState m_readSampleRateState;
    QState m_readChannelInformationState;
    QState m_readChannelAliasState;
    QState m_readChannelUnitState;
    QState m_readDspChannelState;
    QState m_readDspChannelDoneState;

    QState m_dspserverConnectState;
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
    cMovingwindowFilter* m_pMovingwindowFilter;
    QVector<float> m_FFTModuleActualValues;

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
    void readSampleRate();
    void readChannelInformation();
    void readChannelAlias();
    void readChannelUnit();
    void readDspChannel();
    void readDspChannelDone();

    void dspserverConnect();
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
    void newRefChannel(QVariant chn);

};

}
#endif // FFTMODULEMEASPROGRAM_H