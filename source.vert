#version 300 es
layout(location = 0) in vec4 a_position;
uniform vec2 min;
uniform vec2 del;
uniform mat4 view;

float modelToWorld(float pos, float min, float delta) {
  return (pos - min)/delta - 1.0f;
};

void main() {
  float x = modelToWorld(a_position[0], min[0], del[0]);
  float y = modelToWorld(a_position[1], min[1], del[1]);
  float z = modelToWorld(a_position[0], min[0], del[0]);
  vec3 position = vec3(z, y, 0.0f);
  gl_Position = view * vec4(position, 1.0f);
  gl_PointSize = 1.0f;
};