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
	}

	void MaterialComponent::Deserialize(std::fstream& pStream)
	{
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