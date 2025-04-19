#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
// Instanced rendering uses mat4 for model matrix
layout (location = 2) in mat4 instanceModel; // VEC4 VEC4 VEC4 VEC4

// OR standard model matrix if not instancing
uniform mat4 model; // Use this for non-instanced geometry (lines, tubes)

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;  // Vertex position in world space (or view space if preferred)
out vec3 Normal;   // Normal in world space (or view space)

// Flag to determine if we are using instancing
// You might set this based on draw style, or just always pass instanceModel
// and use a separate 'model' uniform for non-instanced. Let's use separate.
uniform bool useInstancing;

void main()
{
    mat4 currentModel = useInstancing ? instanceModel : model;

    // Calculate world position (adjust if view space is preferred)
    FragPos = vec3(currentModel * vec4(aPos, 1.0));

    // Calculate world normal (using inverse transpose is safer for non-uniform scaling)
    // Normal = mat3(transpose(inverse(currentModel))) * aNormal; // More robust
    Normal = mat3(currentModel) * aNormal; // Simpler if scaling is uniform or non-existent

    gl_Position = projection * view * vec4(FragPos, 1.0);
}