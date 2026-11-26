shader_type canvas_item;

uniform float seed;

mat2 rot2D(float theta)
{
	return mat2(vec2(cos(theta), sin(theta)), vec2(-sin(theta), cos(theta)));
}

void fragment() {
	vec3 clr = vec3(1.);
	float alpha = 1.0;
	
	vec2 uv = 2.0 * UV - 1.0;
	vec2 repeated = 2. * fract(5.0 * UV + 0.1 * vec2(TIME)) - 1.;
	repeated.x *= fwidth(UV).y / fwidth(UV).x; // aspect ratio
	
	float circle = 1. - step(0.3, length(repeated)) * step(length(repeated), 0.4);
	vec3 clrBack = vec3(0.5, 0.8, 0.45);
	vec3 clrFront = 0.88 * clrBack;
	
	const float n = 8.0;
	float Ln = pow(pow(abs(uv.x), n) + pow(abs(uv.y), n), 1. / n);
	float theta = atan(uv.y, uv.x);
	
	float smallWave = sin(4. * theta + 8. * seed + 0.15 * TIME);
	float bigWave = sin(6. * theta + 1. * TIME + 5. * seed);
	alpha = 1. - step(0.9 + (0.02 + 0.01 * smallWave) * bigWave, Ln);
	
	clr = mix(clrFront, clrBack, circle);
	
	
	COLOR = vec4(clr, alpha);
}
