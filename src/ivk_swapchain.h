#pragma once
#include "vulkan/vulkan.h"

typedef struct
    {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR*      formats;
    unsigned int             format_count;
    VkPresentModeKHR*        present_modes;
    unsigned int             present_modes_count;
    } IVK_swapchain_details_type;

/*
 * Query swapchain support details for a certain
 * window surface.
 */
void ivk_swapchain_query_support
    (
    VkPhysicalDevice        physical_device,
    VkSurfaceKHR            surface,
    IVK_swapchain_details_type* swapchain_details
    );

/*
 * Frees the dynamic memory required by the swapchain details struct.
 */
void ivk_swapchain_free_support
    (
    IVK_swapchain_details_type* swapchain_details
    );

/*
 * Creates a swapchain with the following characteristics:
 *      - Maximum image extents
 *      - 8B8G8R8A color format and SRGB color space
 *      - Mailbox presentation mode
 */
void ivk_swapchain_create
    (
    VkDevice                    device,
    IVK_swapchain_details_type* swapchain_details,
    VkSurfaceKHR                surface,
    unsigned int                graphics_family_idx,
    unsigned int                present_family_idx,
    VkSwapchainKHR*             swapchain,
    VkFormat*                   ctx_format,
    VkExtent2D*                 ctx_extent
    );


/*
 * Creates the image views for the images
 * in the swapchain.
 */
void ivk_swapchain_create_image_views
    (
    VkDevice        device,
    unsigned int    image_count,
    VkImage*        images,
    VkFormat        format,
    VkImageView**   image_views
    );


/*
 * Destroys the image views and deallocates the memory.
 */
void ivk_swapchain_destroy_image_views
    (
    VkDevice        device,
    unsigned int    image_count,
    VkImageView*    image_views
    );


/*
 * Retrieve the swapchain images.
 */
void ivk_swapchain_retrieve_images
    (
    VkDevice        device,
    VkSwapchainKHR  swapchain,
    unsigned int*   image_count,
    VkImage**       swapchain_images
    );
