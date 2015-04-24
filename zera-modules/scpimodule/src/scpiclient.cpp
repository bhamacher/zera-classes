#include <QTcpSocket>
#include <scpi.h>

#include "scpiclient.h"
#include "scpiinterface.h"

namespace SCPIMODULE
{

cSCPIClient::cSCPIClient(QTcpSocket *socket, cSCPIInterface *iface)
    :m_pSocket(socket), m_pSCPIInterface(iface)
{
    m_bAuthorisation = false;

    connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(cmdInput()));
    connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
}


cSCPIClient::~cSCPIClient()
{
    m_pSocket->abort();
}


void cSCPIClient::setAuthorisation(bool auth)
{
    m_bAuthorisation = auth;
}


void cSCPIClient::cmdInput()
{
    QString m_sInput;

    m_sInput = "";
    while ( m_pSocket->canReadLine() )
        m_sInput += m_pSocket->readLine();

    m_sInput.remove('\r'); // we remove cr lf
    m_sInput.remove('\n');

    m_pSCPIInterface->executeCmd(this, m_sInput);

    // QStringList cmds = m_sInput.split(';');

    //for (int i = 0; i < cmds.count(); i++)
        //m_pSCPIInterface->executeCmd(cmds.at(i));
}


void cSCPIClient::receiveStatus(quint8 stat)
{
    QString answer;
    QByteArray ba;
    answer = QString("%1\n").arg(SCPI::scpiAnswer[stat]);
    ba = answer.toLatin1();
    m_pSocket->write(ba);
}


void cSCPIClient::receiveAnswer(QString answ)
{
    QString answer;
    QByteArray ba;
    answer = answ +"\n";
    ba = answer.toLatin1();
    m_pSocket->write(ba);
}

}
