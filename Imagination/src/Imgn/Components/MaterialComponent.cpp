#include "pch.hpp"
#include "MaterialComponent.h"
#include "Imgn/ImgnRenderer.h"

namespace Imgn
{
	MaterialComponent::MaterialComponent() : Component("Materials")
	{

	}

	MaterialComponent::~MaterialComponent() = default;

	void MaterialComponent::Serialize(std::fstream& pStream)
	{
		pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));

		uint32_t materialCount = static_cast<uint32_t>(_materials.size());
		pStream.write(reinterpret_cast<const char*>(&materialCount), sizeof(uint32_t));

		for (const shared<Material>& material : _materials)
		{
			bool hasMaterial = material != nullptr;
			pStream.write(reinterpret_cast<const char*>(&hasMaterial), sizeof(bool));

			if (!hasMaterial) continue;

			pStream.write(reinterpret_cast<const char*>(material->baseColorFactor.data()), material->baseColorFactor.size() * sizeof(float));
			pStream.write(reinterpret_cast<const char*>(material->emissiveFactor.data()), material->emissiveFactor.size() * sizeof(float));

			pStream.write(reinterpret_cast<const char*>(material->textureIndices0.data()), material->textureIndices0.size() * sizeof(int32_t));
			pStream.write(reinterpret_cast<const char*>(material->textureIndices1.data()), material->textureIndices1.size() * sizeof(int32_t));

			pStream.write(reinterpret_cast<const char*>(material->materialFactors.data()), material->materialFactors.size() * sizeof(float));
			pStream.write(reinterpret_cast<const char*>(material->extraFactors.data()), material->extraFactors.size() * sizeof(float));
		}
	}

	void MaterialComponent::Deserialize(std::fstream& pStream)
	{
		uint32_t materialCount = 0;
		pStream.read(reinterpret_cast<char*>(&materialCount), sizeof(uint32_t));

		_materials.clear();
		_materials.reserve(materialCount);

		for (uint32_t i = 0; i < materialCount; i++)
		{
			bool hasMaterial = false;
			pStream.read(reinterpret_cast<char*>(&hasMaterial), sizeof(bool));

			if (!hasMaterial)
			{
				_materials.push_back(nullptr);
				continue;
			}

			shared<Material> material = Shared<Material>();

			pStream.read(reinterpret_cast<char*>(material->baseColorFactor.data()), material->baseColorFactor.size() * sizeof(float));
			pStream.read(reinterpret_cast<char*>(material->emissiveFactor.data()), material->emissiveFactor.size() * sizeof(float));

			pStream.read(reinterpret_cast<char*>(material->textureIndices0.data()), material->textureIndices0.size() * sizeof(int32_t));
			pStream.read(reinterpret_cast<char*>(material->textureIndices1.data()), material->textureIndices1.size() * sizeof(int32_t));

			pStream.read(reinterpret_cast<char*>(material->materialFactors.data()), material->materialFactors.size() * sizeof(float));
			pStream.read(reinterpret_cast<char*>(material->extraFactors.data()), material->extraFactors.size() * sizeof(float));

			_materials.push_back(std::move(material));
		}

		_materialBuffer.reset();
		_materialBufferSize = 0;
		_materialBufferDirty = true;
	}

	bool MaterialComponent::SetMaterial(uint32_t pSlot, shared<Material> pMaterial)
	{
		if (pSlot >= _materials.size() || !pMaterial) return false;

		_materials[pSlot] = std::move(pMaterial);
		_materialBufferDirty = true;

		return true;
	}

	bool MaterialComponent::SwapMaterials(uint32_t pFirstSlot, uint32_t pSecondSlot)
	{
		if (pFirstSlot >= _materials.size() || pSecondSlot >= _materials.size()) return false;

		std::swap(_materials[pFirstSlot], _materials[pSecondSlot]);
		_materialBufferDirty = true;

		return true;
	}

	void MaterialComponent::SyncMaterialBuffer(ImgnRenderer& pRenderer)
	{
		if (!_materialBufferDirty) return;

		std::vector<Material> materials;
		materials.reserve(_materials.size());

		for (const shared<Material>& material : _materials)
		{
			materials.push_back(material ? *material : Material{});
		}

		_materialBuffer = pRenderer.CreateMaterialBuffer(materials);
		_materialBufferSize = static_cast<uint64_t>(materials.size()) * sizeof(Material);
		_materialBufferDirty = false;
	}
}