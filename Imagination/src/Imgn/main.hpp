#pragma once

#ifdef IMGN_PLATFORM_WINDOWS

extern ImgnApp* IMGN::CreateApplication();

int main(int argc, char** argv)
{
	ImgnApp* app = IMGN::CreateApplication();
	app->Run();

	delete app;
}
#endif