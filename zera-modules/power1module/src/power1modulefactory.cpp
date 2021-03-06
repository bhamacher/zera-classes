#include "power1modulefactory.h"
#include "power1module.h"

namespace POWER1MODULE
{

ZeraModules::VirtualModule* Power1ModuleFactory::createModule(Zera::Proxy::cProxy* proxy, int entityId, VeinEvent::StorageSystem *storagesystem, QObject *parent)
{
    ZeraModules::VirtualModule *module = new cPower1Module(m_ModuleList.count()+1, proxy, entityId, storagesystem, parent);
    m_ModuleList.append(module);
    return module;
}

void Power1ModuleFactory::destroyModule(ZeraModules::VirtualModule *module)
{
    m_ModuleList.removeAll(module);
    connect(module, SIGNAL(deactivationReady()), module, SIGNAL(moduleDeactivated()));
    if (!module->m_DeactivationMachine.isRunning())
        module->m_DeactivationMachine.start();
}


QList<ZeraModules::VirtualModule *> Power1ModuleFactory::listModules() const
{
    return m_ModuleList;
}


QString Power1ModuleFactory::getFactoryName() const
{
    return QString(BaseModuleName).toLower();
}

}

