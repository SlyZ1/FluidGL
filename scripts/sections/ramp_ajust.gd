extends Node3D

@export var offset: float = 0.1
@export var scale_offset: float = 0.1

func _ready() -> void:
	if global_rotation_degrees.y > 0:
		position.y += offset
		scale.y += scale_offset
	else:
		position.y -= offset
		scale.y -= scale_offset
