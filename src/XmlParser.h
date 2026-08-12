#pragma once

#include "TypeItem.h"
#include <QString>
#include <QVector>

// Handles reading and writing the DayZ types.xml file
class XmlParser
{
public:
    XmlParser() = default;

    // Reads the file and returns a vector of TypeItem
    // Returns an empty vector on failure (check lastError())
    QVector<TypeItem> parseFile(const QString &path);

    // Saves the items to the file
    // Returns true on success
    bool saveFile(const QString &path, const QVector<TypeItem> &items);

    // Last error message
    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;

    TypeItem readTypeElement(class QXmlStreamReader &xml);
    void     writeTypeElement(class QXmlStreamWriter &xml, const TypeItem &item);
};
