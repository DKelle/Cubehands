R"zzz(#version 330 core
uniform vec4 colorC;

out vec4 fragment_color;
void main()
{
	fragment_color = colorC; //vec4(1.0, 1.0, 0.0, 1.0);
}
)zzz"