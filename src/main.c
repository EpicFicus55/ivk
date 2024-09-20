#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include "glfw/glfw3.h"

#include "vulkan/vulkan.h"

#include "ivk.h"

/* Project constants */
#define WINDOW_WIDTH    600
#define WINDOW_HEIGHT   600
#define WINDOW_NAME     "IVK Window"

/*
 * Global data
 */
ivk_2p3c_type triangle_data[] = 
    {
    { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { {  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { {  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f } }
    };
unsigned int indices[] =
    {
    0, 1, 2,
    2, 3, 0
    };

/*
 * Initialize GLFW
 */
GLFWwindow* init_glfw
    (
    unsigned int window_width,
    unsigned int window_height,
    char*        window_name
    );


/*
 * Main program
 *  - Initializes the windowing library
 *  - Initializes the IVK library
 */
int main
    (
    void
    )
{
/* Local variables */
GLFWwindow*     glfw_window_handle = NULL;
unsigned int    glfw_extension_count = 0;
const char**    glfw_extensions = NULL;

/* Initialize the GLFW windowing library */
glfw_window_handle = init_glfw( WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME );
glfw_extensions = glfwGetRequiredInstanceExtensions( &glfw_extension_count );

/* Initialie IVK library */
ivk_init( glfw_extension_count, glfw_extensions, glfw_window_handle );

/* Initialize a triangle for rendering */
ivk_init_triangle
    ( 
    &triangle_data[ 0 ], 
    4,
    &indices[ 0 ],
    6
    );

/* Main loop */
while( !glfwWindowShouldClose( glfw_window_handle ) )
    {
    ivk_render();
    glfwPollEvents();
    }

/* Teardown */
ivk_teardown();
glfwDestroyWindow( glfw_window_handle );
glfwTerminate();

return 0;

}


/*
 * Initialize GLFW
 */
GLFWwindow* init_glfw
    (
    unsigned int window_width,
    unsigned int window_height,
    char*        window_name
    )
{
GLFWwindow* glfw_window_hndl = NULL;

if( !glfwInit() )
    {
    printf( "GLFW initialization failed!\n" );
    return( NULL );
    }

glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

/* Create the window */
glfw_window_hndl = glfwCreateWindow
                        (
                         window_width,
                         window_height,
                         window_name,
                         NULL,
                         NULL
                         );

if( !glfw_window_hndl )
    {
    printf( "Invalid window!\n" );
    return( NULL );
    }

return( glfw_window_hndl );

}
