#version 450 core
layout (location = 0) in vec3 aPos;

out vec3 FragPos; 
out vec3 Normal; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); 
    Normal = mat3(transpose(inverse(model))) * aPos; 

    gl_Position = projection * view * model * vec4(aPos / 10.0, 1.0);
}