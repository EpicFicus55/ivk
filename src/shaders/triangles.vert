#version 450

layout(location = 0) in  vec2 vertPos;
layout(location = 1) in  vec3 vertColor;

layout(location = 0) out vec3 fragColor;

layout( binding = 0 ) uniform mvk_mvp_type
    {
    mat4 model;
    mat4 view;
    mat4 proj;
    } ubo;

void main() 
{
gl_Position = ubo.proj * ubo.view * ubo.model * vec4( vertPos, 0.0f, 1.0f );
fragColor = vertColor;
}
