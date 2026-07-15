#pragma once
#include <ImgnVulkan.hpp>
#include "ImgnRenderResources.h"
#include "ImgnRenderGraph.h"

constexpr uint32_t NumDescriptorsStreaming = 2048;
constexpr const wchar_t* VertexTarget = L"vs_6_6";
constexpr const wchar_t* FragmentTarget = L"ps_6_6";
constexpr const wchar_t* ComputeTarget = L"cs_6_6";

//enum class RendererBackend
//{
//	Vulkan, D3D12, Metal
//};
//
//struct RendererCreateInfo
//{
//	RendererBackend backend = RendererBackend::Vulkan;
//
//	void* windowHandle = nullptr, *displayHandle = nullptr;
//
//	uint32_t width = 1280, height = 720;
//
//	bool enableValidation = true;
//};

//struct Vertex
//{
//	/*
//	* std::array<float, 3> position{};
//    std::array<float, 3> normal{};
//    std::array<float, 2> uv0{};
//    std::array<float, 4> tangent{};
//    std::array<float, 3> color{ 1.0f, 1.0f, 1.0f };
//	*/
//	vec3 pos;
//	vec3 nrm;
//	vec2 uv0;
//	vec4 tan;
//	vec3 col;
//};

//note to self, renderer only stores external images.
class ImgnRenderer
{
	ImgnRenderGraph _renderGraph;
	ImgnVulkan::Ctx _vkCtx;
	uint32_t _w, _h;
	void SetupDeferedRenderer();

	RendererCreateInfo _info;

	std::vector<ImgnImage> _images;
	std::vector<ImgnBuffer> _buffers;
	std::vector<ImgnMesh> _meshes;
	std::vector<ImgnMaterial> _materials;

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

	void UploadMesh(const Vertex* pVertices, uint64_t pVertexCount);
	void UploadIndices(const uint32_t* pIndices, uint64_t pIndexCount);

	uint32_t AddMesh(const ImgnMesh& pMesh);
	uint32_t CreateBuffer(const ImgnBufferDesc& pDesc);
	uint32_t CreateImage(ImgnImageDesc pDesc);
	uint32_t CreateMaterial(const ImgnMaterialDesc& pDesc);
	uint32_t AddMaterial(const ImgnMaterial& pMaterial);
	uint32_t CreateImage(const uint8_t* pImageData, uint32_t pWidth, uint32_t pHeight);


};