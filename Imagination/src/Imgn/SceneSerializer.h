#pragma once
#include "ImgnScene.h"

namespace Imgn
{
    class IMGN_API SceneSerializer
    {
        shared<Scene> _scene;
        std::fstream _stream;

    public:
        SceneSerializer(const shared<Scene>& pScene) /*Constructor*/
        {
            _scene = pScene;
        }

        ~SceneSerializer() /*Destructor*/
        {
        }

        /*Copy Constructor*/
        SceneSerializer(const SceneSerializer& pOther) = default;

        /*Copy Assignment Operator*/
        SceneSerializer& operator=(const SceneSerializer& pOther) = default;

        /*Move Constructor*/
        SceneSerializer(SceneSerializer&& pOther) noexcept = default;

        /*Move Assignment Operator*/
        SceneSerializer& operator=(SceneSerializer&& pOther) noexcept = default;

        /*Class Functions*/
        void Serialize(const std::filesystem::path& pFilepath);
        void SerializeRuntime(const std::string& pFilepath);
        bool Deserialize(const std::string& pFilepath);
        bool DeserializeRuntime(const std::string& pFilepath);

        void Read(void* pData, uint64_t pSize) { _stream.read(reinterpret_cast<char*>(pData), pSize); }
        void Write(const void* pData, uint64_t pSize) { _stream.write(reinterpret_cast<const char*>(pData), pSize); }
    };
}