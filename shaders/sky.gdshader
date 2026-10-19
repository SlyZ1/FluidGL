shader_type sky;

uniform vec4 backgroundColor : source_color = vec4(0.45, 0.65, 0.95, 1.0);
uniform vec4 imageColor : source_color = vec4(0, 0.65, 0.95, 1.0);
uniform vec4 image2Color : source_color = vec4(0.2, 0.65, 0.95, 1.0);
uniform sampler2D image : source_color, filter_linear_mipmap;
uniform sampler2D image2 : source_color, filter_linear_mipmap;
uniform float scale = 1.0;                 // 1 = l'image fait la hauteur de l'écran
uniform vec2 padding = vec2(0.2, 0.2);     // espace entre les images, en fraction de la taille de l'image
uniform vec2 vitesse = vec2(0.05, 0.0);    // défilement, en écrans par seconde
uniform float opacite : hint_range(0.0, 1.0) = 1.0;
uniform float ratio_ecran = 1.7778;        // largeur / hauteur de l'écran (réglé par script)
uniform float decalage_colonne = 0.5;   // décalage vertical entre deux colonnes, en fraction de la hauteur d'une case

void sky() {
	vec2 uv = SCREEN_UV;
	uv.x *= ratio_ecran;

	vec2 taille_tex = vec2(textureSize(image, 0));
	uv.x *= taille_tex.y / taille_tex.x;   // respecte les proportions de l'image

	uv = uv / max(scale, 0.001) + vitesse * TIME;

	// Chaque case = l'image + le padding autour
	vec2 cellule = vec2(1.0) + padding;
	// Chaque colonne est décalée verticalement par rapport à la précédente
	float colonne = floor(uv.x / cellule.x);
	uv.y += decalage_colonne * cellule.y * colonne;

	vec2 local = mod(uv, cellule) - padding * 0.5;   // 0..1 = dans l'image, sinon dans le padding

	// Dérivées calculées sur les UV continues, pour éviter une ligne parasite aux bords des cases
	vec4 tex1 = textureGrad(image, local, dFdx(uv), dFdy(uv)) * imageColor;
	vec4 tex2 = textureGrad(image2, local, dFdx(uv), dFdy(uv)) * image2Color;
	
	vec4 tex = tex1.a > 0.5 ? tex1 : tex2;

	float dedans = step(0.0, local.x) * step(local.x, 1.0) * step(0.0, local.y) * step(local.y, 1.0);
	COLOR = tex.a * opacite * dedans > 0.5 ? tex.rgb : backgroundColor.rgb;
}