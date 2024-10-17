#include "QuestData.h"

#include <QBuffer>
#include <QDataStream>

Resources::QuestData Resources::QuestData::deserialize(const QByteArray& data)
{
    QDataStream stream(data);

    u32 magic;
    s32 count;
    stream >> magic;
    stream >> count;

    if (magic != Magic)
    {
        qCritical("Invalid magic %08X", magic);
        return {};
    }

    if (count > 1) 
    {
        qCritical("Multiple Quest Data per Quest Resource is not supported");
        return {};
    }

    QuestData quest;
    stream.readRawData((char*)&quest, sizeof(QuestData));

    return quest;
}

QByteArray Resources::QuestData::serialize(const QuestData& quest)
{
    QByteArray data;
    QDataStream stream(&data, QIODeviceBase::WriteOnly);

    stream << Magic;
    stream << 1;
    stream.writeRawData((const char*)&quest, sizeof(QuestData));

    return data;
}
