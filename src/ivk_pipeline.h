#pragma once
#include "vulkan/vulkan.h"


/*
 * Creates a pipeline layout object
 */
void ivk_pipeline_create_layout
    (
    VkDevice            device,
    VkPipelineLayout*   pipeline_layout
    );

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
    );
