@tool
extends ObstacleSnap
class_name Rail

@export var _mesh: MeshInstance3D

@export var path3D: Path3D
var bounds: AABB

func _ready() -> void:
    bounds = _mesh.global_transform * _mesh.get_aabb()