#include "pch.h"
#include "Renderer.h"

int main()
{
	GWindow win;
	Renderer* renderer;
	entt::registry registry;
	bool useVulkan = true;

	if (+win.Create(0, 0, 800, 600, GWindowStyle::WINDOWEDBORDERLESS))
	{
		renderer = useVulkan ? static_cast<Renderer*>(new VulkanRenderer(win)) : static_cast<Renderer*>(new DX12Renderer(win));


	}

	delete renderer;
}