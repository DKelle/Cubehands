R"zzz(
#version 330 core
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform vec4 highlighted_bone;
in vec4 vertex_position;
out vec4 fragment_color;

void main() {
	fragment_color = vec4(0,0,1, 1.0);
}
)zzz"
