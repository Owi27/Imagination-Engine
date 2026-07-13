#pragma once

constexpr uint32_t InvalidHandle = 0xFFFFFFFF;

enum class MemoryUsage
{
	GpuOnly,
	CpuOnly,
	CpuToGpu,
	GpuToCpu
};

enum class ImgnBufferUsage : uint32_t
{
	None = 0,
	Vertex = 1 << 0,
	Index = 1 << 1,
	Uniform = 1 << 2,
	Storage = 1 << 3,
	Indirect = 1 << 4,
	TransferSrc = 1 << 5,
	TransferDst = 1 << 6,
	Readback = 1 << 7,
	AccelerationAS = 1 << 8,
	ShaderBindingTable = 1 << 9
};

inline ImgnBufferUsage operator|(ImgnBufferUsage a, ImgnBufferUsage b)
{
	return static_cast<ImgnBufferUsage>(
		static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
		);
}

inline bool HasFlag(ImgnBufferUsage value, ImgnBufferUsage flag)
{
	return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

struct ImgnBufferDesc
{
    std::string name;

	uint64_t size = 0, stride = 0;
	const void* data = nullptr;

    ImgnBufferUsage usage = ImgnBufferUsage::None;
	MemoryUsage memoryUsage = MemoryUsage::GpuOnly;

	bool mappedAtCreation = false;

};

enum class TextureType
{
	Texture1D,
	Texture2D,
	Texture3D,
	TextureCube,
	Texture2DArray,
	TextureCubeArray
};

enum class ImgnFormat
{
    Unknown,

    R8_UNorm,
    R8_SNorm,
    R8_UInt,
    R8_SInt,

    RG8_UNorm,
    RG8_SNorm,
    RG8_UInt,
    RG8_SInt,

    RGBA8_UNorm,
    RGBA8_SRGB,
    BGRA8_UNorm,
    BGRA8_SRGB,

    RGBA16_Float,
    RGBA16_UNorm,
    RGBA16_UInt,
    RGBA16_SInt,

    R16_Float,
    RG16_Float,

    R32_Float,
    RG32_Float,
    RGB32_Float,
    RGBA32_Float,

    R32_UInt,
    RG32_UInt,
    RGBA32_UInt,

    D16_UNorm,
    D24_UNorm_S8_UInt,
    D32_Float,
    D32_Float_S8_UInt,

    BC1_RGBA_UNorm,
    BC1_RGBA_SRGB,
    BC3_RGBA_UNorm,
    BC3_RGBA_SRGB,
    BC5_RG_UNorm,
    BC7_RGBA_UNorm,
    BC7_RGBA_SRGB
};

enum class ImgnImageUsage : uint32_t
{
    None = 0,
    ColorAttachment = 1 << 0,
    Storage = 1 << 1,
    RenderTarget = 1 << 2,
    DepthStencil = 1 << 3,
    TransferSrc = 1 << 4,
    TransferDst = 1 << 5,
    Present = 1 << 6
};

inline ImgnImageUsage operator|(ImgnImageUsage a, ImgnImageUsage b)
{
    return static_cast<ImgnImageUsage>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
        );
}

inline bool HasFlag(ImgnImageUsage value, ImgnImageUsage flag)
{
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

struct ImgnExtent2D
{
    uint32_t width = 0, height = 0;

    vk::Extent2D VkExtent()
    {
        return vk::Extent2D
        {
            .width = width,
            .height = height
        };
    }
};

struct ImgnExtent3D
{
    uint32_t width = 1, height = 1, depth = 1;

    vk::Extent2D VkExtent2D()
    {
        return vk::Extent2D
        {
            .width = width,
            .height = height
        };
    }

    vk::Extent3D VkExtent()
    {
        return vk::Extent3D
        {
            .width = width,
            .height = height,
            .depth = depth
        };
    }
};

struct ImgnOffset2D
{
    int x = 0, y = 0;
};

struct ImgnRect2D
{
    ImgnOffset2D offset{};
    ImgnExtent2D extent{};
};

struct ImgnImageDesc
{
    std::string name;

    TextureType type = TextureType::Texture2D;
    ImgnFormat format = ImgnFormat::RGBA8_UNorm;
    ImgnImageUsage usage = ImgnImageUsage::ColorAttachment;

    ImgnExtent3D extent;

    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    uint32_t samples = 1;

    bool generateMips = false;
    bool isCubeMap = false;

    const void* initialData = nullptr;
    uint64_t initialDataSize = 0;
};

struct ImgnMeshDesc
{
    std::string name;

    const void* vertexData = nullptr;
    uint64_t vertexDataSize = 0;
    uint32_t vertexCount = 0, vertexStride = 0;

    const void* indexData = nullptr;
    uint64_t indexDataSize = 0;
    uint32_t indexCount = 0;
};

struct ImgnMaterialDesc
{
    std::string name;

    vec4 baseColor = { 1.f, 1.f, 1.f, 1.f };
    vec3 emissive = { 1.f, 1.f, 1.f };

    uint32_t baseColorTexture = InvalidHandle, normalTexture = InvalidHandle, metallicRoughnessTexture = InvalidHandle, occlusionTexture = InvalidHandle, emissiveTexture = InvalidHandle;
    //uint32_t sampler;

    float metallic = 1.f, roughness = 1.f;
};

struct ImgnPrimitive
{
    std::string name;

    int vertexOffset = 0;
    uint32_t firstIndex = 0, indexCount = 0, firstVertex = 0, vertexCount = 0, material = InvalidHandle;
};

struct ImgnMesh
{
    std::string name;

    uint32_t vertexBuffer = InvalidHandle, indexBuffer = InvalidHandle;

    std::vector<ImgnPrimitive> primitives;
};

enum class ImgnAlphaMode
{
    Opaque,
    Mask,
    Blend
};

struct ImgnMaterial
{
    vec4 baseColor = { 1.f, 1.f, 1.f, 1.f };
    vec3 emissive = { 0.f, 0.f, 0.f };

    uint32_t baseColorTexture = InvalidHandle, normalTexture = InvalidHandle, metallicRoughnessTexture = InvalidHandle, occlusionTexture = InvalidHandle, emissiveTexture = InvalidHandle;
    float metallic = 1.f, roughness = 1.f, normalScale = 1.f, occlusion = 1.f, alphaCutoff = .5f;

    ImgnAlphaMode alphaMode = ImgnAlphaMode::Opaque;
    bool doubleSided = false;



};


struct ImgnViewport
{
    float x = 0.0f, y = 0.0f;
    float width = 0.0f, height = 0.0f;
    float minDepth = 0.0f, maxDepth = 1.0f;
};

struct ImgnCommandBuffer
{
    vk::raii::CommandBuffer vkCommandBuffer;
};

enum class ImgnImageLayout
{
    Undefined_ImageLayout,
    ShaderReadOnly_ImageLayout,
};

enum ImgnAspect
{
    Color_Aspect,
    Depth_Aspect
};