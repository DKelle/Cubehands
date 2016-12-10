R"zzz(
#version 330 core
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

in vec4 vertex_position;

void main() {
	//float theta = 2 * 3.14159265359;
	//float radius = 0.25;
	//vec4 new_pos = vertex_position;
	//new_pos.x = radius * cos(vertex_position[0] * theta);
	//new_pos.y = radius * sin(vertex_position[0] * theta);
	//new_pos.z = cylinder_length * vertex_position[2];
	//gl_Position = projection * view * model * cylinder_transform * new_pos;
	gl_Position = projection * view * model * vertex_position;

}
)zzz"
