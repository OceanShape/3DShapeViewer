#version 300 es
layout(location = 0) in vec4 a_position;
uniform vec2 minimum;
uniform vec2 delta;
uniform mat4 view;

float modelToWorld(float pos, float m, float d) {
  return (pos - m) / d - 1.0f;
}

void main() {
  float x = modelToWorld(a_position[0], minimum[0], delta[0]);
  float y = modelToWorld(a_position[1], minimum[1], delta[1]);
  vec3 position = vec3(x, y, 0.0f);
  gl_Position = view * vec4(position, 1.0f);
  gl_PointSize = 1.0f;
}
