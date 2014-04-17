#ifndef BASEMEASPROGRAM_H
#define BASEMEASPROGRAM_H

#include <QObject>
#include <QStringList>
#include <QVector>
#include <QHash>

#include "moduleacitvist.h"

class cBaseModule;
class VeinPeer;
class cSocket;

namespace Zera
{
namespace Server
{
    class cDSPInterface;
}
}


class cBaseMeasProgram: public cModuleActivist
{
    Q_OBJECT

public:
    cBaseMeasProgram(VeinPeer* peer, Zera::Server::cDSPInterface* iface, cSocket* rmsocket, QStringList chnlist);
    virtual ~cBaseMeasProgram();

signals:
    void actualValues(QVector<double>*);

public slots:
    virtual void activate() = 0; // here we query our properties and activate ourself
    virtual void deactivate() = 0; // what do you think ? yes you're right
    virtual void start() = 0; // difference between start and stop is that actual values
    virtual void stop() = 0; // in interface are not updated when stop

protected:
    VeinPeer* m_pPeer; // the peer where we set our entities
    Zera::Server::cDSPInterface* m_pDSPIFace; // our interface to dsp
    cSocket* m_pRMSocket;

    QStringList m_ChannelList; // the list of channels we work on
    QVector<double> m_pModuleActualValues; // a modules actual values
    QHash<quint32, int> m_MsgNrCmdList;

    virtual void generateInterface() = 0; // here we export our interface (entities)
    virtual void deleteInterface() = 0; // we delete interface in case of reconfiguration
    virtual void setDspVarList() = 0; // dsp related stuff
    virtual void deleteDspVarList() = 0;
    virtual void setDspCmdList() = 0;
    virtual void deleteDspCmdList() = 0;

};
#endif // BASEMEASPROGRAM_H