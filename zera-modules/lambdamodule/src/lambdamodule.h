#ifndef LAMBDAMODULE_H
#define LAMBDAMODULE_H

#include <QObject>
#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include <QList>

#include "basemeasmodule.h"

class cModuleError;

namespace Zera {
namespace Server {
 class cDSPInterface;
}

namespace Proxy {
 class cProxyClient;
}
}


namespace LAMBDAMODULE
{

class cLambdaModuleConfiguration;
class cLambdaModuleMeasProgram;
class cLambdaModuleObservation;

#define BaseModuleName "LambdaModule"
#define BaseSCPIModuleName "LAM"

class cLambdaModule : public cBaseMeasModule
{
Q_OBJECT

public:
    cLambdaModule(quint8 modnr, Zera::Proxy::cProxy* proxy, int entityId, VeinEvent::StorageSystem* storagesystem, QObject* parent = 0);
    virtual ~cLambdaModule();
    virtual QByteArray getConfiguration() const;

protected:
    cLambdaModuleObservation *m_pPower3ModuleObservation;
    cLambdaModuleMeasProgram *m_pMeasProgram; // our measuring program, lets say the working horse
    virtual void doConfiguration(QByteArray xmlConfigData); // here we have to do our configuration
    virtual void setupModule(); // after xml configuration we can setup and export our module
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
    qint32 m_nActivationIt;

private slots:
    void activationStart();
    void activationExec();
    void activationDone();
    void activationFinished();

    void deactivationStart();
    void deactivationExec();
    void deactivationDone();
    void deactivationFinished();

    void lambdaModuleReconfigure();

};

}

#endif // LAMBDAMODULE_H
