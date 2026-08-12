#include "XmlParser.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// ────────────────────────────────────────────────────────────
// parseFile — reads types.xml and returns the items
// ────────────────────────────────────────────────────────────
QVector<TypeItem> XmlParser::parseFile(const QString &path)
{
    m_lastError.clear();
    QVector<TypeItem> items;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(file.errorString());
        return items;
    }

    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == QLatin1String("type")) {
                TypeItem item = readTypeElement(xml);
                items.append(item);
            }
        }
    }

    if (xml.hasError()) {
        m_lastError = QString("XML parse error at line %1: %2")
                          .arg(xml.lineNumber())
                          .arg(xml.errorString());
        items.clear();
    }

    return items;
}

// ────────────────────────────────────────────────────────────
// readTypeElement — reads a <type> element
// ────────────────────────────────────────────────────────────
TypeItem XmlParser::readTypeElement(QXmlStreamReader &xml)
{
    TypeItem item;

    // Read the name attribute from <type name="...">
    item.name = xml.attributes().value("name").toString();

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == QLatin1String("type"))
            break;

        if (!xml.isStartElement())
            continue;

        const QString tag = xml.name().toString();

        if (tag == "nominal") {
            item.nominal = xml.readElementText().toInt();
        } else if (tag == "lifetime") {
            item.lifetime = xml.readElementText().toInt();
        } else if (tag == "restock") {
            item.restock = xml.readElementText().toInt();
        } else if (tag == "min") {
            item.min = xml.readElementText().toInt();
        } else if (tag == "quantmin") {
            item.quantmin = xml.readElementText().toInt();
        } else if (tag == "quantmax") {
            item.quantmax = xml.readElementText().toInt();
        } else if (tag == "cost") {
            item.cost = xml.readElementText().toInt();
        } else if (tag == "flags") {
            const auto attrs = xml.attributes();
            item.count_in_cargo   = attrs.value("count_in_cargo").toInt();
            item.count_in_hoarder = attrs.value("count_in_hoarder").toInt();
            item.count_in_map     = attrs.value("count_in_map").toInt();
            item.count_in_player  = attrs.value("count_in_player").toInt();
            item.crafted          = attrs.value("crafted").toInt();
            item.deloot           = attrs.value("deloot").toInt();
            // self-closing element — consume it
            xml.readElementText();
        } else if (tag == "category") {
            item.category = xml.attributes().value("name").toString();
            xml.readElementText();
        } else if (tag == "usage") {
            item.usages.append(xml.attributes().value("name").toString());
            xml.readElementText();
        } else if (tag == "value") {
            item.values.append(xml.attributes().value("name").toString());
            xml.readElementText();
        } else {
            // Unknown tag — just consume it
            xml.readElementText();
        }
    }

    return item;
}

// ────────────────────────────────────────────────────────────
// saveFile — writes the items to the file
// ────────────────────────────────────────────────────────────
bool XmlParser::saveFile(const QString &path, const QVector<TypeItem> &items)
{
    m_lastError.clear();

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot write file: %1").arg(file.errorString());
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(4);

    xml.writeStartDocument();
    xml.writeStartElement("types");

    for (const TypeItem &item : items) {
        writeTypeElement(xml, item);
    }

    xml.writeEndElement(); // </types>
    xml.writeEndDocument();

    return true;
}

// ────────────────────────────────────────────────────────────
// writeTypeElement — writes a <type> element
// ────────────────────────────────────────────────────────────
void XmlParser::writeTypeElement(QXmlStreamWriter &xml, const TypeItem &item)
{
    xml.writeStartElement("type");
    xml.writeAttribute("name", item.name);

    xml.writeTextElement("nominal",  QString::number(item.nominal));
    xml.writeTextElement("lifetime", QString::number(item.lifetime));
    xml.writeTextElement("restock",  QString::number(item.restock));
    xml.writeTextElement("min",      QString::number(item.min));
    xml.writeTextElement("quantmin", QString::number(item.quantmin));
    xml.writeTextElement("quantmax", QString::number(item.quantmax));
    xml.writeTextElement("cost",     QString::number(item.cost));

    // <flags .../> self-closing
    xml.writeStartElement("flags");
    xml.writeAttribute("count_in_cargo",   QString::number(item.count_in_cargo));
    xml.writeAttribute("count_in_hoarder", QString::number(item.count_in_hoarder));
    xml.writeAttribute("count_in_map",     QString::number(item.count_in_map));
    xml.writeAttribute("count_in_player",  QString::number(item.count_in_player));
    xml.writeAttribute("crafted",          QString::number(item.crafted));
    xml.writeAttribute("deloot",           QString::number(item.deloot));
    xml.writeEndElement();

    if (!item.category.isEmpty()) {
        xml.writeStartElement("category");
        xml.writeAttribute("name", item.category);
        xml.writeEndElement();
    }

    for (const QString &u : item.usages) {
        xml.writeStartElement("usage");
        xml.writeAttribute("name", u);
        xml.writeEndElement();
    }

    for (const QString &v : item.values) {
        xml.writeStartElement("value");
        xml.writeAttribute("name", v);
        xml.writeEndElement();
    }

    xml.writeEndElement(); // </type>
}
