#pragma once

#include <QString>
#include <QStringList>

// Represents a type entry from the DayZ types.xml
struct TypeItem
{
    // Basic info
    QString name;

    // Spawn/loot numeric fields
    int nominal  = 0;
    int lifetime = 3600;
    int restock  = 1800;
    int min      = 0;
    int quantmin = -1;
    int quantmax = -1;
    int cost     = 100;

    // Flags (stored as int for easy serialization, values 0/1)
    int count_in_cargo   = 0;
    int count_in_hoarder = 0;
    int count_in_map     = 1;
    int count_in_player  = 0;
    int crafted          = 0;
    int deloot           = 0;

    // Category
    QString category;

    // Multiple usage / value tags
    QStringList usages;
    QStringList values;

    // Change tracking
    bool modified = false;

    // Comparison for undo/redo
    bool operator==(const TypeItem &o) const
    {
        return name == o.name &&
               nominal == o.nominal &&
               lifetime == o.lifetime &&
               restock == o.restock &&
               min == o.min &&
               quantmin == o.quantmin &&
               quantmax == o.quantmax &&
               cost == o.cost &&
               count_in_cargo == o.count_in_cargo &&
               count_in_hoarder == o.count_in_hoarder &&
               count_in_map == o.count_in_map &&
               count_in_player == o.count_in_player &&
               crafted == o.crafted &&
               deloot == o.deloot &&
               category == o.category &&
               usages == o.usages &&
               values == o.values;
    }

    bool operator!=(const TypeItem &o) const { return !(*this == o); }

    // Returns a summary of the flags as a string
    QString flagsSummary() const
    {
        QStringList parts;
        if (count_in_map)     parts << "map";
        if (count_in_cargo)   parts << "cargo";
        if (count_in_hoarder) parts << "hoarder";
        if (count_in_player)  parts << "player";
        if (crafted)          parts << "crafted";
        if (deloot)           parts << "deloot";
        return parts.isEmpty() ? "-" : parts.join(", ");
    }
};
