#include "pch.hpp"
#include "MeshComponent.h"

#include "Imgn/ImgnRenderer.h"

namespace Imgn
{
	MeshComponent::MeshComponent() : Component("Mesh")
	{
	}

	MeshComponent::~MeshComponent() = default;

	void MeshComponent::SetMesh(std::string pName, std::filesystem::path pAssetPath, unique<Buffer> pVertexBuffer, unique<Buffer> pIndexBuffer, std::vector<Primitive> pPrimitives)
	{
		_name = std::move(pName);
		_assetPath = std::move(pAssetPath);
		_vertex = std::move(pVertexBuffer);
		_index = std::move(pIndexBuffer);
		_primitives = std::move(pPrimitives);
	}

	void MeshComponent::WriteString(std::fstream& pStream, const std::string& pValue)
	{
		uint32_t size = static_cast<uint32_t>(pValue.size());

		pStream.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

		if (size > 0) pStream.write(pValue.data(), size);
	}

	bool MeshComponent::ReadString(std::fstream& pStream, std::string& pValue)
	{
		uint32_t size = 0;
		pStream.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));

		if (!pStream.good()) return false;

		pValue.resize(size);

		if (size > 0) pStream.read(pValue.data(), size);

		return pStream.good();
	}

	void MeshComponent::Serialize(std::fstream& pStream)
	{
		pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));

		WriteString(pStream, _name);
		WriteString(pStream, _assetPath.string());

		pStream.write(reinterpret_cast<const char*>(&visible), sizeof(bool));
	}

	void MeshComponent::Deserialize(std::fstream& pStream)
	{
		std::string path;

		if (!ReadString(pStream, _name)) return;
		if (!ReadString(pStream, path)) return;

		_assetPath = path;

		pStream.read(reinterpret_cast<char*>(&visible), sizeof(bool));
	}
}