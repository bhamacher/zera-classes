#ifndef SAMPLEMODULE_H
#define SAMPLEMODULE_H

#include <QObject>
#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include <QList>

#include "basemodule.h"


namespace Zera {
namespace Server {
 class cDSPInterface;
}

namespace Proxy {
 class cProxyClient;
}
}

class cModuleInfo;
class cModuleError;

namespace SAMPLEMODULE
{

class cSampleChannel;
class cPllMeasChannel;
class cSampleModuleConfiguration;
class cSampleModuleMeasProgram;
class cPllObsermatic;
class cSampleModuleObservation;


class cSampleModule : public cBaseModule
{
Q_OBJECT

public:
    cSampleModule(quint8 modnr, Zera::Proxy::cProxy* proxi, VeinPeer* peer, QObject* parent = 0);
    virtual ~cSampleModule();
    virtual QByteArray getConfiguration();
    cPllMeasChannel* getPllMeasChannel(QString name);

signals:
    void activationContinue();
    void activationNext();
    void deactivationContinue();
    void deactivationNext();

protected:
    cSampleModuleObservation *m_pSampleModuleObservation;
    cSampleModuleMeasProgram *m_pMeasProgram; // our measuring program, lets say the working horse
    cPllObsermatic *m_pPllObsermatic; // our pll handling

    virtual void doConfiguration(QByteArray xmlConfigData); // here we have to do our configuration
    virtual void setupModule(); // after xml configuration we can setup and export our module
    virtual void unsetModule(); // in case of reconfiguration we must unset module first
    virtual void startMeas(); // we make the measuring program start here
    virtual void stopMeas();

    // our states for base modules activation statemacine
    QState m_ActivationStartState;
    QState m_ActivationExecState;
    QState m_ActivationDoneState;
    QFinalState m_ActivationFinishedState;

    // our states for base modules deactivation statemacine
    QState m_DeactivationStartState;
    QState m_DeactivationExecState;
    QState m_DeactivationDoneState;
    QFinalState m_DeactivationFinishedState;

private:
    cModuleError *errorMessage;
    qint32 m_nActivationIt;
    QList<cPllMeasChannel*> m_pllMeasChannelList; // our pll meas channels
    QList<cSampleChannel*>  m_sampleChannelList;

private slots:
    void activationStart();
    void activationExec();
    void activationDone();
    void activationFinished();

    void deactivationStart();
    void deactivationExec();
    void deactivationDone();
    void deactivationFinished();

    void sampleModuleReconfigure();

};

}

#endif // SAMPLEMODULE_H