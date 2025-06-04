#include "pch.h"
#include "Renderer.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1); // set block of memory to find memory leak
	_CrtDumpMemoryLeaks();

	GWindow win;
	Renderer* renderer;
	entt::registry registry;
	bool useVulkan = true;

	if (+win.Create(0, 0, 1280, 720, GWindowStyle::WINDOWEDBORDERED))
	{
		renderer = useVulkan ? static_cast<Renderer*>(new VulkanRenderer(win)) : static_cast<Renderer*>(new DX12Renderer(win));

		while (+win.ProcessWindowEvents())
		{
			renderer->Render();
			renderer->UpdateCamera();
		}
	}
	
	delete renderer;
}