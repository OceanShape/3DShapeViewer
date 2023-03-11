#version 300 es
layout(location = 0) in vec4 a_position;
uniform vec2 minimum;
uniform vec2 delta;
uniform mat4 view;

float modelToWorld(int idx) {
  return (a_position[idx] - minimum[idx]) / delta[idx] - 1.0f;
}

void main() {
  float x = modelToWorld(0);
  float y = modelToWorld(1);
  vec3 position = vec3(x, y, 0.0f);
  gl_Position = view * vec4(position, 1.0f);
  gl_PointSize = 1.0f;
}
