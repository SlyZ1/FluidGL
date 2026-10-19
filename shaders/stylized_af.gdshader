shader_type spatial;


uniform sampler2D noise;
uniform sampler2D palette : source_color;
//uniform float ref = 0.5;
uniform float sig = 0.25;

//void fragment() {
//}

void light() {
	float nz = texture(noise, UV).r;
	//float nz11 = 0.5 * nz - 1.0;
	//vec2 tp = texture(noise, UV).rg;
	//vec3 nn = vec3(sin(tp.x) * cos(tp.y), cos(tp.x), sin(tp.x) * sin(tp.y));
	
	float l = max(0.001, dot(NORMAL, LIGHT));
	float sl = pow(max(0.001, dot(NORMAL, normalize(LIGHT + VIEW))), 8.0);
	
	l = 0.75 * clamp(l + sig * nz, 0.001, 0.99);
	sl = 0.5 * clamp(sl + 1.3 * sig * nz, 0.001, 0.999);
	
	DIFFUSE_LIGHT += texture(palette, vec2(clamp(l + sl, 0.01, 0.99), 0.)).rgb;
	//SPECULAR_LIGHT += texture(palette, vec2(sl, 0.)).rgb;
	//SPECULAR_LIGHT += texture(palette, vec2(sl, 0.)).rgb;
}