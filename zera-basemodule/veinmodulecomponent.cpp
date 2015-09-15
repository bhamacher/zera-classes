#include <QJsonObject>
#include <QJsonArray>

#include <vcmp_componentdata.h>
#include <ve_commandevent.h>
#include <ve_eventsystem.h>

#include "veinmodulecomponent.h"


cVeinModuleComponent::cVeinModuleComponent(int entityId, VeinEvent::EventSystem *eventsystem, QString name, QString description, QVariant initval)
    :m_nEntityId(entityId), m_pEventSystem(eventsystem), m_sName(name), m_sDescription(description), m_vValue(initval)
{
    sendNotification(VeinComponent::ComponentData::Command::CCMD_ADD);
}


void cVeinModuleComponent::exportMetaData(QJsonObject &jsObj)
{
    QJsonObject jsonObj;

    jsonObj.insert("Description", m_sDescription);
    if (!m_sChannelName.isEmpty())
        jsonObj.insert("ChannelName", m_sChannelName);
    if (!m_sChannelUnit.isEmpty())
        jsonObj.insert("Unit", m_sChannelUnit);

    jsObj.insert(m_sName, jsonObj);
}


void cVeinModuleComponent::setChannelName(QString name)
{
    m_sChannelName = name;
}


void cVeinModuleComponent::setUnit(QString unit)
{
    m_sChannelUnit = unit;
}


QString cVeinModuleComponent::getName()
{
    return m_sName;
}


void cVeinModuleComponent::setValue(QVariant value)
{
    m_vValue = value;
    sendNotification(VeinComponent::ComponentData::Command::CCMD_SET);
}


void cVeinModuleComponent::sendNotification(VeinComponent::ComponentData::Command vcmd)
{
    VeinComponent::ComponentData *cData;

    cData = new VeinComponent::ComponentData();

    cData->setEntityId(m_nEntityId);
    cData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
    cData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);
    cData->setCommand(vcmd);
    cData->setComponentName(m_sName);
    cData->setNewValue(m_vValue);

    VeinEvent::CommandEvent *event;
    event = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, cData);

    m_pEventSystem->sigSendEvent(event);
}
