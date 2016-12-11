R"zzz(
#version 330 core
in vec4 face_normal;
in vec4 vertex_normal;
in vec4 light_direction;
in vec4 world_position;
out vec4 fragment_color;

uniform vec4 diffuse;
uniform vec4 ambient;
uniform vec4 specular;
uniform float shininess;

void main() {
	vec4 pos = world_position;
	float check_width = 5.0;
	//float i = floor(pos.x / check_width);
	//float j  = floor(pos.z / check_width);
	float i = mod(pos.x, check_width);
	float j = mod(pos.z, check_width);
	//vec3 color = mod(i + j, 2) * vec3(1.0, 1.0, 1.0);
	vec3 color = vec3(0.2,0.2,0.2);
	vec3 altColor = vec3(0,0,0);

	if(0 <= i && i <= 0.2) {
		color = altColor;
	}
	if(0 <= j && j <= 0.2)
		color = altColor;
		
	float dot_nl = dot(normalize(light_direction), normalize(face_normal));
	dot_nl = clamp(dot_nl, 0.0, 1.0);
	color = clamp(dot_nl * color, 0.0, 1.0);
	fragment_color = vec4(0.1, 0.1, 0.1, 0.1) + shininess*diffuse + ambient + specular;//vec4(color, 1.0);
}
)zzz"
