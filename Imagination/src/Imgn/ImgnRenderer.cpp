#include "pch.hpp"
#include "ImgnRenderer.h"

void ImgnRenderer::SetupDeferedRenderer()
{
	//CreateImage("GBuffer-Position", vk::Format::eR16G16B16A16Sfloat, { _w, _h }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);
	//CreateImage("GBuffer-Normal", vk::Format::eR16G16B16A16Sfloat, { _w, _h }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);
	//CreateImage("GBuffer-Albedo", vk::Format::eR8G8B8A8Unorm, { _w, _h }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);
	////_graph.AddResource("Depth", vk::Format::eD32Sfloat, { width, height }, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageAspectFlagBits::eDepth);
	////_graph.AddResource("FinalColor", vk::Format::eR8G8B8A8Unorm, { width, height }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, vk::ImageAspectFlagBits::eColor);

	CreateImage(ImgnImageDesc
		{
			.name = "GBuffer-Albedo",
			.format = ImgnFormat::RGBA8_UNorm,
			.usage = ImgnImageUsage::ColorAttachment,
			.extent = {_info.width, _info.height, 1},
		});
	CreateImage(ImgnImageDesc
		{
			.name = "GBuffer-Normal",
			.format = ImgnFormat::RGBA8_UNorm,
			.usage = ImgnImageUsage::ColorAttachment,
			.extent = {_info.width, _info.height, 1},
		});
	CreateImage(ImgnImageDesc
		{
			.name = "GBuffer-Material",
			.format = ImgnFormat::RGBA8_UNorm,
			.usage = ImgnImageUsage::ColorAttachment,
			.extent = {_info.width, _info.height, 1},
		});

	//_renderGraph.AddResource();
	// 
	//CreateImage()
	//GBufferUBO gBufferUBO
	//{
	//	.world = GW::MATH::GIdentityMatrixF,
	//	.view = GW::MATH::GIdentityMatrixF,
	//	.proj = GW::MATH::GIdentityMatrixF,
	//};

	//GMatrix::LookAtLHF({ 0.f, 0.25f, 0 }, { 1.f, 0.f, 1.f }, { 0.f, 1.f, 0.f }, gBufferUBO.view);
	//GMatrix::ProjectionVulkanLHF(45.f, 16 / 9, 0.01, 100.f, gBufferUBO.proj);

	////_graph.AddResource("GBuffer-UBO", sizeof(GBufferUBO), vk::BufferUsageFlagBits::eUniformBuffer, &gBufferUBO);

	//ImgnVulkan::RenderPass gBufferPass
	//{
	//	.name = "GeometryPass",
	//	.inputs = {},
	//	.outputs = { "GBuffer-Position", "GBuffer-Normal", "GBuffer-Albedo", "Depth" },
	//	.bufferInputs = {"GBuffer-UBO"},
	//	.Execute = [&](vk::raii::CommandBuffer& commandBuffer)
	//	{
	//		auto ubo = _graph.GetBufferResource("GBuffer-UBO");
	//		UpdateDescriptorSet(ImgnVulkan::frameIdx, RenderPassIdx::GBuffer, ubo->buffer, ubo->size, nullptr, 0, {}, *_textureSampler);

	//		std::array<vk::RenderingAttachmentInfo, 3> colorAttachments;
	//		vk::RenderingAttachmentInfoKHR depthAttachment;
	//		vk::RenderingInfoKHR renderingInfo;

	//		colorAttachments[0].setImageView(_vkCtx.GetImageResource("GBuffer-Position")->view).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setClearValue(vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} });
	//		colorAttachments[1].setImageView(_vkCtx.GetImageResource("GBuffer-Normal")->view).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setClearValue(vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} });
	//		colorAttachments[2].setImageView(_vkCtx.GetImageResource("GBuffer-Albedo")->view).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setClearValue(vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} });

	//		depthAttachment.setImageView(_graph.GetImageResource("Depth")->view).setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setClearValue(vk::ClearDepthStencilValue{ 1.0f, 0 });

	//		renderingInfo.setRenderArea({ {0, 0}, {_w, _h} }).setLayerCount(1).setColorAttachmentCount(colorAttachments.size()).setPColorAttachments(colorAttachments.data()).setPDepthAttachment(&depthAttachment);

	//		commandBuffer.beginRendering(renderingInfo);

	//		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipelines.gBufferPipeline);
	//		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(_w), static_cast<float>(_h), 0.0f, 1.0f));
	//		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), ImgnVulkan::swapchainExtent));


	//		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelines.pipelineLayout, 0, *ImgnVulkan::descriptorSets[DescriptorSetIndex(ImgnVulkan::frameIdx, RenderPassIdx::GBuffer)], nullptr);
	//		commandBuffer.bindVertexBuffers(0, *_vkCtx.GetBufferResource("VertexBuffer")->buffer, {0});
	//		commandBuffer.bindIndexBuffer(_vkCtx.GetBufferResource("IndexBuffer")->buffer, 0, vk::IndexType::eUint32);

	//		commandBuffer.drawIndexed(_sponza->GetIndexCount(), 1, 0, 0, 0);

	//		commandBuffer.endRendering();
	//	}
	//};

	//RenderPass lightingPass
	//{
	//	.name = "LightingPass",
	//	.inputs = { "GBuffer-Position", "GBuffer-Normal", "GBuffer-Albedo", "Depth" },
	//	.outputs = {"FinalColor"},
	//	.descriptorSetLayout = ImgnVulkan::descriptorSetLayout,
	//	.Execute = [&](vk::raii::CommandBuffer& commandBuffer)
	//	{
	//		vk::RenderingAttachmentInfo colorAttachment
	//		{
	//			.imageView = _graph.GetImageResource("FinalColor")->view,
	//			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
	//			.loadOp = vk::AttachmentLoadOp::eClear,
	//			.storeOp = vk::AttachmentStoreOp::eStore,
	//			.clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)
	//		};

	//		vk::RenderingInfoKHR renderingInfo
	//		{
	//			.renderArea = { {0, 0}, {ImgnVulkan::swapchainExtent.width, ImgnVulkan::swapchainExtent.height} },
	//			.layerCount = 1,
	//			.colorAttachmentCount = 1,
	//			.pColorAttachments = &colorAttachment,
	//		};

	//		std::vector<vk::ImageView> imageViews =
	//		{
	//			_graph.GetImageResource("GBuffer-Position")->view,
	//			_graph.GetImageResource("GBuffer-Normal")->view,
	//			_graph.GetImageResource("GBuffer-Albedo")->view,
	//			_graph.GetImageResource("Depth")->view,
	//		};
	//		UpdateDescriptorSet(ImgnVulkan::frameIdx, RenderPassIdx::Lighting, nullptr, 0, nullptr, 0, imageViews, *_textureSampler);

	//		commandBuffer.beginRendering(renderingInfo);

	//		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipelines.lightingPipeline);
	//		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(ImgnVulkan::swapchainExtent.width), static_cast<float>(ImgnVulkan::swapchainExtent.height), 0.0f, 1.0f));
	//		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), ImgnVulkan::swapchainExtent));


	//		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelines.pipelineLayout, 0, *ImgnVulkan::descriptorSets[DescriptorSetIndex(ImgnVulkan::frameIdx, RenderPassIdx::Lighting)], nullptr);
	//		//commandBuffer.bindVertexBuffers(0, _sponza->GetVertexBuffer(), {0});
	//		//commandBuffer.bindIndexBuffer(_sponza->GetIndexBuffer(), 0, vk::IndexType::eUint32);

	//		commandBuffer.draw(3, 1, 0, 0);

	//		commandBuffer.endRendering();
	//	}
	//};


}

vk::Format ImgnRenderer::ToVkFormat(ImgnFormat pFormat)
{
	switch (pFormat)
	{
	//case ImgnFormat::Unknown:
	//	IMGN_CO
	//	return vk::Format::eUndefined;
	case ImgnFormat::R8_UNorm:
		return vk::Format::eR8Unorm;
	case ImgnFormat::R8_SNorm:
		return vk::Format::eR8Snorm;
	case ImgnFormat::R8_UInt:
		return vk::Format::eR8Uint;
	case ImgnFormat::R8_SInt:
		return vk::Format::eR8Sint;
	case ImgnFormat::RG8_UNorm:
		return vk::Format::eR8G8Unorm;
	case ImgnFormat::RG8_SNorm:
		return vk::Format::eR8G8Snorm;
	case ImgnFormat::RG8_UInt:
		return vk::Format::eR8G8Uint;
	case ImgnFormat::RG8_SInt:
		return vk::Format::eR8G8Sint;
	case ImgnFormat::RGBA8_UNorm:
		return vk::Format::eR8G8B8A8Unorm;
	case ImgnFormat::RGBA8_SRGB:
		return vk::Format::eR8G8B8A8Srgb;
	case ImgnFormat::BGRA8_UNorm:
		return vk::Format::eB8G8R8A8Unorm;
	case ImgnFormat::BGRA8_SRGB:
		return vk::Format::eB8G8R8A8Srgb;
	case ImgnFormat::RGBA16_Float:
		return vk::Format::eR16G16B16A16Sfloat;
	case ImgnFormat::RGBA16_UNorm:
		return vk::Format::eR16G16B16A16Unorm;
	case ImgnFormat::RGBA16_SInt:
		return vk::Format::eR16G16B16A16Sint;
	case ImgnFormat::R16_Float:
		return vk::Format::eR16Sfloat;
	case ImgnFormat::RG16_Float:
		return vk::Format::eR16G16Sfloat;
	case ImgnFormat::R32_Float:
		return vk::Format::eR32Sfloat;
	case ImgnFormat::RG32_Float:
		return vk::Format::eR32G32Sfloat;
	case ImgnFormat::RGB32_Float:
		return vk::Format::eR32G32B32Sfloat;
	case ImgnFormat::RGBA32_Float:
		return vk::Format::eR32G32B32A32Sfloat;
	case ImgnFormat::R32_UInt:
		return vk::Format::eR32Uint;
	case ImgnFormat::RG32_UInt:
		return vk::Format::eR32G32Uint;
	case ImgnFormat::RGBA32_UInt:
		return vk::Format::eR32G32B32A32Uint;
	case ImgnFormat::D16_UNorm:
		return vk::Format::eD16Unorm;
	case ImgnFormat::D24_UNorm_S8_UInt:
		return vk::Format::eD24UnormS8Uint;
	case ImgnFormat::D32_Float:
		return vk::Format::eD32Sfloat;
	case ImgnFormat::D32_Float_S8_UInt:
		return vk::Format::eD32SfloatS8Uint;
	case ImgnFormat::BC1_RGBA_UNorm:
		return vk::Format::eBc1RgbaUnormBlock;
	case ImgnFormat::BC1_RGBA_SRGB:
		return vk::Format::eBc1RgbaSrgbBlock;
	case ImgnFormat::BC3_RGBA_UNorm:
		return vk::Format::eBc3UnormBlock;
	case ImgnFormat::BC3_RGBA_SRGB:
		return vk::Format::eBc3SrgbBlock;
	case ImgnFormat::BC5_RG_UNorm:
		return vk::Format::eBc5UnormBlock;
	case ImgnFormat::BC7_RGBA_UNorm:
		return vk::Format::eBc7UnormBlock;
	case ImgnFormat::BC7_RGBA_SRGB:
		return vk::Format::eBc7SrgbBlock;
	}

	return vk::Format::eUndefined;
}

void ImgnRenderer::Init(RendererCreateInfo pCreateInfo)
{
	_info = pCreateInfo;
	switch (_info.backend)
	{
	case RendererBackend::Vulkan:
		{
			_vkCtx = Unique<Vulkan>();
			_vkCtx->Init(_info);
		}
		break;
	case RendererBackend::D3D12:
		break;
	case RendererBackend::Metal:
		break;
	default:
		//todo write failed to init graphics device
		break;
	}
}

void ImgnRenderer::DrawFrame()
{
	switch (_info.backend)
	{
	case RendererBackend::Vulkan:
		//_vkCtx.DrawFrame();
		break;
	case RendererBackend::D3D12:
		break;
	case RendererBackend::Metal:
		break;
	default:
		//todo write failed to init graphics device
		break;
	}

}

void ImgnRenderer::CompileGraph()
{
	//_vkCtx.CompileGraph();
}

void ImgnRenderer::UploadMesh(const Vertex* pVertices, uint64_t pVertexCount)
{
	/*ImgnVulkan::Vertex* vertices = new ImgnVulkan::Vertex[pVertexCount];

	memcpy(vertices, pVertices, sizeof(Vertex) * pVertexCount);

	_vkCtx.UploadMesh(vertices, pVertexCount);

	delete[] vertices;*/
}

void ImgnRenderer::UploadIndices(const uint32_t* pIndices, uint64_t pIndexCount)
{
	//_vkCtx.UploadIndices(pIndices, pIndexCount);
}

uint32_t ImgnRenderer::AddMesh(const ImgnMesh& pMesh)
{
	_meshes.push_back(pMesh);

	return static_cast<uint32_t>(_meshes.size() - 1);
}

uint32_t ImgnRenderer::CreateBuffer(const ImgnBufferDesc& pDesc)
{
	switch (_info.backend)
	{
	case RendererBackend::Vulkan:
		{
			/*vk::BufferUsageFlags usage;

			if (HasFlag(pDesc.usage, ImgnBufferUsage::Vertex)) usage |= vk::BufferUsageFlagBits::eVertexBuffer;
			if (HasFlag(pDesc.usage, ImgnBufferUsage::Index)) usage |= vk::BufferUsageFlagBits::eIndexBuffer;
			if (HasFlag(pDesc.usage, ImgnBufferUsage::Uniform)) usage |= vk::BufferUsageFlagBits::eUniformBuffer;
			if (HasFlag(pDesc.usage, ImgnBufferUsage::Storage)) usage |= vk::BufferUsageFlagBits::eStorageBuffer;
			if (HasFlag(pDesc.usage, ImgnBufferUsage::TransferSrc)) usage |= vk::BufferUsageFlagBits::eTransferSrc;
			if (HasFlag(pDesc.usage, ImgnBufferUsage::TransferDst)) usage |= vk::BufferUsageFlagBits::eTransferDst;

			_vkCtx.CreateBuffer(pDesc.name, pDesc.size, usage, pDesc.data);*/
		}
		break;
	case RendererBackend::D3D12:
		break;
	case RendererBackend::Metal:
		break;
	default:
		break;
	}
	return 0;
}

uint32_t ImgnRenderer::CreateImage(ImgnImageDesc pDesc)
{
	switch (_info.backend)
	{
	case RendererBackend::Vulkan:
		{
			/*vk::ImageUsageFlags usage;

			if (HasFlag(pDesc.usage, ImgnImageUsage::ColorAttachment)) usage |= vk::ImageUsageFlagBits::eColorAttachment;
			if (HasFlag(pDesc.usage, ImgnImageUsage::Storage)) usage |= vk::ImageUsageFlagBits::eStorage;
			if (HasFlag(pDesc.usage, ImgnImageUsage::TransferSrc)) usage |= vk::ImageUsageFlagBits::eTransferSrc;
			if (HasFlag(pDesc.usage, ImgnImageUsage::TransferDst)) usage |= vk::ImageUsageFlagBits::eTransferDst;

			ImgnImage image
			{
				.name = pDesc.name,
				.handle = static_cast<uint32_t>(_images.size()),
				.vkImage = _vkCtx.CreateImage(pDesc.name, ToVkFormat(pDesc.format), pDesc.extent.VkExtent2D(), usage, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor)
			};

			_images.push_back(std::move(image));

			_renderGraph.AddResource(pDesc.name, pDesc.format, pDesc.extent, pDesc.usage, ImgnImageLayout::Undefined_ImageLayout, ImgnImageLayout::ShaderReadOnly_ImageLayout, ImgnAspect::Color_Aspect);
			return static_cast<uint32_t>(_images.size() - 1);*/
		}
		break;
	case RendererBackend::D3D12:
		break;
	case RendererBackend::Metal:
		break;
	default:
		break;
	}

	return 0;
}

//uint32_t ImgnRenderer::CreateTexture(const ImgnTextureDesc& pDesc)
//{
//	return 0;
//}

uint32_t ImgnRenderer::CreateMaterial(const ImgnMaterialDesc& pDesc)
{
	return 0;
}

uint32_t ImgnRenderer::AddMaterial(const ImgnMaterial& pMaterial)
{
	_materials.push_back(pMaterial);

	return static_cast<uint32_t>(_materials.size() - 1);
}

uint32_t ImgnRenderer::CreateImage(uint32_t pWidth, uint32_t pHeight, const uint8_t* pImageData)
{
	///*ImgnImage image
	//{
	//	.handle = static_cast<uint32_t>(_images.size())
	//};*/

	//switch (_info.backend)
	//{
	//case RendererBackend::Vulkan:
	//	image .vkImage = _vkCtx->CreateTextureImage(pWidth, pHeight, pImageData);
	//	break;
	//case RendererBackend::D3D12:
	//	break;
	//case RendererBackend::Metal:
	//	break;
	//default:
	//	break;
	//}

	//_images.push_back(std::move(image));
	//return static_cast<uint32_t>(_images.size() - 1);
	return 0;
}

uint32_t ImgnRenderer::CreateVertexBuffer(std::vector<Vertex>& pVertices)
{
	_buffers.push_back(_vkCtx->CreateVertexBuffer(pVertices.data(), pVertices.size() * sizeof(Vertex)));

	return static_cast<uint32_t>(_buffers.size() - 1);
}

uint32_t ImgnRenderer::CreateIndexBuffer(std::vector<uint32_t>& pIndices)
{
	_buffers.push_back(_vkCtx->CreateIndexBuffer(pIndices.data(), pIndices.size() * sizeof(uint32_t)));

	return static_cast<uint32_t>(_buffers.size() - 1);
}

void ImgnRenderer::BeginScene()
{
}

void ImgnRenderer::EndScene()
{
}

//void ImgnRenderer::CreateBuffer(const std::string& pName, vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, const void* pData)
//{
//	_vkCtx.CreateBuffer(pName, pSize, pUsage, pData);
//}
//
//void ImgnRenderer::CreateImage(const std::string& pName, vk::Format pFormat, vk::Extent2D pExtent, vk::ImageUsageFlags pUsage, vk::ImageLayout pInitialLayout, vk::ImageLayout pFinalLayout, vk::ImageAspectFlags pAspect)
//{
//	_vkCtx.CreateImage(pName, pFormat, pExtent, pUsage, pInitialLayout, pFinalLayout, pAspect);
//}
