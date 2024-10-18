#include "StatTable.h"

#include <QDataStream>


Resources::StatTable Resources::StatTable::deserialize(const QByteArray& data)
{
    QDataStream stream(data);

    u32 magic, version;
    stream >> magic;
    stream >> version;

    if (magic != Magic)
    {
        qCritical("Invalid magic %08X", magic);
        return {};
    }

    if (version != Version)
    {
        qCritical("Invalid version %08X", version);
        return {};
    }

    int count;
    stream >> count;

    StatTable table;

    for (int i = 0; i < count; ++i)
    {
        StatTableEntry entry;
        stream >> entry.Health;
        stream >> entry.Attack;
        stream >> entry.Defense;
        stream >> entry.Stagger;
        stream >> entry.Exhaust;
        stream >> entry.KO;
        stream >> entry.Mount;
        stream >> entry.Unknown;

        table.entries.emplace_back(entry);
    }

    return table;
}
