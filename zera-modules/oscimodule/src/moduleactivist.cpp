#include "moduleactivist.h"

namespace OSCIMODULE
{

void cModuleActivist::activate()
{
    m_activationMachine.start();
}


void cModuleActivist::deactivate()
{
    m_deactivationMachine.start();
}

}
