#include "pch.h"
#include "GuiContext.h"

void GuiContext::ReadInputs(GInput& input)
{
	ImGuiIO& io = ImGui::GetIO();

	float leftMouseButton = 0.f, rightMouseButton = 0.f, middleMouseButton = 0.f, mouseWheelUp = 0.f, mouseWheelDown = 0.f, mousePosX = 0.f, mousePosY = 0.f;
	
	input.GetState(G_BUTTON_LEFT, leftMouseButton);
	input.GetState(G_BUTTON_RIGHT, rightMouseButton);
	input.GetState(G_BUTTON_MIDDLE, middleMouseButton);
	input.GetState(G_MOUSE_SCROLL_UP, mouseWheelUp);
	input.GetState(G_MOUSE_SCROLL_DOWN, mouseWheelDown);
	input.GetMousePosition(mousePosX, mousePosY);

	if (leftMouseButton)
	{
		std::cout << "YEA\n";
	}

	if (leftMouseButton) io.MouseDown[0] = true;
	else io.MouseDown[0] = false;
	if (rightMouseButton) io.MouseDown[1] = true;
	else io.MouseDown[1] = false;
	if (middleMouseButton) io.MouseDown[2] = true;
	else io.MouseDown[2] = false;

	if (mouseWheelUp) io.MouseWheel += 1.f;
	if (mouseWheelDown) io.MouseWheel -= 1.f;

	io.MousePos = ImVec2(mousePosX, mousePosY);

	std::cout << "X: " << mousePosX << "Y " << mousePosY << '\n';
}

void GuiContext::BuildCommandBuffer(VkCommandBuffer& commandBuffer)
{
	//GvkHelper::signal_command_start(_vk.GetDevice(), _vk.GetCommandPool(), &_commandBuffer);

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::SetWindowPos(ImVec2(20, 360), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);

	ImGui::Begin("Render Options");

	ImGui::ShowDemoWindow();

	ImGui::End();

	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	//GvkHelper::signal_command_end(_vk.GetDevice(), _vk.GetGraphicsQueue(), _vk.GetCommandPool(), &_commandBuffer);
	//ImDrawData* drawData = ImGui::GetDrawData();

	//VkDeviceSize vertexBufferSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
	//VkDeviceSize indexBufferSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

	//if ((vertexBufferSize == 0) || (indexBufferSize == 0)) return;

	////vertex buffer
	//if (!_buffers[0] || vertexCount != drawData->TotalVtxCount)
	//{
	//	_buffers[0] = std::make_unique<Buffer>(vertexBufferSize, nullptr, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true);
	//	vertexCount = drawData->TotalVtxCount;
	//}

	//if (!_buffers[1] || vertexCount != drawData->TotalIdxCount)
	//{
	//	_buffers[1] = std::make_unique<Buffer>(indexBufferSize, nullptr, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true);
	//	indexCount = drawData->TotalIdxCount;
	//}

	//ImDrawVert* vertexDestination = (ImDrawVert*)_buffers[0]->GetData();
	//ImDrawIdx* indexDestination = (ImDrawIdx*)_buffers[1]->GetData();

	//for (size_t i = 0; i < drawData->CmdListsCount; i++)
	//{
	//	const ImDrawList* cmd_list = drawData->CmdLists[i];
	//	_buffers[0]->WriteToBuffer(cmd_list->VtxBuffer.Data);
	//	_buffers[1]->WriteToBuffer(cmd_list->IdxBuffer.Data);
	//	vertexDestination += cmd_list->VtxBuffer.Size;
	//	indexDestination += cmd_list->IdxBuffer.Size;
	//}

	//_buffers[0]->Flush();
	//_buffers[1]->Flush();

	//ImGuiIO& io = ImGui::GetIO();

	//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet, 0, nullptr);
	//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

	//VkViewport viewport =
	//{
	//	.width = ImGui::GetIO().DisplaySize.x,
	//	.height = ImGui::GetIO().DisplaySize.y,
	//	.minDepth = 0.f,
	//	.maxDepth = 1.f,
	//};

	//vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	//_imGuiPushConst.scale = { 2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y };
	//_imGuiPushConst.translate = { -1.f, -1.f };
	//vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ImGuiPushConst), &_imGuiPushConst);

	//// Render commands
	////ImDrawData* drawData = ImGui::GetDrawData();
	//int32_t vertexOffset = 0;
	//int32_t indexOffset = 0;

	//if (drawData->CmdListsCount > 0)
	//{
	//	VkDeviceSize offsets[1] = { 0 };
	//	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_buffers[0]->GetBuffer(), offsets);
	//	vkCmdBindIndexBuffer(commandBuffer, _buffers[1]->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);

	//	for (int32_t i = 0; i < drawData->CmdListsCount; i++)
	//	{
	//		const ImDrawList* cmd_list = drawData->CmdLists[i];

	//		for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
	//		{
	//			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
	//			VkRect2D scissorRect;
	//			scissorRect.offset.x = std::max((int)(pcmd->ClipRect.x), 0);
	//			scissorRect.offset.y = std::max((int)(pcmd->ClipRect.y), 0);
	//			scissorRect.extent.width = (unsigned)(pcmd->ClipRect.z - pcmd->ClipRect.x);
	//			scissorRect.extent.height = (unsigned)(pcmd->ClipRect.w - pcmd->ClipRect.y);
	//			vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
	//			vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
	//			indexOffset += pcmd->ElemCount;
	//		}

	//		vertexOffset += cmd_list->VtxBuffer.Size;
	//	}
	//}
}

//void GuiContext::DrawFrame()
//{
//	ImGuiIO& io = ImGui::GetIO();
//
//	vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet, 0, nullptr);
//	vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
//
//	VkViewport viewport =
//	{
//		.width = ImGui::GetIO().DisplaySize.x,
//		.height = ImGui::GetIO().DisplaySize.y,
//		.minDepth = 0.f,
//		.maxDepth = 1.f,
//	};
//
//	vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);
//
//	_imGuiPushConst.scale = { 2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y };
//	_imGuiPushConst.translate = { -1.f, -1.f };
//	vkCmdPushConstants(_commandBuffer, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ImGuiPushConst), &_imGuiPushConst);
//
//	// Render commands
//	ImDrawData* drawData = ImGui::GetDrawData();
//	int32_t vertexOffset = 0;
//	int32_t indexOffset = 0;
//
//	if (drawData->CmdListsCount > 0) 
//	{
//		VkDeviceSize offsets[1] = { 0 };
//		vkCmdBindVertexBuffers(_commandBuffer, 0, 1, &_buffers[0]->GetBuffer(), offsets);
//		vkCmdBindIndexBuffer(_commandBuffer, _buffers[1]->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
//
//		for (int32_t i = 0; i < drawData->CmdListsCount; i++)
//		{
//			const ImDrawList* cmd_list = drawData->CmdLists[i];
//
//			for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
//			{
//				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
//				VkRect2D scissorRect;
//				scissorRect.offset.x = std::max((int)(pcmd->ClipRect.x), 0);
//				scissorRect.offset.y = std::max((int)(pcmd->ClipRect.y), 0);
//				scissorRect.extent.width = (unsigned)(pcmd->ClipRect.z - pcmd->ClipRect.x);
//				scissorRect.extent.height = (unsigned)(pcmd->ClipRect.w - pcmd->ClipRect.y);
//				vkCmdSetScissor(_commandBuffer, 0, 1, &scissorRect);
//				vkCmdDrawIndexed(_commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
//				indexOffset += pcmd->ElemCount;
//			}
//
//			vertexOffset += cmd_list->VtxBuffer.Size;
//		}
//	}
//}

void GuiContext::Render()
{
	//// Update imGui
	//ImGuiIO& io = ImGui::GetIO();

	//io.DisplaySize = ImVec2((float)width, (float)height);
	//io.DeltaTime = frameTimer;

	//io.MousePos = ImVec2(mousePos.x, mousePos.y);
	//io.MouseDown[0] = mouseButtons.left && UIOverlay.visible;
	//io.MouseDown[1] = mouseButtons.right && UIOverlay.visible;
	//io.MouseDown[2] = mouseButtons.middle && UIOverlay.visible;
}
