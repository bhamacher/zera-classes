#include "dftmodulefactory.h"
#include "dftmodule.h"

namespace DFTMODULE
{

ZeraModules::VirtualModule* DftModuleFactory::createModule(Zera::Proxy::cProxy* proxy, VeinPeer *peer, QObject *parent)
{
    ZeraModules::VirtualModule *module = new cDftModule(m_ModuleList.count()+1, proxy, peer, parent);
    m_ModuleList.append(module);
    return module;
}

void DftModuleFactory::destroyModule(ZeraModules::VirtualModule *module)
{
    m_ModuleList.removeAll(module);
    module->deleteLater();
}


QList<ZeraModules::VirtualModule *> DftModuleFactory::listModules()
{
    return m_ModuleList;
}


QString DftModuleFactory::getFactoryName()
{
    return QString("dftmodule");
}

}

