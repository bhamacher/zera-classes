#include <QPoint>
#include <QString>
#include <xmlconfigreader.h>

#include "lambdamoduleconfiguration.h"
#include "lambdamoduleconfigdata.h"
#include "socket.h"

namespace LAMBDAMODULE
{

cLambdaModuleConfiguration::cLambdaModuleConfiguration()
{
    m_pLambdaModulConfigData = 0;
    connect(m_pXMLReader, SIGNAL(valueChanged(const QString&)), this, SLOT(configXMLInfo(const QString&)));
    connect(m_pXMLReader, SIGNAL(finishedParsingXML(bool)), this, SLOT(completeConfiguration(bool)));
}


cLambdaModuleConfiguration::~cLambdaModuleConfiguration()
{
    if (m_pLambdaModulConfigData) delete m_pLambdaModulConfigData;
}


void cLambdaModuleConfiguration::setConfiguration(QByteArray xmlString)
{
    m_bConfigured = m_bConfigError = false;

    if (m_pLambdaModulConfigData) delete m_pLambdaModulConfigData;
    m_pLambdaModulConfigData = new cLambdaModuleConfigData();

    m_ConfigXMLMap.clear(); // in case of new configuration we completely set up

    // so now we can set up
    // initializing hash table for xml configuration

    m_ConfigXMLMap["lambdamodconfpar:configuration:connectivity:debuglevel"] = setDebugLevel;
    m_ConfigXMLMap["lambdamodconfpar:configuration:measure:system:n"] = setSystemCount;

    if (m_pXMLReader->loadSchema(defaultXSDFile))
        m_pXMLReader->loadXMLFromString(QString::fromUtf8(xmlString.data(), xmlString.size()));
    else
        m_bConfigError = true;
}


QByteArray cLambdaModuleConfiguration::exportConfiguration()
{
    return m_pXMLReader->getXMLConfig().toUtf8();
}


cLambdaModuleConfigData *cLambdaModuleConfiguration::getConfigurationData()
{
    return m_pLambdaModulConfigData;
}


void cLambdaModuleConfiguration::configXMLInfo(QString key)
{
    bool ok;

    if (m_ConfigXMLMap.contains(key))
    {
        ok = true;
        int cmd = m_ConfigXMLMap[key];
        switch (cmd)
        {
        case setDebugLevel:
            m_pLambdaModulConfigData->m_nDebugLevel = m_pXMLReader->getValue(key).toInt(&ok);
            break;
        case setSystemCount:
        {
            lambdasystemconfiguration lsc;

            lsc.m_nInputPEntity = 1006; // some default
            lsc.m_sInputP = "ACT_PQS1";
            lsc.m_nInputSEntity = 1006;
            lsc.m_sInputS = "ACT_PQS1";

            m_pLambdaModulConfigData->m_nLambdaSystemCount = m_pXMLReader->getValue(key).toInt(&ok);
            // here we generate dynamic hash entries for value channel configuration
            for (int i = 0; i < m_pLambdaModulConfigData->m_nLambdaSystemCount; i++)
            {
                m_ConfigXMLMap[QString("lambdamodconfpar:configuration:measure:system:lambda%1:p:inputentity").arg(i+1)] = setLambdaInputPEntity1 + i;
                m_ConfigXMLMap[QString("lambdamodconfpar:configuration:measure:system:lambda%1:p:component").arg(i+1)] = setLambdaInputPComponent1 + i;
                m_ConfigXMLMap[QString("lambdamodconfpar:configuration:measure:system:lambda%1:s:inputentity").arg(i+1)] = setLambdaInputSEntity1 + i;
                m_ConfigXMLMap[QString("lambdamodconfpar:configuration:measure:system:lambda%1:s:component").arg(i+1)] = setLambdaInputSComponent1 + i;

                m_pLambdaModulConfigData->m_lambdaSystemConfigList.append(lsc);
                m_pLambdaModulConfigData->m_lambdaChannelList.append(QString("ACT_Lambda%1").arg(i+1));
            }
            break;
        }
        default:
            // here we decode the dyn. generated cmd's

            if ((cmd >= setLambdaInputPEntity1) && (cmd < setLambdaInputPEntity1 + 12))
            {
                cmd -= setLambdaInputPEntity1;
                // it is command for setting measuring mode

                lambdasystemconfiguration lsc = m_pLambdaModulConfigData->m_lambdaSystemConfigList.at(cmd);
                lsc.m_nInputPEntity = m_pXMLReader->getValue(key).toInt();
                m_pLambdaModulConfigData->m_lambdaSystemConfigList.replace(cmd, lsc);
            }
            else
                if ((cmd >= setLambdaInputSEntity1) && (cmd < setLambdaInputSEntity1 + 12))
                {
                    cmd -= setLambdaInputSEntity1;
                    // it is command for setting measuring mode

                    lambdasystemconfiguration lsc = m_pLambdaModulConfigData->m_lambdaSystemConfigList.at(cmd);
                    lsc.m_nInputSEntity = m_pXMLReader->getValue(key).toInt();
                    m_pLambdaModulConfigData->m_lambdaSystemConfigList.replace(cmd, lsc);
                }
                else
                    if ((cmd >= setLambdaInputPComponent1) && (cmd < setLambdaInputPComponent1 + 12))
                    {
                        cmd -= setLambdaInputPComponent1;
                        // it is command for setting measuring mode

                        lambdasystemconfiguration lsc = m_pLambdaModulConfigData->m_lambdaSystemConfigList.at(cmd);
                        lsc.m_sInputP = m_pXMLReader->getValue(key);
                        m_pLambdaModulConfigData->m_lambdaSystemConfigList.replace(cmd, lsc);
                    }
                    else
                        if ((cmd >= setLambdaInputSComponent1) && (cmd < setLambdaInputSComponent1 + 12))
                        {
                            cmd -= setLambdaInputSComponent1;
                            // it is command for setting measuring mode

                            lambdasystemconfiguration lsc = m_pLambdaModulConfigData->m_lambdaSystemConfigList.at(cmd);
                            lsc.m_sInputS = m_pXMLReader->getValue(key);
                            m_pLambdaModulConfigData->m_lambdaSystemConfigList.replace(cmd, lsc);
                        }

        }
        m_bConfigError |= !ok;
    }

    else
        m_bConfigError = true;
}


void cLambdaModuleConfiguration::completeConfiguration(bool ok)
{
    m_bConfigured = (ok && !m_bConfigError);
    emit configXMLDone();
}

}

