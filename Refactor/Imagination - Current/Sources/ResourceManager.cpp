#include "D:/GitHub/Imagination-Engine/Refactor/Imagination - Current/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
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
