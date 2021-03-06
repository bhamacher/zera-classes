#include <scpi.h>
#include <scpicommand.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantList>

#include <ve_storagesystem.h>
#include <ve_commandevent.h>
#include <vcmp_componentdata.h>

#include "scpimodule.h"
#include "scpicmdinfo.h"
#include "scpipropertydelegate.h"
#include "scpiclient.h"


namespace SCPIMODULE
{

cSCPIPropertyDelegate::cSCPIPropertyDelegate(QString cmdParent, QString cmd, quint8 type, cSCPIModule *scpimodule,  cSCPICmdInfo* scpicmdinfo)
    :cSCPIDelegate(cmdParent, cmd, type), m_pModule(scpimodule), m_pSCPICmdInfo(scpicmdinfo)
{
    setOutput(m_pSCPICmdInfo);
}


cSCPIPropertyDelegate::~cSCPIPropertyDelegate()
{
    delete m_pSCPICmdInfo;
}


bool cSCPIPropertyDelegate::executeSCPI(cSCPIClient *client, QString &sInput)
{
    quint8 scpiCmdType;

    scpiCmdType = getType();
    cSCPICommand cmd = sInput;

    if (cmd.isQuery() && ((scpiCmdType & SCPI::isQuery) > 0)) // test if we got an allowed query
    {
        client->receiveAnswer(m_sAnswer);
    }
    else
        client->receiveStatus(SCPI::nak);

    return true;
}


void cSCPIPropertyDelegate::setOutput(cSCPICmdInfo *scpicmdinfo)
{
    QVariant ModInterface = m_pModule->m_pStorageSystem->getStoredValue(scpicmdinfo->entityId, QString("INF_ModuleInterface"));
    setOutput(ModInterface);
}


void cSCPIPropertyDelegate::setOutput(QVariant modInterface)
{
    QJsonDocument jsonDoc;
    jsonDoc = QJsonDocument::fromJson(modInterface.toByteArray());

    QJsonObject jsonObj;

    jsonObj = jsonDoc.object();
    jsonObj = jsonObj["ComponentInfo"].toObject();
    jsonObj = jsonObj[m_pSCPICmdInfo->componentName].toObject();
    jsonObj = jsonObj["Validation"].toObject();

    QJsonArray jsonArr;
    jsonArr = jsonObj["Data"].toArray();

    QVariantList vaList;
    vaList = jsonArr.toVariantList();

    m_sAnswer = vaList.at(0).toString();
    int n;
    n = vaList.count();
    if (n > 1)
    {
        for (int i = 1; i < vaList.count(); i++)
            m_sAnswer = m_sAnswer +";" + vaList.at(i).toString();
    }
}

}




