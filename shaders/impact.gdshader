shader_type spatial;
uniform sampler2D clr : source_color;
uniform float imp;
uniform sampler2D noise : source_color;
render_mode unshaded;

void vertex() {
	// Called for every vertex the material is visible on.
}

void fragment() {
	vec2 uv3 = (2.0 * UV - 1.0) * 1.0;
	float theta = atan(uv3.y, uv3.x);
	float nz = 2.0 * texture(noise, vec2(atan(UV.y, UV.x) + TIME, 0.)).r - 1.;
	float imp2 = imp + 0.2;
	float sz = 0.025 * nz + mix(0.05, 0.0, max(0., imp2 + 0.2));
	float ext = 0.3;
	float d = step(length(uv3), 0.1 + ext * imp2 + sz) * (1. - step(length(uv3), 0.1 + ext * imp2 - sz));
	ALBEDO = texture(clr, vec2(atan(uv3.y, uv3.x) / 6.28, 0.)).rgb;
	ALPHA = d;
}

//void light() {
//	// Called for every pixel for every light affecting the material.
//	// Uncomment to replace the default light processing function with this one.
//}
