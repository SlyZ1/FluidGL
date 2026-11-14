shader_type spatial;
render_mode unshaded;

uniform float dim = 4.;

float norm(vec2 v)
{
	float n = 8.0;
	return pow(pow(abs(v.x), n) + pow(abs(v.y), n), 1. / n);
}

void fragment() {
	vec2 uv = UV;
	uv.x = fract(dim * uv.x);
	uv.y = fract(dim * uv.y);
	uv = 2.0 * uv - 1.0;
	float d = sign(uv.x) * sign(uv.y);
	ALBEDO = vec3(1.);
	ALPHA = mix(0.67, 0.76, d) * smoothstep(0.0, 0.67, 1. - norm(2. * UV - 1.));
}

//void light() {
//	// Called for every pixel for every light affecting the material.
//	// Uncomment to replace the default light processing function with this one.
//}
