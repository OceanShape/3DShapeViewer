#version 300 es
layout(location = 0) in vec4 a_position;
uniform vec3 minimum;
uniform vec3 delta;
uniform float aspect_ratio;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

float modelToWorld(int idx) {
  return (a_position[idx] - minimum[idx]) / delta[idx] - 1.0f;
}

void main() {
  vec3 position = vec3(modelToWorld(0), modelToWorld(1) / aspect_ratio, 0.0f);
  gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0f);
  gl_PointSize = 1.0f;
}
