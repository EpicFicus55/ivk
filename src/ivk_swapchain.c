#include <stdlib.h>

#include "ivk_util.h"
#include "ivk_swapchain.h"

/*
 * Query swapchain support details for a certain
 * window surface.
 */
void ivk_swapchain_query_support
    (
    VkPhysicalDevice            physical_device,
    VkSurfaceKHR                surface,
    IVK_swapchain_details_type* swapchain_details
    )
{
/* Query the swapchain capabilities */
vkGetPhysicalDeviceSurfaceCapabilitiesKHR
    (
    physical_device,
    surface,
    &swapchain_details->capabilities
    );

/* Query the supported formats */
__vk( vkGetPhysicalDeviceSurfaceFormatsKHR( physical_device, surface, &swapchain_details->format_count, NULL ) );
swapchain_details->formats = ( VkSurfaceFormatKHR* )malloc( swapchain_details->format_count * sizeof( VkSurfaceFormatKHR ) );
__vk( vkGetPhysicalDeviceSurfaceFormatsKHR( physical_device, surface, &swapchain_details->format_count, swapchain_details->formats ) );

/* Query the presentation modes */
__vk( vkGetPhysicalDeviceSurfacePresentModesKHR( physical_device, surface, &swapchain_details->present_modes_count, NULL ) );
swapchain_details->present_modes = ( VkPresentModeKHR* )malloc( swapchain_details->present_modes_count * sizeof( VkPresentModeKHR ) );
__vk( vkGetPhysicalDeviceSurfacePresentModesKHR( physical_device, surface, &swapchain_details->present_modes_count, swapchain_details->present_modes ) );

return;

}

/*
 * Frees the dynamic memory required by the swapchain details struct.
 */
void ivk_swapchain_free_support
    (
    IVK_swapchain_details_type* swapchain_details
    )
{
swapchain_details->format_count = 0;
swapchain_details->present_modes_count = 0;

free( swapchain_details->formats );
free( swapchain_details->present_modes );

return;

}

/*
 * Creates a swapchain with the following characteristics:
 *      - Maximum image extents
 *      - 8B8G8R8A color format and SRGB color space
 *      - Mailbox presentation mode
 * Also sets these values for later use.
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
    )
{
/* Local variables */
VkSurfaceFormatKHR  _surface_format = swapchain_details->formats[ 0 ];
VkPresentModeKHR    _present_mode = VK_PRESENT_MODE_FIFO_KHR;
VkExtent2D          _extent = { 0 };
unsigned int        _image_count = 0;
VkSwapchainCreateInfoKHR
                    _create_info = { 0 };
unsigned int        _queue_family_indices[ 2 ];
VkSwapchainKHR      _swapchain;

/* Make sure the surface supports the requested format. Returns
the first one otherwise. */
for( unsigned int i = 0; i < swapchain_details->format_count; i++ )
    {
    if( swapchain_details->formats[ i ].format == VK_FORMAT_B8G8R8A8_SRGB  &&
        swapchain_details->formats[ i ].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
        {
        _surface_format = swapchain_details->formats[ i ];
        break;
        }
    }

/* Make sure the swapchain supports mailbox presentation types. Use
FIFO otherwise. */
for( unsigned int i = 0; i < swapchain_details->present_modes_count; i++ )
    {
    if( swapchain_details->present_modes[ i ] == VK_PRESENT_MODE_MAILBOX_KHR )
        {
        _present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
        break;
        }
    }

/* We shall use the maximum surface extent for the swapchain.
This might cause problems on other platforms, but on Win32 it
should be fine. YOLO */
_extent.width  = swapchain_details->capabilities.maxImageExtent.width;
_extent.height = swapchain_details->capabilities.maxImageExtent.height;

/* Set the image count; should be 2 */
_image_count = swapchain_details->capabilities.minImageCount; /* Temp */

/* Save the queue family indices */
_queue_family_indices[ 0 ] = graphics_family_idx;
_queue_family_indices[ 1 ] = present_family_idx;

/* Create the swapchain */
_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
_create_info.surface = surface;
_create_info.minImageCount = _image_count;
_create_info.imageFormat = _surface_format.format;
_create_info.imageColorSpace = _surface_format.colorSpace;
_create_info.imageExtent = _extent;
_create_info.imageArrayLayers = 1;
_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
_create_info.pQueueFamilyIndices = &_queue_family_indices[ 0 ];
_create_info.queueFamilyIndexCount = 2;
_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
_create_info.preTransform = swapchain_details->capabilities.currentTransform;
_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
_create_info.presentMode = _present_mode;
_create_info.clipped = VK_TRUE;
_create_info.oldSwapchain = VK_NULL_HANDLE;

/* Create the swapchain */
__vk( vkCreateSwapchainKHR( device, &_create_info, NULL, &_swapchain ) );

*swapchain = _swapchain;
*ctx_format = _surface_format.format;
*ctx_extent = _extent;

return;

}


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
    )
{
/* Local variables */
VkImageView*    _views = NULL;

/* Create an image view for each of the images in the swapchain */
_views = ( VkImageView* )malloc( image_count * sizeof( VkImageView ) );
if( !_views )
    {
    printf( "Failed to allocate memory for the image views.\n" );
    return;
    }

for( unsigned int i = 0; i < image_count; i++ )
    {
    VkImageViewCreateInfo   _image_view_create_info = { 0 };
    _image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    _image_view_create_info.image = images[ i ];
    _image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    _image_view_create_info.format = format;

    _image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    _image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    _image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    _image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    _image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _image_view_create_info.subresourceRange.baseMipLevel = 0;
    _image_view_create_info.subresourceRange.levelCount = 1;
    _image_view_create_info.subresourceRange.baseArrayLayer  = 0;
    _image_view_create_info.subresourceRange.layerCount = 1;

    __vk( vkCreateImageView
            (
            device,
            &_image_view_create_info,
            NULL,
            &_views[ i ]
            ) );

    }

*image_views = _views;
return;

}


/*
 * Destroys the image views and deallocates the memory.
 */
void ivk_swapchain_destroy_image_views
    (
    VkDevice        device,
    unsigned int    image_count,
    VkImageView*    image_views
    )
{
for( unsigned int i = 0; i < image_count; i++ )
    {
    vkDestroyImageView( device, image_views[ i ], NULL );
    }

/* Free the memory */
free( image_views );

return;

}

/*
 * Retrieve the swapchain images.
 */
void ivk_swapchain_retrieve_images
    (
    VkDevice        device,
    VkSwapchainKHR  swapchain,
    unsigned int*   image_count,
    VkImage**       swapchain_images
    )
{
__vk( vkGetSwapchainImagesKHR( device, swapchain, image_count, NULL ) );
*swapchain_images = ( VkImage* )malloc( *image_count * sizeof(VkImage) );

if( *swapchain_images == NULL )
    {
    printf( "Failed to allocate memory for the swapchain images.\n" );
    return;
    }

__vk( vkGetSwapchainImagesKHR( device, swapchain, image_count, *swapchain_images ) );

return;

}