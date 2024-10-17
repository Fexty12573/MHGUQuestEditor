#pragma once

#include <Common.h>

#include <QString>


namespace Resources
{

class ExtensionResolver
{
public:
    static QString resolve(u32 hash);

private:
    static const std::unordered_map<u32, QString> extensions;
};

}
