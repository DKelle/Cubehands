R"zzz(
#version 330 core
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform vec4 highlighted_bone;
in vec4 vertex_position;
void main() {
	//gl_Position =  projection * view * model * vertex_position;
//	if(highlighted_bone == vertex_position)
//	{
//		gl_Position = vertex_position;
//	}
//	else
//	{
		gl_Position = projection * view * model * vertex_position;
//	}
	//gl_Position =  vertex_position;
}
)zzz"
