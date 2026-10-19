shader_type canvas_item;

uniform float remplissage : hint_range(0.0, 1.0) = 0.5;
uniform vec4 couleur_pleine : source_color = vec4(1.0, 0.8, 0.1, 1.0);
uniform vec4 couleur_vide : source_color = vec4(1.0, 1.0, 1.0, 0.35);
uniform vec2 taille = vec2(200.0, 50.0);
uniform bool vertical = false;
uniform float douceur = 0.01;

uniform float sin_spatial_freq = 20.;
uniform float sin_temp_freq = 2;
uniform float sin_ampl = 0.01;

varying vec2 pos_locale;

void vertex() {
	pos_locale = VERTEX;
}

void fragment() {
	vec2 p = pos_locale / taille;
	float t = vertical ? 1.0 - p.y : p.x;
	
	float custom_remplissage = remplissage;
	custom_remplissage += sin(p.x * sin_spatial_freq + TIME * sin_temp_freq) * sin_ampl;
	custom_remplissage += sin(p.x * sin_spatial_freq * 3. - TIME * sin_temp_freq / 2.) * sin_ampl / 4.;
	float plein = 1.0 - smoothstep(custom_remplissage - douceur, custom_remplissage + douceur, t);
	vec4 c = mix(couleur_vide, couleur_pleine, plein);
	COLOR = vec4(c.rgb, c.a * COLOR.a);
}