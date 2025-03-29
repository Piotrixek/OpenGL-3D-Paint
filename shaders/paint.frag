#version 330 core
out vec4 FragColor;
uniform vec4 strokeColor;
void main() {
    FragColor = strokeColor;
}
