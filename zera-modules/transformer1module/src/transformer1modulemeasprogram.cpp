#include <QString>
#include <QStateMachine>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>

#include <ve_storagesystem.h>
#include <modulevalidator.h>
#include <doublevalidator.h>
#include <veinmodulecomponentinput.h>
#include <veinmoduleactvalue.h>
#include <veinmodulemetadata.h>
#include <veinmoduleparameter.h>
#include <scpiinfo.h>

#include "debug.h"
#include "errormessages.h"
#include "reply.h"
#include "measmodeinfo.h"
#include "transformer1module.h"
#include "transformer1modulemeasprogram.h"
#include "transformer1measdelegate.h"


namespace TRANSFORMER1MODULE
{

cTransformer1ModuleMeasProgram::cTransformer1ModuleMeasProgram(cTransformer1Module* module, cTransformer1ModuleConfigData& configdata)
    :m_pModule(module), m_ConfigData(configdata)
{
    m_searchActualValuesState.addTransition(this, SIGNAL(activationContinue()), &m_activationDoneState);

    m_activationMachine.addState(&m_searchActualValuesState);
    m_activationMachine.addState(&m_activationDoneState);

    m_activationMachine.setInitialState(&m_searchActualValuesState);

    connect(&m_searchActualValuesState, SIGNAL(entered()), SLOT(searchActualValues()));
    connect(&m_activationDoneState, SIGNAL(entered()), SLOT(activateDone()));

    // setting up statemachine deactivation
    m_deactivateState.addTransition(this, SIGNAL(deactivationContinue()), &m_deactivateDoneState);

    m_deactivationMachine.addState(&m_deactivateState);
    m_deactivationMachine.addState(&m_deactivateDoneState);

    m_deactivationMachine.setInitialState(&m_deactivateState);

    connect(&m_deactivateState, SIGNAL(entered()), SLOT(deactivateMeas()));
    connect(&m_deactivateDoneState, SIGNAL(entered()), SLOT(deactivateMeasDone()));
}


cTransformer1ModuleMeasProgram::~cTransformer1ModuleMeasProgram()
{
    for (int i = 0; i < m_Transformer1MeasDelegateList.count(); i++)
        delete m_Transformer1MeasDelegateList.at(i);
}


void cTransformer1ModuleMeasProgram::start()
{
    connect(this, SIGNAL(actualValues(QVector<float>*)), this, SLOT(setInterfaceActualValues(QVector<float>*)));
}


void cTransformer1ModuleMeasProgram::stop()
{
    disconnect(this, SIGNAL(actualValues(QVector<float>*)), this, 0);
}


void cTransformer1ModuleMeasProgram::generateInterface()
{
    cVeinModuleActvalue *pActvalue;
    cSCPIInfo* pSCPIInfo;
    QString key, s;

    for (int i = 0; i < m_ConfigData.m_nTransformerSystemCount; i++)
    {
        pActvalue = new cVeinModuleActvalue(m_pModule->m_nEntityId, m_pModule->m_pModuleValidator,
                                            QString("ACT_ERROR%1").arg(i+1),
                                            QString("Component forwards transformer transmission error value"),
                                            QVariant(0.0) );
        pActvalue->setChannelName(QString("ERR%1").arg(i+1));
        pActvalue->setUnit("");

        pSCPIInfo = new cSCPIInfo("MEASURE", pActvalue->getChannelName(), "8", pActvalue->getName(), "0", pActvalue->getUnit());
        pActvalue->setSCPIInfo(pSCPIInfo);

        m_ActValueList.append(pActvalue); // we add the component for our measurement
        m_pModule->veinModuleActvalueList.append(pActvalue); // and for the modules interface

        pActvalue = new cVeinModuleActvalue(m_pModule->m_nEntityId, m_pModule->m_pModuleValidator,
                                            QString("ACT_ANGLE%1").arg(i+1),
                                            QString("Component forwards transformer angle deviation value"),
                                            QVariant(0.0) );
        pActvalue->setChannelName(QString("ANG%1").arg(i+1));
        pActvalue->setUnit("°");

        pSCPIInfo = new cSCPIInfo("MEASURE", pActvalue->getChannelName(), "8", pActvalue->getName(), "0", pActvalue->getUnit());
        pActvalue->setSCPIInfo(pSCPIInfo);

        m_ActValueList.append(pActvalue); // we add the component for our measurement
        m_pModule->veinModuleActvalueList.append(pActvalue); // and for the modules interface

        pActvalue = new cVeinModuleActvalue(m_pModule->m_nEntityId, m_pModule->m_pModuleValidator,
                                            QString("ACT_RATIO%1").arg(i+1),
                                            QString("Component forwards transformer ratio value"),
                                            QVariant(0.0) );
        pActvalue->setChannelName(QString("RAT%1").arg(i+1));
        pActvalue->setUnit("");

        pSCPIInfo = new cSCPIInfo("MEASURE", pActvalue->getChannelName(), "8", pActvalue->getName(), "0", pActvalue->getUnit());
        pActvalue->setSCPIInfo(pSCPIInfo);

        m_ActValueList.append(pActvalue); // we add the component for our measurement
        m_pModule->veinModuleActvalueList.append(pActvalue); // and for the modules interface
    }

    m_pPrimClampPrimParameter = new cVeinModuleParameter(m_pModule->m_nEntityId, m_pModule->m_pModuleValidator,
                                                         key = QString("PAR_PrimClampPrim"),
                                                         QString("Component for setting the modules primary clamp primary value"),
                                                         QVariant(m_ConfigData.primClampPrim.m_fValue));
    s = QString(m_ConfigData.m_clampUnit[0]);
    m_pPrimClampPrimParameter->setUnit(s);
    m_pPrimClampPrimParameter->setSCPIInfo(new cSCPIInfo("CONFIGURATION","PCPRIMARY", "10", "PAR_PrimClampPrim", "0", s));

    cDoubleValidator *dValidator;
    dValidator = new cDoubleValidator(0.000001, 1000000.0, 0.000001);
    m_pPrimClampPrimParameter->setValidator(dValidator);

    m_pModule->veinModuleParameterHash[key] = m_pPrimClampPrimParameter; // for modules use

    m_pPrimClampSecParameter = new cVeinModuleParameter(m_pModule->m_nEntityId, m_pModule->m_pModuleValidator,
                                                        key = QString("PAR_PrimClampSec"),
                                                        QString("Component for setting the modules primary clamp secondary value"),
                                                        QVariant(m_ConfigData.primClampSec.m_fValue));
    s = QString(m_ConfigData.m_clampUnit[1]);
    m_pPrimClampSecParameter->setUnit(s);
    m_pPrimClampSecParameter->setSCPIInfo(new cSCPIInfo("CONFIGURATION","PCSECONDARY", "10", "PAR_PrimClampSec", "0", s));

    m_pPrimClampSecParameter->setValidator(dValidator);

    m_pModule->veinModuleParameterHash[key] = m_pPrimClampSecParameter; // for modules use

    m_pSecClampPrimParameter = new cVeinModuleParameter(m_pModule->m_nEntityId, m_pModule->m_pModuleValidator,
                                                        key = QString("PAR_SecClampPrim"),
                                                        QString("Component for setting the modules secondary clamp primary value"),
                                                        QVariant(m_ConfigData.secClampPrim.m_fValue));
    s = QString(m_ConfigData.m_clampUnit[2]);
    m_pSecClampPrimParameter->setUnit(s);
    m_pSecClampPrimParameter->setSCPIInfo(new cSCPIInfo("CONFIGURATION","SCPRIMARY", "10", "PAR_SecClampPrim", "0", s));

    m_pSecClampPrimParameter->setValidator(dValidator);

    m_pModule->veinModuleParameterHash[key] = m_pSecClampPrimParameter; // for modules use

    m_pSecClampSecParameter = new cVeinModuleParameter(m_pModule->m_nEntityId, m_pModule->m_pModuleValidator,
                                                       key = QString("PAR_SecClampSec"),
                                                       QString("Component for setting the modules secondary clamp secondary value"),
                                                       QVariant(m_ConfigData.secClampSec.m_fValue));
    s = QString(m_ConfigData.m_clampUnit[3]);
    m_pSecClampSecParameter->setUnit(s);
    m_pSecClampSecParameter->setSCPIInfo(new cSCPIInfo("CONFIGURATION","SCSECONDARY", "10", "PAR_SecClampSec", "0", s));

    m_pSecClampSecParameter->setValidator(dValidator);

    m_pModule->veinModuleParameterHash[key] = m_pSecClampSecParameter; // for modules use

    m_pPrimDutParameter = new cVeinModuleParameter(m_pModule->m_nEntityId, m_pModule->m_pModuleValidator,
                                                   key = QString("PAR_DutPrimary"),
                                                   QString("Component for setting the modules secondary clamp secondary value"),
                                                   QVariant(m_ConfigData.secClampSec.m_fValue));
    s = QString(m_ConfigData.m_clampUnit[4]);
    m_pPrimDutParameter->setUnit(s);
    m_pPrimDutParameter->setSCPIInfo(new cSCPIInfo("CONFIGURATION","DUTPRIMARY", "10", "PAR_DutPrimary", "0", s));

    m_pPrimDutParameter->setValidator(dValidator);

    m_pModule->veinModuleParameterHash[key] = m_pPrimDutParameter; // for modules use

    m_pSecDutParameter = new cVeinModuleParameter(m_pModule->m_nEntityId, m_pModule->m_pModuleValidator,
                                                  key = QString("PAR_DutSecondary"),
                                                  QString("Component for setting the modules secondary clamp secondary value"),
                                                  QVariant(m_ConfigData.secClampSec.m_fValue));
    s = QString(m_ConfigData.m_clampUnit[5]);
    m_pSecDutParameter->setUnit(s);
    m_pSecDutParameter->setSCPIInfo(new cSCPIInfo("CONFIGURATION","DUTSECONDARY", "10", "PAR_DutSecondary", "0", s));

    m_pSecDutParameter->setValidator(dValidator);

    m_pModule->veinModuleParameterHash[key] = m_pSecDutParameter; // for modules use

    m_pTRSCountInfo = new cVeinModuleMetaData(QString("TRSCount"), QVariant(m_ConfigData.m_nTransformerSystemCount));
    m_pModule->veinModuleMetaDataList.append(m_pTRSCountInfo);

    m_pMeasureSignal = new cVeinModuleComponent(m_pModule->m_nEntityId, m_pModule->m_pModuleValidator,
                                                QString("SIG_Measuring"),
                                                QString("Component forwards a signal indicating measurement activity"),
                                                QVariant(0));

    m_pModule->veinModuleComponentList.append(m_pMeasureSignal);
}


void cTransformer1ModuleMeasProgram::deleteInterface()
{
}


void cTransformer1ModuleMeasProgram::searchActualValues()
{
    bool error;

    error = false;
    QList<cVeinModuleComponentInput*> inputList;

    for (int i = 0; i < m_ConfigData.m_nTransformerSystemCount; i++)
    {
        // we first test that wanted input components exist
        if ( (m_pModule->m_pStorageSystem->hasStoredValue(m_ConfigData.m_nModuleId, m_ConfigData.m_transformerSystemConfigList.at(i).m_sInputPrimaryVector)) &&
             (m_pModule->m_pStorageSystem->hasStoredValue(m_ConfigData.m_nModuleId, m_ConfigData.m_transformerSystemConfigList.at(i).m_sInputSecondaryVector)) )
        {
            cTransformer1MeasDelegate *cTMD;
            cVeinModuleComponentInput *vmci;

            if (i == (m_ConfigData.m_nTransformerSystemCount-1))
            {
                cTMD = new cTransformer1MeasDelegate(m_ActValueList.at(i*3), m_ActValueList.at(i*3+1), m_ActValueList.at(i*3+2), true);
                connect(cTMD, SIGNAL(measuring(int)), this, SLOT(setMeasureSignal(int)));
            }
            else
                cTMD = new cTransformer1MeasDelegate(m_ActValueList.at(i*3), m_ActValueList.at(i*3+1), m_ActValueList.at(i*3+2));

            m_Transformer1MeasDelegateList.append(cTMD);

            vmci = new cVeinModuleComponentInput(m_ConfigData.m_nModuleId, m_ConfigData.m_transformerSystemConfigList.at(i).m_sInputPrimaryVector);
            inputList.append(vmci);
            connect(vmci, SIGNAL(sigValueChanged(QVariant)), cTMD, SLOT(actValueInput1(QVariant)));

            vmci = new cVeinModuleComponentInput(m_ConfigData.m_nModuleId, m_ConfigData.m_transformerSystemConfigList.at(i).m_sInputSecondaryVector);
            inputList.append(vmci);
            connect(vmci, SIGNAL(sigValueChanged(QVariant)), cTMD, SLOT(actValueInput2(QVariant)));
        }
        else
            error = true;
    }

    if (error)
        emit activationError();
    else
    {
        m_pEventSystem->setInputList(inputList);
        emit activationContinue();
    }
}


void cTransformer1ModuleMeasProgram::activateDone()
{
    m_bActive = true;
    emit activated();
}



void cTransformer1ModuleMeasProgram::deactivateMeas()
{
    m_bActive = false;

    for (int i = 0; i < m_Transformer1MeasDelegateList.count(); i++)
        delete m_Transformer1MeasDelegateList.at(i);

    emit deactivationContinue();
}


void cTransformer1ModuleMeasProgram::deactivateMeasDone()
{
    emit deactivated();
}


void cTransformer1ModuleMeasProgram::setMeasureSignal(int signal)
{
    m_pMeasureSignal->setValue(signal);
}

}


