#version 300 es
layout(location = 0) in vec4 a_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 color;

out vec4 vertexColor;

void main() {
  gl_Position = projection * view * model * vec4(a_position.x, a_position.y, a_position.z, 1.0f);
  gl_PointSize = 1.0f;
  vertexColor = color;
}
