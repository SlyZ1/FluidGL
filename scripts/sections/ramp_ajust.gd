extends Node3D

@export var offset: float = 0.1

func _ready() -> void:
	if global_rotation_degrees.y > 0:
		position.y += offset
	else:
		position.y -= offset
