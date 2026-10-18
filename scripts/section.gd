extends MeshInstance3D

class_name Section

@export var snap_back: Node3D
@export var snap_front: Node3D
@export var rails: Array[Path3D]

func snap(snap_to: Vector3) -> void:
	var offset: Vector3 = global_position - snap_back.global_position
	global_position = snap_to + offset
