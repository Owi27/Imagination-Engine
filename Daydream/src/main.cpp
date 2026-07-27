#include <Imgn.hpp>

struct TestComp : public ImgnComponent
{
	TestComp() : ImgnComponent("Tester")
	{


	}

	void Dream(float pDeltaTime) override
	{
		//IMGN_INFO("{} Test Comp Update", GetName());
	}

	/* Class Functions */
};
class Daydream : public Imgn::ImgnApp
{
public:
	Daydream()
	{
		AddComponent<TestComp>();

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

		uint32_t vertexBuffer = Renderer().CreateVertexBuffer(vertices);

		Imgn::RenderPass gBuffer
		{
			.name = "G-BufferPass",
			.imageOUT =
			{
				Renderer().MakeImageKey("G-BufferAlbedo", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("G-BufferNormal", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("G-BufferMaterial", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("G-BufferEmissive", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("Depth", GetWindow().GetWidth(), GetWindow().GetHeight())
			},
			.bufferIN =
			{
				Renderer().MakeBufferKey("G-BufferUBO", 0), //edit size
				Renderer().MakeBufferKey("G-BufferSBO", 0)
			},
			.bufferOUT =
			{
				Renderer().MakeBufferKey("G-BufferUBO", 0), //edit size
				Renderer().MakeBufferKey("G-BufferSBO", 0)
			},
			.Execute = [&](vk::raii::CommandBuffer& commandBuffer)
			{
				commandBuffer.beginRendering();

				commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, Renderer().GetGBufferPipeline());

				//uniform buffer
				vk::DescriptorBufferInfo uboInfo
				{
					.buffer = *Renderer().GetRenderGraphBuffer("G-BufferUBO").buffer,
					.offset = 0,
					.range = 192
				};

				const std::array writes
				{
					vk::WriteDescriptorSet
					{
						.dstSet = nullptr,
						.dstBinding = 0,
						.dstArrayElement = 0,
						.descriptorCount = 1,
						.descriptorType = vk::DescriptorType::eUniformBuffer,
						.pBufferInfo = &uboInfo
					},
				};

				commandBuffer.pushDescriptorSet(vk::PipelineBindPoint::eGraphics, Renderer().GetPipelineLayout(), 0, writes);

				commandBuffer.endRendering();
			}
		};

	}

	~Daydream()
	{

	}
};

Imgn::ImgnApp* Imgn::CreateApplication()
{
	return new Daydream();
}