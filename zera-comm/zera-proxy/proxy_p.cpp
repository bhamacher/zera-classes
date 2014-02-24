#include <QUuid>
#include <zeraclientnetbase.h>

#include "proxy_p.h"
#include "proxyclient_p.h"
#include "proxyconnection.h"

namespace Zera
{
namespace Proxy
{


cProxy* cProxyPrivate::singletonInstance=0;


cProxyPrivate::cProxyPrivate(cProxy *parent):
    q_ptr(parent)
{
    m_sIPAdress = "127.0.0.1"; // our default
    m_nMessageNumber = 0;
}


cProxyClient* cProxyPrivate::getConnection(QString ipadress, quint16 port)
{
    Zera::NetClient::cClientNetBase* netClient;
    QUuid uuid;
    QByteArray binUUid;

    if (ipadress == "")
        ipadress = m_sIPAdress;

    if ((netClient = searchConnection(ipadress, port)) == 0) // look for existing connection
    {
        // if not existing we have to create
        netClient = new Zera::NetClient::cClientNetBase(this);
        connect(netClient, SIGNAL(messageAvailable(QByteArray)), this, SLOT(receiveMessage(QByteArray)));
        connect(netClient, SIGNAL(tcpError(QAbstractSocket::SocketError)), this, SLOT(receiveTcpError(QAbstractSocket::SocketError)));
        netClient->startNetwork(ipadress, port);
    }

    cProxyClientPrivate *proxyclient = new cProxyClientPrivate(this);

    uuid.createUuid(); // we use a per client uuid
    binUUid = uuid.toRfc4122();

    cProxyConnection *connection = new cProxyConnection(ipadress, port, binUUid, netClient);
    m_ConnectionHash[proxyclient] = connection;
    m_ClientHash[binUUid] = proxyclient;

    return proxyclient;
}


cProxyClient* cProxyPrivate::getConnection(quint16 port)
{
    return getConnection(m_sIPAdress, port);
}


quint32 cProxyPrivate::transmitCommand(cProxyClientPrivate* client, ProtobufMessage::NetMessage *message)
{
    quint32 nr;

    message->set_clientid(m_ConnectionHash[client]->m_binUUID); // we put the client's id into message

    if (m_nMessageNumber == 0) m_nMessageNumber++; // we never use 0 as message number (reserved)
    message->set_messagenr(nr = m_nMessageNumber);
    m_nMessageNumber++; // increment message number

    m_ConnectionHash[client]->m_pNetClient->sendMessage(message);
    return nr;
}


void cProxyPrivate::setIPAdress(QString ipAddress)
{
    m_sIPAdress = ipAddress;
}


void cProxyPrivate::receiveMessage(QByteArray message)
{
    QHash<cProxyClientPrivate*, cProxyConnection*>::Iterator it;

    ProtobufMessage::NetMessage *netMessage = new ProtobufMessage::NetMessage();
    it = m_ConnectionHash.begin(); // we take first interface to "read" the data

    it.value()->m_pNetClient->readMessage(netMessage, message);

    QByteArray key(netMessage->clientid().c_str(), netMessage->clientid().size());
    if (m_ClientHash.contains(key))
    {
        m_ClientHash[key]->transmitAnswer(netMessage);
    }
    // ? error handling in case of unknown clientid ?
}


void cProxyPrivate::receiveTcpError(QAbstractSocket::SocketError errorCode)
{
    Zera::NetClient::cClientNetBase* netClient = qobject_cast<Zera::NetClient::cClientNetBase*>(QObject::sender());

    QHashIterator<cProxyClientPrivate*, cProxyConnection*> it(m_ConnectionHash);

    while (it.hasNext())
    {
        cProxyConnection *pC;
        pC = it.value();
        if (pC->m_pNetClient == netClient) // we found a client that was connected to netclient
        {
            cProxyClientPrivate* client = it.key();
            client->transmitError(errorCode); // so this client will forward error
        }
    }
}


Zera::NetClient::cClientNetBase* cProxyPrivate::searchConnection(QString ip, quint16 port)
{
    Zera::NetClient::cClientNetBase* lnetClient = 0;
    QHashIterator<cProxyClientPrivate*, cProxyConnection*> it(m_ConnectionHash);

    while (it.hasNext())
    {
        cProxyConnection *pC;
        pC = it.value();
        if ( (pC->m_sIP == ip) && (pC->m_nPort == port))
        {
             lnetClient = pC->m_pNetClient;
             break;
        }
    }

    return lnetClient;
}

}
}