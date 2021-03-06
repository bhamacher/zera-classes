#ifndef PCBINTERFACE_H
#define PCBINTERFACE_H

#include <QObject>
#include <QAbstractSocket>
#include <QVariant>

#include "zera_misc_global.h"

namespace Zera
{
namespace Proxy
{
    class cProxyClient;
}
}


namespace Zera
{
namespace Server
{

class cPCBInterfacePrivate;


class ZERA_MISCSHARED_EXPORT cPCBInterface: public QObject
{
    Q_OBJECT

public:
    cPCBInterface();
    ~cPCBInterface();
    virtual void setClient(Zera::Proxy::cProxyClient *client);
    // all commands to sense interface
    virtual quint32 getDSPChannel(QString chnName); // int
    virtual quint32 getStatus(QString chnName); // int
    virtual quint32 resetStatus(QString chnName); // reply (ack, nak..)
    virtual quint32 getAlias(QString chnName); // qstring
    virtual quint32 getType(QString chnName); // qstring
    virtual quint32 getUnit(QString chnName); // qstring
    virtual quint32 getRange(QString chnName); // qstring
    virtual quint32 getRangeList(QString chnName); // qstringlist
    virtual quint32 getAlias(QString chnName, QString rngName); // qstring
    virtual quint32 getType(QString chnName, QString rngName); // int
    virtual quint32 getUrvalue(QString chnName, QString rngName); // double
    virtual quint32 getUrvalue(QString chnName); // double
    virtual quint32 getRejection(QString chnName, QString rngName); // double
    virtual quint32 getOVRejection(QString chnName, QString rngName); // double
    virtual quint32 getADCRejection(QString chnName, QString rngName); // double
    virtual quint32 isAvail(QString chnName, QString rngName); // bool
    virtual quint32 getGainCorrection(QString chnName, QString rngName, double at); // double
    virtual quint32 getAdjGainCorrection(QString chnName, QString rngName, double at); // double
    virtual quint32 getOffsetCorrection(QString chnName, QString rngName, double at); // double
    virtual quint32 getAdjOffsetCorrection(QString chnName, QString rngName, double at); // double
    virtual quint32 getPhaseCorrection(QString chnName, QString rngName, double at); // double
    virtual quint32 getAdjPhaseCorrection(QString chnName, QString rngName, double at); // double
    virtual quint32 setRange(QString chnName, QString rngName); // reply (ack, nak..)
    virtual quint32 setMMode(QString mmode); // we set the sense mode we get  reply (ack, nak..)
    virtual quint32 adjustComputation(); // all correction coefficients will be computed
    virtual quint32 adjustStorage(); // all correction data will be stored
    virtual quint32 adjustStorageClamp(); // all clamp correction data will be stored
    virtual quint32 adjustInit(QString chnName, QString rngName);
    virtual quint32 setAdjustGainStatus(QString chnName, QString rngName, int stat);
    virtual quint32 setAdjustPhaseStatus(QString chnName, QString rngName, int stat);
    virtual quint32 setAdjustOffsetStatus(QString chnName, QString rngName, int stat);
    virtual quint32 setGainNode(QString chnName, QString rngName, int nr, double corr, double at); // node nr, correction, loadpoint
    virtual quint32 setPhaseNode(QString chnName, QString rngName, int nr, double corr, double at); // node nr, correction, loadpoint = frequency
    virtual quint32 setOffsetNode(QString chnName, QString rngName, int nr, double corr, double at); // node nr, correction, loadpoint

    // all commands to sample interface
    virtual quint32 getAliasSample(QString chnName); // qstring
    virtual quint32 getSampleRate(); // int the actual set sampling rate
    virtual quint32 getRangeListSample(QString chnName); // qstringlist
    virtual quint32 setRangeSample(QString chnName, QString rngName);
    virtual quint32 setPLLChannel(QString samplechnName, QString pllchnName);

    // all commands to source interface
    virtual quint32 getAliasSource(QString chnName); // qstring
    virtual quint32 getDSPChannelSource(QString chnName); // int
    virtual quint32 getFormFactorSource(QString chnName); // double
    virtual quint32 getConstantSource(QString chnName); // double
    virtual quint32 setConstantSource(QString chnName, double constant); // double
    virtual quint32 getPowTypeSource(QString chnName); // qstring
    virtual quint32 setPowTypeSource(QString chnName, QString ptype);

    // all commands to schead interface
    virtual quint32 getAliasSchead(QString chnName); // qstring

    // all commands to frqinput interface
    virtual quint32 getAliasFrqinput(QString chnName); // qstring

    // all commands to status interface
    virtual quint32 getAdjustmentStatus(); // int
    virtual quint32 getAuthorizationStatus();
    virtual quint32 getPCBErrorStatus();

    // resource query more generell
    virtual quint32 resourceAliasQuery(QString resourceType, QString resourceName);

    virtual quint32 registerNotifier(QString query, QString notifier); // register for notification on change
    virtual quint32 unregisterNotifiers(); // unregister from all notifications

    // all commands to system interface
    virtual quint32 readServerVersion();
    virtual quint32 readPCBVersion();
    virtual quint32 readFPGAVersion();
    virtual quint32 readCTRLVersion();
    virtual quint32 readSerialNr();
    virtual quint32 getAdjustmentChksum();
    virtual quint32 getPCBAdjustmentData();
    virtual quint32 setPCBAdjustmentData(QString xmlpcb);
    virtual quint32 getClampAdjustmentData();
    virtual quint32 setClampAdjustmentData(QString xmlclamp);

    // sending a transparent command
    virtual quint32 transparentCommand(QString cmd);

signals:
    void tcpError(QAbstractSocket::SocketError errorCode);
    void serverAnswer(quint32 msgnr, quint8 reply, QVariant answer);

private:
    Q_DECLARE_PRIVATE(cPCBInterface)
    cPCBInterfacePrivate *d_ptr;
};

}
}

#endif // PCBINTERFACE_H
