#include "StatTable.h"

#include <QDataStream>


Resources::StatTable Resources::StatTable::deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

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
    stream.skipRawData(4); // Padding

    StatTable table;

    for (int i = 0; i < count; ++i)
    {
        StatTableEntry entry;
        stream.readRawData((char*)&entry, sizeof(StatTableEntry));
        table.entries.emplace_back(entry);
    }

    return table;
}
