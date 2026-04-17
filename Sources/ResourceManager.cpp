#include "pch.cpp"
#include "ResourceManager.h"

bool Resource::Load()
{
    _loaded = DoLoad();
    return _loaded;
}

void Resource::Unload()
{
    DoUnload();
    _loaded = false;
}
