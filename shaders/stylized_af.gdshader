shader_type spatial;

global uniform int NB_COLORS_IN_PALETTE;
global uniform sampler2D palette : source_color;
global uniform sampler2D light_levels : source_color;
global uniform sampler2D light_noise;

uniform int color_id;
//uniform float ref = 0.5;
uniform float sig = 0.25;

//void fragment() {
//}

void light() {
	float nz = texture(light_noise, UV).r;
	//float nz11 = 0.5 * nz - 1.0;
	//vec2 tp = texture(noise, UV).rg;
	//vec3 nn = vec3(sin(tp.x) * cos(tp.y), cos(tp.x), sin(tp.x) * sin(tp.y));
	
	float l = max(0.001, dot(NORMAL, LIGHT));
	float sl = pow(max(0.001, dot(NORMAL, normalize(LIGHT + VIEW))), 8.0);
	
	l = 0.75 * clamp(l + sig * nz, 0.001, 0.99);
	sl = 0.5 * clamp(sl + 1.3 * sig * nz, 0.001, 0.999);
	
	float interval = 1.0 / float(NB_COLORS_IN_PALETTE);
	vec3 clr = texture(palette, vec2(interval * (0.5 + float(color_id)), 0.)).rgb;
	
	float level = clamp(l + sl, 0.01, 0.99);
	const float start = 0.2;
	const float end = 1.0;
	const float subdiv = 5.0;
	float lilevel = start + float(int(level * subdiv)) * (end - start) / subdiv;
	DIFFUSE_LIGHT += lilevel * clr;
}