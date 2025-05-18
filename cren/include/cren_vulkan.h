#ifndef CREN_VULKAN_INCLUDED
#define CREN_VULKAN_INCLUDED

#include "cren_context.h"
#include "cren_defines.h"
#include "cren_math.h"
#include "cren_platform.h"
#include "cren_utils.h"

#include <vulkan/vulkan.h>

#ifdef __cplusplus 
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instance-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief cren vulkan instance
typedef struct {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugger;
} vkInstance;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Device-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief cren vulkan queues info
typedef struct {
    int graphicFamily;
    int presentFamily;
    int computeFamily;
    int graphicFound;
    int presentFound;
    int computeFound;
} vkQueueFamilyIndices;

/// @brief cren vulkan device
typedef struct {
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;

    unsigned int imageIndex;
    unsigned int currentFrame;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* finishedRenderingSemaphores;
    VkFence* framesInFlightFences;
} vkDevice;

/// @brief creates a gpu buffer
/// @param device vulkan device
/// @param physicalDevice vulkan physical device
/// @param usage the intent usage mode for the buffer
/// @param properties the memory properties for the buffer
/// @param size buffer's size in bytes
/// @param buffer the output buffer
/// @param memory the output buffer memory
/// @param data data to map the buffer to, or NULL if no data is to be sent
/// @return 1 on success, 0 on failure
CREN_API int crenvk_device_create_buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Swapchain-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief cren vulkan swapchain details
typedef struct {
    VkExtent2D extent;
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* pSurfaceFormats;
    VkPresentModeKHR* pPresentModes;
    unsigned int surfaceFormatCount;
    unsigned int presentModeCount;
} vkSwapchainDetails;

/// @brief cren vulkan swapchain
typedef struct {
    VkSurfaceFormatKHR swapchainFormat;
    VkPresentModeKHR swapchainPresentMode;
    VkExtent2D swapchainExtent;
    unsigned int swapchainImageCount;
    VkSwapchainKHR swapchain;
    VkImage* swapchainImages;
    VkImageView* swapchainImageViews;
} vkSwapchain;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pipeline-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief cren vulkan renderpass
typedef struct {
    const char* name;
    VkSampleCountFlagBits msaa;
    VkFormat surfaceFormat;
    VkRenderPass renderPass;
    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;
    VkCommandBuffer* commandBuffers;
    VkFramebuffer* framebuffers;
    unsigned int commandBufferCount;
    unsigned int framebufferCount;
} vkRenderpass;

/// @brief all kinds of shaders
typedef enum {
    SHADER_TYPE_VERTEX = 0,
    SHADER_TYPE_FRAGMENT,
    SHADER_TYPE_COMPUTE,
    SHADER_TYPE_GEOMETRY,
    SHADER_TYPE_TESS_CTRL,
    SHADER_TYPE_TESS_EVAL
} vkShaderType;

/// @brief cren vulkan shader usefull data
typedef struct {
    const char* name;
    const char* path;
    vkShaderType type;
    VkShaderModule shaderModule;
    VkPipelineShaderStageCreateInfo shaderStageCI;
} vkShader;

/// @brief cren vertex components, custom attributes not supported, also only covering 1 of each type at the momment
typedef enum 
{
    VK_VERTEX_COMPONENT_POSITION = 0,
    VK_VERTEX_COMPONENT_NORMAL,
    VK_VERTEX_COMPONENT_UV_0,          // UV_1, UV_2 ...
    VK_VERTEX_COMPONENT_COLOR_0,       // COLOR_1, COLOR_2 ...
    VK_VERTEX_COMPONENT_JOINTS_0,      // JOINTS_1, JOINTS_2 ...
    VK_VERTEX_COMPONENT_WEIGHTS_0,     // WEIGHTS_1, WEIGHTS_2, ...

    VK_VERTEX_COMPONENTS_MAX
} vkVertexComponent;

/// @brief cren vertex structure
typedef struct {
    float3 position;
    float3 normal;
    float2 uv_0;
    float4 color_0;
    float4 joints_0;
    float4 weights_0;
} vkVertex;

/// @brief cren pipeline create info, needed data to create a pipeline
typedef struct vkPipelineCreateInfo
{
	vkRenderpass* renderpass;
	VkPipelineCache pipelineCache;
	vkShader vertexShader;
	vkShader fragmentShader;
	unsigned int passingVertexData;
	unsigned int alphaBlending;
	VkDescriptorSetLayoutBinding bindings[CREN_PIPELINE_DESCRIPTOR_SET_LAYOUT_BINDING_MAX];
	unsigned int bindingsCount;
	VkPushConstantRange pushConstants[CREN_PIPELINE_PUSH_CONSTANTS_MAX];
	unsigned int pushConstantsCount;
	vkVertexComponent vertexComponents[VK_VERTEX_COMPONENTS_MAX];
	unsigned int vertexComponentsCount;
} vkPipelineCreateInfo;

/// @brief cren vulka npipeline
typedef struct {
    vkRenderpass* renderpass;
	unsigned int passingVertexData;
	unsigned int alphaBlending;
	VkPipelineCache cache;

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout layout;
	VkPipeline pipeline;

	VkVertexInputBindingDescription* pBindingsDescription;
	VkVertexInputAttributeDescription* pAttributesDescription;
	unsigned int bindingsDescriptionCount;
	unsigned int attributesDescriptionCount;

	// auto-generated, can be modified before building pipeline
	VkPipelineShaderStageCreateInfo shaderStages[CREN_PIPELINE_SHADER_STAGES_COUNT];
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	VkPipelineInputAssemblyStateCreateInfo inputVertexAssemblyState;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizationState;
	VkPipelineMultisampleStateCreateInfo multisampleState;
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
	VkPipelineColorBlendStateCreateInfo colorBlendState;
} vkPipeline;

/// @brief creates a cren vulkan pipeline
/// @param device cren vulkan pipeline
/// @param ci pipeline create info
/// @return the pipeline or NULL if an error has happend
CREN_API vkPipeline* crenvk_pipeline_create(VkDevice device, vkPipelineCreateInfo* ci);

/// @brief destroys a cren vulkan pipeline
/// @param device cren vulkan device
/// @param pipeline cren vulkan pipeline
CREN_API void crenvk_pipeline_destroy(VkDevice device, vkPipeline* pipeline);

/// @brief builds a pipeline with it's setup configuration
/// @param device vulkan device
/// @param pipeline cren vulkan pipeline
CREN_API void crenvk_pipeline_build(VkDevice device, vkPipeline* pipeline);

/// @brief destroy all resources related to the renderpass and itself
/// @param device vulkan device
/// @param renderpass cren vulkan renderpass
CREN_API void crenvk_renderpass_destroy(VkDevice device, vkRenderpass* renderpass);

/// @brief creates a cren vulkan shader
/// @param device vulkan device
/// @param name shader's name
/// @param path SPIRV shader's path
/// @param type the type of shader
/// @return the cren vulkan shader
CREN_API vkShader crenvk_shader_create(VkDevice device, const char* name, const char* path, vkShaderType type);

/// @brief destroys the shader's resources
/// @param device vulkan device
/// @param shader shader object
CREN_API void crenvk_shader_destroy(VkDevice device, vkShader shader);

/// @brief checks if two given vertices are equivalent/same
/// @param v0 vertex to compare to
/// @param v1 vertex to be compared
/// @return 1 on equals, 0 otherwise
CREN_API int crenvk_vertex_equals(vkVertex* v0, vkVertex* v1);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Renderphase-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief cren default renderphase
typedef struct {
    vkRenderpass* renderpass;
    vkPipeline* pipeline;

    VkDeviceSize defaultImageSize;
    VkImage colorImage;
    VkImage depthImage;
    VkDeviceMemory colorMemory;
    VkDeviceMemory depthMemory;
    VkImageView colorView;
    VkImageView depthView;
    VkFormat surfaceFormat;
    VkFormat depthFormat;

} vkDefaultRenderphase;

/// @brief cren picking renderphase
typedef struct {
    vkRenderpass* renderpass;
    vkPipeline* pipeline;

    VkDeviceSize imageSize;
    VkImage colorImage;
    VkImage depthImage;
    VkDeviceMemory colorMemory;
    VkDeviceMemory depthMemory;
    VkImageView colorView;
    VkImageView depthView;
    VkFormat surfaceFormat;
    VkFormat depthFormat;
} vkPickingRenderphase;

/// @brief cren ui renderphase
typedef struct {
    vkRenderpass* renderpass;

    // some things may be missing
    VkDescriptorPool descPool;
    VkDescriptorSetLayout descSetLayout;
} vkUIRenderphase;

/// @brief cren viewport render phase
typedef struct {
    vkRenderpass* renderpass;

    VkImage colorImage;
	VkDeviceMemory colorMemory;
	VkImageView colorView;

	VkImage depthImage;
	VkDeviceMemory depthMemory;
	VkImageView depthView;

	VkSampler sampler;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	// viewport boundaries
    float2 vpPosition;
	float2 vpSize;
	float2 vpMin;
	float2 vpMax;
} vkViewportRenderphase;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Core-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief cren vulkan backend objects
typedef struct {
    vkInstance instance;
    vkDevice device;
    vkSwapchain swapchain;
    int hint_resize;
    int hint_minimized;
    int hint_viewport;

    vkDefaultRenderphase defaultRenderphase;
    vkPickingRenderphase pickingRenderphase;
    vkUIRenderphase uiRenderphase;
    vkViewportRenderphase viewportRenderphase;

    Hashtable* buffersLib;
    Hashtable* pipelinesLib;
} CRenVulkanBackend;

/// @brief initializes the vulkan renderer, this is called by cren
/// @param backend ptr to the backend
/// @param ci ptr to the create info specification
/// @return 1 on success, 0 on failure
CREN_API int cren_vulkan_init(CRenVulkanBackend* backend, CRenCreateInfo* ci);

/// @brief shuts down all vulkan resources used by cren, this is called by cren
/// @param backend vulkan backend memory address
CREN_API void cren_vulkan_shutdown(CRenVulkanBackend* backend);

/// @brief updates the current frame
/// @param context cren context memory address
/// @param timestep interpolation value between frames
CREN_API void cren_vulkan_update(CRenContext* context, double timestep);

/// @brief renders the current frame
/// @param context cren context memory address
/// @param timestep interpolation value between frames
CREN_API void cren_vulkan_render(CRenContext* context, double timestep);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Image-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief creates a vulkan image based on requirements
/// @param width image's width
/// @param height image's height
/// @param mipLevels the quantity of miplevels, usually 1
/// @param arrayLayers the quantity of layers the image has, usually 1
/// @param device vulkan device
/// @param physicalDevice vulkan physical device
/// @param image output image
/// @param memory output image memory
/// @param format image's desired format
/// @param samples anti-alisign multisample
/// @param tiling image's tiling, usually VK_IMAGE_TYLING_OPTIMAL
/// @param usage image's usage
/// @param memoryProperties image's memory property
/// @param flags image's flag, usually 0
/// @return 
CREN_API int crenvk_image_create(unsigned int width, unsigned int height, unsigned int mipLevels, unsigned int arrayLayers, VkDevice device, VkPhysicalDevice physicalDevice, VkImage* image, VkDeviceMemory* memory, VkFormat format, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImageCreateFlags flags);

/// @brief creates and reutnrs a vulkan image view based on arguments configuration
/// @param device vulkan device
/// @param image image reference for the view
/// @param format image format
/// @param aspect image aspect
/// @param mipLevel the mip map level ammout
/// @param layerCount how many layers the image view has, usually 1
/// @param viewType viewing type, usually VK_IMAGE_VIEW_TYPE_2D
/// @return the image view
CREN_API VkImageView crenvk_image_view_create(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect, unsigned int mipLevel, unsigned int layerCount, VkImageViewType viewType);

/// @brief creates a vulkan image sampler
/// @param device vulkan device
/// @param physicalDevice vulkan physical device
/// @param min filter
/// @param mag filter
/// @param u address mode
/// @param v address mode
/// @param w address mode
/// @param mipLevels desired miplevel
/// @return the created sampler
CREN_API VkSampler crenvk_image_sampler_create(VkDevice device, VkPhysicalDevice physicalDevice, VkFilter min, VkFilter mag, VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w, float mipLevels);

/// @brief creates a descriptor set based on sampler and view
/// @param device vulkan device
/// @param descriptorPool wich descriptor pool to record the descriptor 
/// @param descriptorSetLayout the descriptor layout
/// @param sampler image's sampler
/// @param view image's view
/// @return the created descriptor set
CREN_API VkDescriptorSet crenvk_image_descriptor_set_create(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkSampler sampler, VkImageView view);

/// @brief generates mipmaps for the image
/// @param device vulkan device
/// @param queue vulkan graphics queue
/// @param cmdPool command pool to record the buffer
/// @param width image's width
/// @param height image's height
/// @param mipLevels the desired miplevels
/// @param image the reference image
CREN_API void crenvk_image_mipmaps_create(VkDevice device, VkQueue queue, VkCommandPool cmdPool, int width, int height, int mipLevels, VkImage image);

/// @brief inserts a memory barrier in the image, changing image layout
/// @param cmdBuffer command buffer
/// @param image vulkan image
/// @param srcAccessFlags access flags 
/// @param dstAccessFlags access flags 
/// @param oldImageLayout old image layout
/// @param newImageLayout desired new image layout
/// @param srcStageMask pipeline stage mask
/// @param dstStageMask pipeline stage mask
/// @param subresourceRange VkImageSubresourceRange object
CREN_API void crenvk_image_memory_barrier_insert(VkCommandBuffer cmdBuffer, VkImage image, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);

/// @brief changes the image layout
/// @param device vulkan device
/// @param queue vulkan graphics queue
/// @param cmdPool the command pool to make the command
/// @param image vulkan image
/// @param oldLayout previous image layout
/// @param newLayout new desired layout
/// @param mipLevels image's mip level
/// @param layerCount image's layer count, usually 1
/// @return 1 on success, 0 on failure
CREN_API int crenvk_image_transition_layout(VkDevice device, VkQueue queue, VkCommandPool cmdPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, unsigned int mipLevels, unsigned int layerCount);

/// @brief returns the most suitable format within the listed ones
/// @param physicalDevice vulkan physical device
/// @param candidates list of format candidates
/// @param candidatesCount quantity of candidates
/// @param tiling tiling mode
/// @param features format features
/// @return the most suitable format or asserts if none could be found
CREN_API VkFormat crenvk_find_suitable_format(VkPhysicalDevice physicalDevice, const VkFormat* candidates, unsigned int candidatesCount, VkImageTiling tiling, VkFormatFeatureFlags features);

/// @brief returns the most suitable depth format available from a defined list
/// @param physicalDevice vulkan physical device
/// @return the choosen format
CREN_API VkFormat crenvk_find_depth_format(VkPhysicalDevice physicalDevice);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief cren push-constant buffer
typedef struct {
    align_as(8) unsigned long long id;
    align_as(16) mat4 model;
} vkPushConstant;

/// @brief cren camera buffer
typedef struct {
    align_as(16) mat4 view;
    align_as(16) mat4 viewInverse;
    align_as(16) mat4 proj;
} vkBufferCamera;

/// @brief cren vulkan buffer
typedef struct {
    int mapped;
    VkBuffer* buffers;
    VkDeviceMemory* memories;
    CRenArray* mappedData;
} vkBuffer;

/// @brief creates a vulkan buffer based on parameters
/// @param device vulkan device
/// @param physicalDevice vulkan physical device
/// @param usageFlags desired usage for the buffer
/// @param memoryFlags desired memory flags for the buffer
/// @param size the buffer's size in bytes
/// @return the created vkBuffer or NULL if an error has ocurred
CREN_API vkBuffer* crenvk_buffer_create(VkDevice device, VkPhysicalDevice physicalDevice, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, VkDeviceSize size);

/// @brief destroys the buffer and release it's resources
/// @param buffer cren vulkan buffer to be destroyed
/// @param device vulkan device
CREN_API void crenvk_buffer_destroy(vkBuffer* buffer, VkDevice device);

/// @brief maps the buffer, making it to be cpu-visible
/// @param buffer buffer to map
/// @param device vulkan device
/// @return 1 on success/already mapped, 0 on failure
CREN_API int crenvk_buffer_map(vkBuffer* buffer, VkDevice device);

/// @brief unmaps a buffer, making it cpu-nonvisible
/// @param buffer buffer to unmap
/// @param device vulkan device
CREN_API void crenvk_buffer_unmap(vkBuffer* buffer, VkDevice device);

/// @brief creates a command buffer
/// @param device vulkan device
/// @param cmdPool wich command pool to record the command buffer into
/// @param level the commandbuffer level, usualy VK_COMMAND_BUFFER_LEVEL_PRIMARY
/// @param begin hints the start of the recording
/// @return the created command buffer
CREN_API VkCommandBuffer crenvk_commandbuffer_create(VkDevice device, VkCommandPool cmdPool, VkCommandBufferLevel level, int begin);

/// @brief creates a command buffer designed to be used once
/// @param device vulkan device
/// @param cmdPool wich command pool to record the command buffer into
/// @return the created command buffer
CREN_API VkCommandBuffer crenvk_commandbuffer_begin_singletime(VkDevice device, VkCommandPool cmdPool);

/// @brief ends the recording of a command buffer previously created for the one-time usage
/// @param device vulkan device
/// @param cmdPool wich command pool to record the command buffer into
/// @param cmdBuffer the command buffer
/// @param queue wich queue to send the command buffer, usually graphics
CREN_API void crenvk_commandbuffer_end_singletime(VkDevice device, VkCommandPool cmdPool, VkCommandBuffer cmdBuffer, VkQueue queue);

/// @brief starts the recording of a command buffer
/// @param cmdBuffer the command buffer
/// @return 1 on success, 0 on failure
CREN_API int crenvk_commandbuffer_begin(VkCommandBuffer cmdBuffer);

/// @brief stops the recording of a command buffer and send it to the gpu
/// @param device vulkan device
/// @param cmdPool wich command pool to record the buffer to
/// @param cmdBuffer the command buffer
/// @param queue wich queue to send the command buffer, usually graphics
/// @param free hints the command buffers to be freed
/// @return 1 on success, 0 on failure
CREN_API int crenvk_commandbuffer_end(VkDevice device, VkCommandPool cmdPool, VkCommandBuffer cmdBuffer, VkQueue queue, int free);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief 2d texture vulkan objects
typedef struct Texture2DBackend {
    VkImage image;
    VkDeviceMemory memory;
    VkSampler sampler;
    VkImageView view;
    VkDescriptorSet uiDescriptor;
} CRenTexture2DBackend;

/// @brief 2d texture relevant data
typedef struct {
	char path[CREN_PATH_MAX_SIZE];
	int width;
	int height;
	int mipLevels;
	CRenTexture2DBackend* backend;
} CRenTexture2D;

/// @brief 2d texture relevant data, using buffer instead of path
typedef struct {
	char* data;
    size_t lenght;
	int width;
	int height;
} CrenTexture2DBuffer;

/// @brief creates a vulkan texture 2d from disk path
/// @param context cren context
/// @param path the texture's disk path
/// @param gui hints the texture to be used in the ui
/// @return the created 2d texture
CREN_API CRenTexture2D crenvk_texture2d_create_from_path(CRenContext* context, const char* path, int gui);

/// @brief creates a vulakn texture 2d from a buffer
/// @param context cren context
/// @param bufferInfo the buffer data
/// @param gui hints the texture to be used in the ui
/// @return the created 2d texture
CREN_API CRenTexture2D crenvk_texture2d_create_from_buffer(CRenContext* context, CrenTexture2DBuffer* bufferInfo, int gui);

/// @brief release the resources used by the texture
/// @param context cren context
/// @param texture the texture to have it's resources released
CREN_API void crenvk_texture2d_destroy(CRenContext* context, CRenTexture2D* texture);

/// @brief returns the texture's sampler
/// @param cren texture
/// @return texture's VkSampler object
CREN_API VkSampler crenvk_texture2d_get_sampler(CRenTexture2D* texture);

/// @brief returns the texture's iamge view
/// @param cren texture
/// @return texture's VkImageView object
CREN_API VkImageView crenvk_texture2d_get_image_view(CRenTexture2D* texture);

/// @brief returns the texture's descriptor, usually used in ui
/// @param cren texture
/// @return texture's VkDescriptorSet object
CREN_API VkDescriptorSet crenvk_texture2d_get_descriptor(CRenTexture2D* texture);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quad-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief a buffer that hold a quad's parameters
typedef struct {
    align_as(4) unsigned int billboard;	// always faces the camera
    align_as(4) float uv_rotation;		// rotates the uv/texture
    align_as(8) float2 lockAxis;	    // controls wich axis to lock
    align_as(8) float2 uv_offset;	    // offsets the uv/texture
    align_as(8) float2 uv_scale;	    // scales the uv/texture
} QuadParams;

/// @brief holds vulkan information about the quad
typedef struct {
	CRenTexture2D colormap;
	vkBuffer* buffer;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSets[CREN_CONCURRENTLY_RENDERED_FRAMES];
} vkQuadBackend;

/// @brief cren quad, holds information about a quad that may be drawn in the renderer
typedef struct {
	unsigned long long id;
	QuadParams params;
	vkQuadBackend* backend;
} CRenQuad;

/// @brief creates and returns a quad
/// @param context cren context
/// @param albedoPath the quad colormap
/// @return the created quad
CREN_API CRenQuad* crenvk_quad_create(CRenContext* context, const char* albedoPath);

/// @brief release all resources used by a quad
/// @param context cren context
/// @param quad the quad to destroy
CREN_API void crenvk_quad_destroy(CRenContext* context, CRenQuad* quad);

/// @brief sends newer data to the gpu about the quad
/// @param context cren context
/// @param quad the quad to update
CREN_API void crenvk_quad_apply_buffer_changes(CRenContext* context, CRenQuad* quad);

/// @brief renders the quad
/// @param context cren quad
/// @param stage wich render stage is, picking/default
/// @param quad the quad to render
/// @param transform quad's transformation matrix
CREN_API void crenvk_quad_render(CRenContext* context, CRenRenderStage stage, CRenQuad* quad, const mat4 transform);

#ifdef __cplusplus 
}
#endif

#endif // CREN_VULKAN_INCLUDED