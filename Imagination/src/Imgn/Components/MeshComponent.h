#pragma once
#include "Imgn/ImgnComponent.h"

struct Buffer;

namespace Imgn
{
	struct Primitive
	{
		uint32_t firstIndex = 0, indexCount = 0, firstVertex = 0, vertexCount = 0, vertexOffset = 0, materialSlot = InvalidHandle;
	};

	class IMGN_API MeshComponent : public Component
	{
		std::string _name;
		unique<Buffer> _vertex, _index;
		std::filesystem::path _assetPath;
		std::vector<Primitive> _primitives;
		//std::vector<shared<Material>> _materials;

		static bool ReadString(std::fstream& pStream, std::string& pValue);
		static void WriteString(std::fstream& pStream, const std::string& pValue);

	public:
		IMGN_COMPONENT_ID("Imgn.MeshComponent");
		
		MeshComponent();
		~MeshComponent();

		Buffer* GetIndexBuffer() { return _index.get(); }
		const Buffer* GetIndexBuffer() const { return _index.get(); }
		const std::string& GetMeshName() { return _name; }
		Buffer* GetVertexBuffer() { return _vertex.get(); }
		const Buffer* GetVertexBuffer() const { return _vertex.get(); }
		const std::filesystem::path& GetMeshAssetPath() { return _assetPath; }
		const std::vector<Primitive>& GetPrimitives() const { return _primitives; }

		void SetMesh(std::string pName, std::filesystem::path pAssetPath, unique<Buffer> pVertexBuffer, unique<Buffer> pIndexBuffer, std::vector<Primitive> pPrimitives);

		bool visible = true;


		void Serialize(std::fstream& pStream) override;
		void Deserialize(std::fstream& pStream) override;
	};
}