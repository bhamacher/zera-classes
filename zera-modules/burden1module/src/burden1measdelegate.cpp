#include <QDebug>

#include <veinmoduleactvalue.h>
#include <useratan.h>

#include "debug.h"
#include "burden1measdelegate.h"


namespace  BURDEN1MODULE
{

cBurden1MeasDelegate::cBurden1MeasDelegate(cVeinModuleActvalue *actburden, cVeinModuleActvalue *actpowerfactor, cVeinModuleActvalue *actrelburden, QString mode, bool withSignal)
    :m_pActBurden(actburden), m_pActPowerFactor(actpowerfactor), m_pActRelativeBurden(actrelburden), m_sMode(mode), m_bSignal(withSignal)
{
}


void cBurden1MeasDelegate::actValueInput1(QVariant val)
{
    QList<double> list;
    list = val.value<QList<double> >();
    if (list.count() >= 2) // normaly this is true, but we test to avoid crashing
    {
        // we compute vector as complex primary actual values
        m_fVoltageVector = complex(list.at(0), list.at(1));
        computeOutput();
    }
}


void cBurden1MeasDelegate::actValueInput2(QVariant val)
{
    QList<double> list;
    list = val.value<QList<double> >();
    if (list.count() >= 2) // normaly this is true, but we test to avoid crashing
    {
        // we compute vector as complex primary actual values
        m_fCurrentVector = complex(list.at(0), list.at(1));
        if (m_bSignal)
            emit measuring(0);

        computeOutput();

        if (m_bSignal)
            emit measuring(1);
    }
}


void cBurden1MeasDelegate::setNominalBurden(QVariant val)
{
    m_fNominalBurden = val.toDouble();
}


void cBurden1MeasDelegate::setNominalRange(QVariant val)
{
    m_fNominalRange = val.toDouble();
}


void cBurden1MeasDelegate::setWireLength(QVariant val)
{
    m_fWireLength = val.toDouble();
}


void cBurden1MeasDelegate::setWireCrosssection(QVariant val)
{
    m_fWireCrosssection = val.toDouble();
}


void cBurden1MeasDelegate::computeOutput()
{
    double Rwire, ueff, ieff;

    Rwire = (1.0/56) * m_fWireLength / m_fWireCrosssection;
    ueff = fabs(m_fVoltageVector) / 1.41421356;
    ieff = fabs(m_fCurrentVector) / 1.41421356;

    // computation of burden, powerfactoe and rel. burden !!! vectors are complex !!!

    if (m_sMode == "V")
    {
        m_fActBurden = (m_fNominalRange * m_fNominalRange) * ieff / (ueff + Rwire * ieff);
    }
    else
    {
        m_fActBurden = ((m_fNominalRange * m_fNominalRange) * (ueff + Rwire * ieff)) / ieff;
    }

    m_fActPowerFactor = cos(userAtan(m_fVoltageVector.im(), m_fVoltageVector.re()) - userAtan(m_fCurrentVector.im(), m_fCurrentVector.re()));
    m_fActRelativeBurden = m_fActBurden *100.0 / m_fNominalBurden;

    m_pActBurden->setValue(m_fActBurden);
    m_pActPowerFactor->setValue(m_fActPowerFactor);
    m_pActRelativeBurden->setValue(m_fActRelativeBurden);

#ifdef DEBUG
    QString ts;
    ts = QString("Burden: %1; cosß: %2; rel.Burden: %3%").arg(m_fActBurden).arg(m_fActPowerFactor).arg(m_fActRelativeBurden);
    qDebug() << ts;
#endif

}


}