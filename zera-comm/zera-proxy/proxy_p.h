#ifndef PROXY_P_H
#define PROXY_P_H

#include <QObject>
#include <QHash>
#include <QString>
#include <zeraclientnetbase.h>
#include <netmessages.pb.h>

#include "proxy.h"


namespace Zera
{
namespace Proxy
{

class cProxy;
class cProxyClient;
class cProxyClientPrivate;
class cProxyConnection;

class cProxyPrivate: public QObject
{
    Q_OBJECT

public:
    quint32 transmitCommand(cProxyClientPrivate *client, ProtobufMessage::NetMessage *message);

protected:
    cProxyPrivate(cProxy *parent);
    ~cProxyPrivate(){}
    cProxyClient* getConnection(QString ipadress, quint16 port);
    cProxyClient* getConnection(quint16 port);
    void setIPAdress(QString ipAddress);

    /**
       * @brief singletonInstance
       */
    static cProxy* singletonInstance;

    /**
       * @brief q_ptr See Q_DECLARE_PUBLIC
       */
    cProxy *q_ptr;

protected slots:
    void receiveMessage(QByteArray message);
    void receiveTcpError(QAbstractSocket::SocketError errorCode);

private:
    Q_DISABLE_COPY(cProxyPrivate)
    Q_DECLARE_PUBLIC(cProxy)

    Zera::NetClient::cClientNetBase* searchConnection(QString ip, quint16 port); // we search for a netclient that matches ip, port
    QHash<cProxyClientPrivate*, cProxyConnection*> m_ConnectionHash; // holds network connection for each client
    QHash<QByteArray, cProxyClientPrivate*> m_ClientHash; // information for faster redirecting
    QString m_sIPAdress; // ip adress for all zera servers, default localhost
    quint32 m_nMessageNumber; // message number, .. we never use 0    

};

}
}



#endif // PROXY_P_H