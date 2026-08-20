#pragma once
#include "Renderer/Vulkan.hpp"

	struct ImgnPrimitive;
namespace Imgn
{
	class ImgnRenderer;

	class IMGN_API RenderContext
	{
		ImgnRenderer* _renderer = nullptr;
		vk::raii::CommandBuffer* _commandBuffer = nullptr;

		vk::PipelineLayout GetPipelineLayout() const;
	public:
		RenderContext(ImgnRenderer& pRenderer, vk::raii::CommandBuffer& pCommandBuffer) : _renderer(&pRenderer), _commandBuffer(&pCommandBuffer)/*Constructor*/
		{
		}

		~RenderContext() /*Destructor*/
		{
		}

		/*Copy Constructor*/
		RenderContext(const RenderContext& pOther) = default;

		/*Copy Assignment Operator*/
		RenderContext& operator=(const RenderContext& pOther) = default;

		/*Move Constructor*/
		RenderContext(RenderContext&& pOther) noexcept = default;

		/*Move Assignment Operator*/
		RenderContext& operator=(RenderContext&& pOther) noexcept = default;

		/*Class Functions*/
		vk::RenderingAttachmentInfo CreateRenderingAttachmentInfo(std::string_view pName);

		void BeginRendering(uint32_t pWidth, uint32_t pHeight, std::span<const vk::RenderingAttachmentInfo> pColors, const vk::RenderingAttachmentInfo* pDepth);
		void EndRendering();

		void BindPipeline(vk::PipelineBindPoint pBindPoint, vk::Pipeline pPipeline);
		void BindDescriptorSet(vk::PipelineBindPoint pBindPoint, vk::PipelineLayout pPipelineLayout, int pFirstSet, vk::DescriptorSet pDescriptorSet);
		void SetViewport(uint32_t pWidth, uint32_t pHeight);
		void SetScissor(uint32_t pWidth, uint32_t pHeight);

		vk::DescriptorBufferInfo CreateDescriptorBufferInfo(uint32_t pHandle, uint64_t pSize);
		vk::DescriptorImageInfo CreateDescriptorImageInfo(std::string_view pName, vk::ImageLayout pLayout = vk::ImageLayout::eShaderReadOnlyOptimal, vk::Sampler pSampler = nullptr);

		vk::WriteDescriptorSet CreateWriteDescriptorSet(uint32_t pBinding, vk::DescriptorType pDescriptorType, vk::DescriptorBufferInfo& pBufferInfo);
		vk::WriteDescriptorSet CreateWriteDescriptorSet(uint32_t pBinding, vk::DescriptorType pDescriptorType, vk::DescriptorImageInfo& pImageInfo);
		vk::WriteDescriptorSet CreateWriteDescriptorSet(uint32_t pBinding, vk::DescriptorType pDescriptorType, std::span<vk::DescriptorImageInfo> pImageInfo);

		void PushDescriptorSet(vk::PipelineBindPoint pBindPoint, vk::PipelineLayout pPipelineLayout, std::span<vk::WriteDescriptorSet> pWrites);
		
		void Dispatch(uint32_t pX, uint32_t pY, uint32_t pZ);

		template<typename T>
		void PushConstants(vk::ShaderStageFlags pStages, const T& pData)
		{
			_commandBuffer->pushConstants<T>(GetPipelineLayout(), pStages, 0, pData);
		}

		void BindMesh(uint32_t pMeshHandle);
		void DrawPrimitive(const ImgnPrimitive& pPrimitive);
	};
}