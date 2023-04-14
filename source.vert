#version 300 es
layout(location = 0) in vec4 a_position;
uniform vec3 minimum;
uniform vec3 delta;
uniform mat4 view;
uniform float aspect_ratio;

float modelToWorld(int idx) {
  return (a_position[idx] - minimum[idx]) / delta[idx] - 1.0f;
}

void main() {
  float x = modelToWorld(0);
  float y = modelToWorld(1);
  float z = (-1e-6 < delta[2] && delta[2] < 1e-6 ) ? 0.0f : modelToWorld(2) + 2.0f; 
  vec3 position = vec3(x, y / aspect_ratio, z);
  gl_Position = view * vec4(position * 0.99f, 1.0f);
  gl_PointSize = 1.0f;
}
