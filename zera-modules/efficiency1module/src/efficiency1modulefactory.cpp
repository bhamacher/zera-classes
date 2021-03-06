#include "efficiency1modulefactory.h"
#include "efficiency1module.h"

namespace EFFICIENCY1MODULE
{

ZeraModules::VirtualModule* Efficiency1ModuleFactory::createModule(Zera::Proxy::cProxy* proxy, int entityId, VeinEvent::StorageSystem* storagesystem, QObject* parent)
{
    ZeraModules::VirtualModule *module = new cEfficiency1Module(m_ModuleList.count()+1, proxy, entityId, storagesystem, parent);
    m_ModuleList.append(module);
    return module;
}

void Efficiency1ModuleFactory::destroyModule(ZeraModules::VirtualModule *module)
{
    m_ModuleList.removeAll(module);
    connect(module, SIGNAL(deactivationReady()), module, SIGNAL(moduleDeactivated()));
    if (!module->m_DeactivationMachine.isRunning())
        module->m_DeactivationMachine.start();
}


QList<ZeraModules::VirtualModule *> Efficiency1ModuleFactory::listModules() const
{
    return m_ModuleList;
}


QString Efficiency1ModuleFactory::getFactoryName() const
{
    return QString(BaseModuleName).toLower();
}

}

