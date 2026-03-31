#include "D:/GitHub/Imagination-Engine/Refactor/Imagination - Current/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "RenderPass.h"

void RenderPassManager::SortPasses()
{
	// Topologically sort render passes based on dependencies
	_sortedPasses.clear();

	// Create a copy of render passes for sorting
	std::unordered_map<std::string, RenderPass*> passMap;
	for (const auto& [name, pass] : _renderPasses)
	{
		passMap[name] = pass.get();
	}

	// Perform topological sort
	std::unordered_set<std::string> visited;
	std::unordered_set<std::string> visiting;

	for (const auto& [name, pass] : passMap)
	{
		if (visited.find(name) == visited.end())
		{
			TopologicalSort(name, passMap, visited, visiting);
		}
	}
}

void RenderPassManager::TopologicalSort(const std::string& pName, const std::unordered_map<std::string, RenderPass*>& pPassMap, std::unordered_set<std::string>& pVisited, std::unordered_set<std::string>& pVisiting)
{
	pVisiting.insert(pName);

	auto pass = pPassMap.at(pName);
	for (const auto& dep : pass->GetDependencies())
	{
		if (pVisited.find(dep) == pVisited.end())
		{
			if (pVisiting.find(dep) != pVisiting.end())
			{
				// Circular dependency detected
				throw std::runtime_error("Circular dependency detected in render passes");
			}
			TopologicalSort(dep, pPassMap, pVisited, pVisiting);
		}
	}

	pVisiting.erase(pName);
	pVisited.insert(pName);
	_sortedPasses.push_back(pass);
}

RenderPass* RenderPassManager::GetRenderPass(const std::string& pName)
{
	auto it = _renderPasses.find(pName);
	if (it != _renderPasses.end())
	{
		return it->second.get();
	}

	return nullptr;
}

void RenderPassManager::RemoveRenderPass(const std::string& pName)
{
	auto it = _renderPasses.find(pName);
	if (it != _renderPasses.end())
	{
		_renderPasses.erase(it);
		_isDirty = true;
	}
}

void RenderPassManager::Execute(vk::raii::CommandBuffer& pCommandBuffer)
{
	if (_isDirty)
	{
		SortPasses();
		_isDirty = false;
	}

	for (auto pass : _sortedPasses)
	{
		pass->Execute(pCommandBuffer);
	}
}

void RenderPass::Execute(vk::raii::CommandBuffer& pCommandBuffer)
{
	if (!_isEnabled) return;

	BeginPass(pCommandBuffer);
	Render(pCommandBuffer);
	EndPass(pCommandBuffer);
}

void RenderTarget::CreateColorResources()
{
}

void RenderTarget::CreateDepthResources()
{
}
//
//void GeometryPass::BeginPass(vk::raii::CommandBuffer& pCommandBuffer)
//{
//
//	vk::RenderingAttachmentInfoKHR colorAttachment
//	{
//		.imageView = gBuffer->GetColorImageView(),
//		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
//		.loadOp = vk::AttachmentLoadOp::eClear,
//		.storeOp = vk::AttachmentStoreOp::eStore,
//		.clearValue = vk::ClearColorValue(std::array<float, 4>{0.f, 0.f, 0.f, 1.f})
//	};
//
//	vk::RenderingAttachmentInfoKHR depthAttachment
//	{
//		.imageView = gBuffer->GetDepthImageView(),
//		.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
//		.loadOp = vk::AttachmentLoadOp::eClear,
//		.storeOp = vk::AttachmentStoreOp::eStore,
//		.clearValue = vk::ClearDepthStencilValue(1.f, 0)
//	};
//
//	vk::RenderingInfoKHR renderingInfo
//	{
//		.renderArea = vk::Rect2D({0, 0}, {gBuffer->GetWidth(), gBuffer->GetHeight()}),
//		.layerCount = 1,
//		.colorAttachmentCount = 1,
//		.pColorAttachments = &colorAttachment,
//		.pDepthAttachment = &depthAttachment
//	};
//
//	pCommandBuffer.beginRendering(renderingInfo);
//}
//
//void GeometryPass::Render(vk::raii::CommandBuffer& pCommandBuffer)
//{
//}
//
//void GeometryPass::EndPass(vk::raii::CommandBuffer& pCommandBuffer)
//{
//	pCommandBuffer.endRendering();
//}
