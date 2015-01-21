#include "basesamplechannel.h"

cBaseSampleChannel::cBaseSampleChannel(Zera::Proxy::cProxy* proxy, VeinPeer *peer, cSocket* rmsocket, cSocket* pcbsocket, QString name, quint8 chnnr)
    :m_pProxy(proxy), m_pPeer(peer), m_pRMSocket(rmsocket), m_pPCBServerSocket(pcbsocket), m_sName(name), m_nChannelNr(chnnr)
{
    m_sUnit = "";
}


QString cBaseSampleChannel::getName()
{
    return m_sName;
}


QString cBaseSampleChannel::getAlias()
{
    return m_sAlias;
}

