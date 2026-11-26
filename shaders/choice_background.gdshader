shader_type canvas_item;

global uniform int NB_COLORS_IN_PALETTE; 
global uniform sampler2D palette : source_color;

uniform float tt;

mat2 rot2D(float theta)
{
	return mat2(vec2(cos(theta), sin(theta)), vec2(-sin(theta), cos(theta)));
}

void fragment() {
	vec2 uv = 2.0 * UV - 1.0;
	float t = 0.1 * TIME;
	uv *= rot2D(t);
	float get_trigged = sin(1. * cos(sin((4.5 + cos(7. * length(uv))) * atan(4. + sin(t) + uv.y, cos(t) + uv.x))));
	float td = tt * 0.88 + 0.12;
	get_trigged = pow(get_trigged, 3. * tanh(td * td * 7.));
	
	vec3 clr = 0.15 + 0.6 * texture(palette, vec2(get_trigged, 0.)).rgb;
	
	COLOR = vec4(clr, 1. - step(0.7, get_trigged));
}
