
#include "ImaginaryMath.hpp"
#include "RenderContext.h"
#include <memory>
#include <VulkanBackend.h>
using namespace std;

int main()
{
	unique_ptr<RenderContext> renderContext = make_unique<RenderContext>();
	renderContext->Init();

	VulkanBackend* vk = new VulkanBackend();
	vk->Init();

	return 0;
}