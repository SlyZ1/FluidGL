@tool
extends ObstacleSnap
class_name Rail

@export var _mesh: MeshInstance3D

@export var path3D: Path3D
var bounds: AABB

@export_tool_button("Print Ideal OffsetY") var printoffset = _print_offset_y

func _print_offset_y() -> void:
    print(_mesh.global_position.y - bounds.position.y)

func _ready() -> void:
    bounds = _mesh.global_transform * _mesh.get_aabb()