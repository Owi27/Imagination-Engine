#include "ImaginaryMath.hpp"
#include "RenderContext.h"
#include <memory>
#include <IWindow.h>
#include <VulkanBackend.h>
using namespace std;

int main()
{
	ImgnWindow& win = ImgnWindow::GetInstance();
	win.Init(800, 600, "Demo");

	while (win.ProcessEvents())
	{

	}


	return 0;
}