#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "ivk.h"
#include "ivk_buffers.h"
#include "ivk_validation.h"
#include "ivk_pipeline.h"
#include "ivk_util.h"
#include "cglm/cglm.h"

/* Required device extensions */
static const char* g_device_extensions[ 1 ] =
    {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
static const unsigned int g_device_extensions_count = 1;


/* Global context for IVK library */
static IVK_Context g_ivk_context;

/* External validation layer information */
extern const char* g_validation_layers[];
extern const unsigned int g_validation_layer_cnt;

/* Keep track of the current frame */
static unsigned int g_current_frame;


/*** Static functions for initialization ***/
/*
 * Creates the Vulkan instance for IVK
 */
static void ivk_create_instance
    (
    unsigned int instance_extension_count,
    const char** instance_extensions
    );

/*
 * Selects the appropriate physical device
 * for the application
 */
static void ivk_select_physical_device
    (
    void
    );

/*
 * Suitability checks for physical devices
 */
static bool ivk_is_device_suitable
    (
    VkPhysicalDevice
    );

/*
 * Selects the appropriate queue families.
 */
static bool ivk_select_queue_families
    (
    VkPhysicalDevice
    );

/*
 * Creates a logical device.
 */
static void ivk_create_logical_device
    (
    void
    );

/*
 * TEMP. Creates a window surface. This is
 * a bad place to define this, since IVK should
 * be a pure rendering library, without any
 * references to a windowing library.
 */
static void ivk_create_window_surface
    (
    void
    );


/*
 * Initializes the presentation related objects.
 * - swapchain
 * - retrieves images
 * - creates image views
 * - creates framebuffers
 */
static void ivk_init_presentation
    (
    void
    );

/*
 * Cleans up the presentation objects
 */
static void ivk_clean_presentation
    (
    void
    );

/*
 * Re-creates the presentation objects when the swapchain
 * is inadequate.
 */
static void ivk_recreate_presentation
    (
    void
    );


/*
 * Creates a renderpass object.
 */
static void ivk_create_renderpass
    (
    void
    );

/*
 * Creates the framebuffers for the swapchain.
 */
static void ivk_create_framebuffers
    (
    void
    );

/*
 * Creates the command pool.
 */
static void ivk_create_command_pools
    (
    void
    );

/*
 * Creates the command buffer.
 */
static void ivk_create_command_buffers
    (
    void
    );

/*
 * Record the commands in the command buffer.
 */
static void ivk_record_command_buffer
    (
    VkCommandBuffer command_buffer,
    unsigned int    image_index
    );

/*
 * Creates the synchronization primitives.
 */
static void ivk_create_sync_objects
    (
    void
    );

/*** Function definitions ***/
/*
 * Initializes IVK library
 */
void ivk_init
    (
    unsigned int instance_extension_count,
    const char** instance_extensions,
    GLFWwindow*  window
    )
{
g_ivk_context.glfw_window = window;
g_current_frame = 0;

/* Create the instance */
ivk_create_instance( instance_extension_count, instance_extensions );

/* Set the window surface */
ivk_create_window_surface();

/* Select the physical device */
ivk_select_physical_device();

/* Create a logical device */
ivk_create_logical_device();

ivk_init_presentation();

/* Create the renderpass */
ivk_create_renderpass();

/* Create the framebuffers */
ivk_create_framebuffers();

/* Create the pipeline layout and the pipeline */
ivk_pipeline_create_layout( g_ivk_context.vk_device, &g_ivk_context.vk_pipeline_layout );
ivk_pipeline_create
    (
    g_ivk_context.vk_device,
    g_ivk_context.vk_pipeline_layout,
    g_ivk_context.swapchain_extent,
    g_ivk_context.vk_renderpass,
    NULL,
    NULL,
    &g_ivk_context.vk_pipeline
    );

/* Create the command pool */
ivk_create_command_pools();

/* Create the command buffer */
ivk_create_command_buffers();

/* Create the semaphores and the fence */
ivk_create_sync_objects();

/* Free these after initialization as they are no longer necessary */
//ivk_swapchain_free_support( &g_ivk_context.swapchain_details );

}


/*
 * Initializes the triangle for rendering
 */
void ivk_init_triangle
    (
    ivk_2p3c_type*  triangle_data,
    unsigned int    vert_cnt,
    unsigned int*   index_data,
    unsigned int    index_cnt
    )
{
g_ivk_context.index_count = index_cnt;

ivk_buffer_create_vbo
    (
    g_ivk_context.vk_device,
    g_ivk_context.vk_physical_device,
    g_ivk_context.vk_transfer_command_pool,
    g_ivk_context.vk_transfer_queue,
    triangle_data,
    vert_cnt,
    &g_ivk_context.triangle_vert_buffer,
    &g_ivk_context.triangle_buffer_memory
    );

ivk_buffer_create_ibo
    (
    g_ivk_context.vk_device,
    g_ivk_context.vk_physical_device,
    g_ivk_context.vk_transfer_command_pool,
    g_ivk_context.vk_transfer_queue,
    index_data,
    index_cnt,
    &g_ivk_context.triangle_index_buffer,
    &g_ivk_context.triangle_index_buffer_memory
    );
}


/*
 * Renders to the screen.
 */
void ivk_render
    (
    void
    )
{
/* Local variables */
unsigned int            _image_index = 0;
VkSubmitInfo            _submit_info = { 0 };
VkSemaphore             _wait_semaphores[] = { g_ivk_context.image_available_semaphore[ g_current_frame ] };
VkPipelineStageFlags    _wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
VkSemaphore             _signal_semaphores[] = { g_ivk_context.render_finished_semaphore[ g_current_frame ] };
VkPresentInfoKHR        _present_info = { 0 };
VkSwapchainKHR          _swapchains[] = { g_ivk_context.vk_swapchain };
VkResult                _ret = VK_SUCCESS;

/* Set up the submit info */
_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
_submit_info.waitSemaphoreCount = 1;
_submit_info.pWaitSemaphores = _wait_semaphores;
_submit_info.pWaitDstStageMask = _wait_stages;
_submit_info.signalSemaphoreCount = 1;
_submit_info.pSignalSemaphores = _signal_semaphores;

/* Wait for the previous frame to finish */
__vk( vkWaitForFences( g_ivk_context.vk_device, 1, &g_ivk_context.in_flight_fence[ g_current_frame ], VK_TRUE, UINT64_MAX ) );

/* Acquire the next image */
_ret = vkAcquireNextImageKHR
    (
    g_ivk_context.vk_device,
    g_ivk_context.vk_swapchain,
    UINT64_MAX,
    g_ivk_context.image_available_semaphore[ g_current_frame ],
    VK_NULL_HANDLE,
    &_image_index
    );
switch( _ret )
    {
    case VK_ERROR_OUT_OF_DATE_KHR:
        ivk_recreate_presentation();
        return;
    case VK_SUBOPTIMAL_KHR:
    case VK_SUCCESS:
        break;
    default:
        printf( "Error acquiring image.\n" );
        break;
    }

/* Reset the fence */
__vk( vkResetFences( g_ivk_context.vk_device, 1, &g_ivk_context.in_flight_fence[ g_current_frame ] ) );

/* Reset the command buffer */
__vk( vkResetCommandBuffer( g_ivk_context.vk_command_buffer[ g_current_frame ], 0 ) );

/* Record the commands in the command buffer */
ivk_record_command_buffer( g_ivk_context.vk_command_buffer[ g_current_frame ], _image_index );

_submit_info.commandBufferCount = 1;
_submit_info.pCommandBuffers = &g_ivk_context.vk_command_buffer[ g_current_frame ];

/* Submit the command buffer */
__vk( vkQueueSubmit
        (
        g_ivk_context.vk_graphics_queue,
        1,
        &_submit_info,
        g_ivk_context.in_flight_fence[ g_current_frame ]
        ) );

/* Set up the presentation */
_present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
_present_info.waitSemaphoreCount = 1;
_present_info.pWaitSemaphores = _signal_semaphores;
_present_info.swapchainCount = 1;
_present_info.pSwapchains = _swapchains;
_present_info.pImageIndices = &_image_index;
_present_info.pResults = NULL;

/* Present to the screen */
_ret = vkQueuePresentKHR( g_ivk_context.vk_present_queue, &_present_info );
switch( _ret )
    {
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
        ivk_recreate_presentation();
        break;
    case VK_SUCCESS:
        break;
    default:
        printf( "Error acquiring image.\n" );
        break;
    }

/* Move on to the next frame */
g_current_frame = ( g_current_frame + 1 ) % MAX_FRAMES_IN_FLIGHT;

}


/*
 * Teardown of IVK library
 */
void ivk_teardown
    (
    void
    )
{
/* Wait for everything to finish before tearing down the application */
vkDeviceWaitIdle( g_ivk_context.vk_device );

ivk_clean_presentation();

vkDestroyBuffer( g_ivk_context.vk_device, g_ivk_context.triangle_vert_buffer, NULL );
vkFreeMemory( g_ivk_context.vk_device, g_ivk_context.triangle_buffer_memory, NULL );
vkDestroyBuffer( g_ivk_context.vk_device, g_ivk_context.triangle_index_buffer, NULL );
vkFreeMemory( g_ivk_context.vk_device, g_ivk_context.triangle_index_buffer_memory, NULL );

for( unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
    {
    vkDestroySemaphore( g_ivk_context.vk_device, g_ivk_context.image_available_semaphore[ i ], NULL );
    vkDestroySemaphore( g_ivk_context.vk_device, g_ivk_context.render_finished_semaphore[ i ], NULL );
    vkDestroyFence( g_ivk_context.vk_device, g_ivk_context.in_flight_fence[ i ], NULL );
    }

vkDestroyCommandPool( g_ivk_context.vk_device, g_ivk_context.vk_graphics_command_pool, NULL );
vkDestroyCommandPool( g_ivk_context.vk_device, g_ivk_context.vk_transfer_command_pool, NULL );
vkDestroyPipeline( g_ivk_context.vk_device, g_ivk_context.vk_pipeline, NULL );

vkDestroyRenderPass( g_ivk_context.vk_device, g_ivk_context.vk_renderpass, NULL );
vkDestroyPipelineLayout( g_ivk_context.vk_device, g_ivk_context.vk_pipeline_layout, NULL );
vkDestroySurfaceKHR( g_ivk_context.vk_instance, g_ivk_context.vk_surface, NULL );
vkDestroyDevice( g_ivk_context.vk_device, NULL );
vkDestroyInstance( g_ivk_context.vk_instance, NULL );

}


/*
 * Creates the Vulkan instance for IVK
 */
static void ivk_create_instance
    (
    unsigned int instance_extension_count,
    const char** instance_extensions
    )
{
/* Local variables */
VkApplicationInfo       app_info = { 0 };
VkInstanceCreateInfo    create_info = { 0 };

/* Application information */
app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
app_info.pApplicationName = "IVK";
app_info.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
app_info.pEngineName = "No Engine";
app_info.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
app_info.apiVersion = VK_API_VERSION_1_0;

/* Instance information */
create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
create_info.pApplicationInfo = &app_info;
create_info.enabledExtensionCount = instance_extension_count;
create_info.ppEnabledExtensionNames = instance_extensions;
create_info.enabledLayerCount = 0;

/* Local variables */
unsigned int        _layer_count = 0;
VkLayerProperties*  _supported_layers = NULL;

/* Check for validation layer support */
if( VALIDATION_ENABLED && !ivk_check_validation_layer_support() )
    {
    printf( "Validation layers requested but not supported.\n" );
    }
else
    {
    create_info.enabledLayerCount = g_validation_layer_cnt;
    create_info.ppEnabledLayerNames = g_validation_layers;
    }

__vk( vkCreateInstance
        (
        &create_info,
        NULL,
        &g_ivk_context.vk_instance
        ) );

}


/*
 * Selects the appropriate physical device
 * for the application. Also picks the
 * queue families.
 */
static void ivk_select_physical_device
    (
    void
    )
{
/* Local variables */
unsigned int        _device_count = 0;
VkPhysicalDevice*   _physical_devices = NULL;

__vk( vkEnumeratePhysicalDevices( g_ivk_context.vk_instance, &_device_count, NULL ) );
if( _device_count == 0 )
    {
    printf( "No valid GPUs found.\n" );
    return;
    }

_physical_devices = ( VkPhysicalDevice* )malloc( _device_count * sizeof( VkPhysicalDevice ) );
__vk( vkEnumeratePhysicalDevices( g_ivk_context.vk_instance, &_device_count, &_physical_devices[ 0 ] ) );

if( !_physical_devices )
    {
    printf( "Querying the physical devices failed.\n" );
    return;
    }

for( unsigned int i = 0; i < _device_count; i++ )
    {
    if( ivk_is_device_suitable( _physical_devices[ i ] ) )
        {
        g_ivk_context.vk_physical_device = _physical_devices[ i ];
        break;
        }
    }

free( _physical_devices );

if( !g_ivk_context.vk_physical_device )
    {
    printf( "Failed to find physical device.\n" );
    return;
    }

return;

}


/*
 * Suitability checks for physical devices
 */
static bool ivk_is_device_suitable
    (
    VkPhysicalDevice    physical_device
    )
{
/* Local variables */
VkPhysicalDeviceProperties  _device_properties = { 0 };
bool                        _is_device_suitable = true;
unsigned int                _extension_count = 0;
VkExtensionProperties*      _available_extensions = NULL;

/* Check for discrete GPU */
//vkGetPhysicalDeviceProperties( physical_device, &_device_properties );
//_is_device_suitable &= _device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

/* Check for swapchain support */
__vk( vkEnumerateDeviceExtensionProperties( physical_device, NULL, &_extension_count, NULL ) );
_available_extensions = ( VkExtensionProperties* )malloc( _extension_count * sizeof( VkExtensionProperties ) );
__vk( vkEnumerateDeviceExtensionProperties( physical_device, NULL, &_extension_count, &_available_extensions[ 0 ] ) );
for( unsigned int i = 0; i < g_device_extensions_count; i++ )
    {
    bool    _is_extension_supported = false;
    for( unsigned j = 0 ; j < _extension_count; j++ )
        {
        if( strcmp( g_device_extensions[ i ], _available_extensions[ j ].extensionName ) == 0 )
            {
            _is_extension_supported = true;
            break;
            }
        }
    if( !_is_extension_supported )
        {
        _is_device_suitable = false;
        printf( "Device extension %s not supported.\n", g_device_extensions[ i ] );
        free( _available_extensions );
        return _is_device_suitable;
        }
    }
/* Check for swapchain adequacy */
ivk_swapchain_query_support
    (
    physical_device,
    g_ivk_context.vk_surface,
    &g_ivk_context.swapchain_details
    );
if( g_ivk_context.swapchain_details.format_count == 0 || g_ivk_context.swapchain_details.present_modes_count == 0 )
    {
    _is_device_suitable = false;
    printf( "Inadequate swapchain.\n" );
    free( _available_extensions );
    return _is_device_suitable;
    }

/* Check for the necessary queue families. */
_is_device_suitable &= ivk_select_queue_families( physical_device );

free( _available_extensions );
return _is_device_suitable;
}


/*
 * Selects the appropriate queue families.
 * The queue families will be different for graphics and presentation.
 */
static bool ivk_select_queue_families
    (
    VkPhysicalDevice    physical_device
    )
{
/* Local constants */
#define INVALID_QUEUE  ( ( unsigned int )( -1 ) )

/* Local variables */
unsigned int                _graphics_family = INVALID_QUEUE;
unsigned int                _present_family  = INVALID_QUEUE;
unsigned int                _transfer_family  = INVALID_QUEUE;
unsigned int                _queue_family_cnt = 0;
VkQueueFamilyProperties*    _queue_families = NULL;

vkGetPhysicalDeviceQueueFamilyProperties( physical_device, &_queue_family_cnt, NULL );
_queue_families = ( VkQueueFamilyProperties* )malloc( _queue_family_cnt * sizeof( VkQueueFamilyProperties ) );

if( !_queue_families )
    {
    printf( "Querying of the queue families failed.\n" );
    return false;
    }

vkGetPhysicalDeviceQueueFamilyProperties( physical_device, &_queue_family_cnt, &_queue_families[ 0 ] );

/* The queue families for graphics and presentation must be different */
for( unsigned int i = 0; i < _queue_family_cnt; i++ )
    {
    VkBool32    _present_supported = VK_FALSE;

    if( _queue_families[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT )
        {
        if( _graphics_family == INVALID_QUEUE )
            {
            _graphics_family = i;
            continue;
            }
        }
    else if( _queue_families[ i ].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
        if( _transfer_family == INVALID_QUEUE )
            {
            _transfer_family = i;
            continue;
            }
        }

    __vk( vkGetPhysicalDeviceSurfaceSupportKHR( physical_device, i, g_ivk_context.vk_surface, &_present_supported ) );
    if( _present_supported && _present_family == INVALID_QUEUE )
        {
        _present_family = i;
        continue;
        }

    }

if( _graphics_family == INVALID_QUEUE || _present_family == INVALID_QUEUE || _transfer_family == INVALID_QUEUE )
    {
    free( _queue_families );
    printf( "No queue families found.\n" );
    return false;
    }

g_ivk_context.vk_graphics_family_idx = _graphics_family;
g_ivk_context.vk_present_family_idx  = _present_family;
g_ivk_context.vk_transfer_family_idx = _transfer_family;

free( _queue_families );
return true;

/* Undefine local constants */
#undef INVALILD_QUEUE

}


/*
 * Creates a logical device.
 */
static void ivk_create_logical_device
    (
    void
    )
{

/* Local variables */
VkDeviceQueueCreateInfo     _queue_create_info_arr[ 3 ] = { 0 };
VkDeviceCreateInfo          _device_create_info = { 0 };
VkPhysicalDeviceFeatures    _device_features = { 0 }; /* Not used */
float                       _queue_priorities = 1.0f;

/* Set up the graphics queue */
_queue_create_info_arr[ 0 ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
_queue_create_info_arr[ 0 ].queueFamilyIndex = g_ivk_context.vk_graphics_family_idx;
_queue_create_info_arr[ 0 ].pQueuePriorities = &_queue_priorities;
_queue_create_info_arr[ 0 ].queueCount = 1;

/* Set up the presentation queue */
_queue_create_info_arr[ 1 ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
_queue_create_info_arr[ 1 ].queueFamilyIndex = g_ivk_context.vk_present_family_idx;
_queue_create_info_arr[ 1 ].pQueuePriorities = &_queue_priorities;
_queue_create_info_arr[ 1 ].queueCount = 1;

/* Set up the transfer queue */
_queue_create_info_arr[ 2 ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
_queue_create_info_arr[ 2 ].queueFamilyIndex = g_ivk_context.vk_transfer_family_idx;
_queue_create_info_arr[ 2 ].pQueuePriorities = &_queue_priorities;
_queue_create_info_arr[ 2 ].queueCount = 1;

_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
_device_create_info.pQueueCreateInfos = &_queue_create_info_arr[ 0 ];
_device_create_info.queueCreateInfoCount = 3;
_device_create_info.pEnabledFeatures = &_device_features;
_device_create_info.enabledExtensionCount = g_device_extensions_count;
_device_create_info.ppEnabledExtensionNames = &g_device_extensions[ 0 ];

#if defined( VALIDATION_ENABLED ) && ( VALIDATION_ENABLED == 1 )
    _device_create_info.enabledLayerCount = g_validation_layer_cnt;
    _device_create_info.ppEnabledLayerNames = &g_validation_layers[ 0 ];
#else
    _device_create_info.enabledLayerCount = 0;
#endif

/* Create the logical device */
__vk( vkCreateDevice( g_ivk_context.vk_physical_device, &_device_create_info, NULL, &g_ivk_context.vk_device ) );

/* Obtain the queue handles */
vkGetDeviceQueue
    (
    g_ivk_context.vk_device,
    g_ivk_context.vk_graphics_family_idx,
    0,
    &g_ivk_context.vk_graphics_queue
    );
vkGetDeviceQueue
    (
    g_ivk_context.vk_device,
    g_ivk_context.vk_present_family_idx,
    0,
    &g_ivk_context.vk_present_queue
    );
vkGetDeviceQueue
    (
    g_ivk_context.vk_device,
    g_ivk_context.vk_transfer_family_idx,
    0,
    &g_ivk_context.vk_transfer_queue
    );

return;

}


/*
 * TEMP. Creates a window surface. This is
 * a bad place to define this, since IVK should
 * be a pure rendering library, without any
 * references to a windowing library.
 */
static void ivk_create_window_surface
    (
    void
    )
{
glfwCreateWindowSurface
    (
    g_ivk_context.vk_instance,
    g_ivk_context.glfw_window,
    NULL,
    &g_ivk_context.vk_surface
    );

return;

}


/*
 * Initializes the presentation related objects.
 * - swapchain
 * - retrieves images
 * - creates image views
 * - creates framebuffers
 */
static void ivk_init_presentation
    (
    void
    )
{
/* Create the swapchain */
ivk_swapchain_create
    (
    g_ivk_context.vk_device,
    &g_ivk_context.swapchain_details,
    g_ivk_context.vk_surface,
    g_ivk_context.vk_graphics_family_idx,
    g_ivk_context.vk_present_family_idx,
    &g_ivk_context.vk_swapchain,
    &g_ivk_context.swapchain_format,
    &g_ivk_context.swapchain_extent
    );

/* Retrieve the swapchain images */
ivk_swapchain_retrieve_images
    (
    g_ivk_context.vk_device,
    g_ivk_context.vk_swapchain,
    &g_ivk_context.swapchain_image_count,
    &g_ivk_context.vk_images
    );

/* Create the image views for the images in the swapchain */
ivk_swapchain_create_image_views
    (
    g_ivk_context.vk_device,
    g_ivk_context.swapchain_image_count,
    g_ivk_context.vk_images,
    g_ivk_context.swapchain_format,
    &g_ivk_context.vk_image_views
    );

}


/*
 * Cleans up the presentation objects.
 * The swapchain images are automatically freed when destroying the
 * swapchain.
 */
static void ivk_clean_presentation
    (
    void
    )
{
/* Destroy the framebuffers */
for( unsigned int i = 0; i < g_ivk_context.swapchain_image_count; i++ )
    {
    vkDestroyFramebuffer( g_ivk_context.vk_device, g_ivk_context.vk_framebuffers[ i ], NULL );
    }
free( g_ivk_context.vk_framebuffers );

ivk_swapchain_destroy_image_views( g_ivk_context.vk_device, g_ivk_context.swapchain_image_count, g_ivk_context.vk_image_views );
vkDestroySwapchainKHR( g_ivk_context.vk_device, g_ivk_context.vk_swapchain, NULL );

}


/*
 * Re-creates the presentation objects when the swapchain
 * is inadequate.
 */
static void ivk_recreate_presentation
    (
    void
    )
{
/* Local variables */
int _width  = 0;
int _height = 0;

/* Wait for the GPU to finish any ongoing operation */
vkDeviceWaitIdle( g_ivk_context.vk_device );

/* Check for swapchain adequacy */
ivk_swapchain_free_support( &g_ivk_context.swapchain_details );
ivk_swapchain_query_support
    (
    g_ivk_context.vk_physical_device,
    g_ivk_context.vk_surface,
    &g_ivk_context.swapchain_details
    );

/* If the max Image extent is 0, the window was minimized and
the application needs to wait */
_width = g_ivk_context.swapchain_details.capabilities.maxImageExtent.width;
_height = g_ivk_context.swapchain_details.capabilities.maxImageExtent.height;
while( _width == 0 ||
       _height == 0 )
    {
    glfwGetFramebufferSize( g_ivk_context.glfw_window, &_width, &_height );
    glfwWaitEvents();
    }

ivk_swapchain_free_support( &g_ivk_context.swapchain_details );
ivk_swapchain_query_support
    (
    g_ivk_context.vk_physical_device,
    g_ivk_context.vk_surface,
    &g_ivk_context.swapchain_details
    );

ivk_clean_presentation();
ivk_init_presentation();

ivk_create_framebuffers();

}


/*
 * Creates a renderpass object.
 */
static void ivk_create_renderpass
    (
    void
    )
{
/* Local variables */
VkAttachmentDescription _color_attachment = { 0 };
VkAttachmentReference   _color_attachment_reference = { 0 };
VkSubpassDescription    _subpass = { 0 };
VkRenderPassCreateInfo  _renderpass_create_info = { 0 };
VkSubpassDependency     _dependency = { 0 };

_color_attachment.format = g_ivk_context.swapchain_format;
_color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
_color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
_color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
_color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
_color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
_color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
_color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

_color_attachment_reference.attachment = 0;
_color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
_subpass.colorAttachmentCount = 1;
_subpass.pColorAttachments = &_color_attachment_reference;

_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
_dependency.dstSubpass = 0;
_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
_dependency.srcAccessMask = 0;
_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

_renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
_renderpass_create_info.attachmentCount = 1;
_renderpass_create_info.pAttachments = &_color_attachment;
_renderpass_create_info.subpassCount = 1;
_renderpass_create_info.dependencyCount = 1;
_renderpass_create_info.pDependencies = &_dependency;
_renderpass_create_info.pSubpasses = &_subpass;

__vk( vkCreateRenderPass
        (
        g_ivk_context.vk_device,
        &_renderpass_create_info,
        NULL,
        &g_ivk_context.vk_renderpass
        ) );

}


/*
 * Creates the framebuffers for the swapchain.
 */
static void ivk_create_framebuffers
    (
    void
    )
{
/* Local variables */
VkFramebufferCreateInfo _framebuffer_create_info = { 0 };

/* Allocate memory for the framebuffers */
g_ivk_context.vk_framebuffers = ( VkFramebuffer* )calloc( g_ivk_context.swapchain_image_count, sizeof( VkFramebuffer ) );

if( !g_ivk_context.vk_framebuffers )
    {
    printf( "Failed to allocate memory for the framebuffers.\n" );
    return;
    }
for( unsigned int i = 0; i < g_ivk_context.swapchain_image_count; i++ )
    {
    VkImageView _attachments[] = { g_ivk_context.vk_image_views[ i ] };

    memset( &_framebuffer_create_info, 0, sizeof( _framebuffer_create_info ) );
    _framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    _framebuffer_create_info.renderPass = g_ivk_context.vk_renderpass;
    _framebuffer_create_info.attachmentCount = 1;
    _framebuffer_create_info.pAttachments = _attachments;
    _framebuffer_create_info.width = g_ivk_context.swapchain_extent.width;
    _framebuffer_create_info.height = g_ivk_context.swapchain_extent.height;
    _framebuffer_create_info.layers = 1;

    /* Create the framebuffer */
    __vk( vkCreateFramebuffer( g_ivk_context.vk_device, &_framebuffer_create_info, NULL, &g_ivk_context.vk_framebuffers[ i ] ) );
    }
}


/*
 * Creates the command pool.
 */
static void ivk_create_command_pools
    (
    void
    )
{
/* Local variables */
VkCommandPoolCreateInfo _command_pool_create_info[ 2 ] = { 0 };

_command_pool_create_info[ 0 ].sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
_command_pool_create_info[ 0 ].flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
_command_pool_create_info[ 0 ].queueFamilyIndex = ( uint32_t )g_ivk_context.vk_graphics_family_idx;

_command_pool_create_info[ 1 ].sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
_command_pool_create_info[ 1 ].flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
_command_pool_create_info[ 1 ].queueFamilyIndex = ( uint32_t )g_ivk_context.vk_transfer_family_idx;

__vk( vkCreateCommandPool
        (
        g_ivk_context.vk_device,
        &_command_pool_create_info[ 0 ],
        NULL,
        &g_ivk_context.vk_graphics_command_pool
        ) );
__vk( vkCreateCommandPool
        (
        g_ivk_context.vk_device,
        &_command_pool_create_info[ 1 ],
        NULL,
        &g_ivk_context.vk_transfer_command_pool
        ) );

}


/*
 * Creates the command buffer.
 */
static void ivk_create_command_buffers
    (
    void
    )
{
/* Local variables */
VkCommandBufferAllocateInfo _command_buffer_alloc_info = { 0 };

_command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
_command_buffer_alloc_info.commandPool = g_ivk_context.vk_graphics_command_pool;
_command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
_command_buffer_alloc_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

__vk( vkAllocateCommandBuffers
        (
        g_ivk_context.vk_device,
        &_command_buffer_alloc_info,
        &g_ivk_context.vk_command_buffer[ 0 ]
        ) );

}


/*
 * Record the commands in the command buffer.
 */
static void ivk_record_command_buffer
    (
    VkCommandBuffer command_buffer,
    unsigned int    image_index
    )
{
/* Local variables */
VkCommandBufferBeginInfo    _command_buffer_begin_info = { 0 };
VkRenderPassBeginInfo       _render_pass_begin_info = { 0 };
VkClearValue                _clear_color = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
VkViewport                  _viewport = { 0 };
VkRect2D                    _scissor = { 0 };
VkBuffer                    _vert_buffers[] = { 0 };
VkDeviceSize                _offsets[] = { 0 };

_command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
_command_buffer_begin_info.flags = 0;
_command_buffer_begin_info.pInheritanceInfo = NULL;

_render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
_render_pass_begin_info.renderPass = g_ivk_context.vk_renderpass;
_render_pass_begin_info.framebuffer = g_ivk_context.vk_framebuffers[ image_index ];
_render_pass_begin_info.renderArea.offset.x = 0;
_render_pass_begin_info.renderArea.offset.y = 0;
_render_pass_begin_info.renderArea.extent = g_ivk_context.swapchain_extent;
_render_pass_begin_info.clearValueCount = 1;
_render_pass_begin_info.pClearValues = &_clear_color;

_viewport.x = 0.0f;
_viewport.y = 0.0f;
_viewport.width = g_ivk_context.swapchain_extent.width;
_viewport.height = g_ivk_context.swapchain_extent.height;
_viewport.minDepth = 0.0f;
_viewport.maxDepth = 1.0f;

_scissor.offset.x = 0.0f;
_scissor.offset.y = 0.0f;
_scissor.extent = g_ivk_context.swapchain_extent;

_vert_buffers[ 0 ] = g_ivk_context.triangle_vert_buffer;

__vk( vkBeginCommandBuffer( command_buffer, &_command_buffer_begin_info ) );
vkCmdBeginRenderPass( command_buffer, &_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );
vkCmdBindPipeline( command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_ivk_context.vk_pipeline );
vkCmdSetViewport( command_buffer, 0, 1, &_viewport );
vkCmdSetScissor( command_buffer, 0, 1, &_scissor );
vkCmdBindVertexBuffers( command_buffer, 0, 1, _vert_buffers, _offsets );
vkCmdBindIndexBuffer( command_buffer, g_ivk_context.triangle_index_buffer, 0, VK_INDEX_TYPE_UINT32 );
vkCmdDrawIndexed( command_buffer, g_ivk_context.index_count, 1, 0, 0, 0 );
vkCmdEndRenderPass( command_buffer );
__vk( vkEndCommandBuffer( command_buffer ) );

}


/*
 * Creates the synchronization primitives.
 */
static void ivk_create_sync_objects
    (
    void
    )
{
/* Local variables */
VkSemaphoreCreateInfo   _semaphore_create_info = { 0 };
VkFenceCreateInfo       _fence_create_info = { 0 };

_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
_fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
_fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

for( unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
    {
    __vk( vkCreateSemaphore( g_ivk_context.vk_device, &_semaphore_create_info, NULL, &g_ivk_context.image_available_semaphore[ i ] ) );
    __vk( vkCreateSemaphore( g_ivk_context.vk_device, &_semaphore_create_info, NULL, &g_ivk_context.render_finished_semaphore[ i ] ) );
    __vk( vkCreateFence( g_ivk_context.vk_device, &_fence_create_info, NULL, &g_ivk_context.in_flight_fence[ i ] ) );
    }
}
