extends Node

@export var sky_mat: ShaderMaterial

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	var taille: Vector2 = get_viewport().get_visible_rect().size
	var ratio: float = taille.x / taille.y
	sky_mat.set_shader_parameter("ratio_ecran", ratio)
