#include "../pch.hpp"
#include "ImgnWindow.h"

void ImgnWindow::Dream()
{
	_window.ProcessWindowEvents();
	_ctx->DrawFrame();

	std::vector vertices =
	{
		Vertex
		{
			.pos = { 0.f, .5f, 0.f}
		},
		Vertex
		{
			.pos = { .5f, -.5f, 0.f}
		},
		Vertex
		{
			.pos = { -.5f, -.5f, 0.f}
		}
	};

	uint32_t vertexBuffer = _ctx->CreateVertexBuffer(vertices);
}

uint32_t ImgnWindow::GetWidth()
{
	_window.GetClientWidth(_width);
	return _width;
}

uint32_t ImgnWindow::GetHeight()
{
	_window.GetClientWidth(_height);
	return _height;
}