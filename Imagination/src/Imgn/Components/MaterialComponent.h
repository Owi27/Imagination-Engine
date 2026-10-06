#pragma once
#include "Imgn/ImgnComponent.h"

struct Buffer;

namespace Imgn
{
	class ImgnRenderer;

	struct Material
	{
		std::array<float, 4> baseColorFactor = { 1.f, 1.f, 1.f, 1.f };
		std::array<float, 4> emissiveFactor = { 0.f, 0.f, 0.f, 0.f };

		std::array<int32_t, 4> textureIndices0 = { -1, -1, -1, -1 };
		std::array<int32_t, 4> textureIndices1 = { -1, 0, 0, 0 };

		std::array<float, 4> materialFactors = { 1.f, 1.f, .5f, 1.f };
		std::array<float, 4> extraFactors = { 1.f, 0.f, 0.f, 0.f };
	};

	class IMGN_API MaterialComponent : public Component
	{
		std::vector<shared<Material>> _materials;

		unique<Buffer> _materialBuffer;

		uint64_t _materialBufferSize = 0;
		bool _materialBufferDirty = true;

	public:
		IMGN_COMPONENT_ID("Imgn.MaterialComponent");

		MaterialComponent();
		~MaterialComponent();

		const std::vector<shared<Material>>& GetMaterials() const { return _materials; }

		Buffer* GetMaterialBuffer() { return _materialBuffer.get(); }
		const Buffer* GetMaterialBuffer() const { return _materialBuffer.get(); }

		shared<Material> GetMaterial(uint32_t pIndex) const { return pIndex < _materials.size() ? _materials[pIndex] : nullptr; }
		void SetMaterials(const std::vector<shared<Material>> pMaterials) { _materials = std::move(pMaterials); }

		uint32_t GetMaterialCount() const { return static_cast<uint32_t>(_materials.size()); }
		uint64_t GetMaterialBufferSize() const { return _materialBufferSize; }


		bool SetMaterial(uint32_t pSlot, shared<Material> pMaterial);
		bool SwapMaterials(uint32_t pFirstSlot, uint32_t pSecondSlot);
		void SyncMaterialBuffer(ImgnRenderer& pRenderer);

		void Serialize(std::fstream& pStream) override;
		void Deserialize(std::fstream& pStream) override;
	};
}