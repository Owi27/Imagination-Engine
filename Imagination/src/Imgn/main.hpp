#pragma once

#ifdef IMGN_PLATFORM_WINDOWS

extern Imgn::ImgnApp* Imgn::CreateApplication();

int main(int argc, char** argv)
{
	Imgn::ImgnApp* app = Imgn::CreateApplication();
	app->Run();

	delete app;
}
#endif