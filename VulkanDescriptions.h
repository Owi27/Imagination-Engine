#pragma once
#include "pch.h"
#include "Shader.h"

enum VertexInput
{
	POSITION = (1 << 0),
	NORMAL = (1 << 1),
	TEXCOORD = (1 << 2),
	TANGENT = (1 << 3)
};

enum Topology : unsigned char
{
	POINT_TOPOLOGY,
	LINE_TOPOLOGY,
	LINE_STRIP_TOPOLOGY,
	TRIANGLE_TOPOLOGY,
	TRIANGLE_STRIP_TOPOLOGY
};

enum PolygonMode : unsigned char
{
	FILL,
	LINE,
};

enum CullMode : unsigned char
{
	FRONT,
	BACK,
	NONE
};

enum FrontFace : unsigned char
{
	CLOCKWISE,
	COUNTER_CLOCKWISE
};

struct PipelineDescription
{
	Topology topology = TRIANGLE_TOPOLOGY;
	unsigned vertexInput;

	std::shared_ptr<Shader> vertexShader;
	std::shared_ptr<Shader> fragmentShader;
	//... rest of shader types

	std::vector<VkFormat> colorAttachmentFormats;
	VkFormat depthFormat;

	PolygonMode polygonMode = FILL;
	CullMode cullMode = NONE;
	FrontFace frontFace = COUNTER_CLOCKWISE;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
};

