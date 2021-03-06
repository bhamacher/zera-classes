#ifndef XMLCONFIGREADER_PRIVATE_H
#define XMLCONFIGREADER_PRIVATE_H

#include "messagehandler.h"

#include <QList>
#include <QString>
#include <QDebug>


/**
 * @b A 2d structure that mirrors some of the Qt container API functions and preserves the order of inserted values
 */
template<class T_key, class T_value> class FCKDUPHash {

public:
  void clear()
  {
    m_keyList.clear();
    m_valueList.clear();
  }

  void insert(T_key key, T_value value)
  {
    if(contains(key) == false)
    {
      m_keyList.append(key);
      m_valueList.append(value);
    }
    else
    {
      m_valueList.replace(m_keyList.indexOf(key), value);
    }
  }


  T_value value(T_key key) const
  {
    T_value retVal;
    if(contains(key))
    {
      retVal = m_valueList.at(m_keyList.indexOf(key));
    }
    return retVal;
  }

  bool contains(T_key key) const
  {
    return m_keyList.contains(key);
  }

  QList<T_key> keys() const
  {
    return m_keyList;
  }

  QList<T_value> values() const
  {
    return m_valueList;
  }

private:

  QList<T_key> m_keyList;
  QList<T_value> m_valueList;
};


namespace Zera
{
  namespace XMLConfig
  {
    class cReader;

    /**
     * @brief Private data of Zera::XMLConfig::Reader for ABI safety
     */
    class cReaderPrivate
    {
    public:
      /**
       * @brief cReaderPrivate Default constructor
       * @param parent the q_ptr of this instance
       */
      cReaderPrivate(cReader *parent);

    private:

      /**
       * @brief data Data member holding all config values
       */
       FCKDUPHash<QString, QString>data;

      /**
       * @brief schemaFilePath cached path to the schema file
       */
      QString schemaFilePath;

      MessageHandler messageHandler;

      /**
       * @brief q_ptr See Q_DECLARE_PUBLIC
       */
      Zera::XMLConfig::cReader *q_ptr;

      Q_DECLARE_PUBLIC(cReader)
    };
  }
}
#endif // XMLCONFIGREADER_PRIVATE_H
