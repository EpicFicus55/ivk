#include "ivk_pipeline.h"
#include "ivk_buffers.h"
#include "ivk_util.h"
#include "vulkan/vulkan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*** Static functions for initialization ***/

/*
 * Create a vulkan shader module
 */
static VkShaderModule create_shader_module
    (
    VkDevice        device,
    char*           shader_spv,
    unsigned int    shader_spv_size
    );

/*
 * Read a binary file from a given path and stores it
 * to a buffer. Note that the function dynamically
 * allocates memory, which must be freed by the caller.
 */
static void read_binary_file_into
    (
    char*           input_file_name,
    char**          output_buffer,
    unsigned int*   buffer_size
    );


/*
 * Creates a pipeline layout object
 */
void ivk_pipeline_create_layout
    (
    VkDevice            device,
    VkPipelineLayout*   pipeline_layout
    )
{
/* Local variables */
VkPipelineLayoutCreateInfo _create_info = { 0 };

_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
_create_info.setLayoutCount = 0;
_create_info.pSetLayouts = NULL;
_create_info.pushConstantRangeCount = 0;
_create_info.pPushConstantRanges = NULL;

/* Create the pipeline layout object */
__vk( vkCreatePipelineLayout( device, &_create_info, NULL, pipeline_layout ) );

}


/*
 * Creates a graphics pipeline based on the shaders
 * provided
 */
void ivk_pipeline_create
    (
    VkDevice            device,
    VkPipelineLayout    pipeline_layout,
    VkExtent2D          extent,
    VkRenderPass        renderpass,
    char*               vert_shader,
    char*               frag_shader,
    VkPipeline*         pipeline
    )
{
/* Local variables */
char*           _vert_shdr = NULL;
unsigned int    _vert_shdr_spv_size = 0;
VkShaderModule  _vert_shader_module = { 0 };
VkPipelineShaderStageCreateInfo _vert_shader_stage_create_info = { 0 };

char*           _frag_shdr = NULL;
unsigned int    _frag_shdr_spv_size = 0;
VkShaderModule  _frag_shader_module = { 0 };
VkPipelineShaderStageCreateInfo _frag_shader_stage_create_info = { 0 };

VkPipelineShaderStageCreateInfo _pipeline_shader_stages[ 2 ];

VkDynamicState  _dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
VkPipelineDynamicStateCreateInfo _dynamic_state_create_info = { 0 };

VkPipelineVertexInputStateCreateInfo _vertex_input_info = { 0 };
VkVertexInputBindingDescription*     _veretx_bind_desc = { 0 };
VkVertexInputAttributeDescription*   _veretx_bind_desc_arr = NULL;
VkPipelineInputAssemblyStateCreateInfo _input_assembly_create_info = { 0 };
VkViewport  _viewport = { 0 };
VkRect2D    _scissor = { 0 };

VkPipelineViewportStateCreateInfo _viewport_state_create_info = { 0 };
VkPipelineRasterizationStateCreateInfo _rasterizer_create_info = { 0 };
VkPipelineMultisampleStateCreateInfo  _multi_sampling_create_info = { 0 };

VkPipelineColorBlendAttachmentState _color_blending_attachment = { 0 };
VkPipelineColorBlendStateCreateInfo _color_blending_create_info = { 0 };

VkGraphicsPipelineCreateInfo _pipeline_create_info = { 0 };

/* Read the shader files */
read_binary_file_into( "..\\src\\shaders\\triangles.vert.spv", &_vert_shdr, &_vert_shdr_spv_size );
read_binary_file_into( "..\\src\\shaders\\triangles.frag.spv", &_frag_shdr, &_frag_shdr_spv_size );

_vert_shader_module = create_shader_module( device, _vert_shdr, _vert_shdr_spv_size );
_frag_shader_module = create_shader_module( device, _frag_shdr, _frag_shdr_spv_size );

/* Initialize the vertex shader stage */
_vert_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
_vert_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
_vert_shader_stage_create_info.module = _vert_shader_module;
_vert_shader_stage_create_info.pName = "main";

/* Initialize the fragment shader stage */
_frag_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
_frag_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
_frag_shader_stage_create_info.module = _frag_shader_module;
_frag_shader_stage_create_info.pName = "main";

_pipeline_shader_stages[ 0 ] = _vert_shader_stage_create_info;
_pipeline_shader_stages[ 1 ] = _frag_shader_stage_create_info;

/* Set up the vertex input */
_veretx_bind_desc = ivk_2p3c_get_bind_desc();
_veretx_bind_desc_arr = ivk_2p3c_get_attr_desc();

_vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
_vertex_input_info.vertexBindingDescriptionCount = IVK_2P3C_BIND_CNT;
_vertex_input_info.pVertexBindingDescriptions = _veretx_bind_desc;
_vertex_input_info.vertexAttributeDescriptionCount = IVK_2P3C_ATTR_CNT;
_vertex_input_info.pVertexAttributeDescriptions = _veretx_bind_desc_arr;

/* Set up the input assembly */
_input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
_input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
_input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

/* Set up the viewport */
_viewport.x = 0.0f;
_viewport.y = 0.0f;
_viewport.width = extent.width;
_viewport.height = extent.height;
_viewport.minDepth = 0.0f;
_viewport.maxDepth = 1.0f;

/* Set up the scissor rectangle */
_scissor.offset.x = 0;
_scissor.offset.y = 0;
_scissor.extent = extent;

/* Set up the dynamic states - scissor box and viewport */
_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
_dynamic_state_create_info.dynamicStateCount = 2;
_dynamic_state_create_info.pDynamicStates = &_dynamic_states[ 0 ];

_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
_viewport_state_create_info.viewportCount = 1;
_viewport_state_create_info.scissorCount = 1;

/* Set up the rasterizer */
_rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
_rasterizer_create_info.depthClampEnable  = VK_FALSE;
_rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
_rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
_rasterizer_create_info.lineWidth = 1.0f;
_rasterizer_create_info.cullMode = VK_CULL_MODE_NONE;
_rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
_rasterizer_create_info.depthBiasEnable = VK_FALSE;
_rasterizer_create_info.depthBiasConstantFactor = 0.0f;
_rasterizer_create_info.depthBiasClamp = 0.0f;
_rasterizer_create_info.depthBiasSlopeFactor = 0.0f;

/* Set up multi-sampling */
_multi_sampling_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
_multi_sampling_create_info.sampleShadingEnable = VK_FALSE;
_multi_sampling_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
_multi_sampling_create_info.minSampleShading = 1.0f;
_multi_sampling_create_info.pSampleMask = NULL;
_multi_sampling_create_info.alphaToCoverageEnable = VK_FALSE;
_multi_sampling_create_info.alphaToOneEnable = VK_FALSE;

/* Set up color blending */
_color_blending_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
_color_blending_attachment.blendEnable = VK_FALSE;
_color_blending_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
_color_blending_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
_color_blending_attachment.colorBlendOp = VK_BLEND_OP_ADD;
_color_blending_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
_color_blending_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
_color_blending_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

_color_blending_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
_color_blending_create_info.logicOpEnable = VK_FALSE;
_color_blending_create_info.logicOp = VK_LOGIC_OP_COPY;
_color_blending_create_info.attachmentCount = 1;
_color_blending_create_info.pAttachments = &_color_blending_attachment;
_color_blending_create_info.blendConstants[ 0 ] = 0.0f;
_color_blending_create_info.blendConstants[ 1 ] = 0.0f;
_color_blending_create_info.blendConstants[ 2 ] = 0.0f;
_color_blending_create_info.blendConstants[ 3 ] = 0.0f;

/* Create the pipeline */
_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
_pipeline_create_info.stageCount = 2;
_pipeline_create_info.pStages = &_pipeline_shader_stages[ 0 ];
_pipeline_create_info.pVertexInputState = &_vertex_input_info;
_pipeline_create_info.pInputAssemblyState = &_input_assembly_create_info;
_pipeline_create_info.pViewportState = &_viewport_state_create_info;
_pipeline_create_info.pRasterizationState = &_rasterizer_create_info;
_pipeline_create_info.pMultisampleState = &_multi_sampling_create_info;
_pipeline_create_info.pDepthStencilState = NULL;
_pipeline_create_info.pColorBlendState = &_color_blending_create_info;
_pipeline_create_info.pDynamicState = &_dynamic_state_create_info;
_pipeline_create_info.layout = pipeline_layout;
_pipeline_create_info.renderPass = renderpass;
_pipeline_create_info.subpass = 0;
_pipeline_create_info.basePipelineHandle = NULL;
_pipeline_create_info.basePipelineIndex = -1;

__vk( vkCreateGraphicsPipelines( device, VK_NULL_HANDLE, 1, &_pipeline_create_info, NULL, pipeline ) );

/* Free the files */
free( _vert_shdr );
free( _frag_shdr );

/* Destroy shader modules */
vkDestroyShaderModule( device, _vert_shader_module, NULL );
vkDestroyShaderModule( device, _frag_shader_module, NULL );
}


/*
 * Create a vulkan shader module
 */
static VkShaderModule create_shader_module
    (
    VkDevice        device,
    char*           shader_spv,
    unsigned int    shader_spv_size
    )
{
/* Local variables */
VkShaderModuleCreateInfo _create_info = { 0 };
VkShaderModule           _ret = { 0 };

_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
_create_info.codeSize = shader_spv_size;
_create_info.pCode = ( uint32_t* )shader_spv;

__vk( vkCreateShaderModule( device, &_create_info, NULL, &_ret ) );

return _ret;

}


/*
 * Read a binary file from a given path and stores it
 * to a buffer. Note that the function dynamically
 * allocates memory, which must be freed by the caller.
 */
static void read_binary_file_into
    (
    char*           input_file_name,
    char**          output_buffer,
    unsigned int*   buffer_size
    )
{
/* Local variables */
FILE*   _input_stream = NULL;
errno_t _err = 0;

/* Begin the file stream. Print a message if file doesn't exist */
_err = fopen_s( &_input_stream, input_file_name, "rb" );
if( _err )
    {
    printf( "Error reading SPV file: %s\n", input_file_name );
    return;
    }

/* Find out how long the file is */
fseek( _input_stream, 0, SEEK_END );
*buffer_size = ftell( _input_stream );

/* Reset the stream */
rewind( _input_stream );

/* Allocate enough memory to keep all the contents of the file */
*output_buffer = ( char* )calloc( *buffer_size, sizeof(char) );
fread_s( *output_buffer, *buffer_size, 1, *buffer_size, _input_stream );

/* Close the file stream */
fclose( _input_stream );

}