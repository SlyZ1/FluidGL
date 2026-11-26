shader_type spatial;
render_mode unshaded;

void fragment() {
	vec2 UVC = UV * 2.0 - 1.0;
	// float wave1 = 0.25 * sin((2.0 + 2.0 * sin(0.3 * TIME)) * atan(UVC.y, UVC.x) + TIME);
	// float wave2 = 0.4 * cos(3.0 * atan(UVC.y, UVC.x) + 0.3 * TIME);
	float circle = 1. - smoothstep(0.6, 0.8, length(UVC));
	vec2 uv = 2.0 * fract(6.0 * UV + 0.2 * TIME) - 1.0;
	float checkerboard = step(0.0, uv.x * uv.y);
	const vec3 clrA = vec3(0.), clrB = vec3(1.);
	vec3 clr = mix(clrA, clrB, checkerboard);
	ALBEDO = clr;
	ALPHA = 0.75 * circle;
}
