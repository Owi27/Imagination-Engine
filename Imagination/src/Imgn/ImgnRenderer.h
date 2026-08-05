#pragma once
//#include <ImgnVulkan.hpp>
#include "ImgnRenderResources.h"
#include "Renderer/Vulkan.hpp"
#include "ImgnRenderGraph.h"

namespace Imgn
{
	constexpr const wchar_t* VertexTarget = L"vs_6_6";
	constexpr const wchar_t* FragmentTarget = L"ps_6_6";
	constexpr const wchar_t* ComputeTarget = L"cs_6_6";

	//note to self, renderer only stores external images.
	class IMGN_API ImgnRenderer
	{
		//ImgnRenderGraph _renderGraph;
		unique<Vulkan> _vkCtx;
		unique<ImgnRenderGraph> _graph;
		uint32_t _w, _h;
		void SetupDeferedRenderer();

		RendererCreateInfo _info;

		//std::vector<ImgnImage> _images;
		//std::vector<ImgnBuffer> _buffers;

		std::vector<Image> _images;
		std::vector<Buffer> _buffers;

		std::vector<ImgnMesh> _meshes;
		std::vector<Material> _materials;

		std::map<std::string, uint32_t> _renderPassMap; //use pass to get idx

		vk::Format ToVkFormat(ImgnFormat pFormat);

	public:
		/* Class Defaults */
		ImgnRenderer()
		{
			
		}

		~ImgnRenderer()
		{

		}

		/* Class Functions */
		void Init(RendererCreateInfo pCreateInfo);
		//void Shudown();
		//void WaitIdle();
		//void Resize();

		void DrawFrame();
		void CompileGraph();
		void ExecuteGraph();

		void UploadMesh(const Vertex* pVertices, uint64_t pVertexCount);
		void UploadIndices(const uint32_t* pIndices, uint64_t pIndexCount);

		uint32_t AddMesh(const ImgnMesh& pMesh);
		uint32_t CreateBuffer(const ImgnBufferDesc& pDesc);
		uint32_t CreateImage(ImgnImageDesc pDesc);
		uint32_t CreateMaterial(const ImgnMaterialDesc& pDesc);
		uint32_t AddMaterial(const Material& pMaterial);
		uint32_t CreateImage(uint32_t pWidth, uint32_t pHeight, const uint8_t* pImageData);

		uint32_t CreateVertexBuffer(std::vector<Vertex>& pVertices);
		uint32_t CreateIndexBuffer(std::vector<uint32_t>& pIndices);
		uint32_t CreateUniformBuffer(void* pData, uint64_t pSize);
		uint32_t CreateStorageBuffer(void* pData, uint64_t pSize);
		uint32_t CreateMaterialBuffer(std::span<const uint32_t> pMaterialHandles);

		Image& GetImage(uint32_t pHandle) { return _images[pHandle]; }
		Buffer& GetBuffer(uint32_t pHandle) { return _buffers[pHandle]; }
		ImgnMesh GetMesh(uint32_t pHandle) { return _meshes[pHandle]; }
		const Material& GetMaterial(uint32_t pHandle) const { return _materials[pHandle]; }

		std::string MakeBufferKey(std::string_view pName, uint64_t pSize) { return _graph->MakeBufferKey(pName, pSize); }
		std::string MakeImageKey(std::string_view pName, uint32_t pWidth, uint32_t pHeight) { return _graph->MakeImageKey(pName, pWidth, pHeight); }

		RGImage& GetRenderGraphImage(std::string_view pName) { return _graph->GetImage(pName); }
		RGBuffer& GetRenderGraphBuffer(std::string_view pName) { return _graph->GetBuffer(pName); }

		vk::raii::DescriptorSet& GetTextureDescriptorSet() { return _vkCtx->GetTextureDescriptorSet(); }
		vk::raii::Pipeline& GetGBufferPipeline() { return *_vkCtx->_pipelines.gBufferPipeline; }
		vk::raii::Pipeline& GetLightingPipeline() { return *_vkCtx->_pipelines.lightingPipeline; }
		vk::raii::PipelineLayout& GetPipelineLayout() { return *_vkCtx->_pipelines.pipelineLayout; }

		void MapBufferData(std::string_view pKey, void* pData, uint64_t pSize) { _vkCtx->MapBufferData(pData, pSize, &_graph->GetBuffer(pKey).buffer); }

		void AddPass(RenderPass& pRenderPass) { _graph->AddPass(pRenderPass); }

		bool StartFrame();
		void EndFrame();
		void EndFrame(std::string_view pName);

		void BeginScene();
		void EndScene();
	};
}