#pragma once

#include <stdio.h>
#include "vulkan/vulkan.h"

/* This needs to go away ASAP */
#define GLFW_INCLUDE_VULKAN
#include "glfw/glfw3.h"

#include "ivk_buffers.h"
#include "ivk_util.h"
#include "ivk_swapchain.h"

/*
 * Debug macros
 */
#if defined( _DEBUG )
    #define VALIDATION_ENABLED  1
#else
    #define VALIDATION_ENABLED  0
#endif

#define MAX_FRAMES_IN_FLIGHT    2

typedef struct
    {
    /* Graphics components */
    VkInstance          vk_instance;
    VkPhysicalDevice    vk_physical_device;
    VkDevice            vk_device;
    VkCommandPool       vk_graphics_command_pool;
    VkQueue             vk_graphics_queue;
    unsigned int        vk_graphics_family_idx;
    VkDescriptorSetLayout vk_pipeline_descriptor_set_layout;
    VkPipelineLayout    vk_pipeline_layout;
    VkPipeline          vk_pipeline;
    VkCommandBuffer     vk_command_buffer[ MAX_FRAMES_IN_FLIGHT ];
    VkRenderPass        vk_renderpass;

    /* Transfer components */
    VkQueue             vk_transfer_queue;
    unsigned int        vk_transfer_family_idx;
    VkCommandPool       vk_transfer_command_pool;

    /* Presentation components */
    GLFWwindow*         glfw_window;
    VkSurfaceKHR        vk_surface;
    unsigned int        vk_present_family_idx;
    VkQueue             vk_present_queue;

    /* Swapchain information */
    IVK_swapchain_details_type
                        swapchain_details;
    VkSwapchainKHR      vk_swapchain;
    VkFormat            swapchain_format;
    VkExtent2D          swapchain_extent;
    unsigned int        swapchain_image_count;
    VkImage*            vk_images;
    VkImageView*        vk_image_views;
    VkFramebuffer*      vk_framebuffers;

    /* Synchronization mechanisms */
    VkSemaphore         image_available_semaphore[ MAX_FRAMES_IN_FLIGHT ];
    VkSemaphore         render_finished_semaphore[ MAX_FRAMES_IN_FLIGHT ];
    VkFence             in_flight_fence[ MAX_FRAMES_IN_FLIGHT ];

    /* User data - will go away soon */
    VkBuffer            triangle_vert_buffer;
    VkDeviceMemory      triangle_buffer_memory;
    VkBuffer            triangle_index_buffer;
    VkDeviceMemory      triangle_index_buffer_memory;
    unsigned int        index_count;
    } IVK_Context;


/*
 * Initializes IVK library
 */
void ivk_init
    (
    unsigned int instance_extension_count,
    const char** instance_extensions,
    GLFWwindow*  window
    );

/*
 * Initializes the triangle for rendering
 */
void ivk_init_triangle
    (
    ivk_2p3c_type*  triangle_data,
    unsigned int    vert_cnt,
    unsigned int*   index_data,
    unsigned int    index_cnt
    );

/*
 * Renders to the screen.
 */
void ivk_render
    (
    void
    );

/*
 * Teardown of IVK library
 */
void ivk_teardown
    (
    void
    );
    