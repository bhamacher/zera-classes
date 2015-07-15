#include "rangemodulefactory.h"
#include "rangemodule.h"

namespace RANGEMODULE
{

ZeraModules::VirtualModule* RangeModuleFactory::createModule(Zera::Proxy::cProxy* proxy,  int entityId, VeintEvent::EventSystem* eventsystem, QObject *parent)
{
    ZeraModules::VirtualModule *module = new cRangeModule(m_ModuleList.count()+1, proxy, entityId, eventsystem, parent);
    m_ModuleList.append(module);
    return module;
}


void RangeModuleFactory::destroyModule(ZeraModules::VirtualModule *module)
{
    m_ModuleList.removeAll(module);
    connect(module, SIGNAL(deactivationReady()), module, SIGNAL(moduleDeactivated()));
    if (!module->m_DeactivationMachine.isRunning())
        module->m_DeactivationMachine.start();
}


QList<ZeraModules::VirtualModule *> RangeModuleFactory::listModules()
{
    return m_ModuleList;
}


QString RangeModuleFactory::getFactoryName()
{
    return QString("rangemodule");
}

}

