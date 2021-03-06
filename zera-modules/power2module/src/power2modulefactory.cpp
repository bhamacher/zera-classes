#include "power2modulefactory.h"
#include "power2module.h"

namespace POWER2MODULE
{

ZeraModules::VirtualModule* Power2ModuleFactory::createModule(Zera::Proxy::cProxy* proxy, int entityId, VeinEvent::StorageSystem* storagesystem, QObject* parent)
{
    ZeraModules::VirtualModule *module = new cPower2Module(m_ModuleList.count()+1, proxy, entityId, storagesystem, parent);
    m_ModuleList.append(module);
    return module;
}

void Power2ModuleFactory::destroyModule(ZeraModules::VirtualModule *module)
{
    m_ModuleList.removeAll(module);
    connect(module, SIGNAL(deactivationReady()), module, SIGNAL(moduleDeactivated()));
    if (!module->m_DeactivationMachine.isRunning())
        module->m_DeactivationMachine.start();
}


QList<ZeraModules::VirtualModule *> Power2ModuleFactory::listModules() const
{
    return m_ModuleList;
}


QString Power2ModuleFactory::getFactoryName() const
{
    return QString(BaseModuleName).toLower();
}

}

