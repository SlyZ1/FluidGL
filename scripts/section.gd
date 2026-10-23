extends Node3D

class_name Section

@export var snap_back: Node3D
@export var snap_front: Node3D
@export var rails: Array[Path3D]

func snap(snap_to: Vector3, parent: Node3D) -> void:
	position = parent.to_local(snap_to) - basis * snap_back.position
