#include "pch.h"
#include "RenderPass.h"

Texture& RenderPass::AddTextureInput(std::string& name)
{
	//Texture& gottenTex;
	//graph.get texture
	//add to graphics queue through texture
	//read in passes from renderresource (this pass name)
	//gottenTex.ReadInPass(_name); 
}

void RenderPass::Setup()
{
}

void RenderPass::Execute()
{
	VkCommandBuffer commandBuffer;

	VkRenderingAttachmentInfoKHR renderingAttachmentInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR
	};

	VkRenderingInfo renderingInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea
		{
			.offset = {0, 0},
			.extent = {_colorInputs[0].get()->GetExtent().width, _colorInputs[0].get()->GetExtent().height}
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &renderingAttachmentInfo
	};

	vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);

}
