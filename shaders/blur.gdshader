shader_type canvas_item;

// Allows you to adjust the blur intensity directly from the Inspector
uniform float blur_amount : hint_range(0.0, 5.0) = 1.0;
uniform sampler2D screen_texture : hint_screen_texture, filter_linear_mipmap;

void fragment() {
    // Samples the screen texture with a mipmap offset to create a fast blur effect
    vec4 color = textureLod(screen_texture, SCREEN_UV, blur_amount);
    COLOR = color;
}