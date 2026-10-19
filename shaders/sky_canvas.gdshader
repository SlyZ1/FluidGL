shader_type canvas_item;

uniform vec4 backgroundColor : source_color = vec4(0.45, 0.65, 0.95, 1.0);
uniform vec4 imageColor : source_color = vec4(0.0, 0.65, 0.95, 1.0);
uniform vec4 image2Color : source_color = vec4(0.2, 0.65, 0.95, 1.0);
uniform sampler2D image : source_color, filter_linear_mipmap;
uniform sampler2D image2 : source_color, filter_linear_mipmap;
uniform float scale = 1.0;
uniform vec2 padding = vec2(0.2, 0.2);
uniform vec2 vitesse = vec2(0.05, 0.0);
uniform float opacite : hint_range(0.0, 1.0) = 1.0;
uniform float decalage_colonne = 0.5;

void fragment() {
	vec2 uv = SCREEN_UV;
	uv.x *= SCREEN_PIXEL_SIZE.y / SCREEN_PIXEL_SIZE.x;   // ratio d'écran automatique

	vec2 taille_tex = vec2(textureSize(image, 0));
	uv.x *= taille_tex.y / taille_tex.x;

	uv = uv / max(scale, 0.001) + vitesse * TIME;

	vec2 cellule = vec2(1.0) + padding;
	float colonne = floor(uv.x / cellule.x);
	uv.y += decalage_colonne * cellule.y * colonne;

	vec2 local = mod(uv, cellule) - padding * 0.5;

	vec4 tex1 = textureGrad(image, local, dFdx(uv), dFdy(uv)) * imageColor;
	vec4 tex2 = textureGrad(image2, local, dFdx(uv), dFdy(uv)) * image2Color;
	vec4 tex = tex1.a > 0.5 ? tex1 : tex2;

	float dedans = step(0.0, local.x) * step(local.x, 1.0) * step(0.0, local.y) * step(local.y, 1.0);
	COLOR = vec4(tex.a * opacite * dedans > 0.5 ? tex.rgb : backgroundColor.rgb, 1.0);
}