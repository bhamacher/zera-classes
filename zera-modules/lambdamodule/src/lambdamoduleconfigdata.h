#ifndef LAMBDAMODULECONFIGDATA_H
#define LAMBDAMODULECONFIGDATA_H

#include <QString>
#include <QStringList>
#include <QList>

#include "socket.h"

namespace LAMBDAMODULE
{

struct lambdasystemconfiguration
{
    quint16 m_nInputPEntity; // entity for p input
    QString m_sInputP; // component name for p input
    quint16 m_nInputSEntity; // entity for s input
    QString m_sInputS; // component name for current input
};


class cLambdaModuleConfigData
{
public:
    cLambdaModuleConfigData(){}

    quint8 m_nDebugLevel;
    quint8 m_nLambdaSystemCount;
    int m_nModuleId;
    QList<lambdasystemconfiguration> m_lambdaSystemConfigList;
    QStringList m_lambdaChannelList;
};

}
#endif // LAMBDAMODULECONFIGDATA_H
