shader_type sky;

uniform samplerCube skybox : source_color, filter_linear_mipmap;

void sky() {
	COLOR = texture(skybox, EYEDIR).rgb;
}