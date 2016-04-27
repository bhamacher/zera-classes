#ifndef PCBINTERFACE_P_H
#define PCBINTERFACE_P_H

#include <QObject>

#include "interface_p.h"

namespace Zera
{
namespace Server
{

enum pcbcommands
{
    getdspchannel,
    getstatus,
    resetstatus,
    getalias,
    gettype,
    getunit,
    getrange,
    getrangelist,
    getalias2,
    gettype2,
    geturvalue,
    geturvalue2,
    getrejection,
    getovrejection,
    isavail,
    getgaincorrection,
    getoffsetcorrection,
    getphasecorrection,
    setrange,
    setmeasuringmode,

    regnotifier,
    unregnotifier,
    pcbinterrupt,
    getsamplerate,
    getaliassample,
    getrangelistsample,
    setrangesample,
    setpllchannel,

    getaliassource,
    getdspchannelsource,
    getadjustmentstatus,
    getformfactorsource,
    getconstantsource,
    setconstantsource,

    getaliassschead,
    getaliasfrqinput,
    resourcealiasquery,
};


class cPCBInterface;

class cPCBInterfacePrivate: public cInterfacePrivate
{
    Q_OBJECT

public:
    cPCBInterfacePrivate(cPCBInterface* iface);
    virtual void setClient(Zera::Proxy::cProxyClient *client);
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
    virtual quint32 isAvail(QString chnName, QString rngName); // bool
    virtual quint32 getGainCorrection(QString chnName, QString rngName, double ampl); // double
    virtual quint32 getOffsetCorrection(QString chnName, QString rngName, double ampl); // double
    virtual quint32 getPhaseCorrection(QString chnName, QString rngName, double ampl); // double
    virtual quint32 setRange(QString chnName, QString rngName); // reply (ack, nak..)
    virtual quint32 setMMode(QString mmode);

    // all commands to source interface
    virtual quint32 getAliasSource(QString chnName); // qstring
    virtual quint32 getDSPChannelSource(QString chnName); // int
    virtual quint32 getFormFactorSource(QString chnName); // double
    virtual quint32 getConstantSource(QString chnName); // double
    virtual quint32 setConstantSource(QString chnName, double constant); // double

    // all commands to sample interface
    virtual quint32 getAliasSample(QString chnName); // qstring
    virtual quint32 getSampleRate(); // int the actual set sampling rate
    virtual quint32 getRangeListSample(QString chnName); // qstringlist
    virtual quint32 setRangeSample(QString chnName, QString rngName);
    virtual quint32 setPLLChannel(QString samplechnName, QString pllchnName);

    // all commands to schead interface
    virtual quint32 getAliasSchead(QString chnName); // qstring

    // all commands to frqinput interface
    virtual quint32 getAliasFrqinput(QString chnName); // qstring

    // all commands to status interface
    virtual quint32 getAdjustmentStatus(); // int

    // resource query more generell
    virtual quint32 resourceAliasQuery(QString resourceType, QString resourceName);

    virtual quint32 registerNotifier(QString query, QString notifier); // register for notification on change
    virtual quint32 unregisterNotifiers(); // unregister from all notifications


protected slots:
    virtual void receiveAnswer(ProtobufMessage::NetMessage *message);
    virtual void receiveError(QAbstractSocket::SocketError errorCode);

private:
    Q_DECLARE_PUBLIC(cPCBInterface)
    cPCBInterface *q_ptr;
};

}
}

#endif // PCBINTERFACE_P_H
