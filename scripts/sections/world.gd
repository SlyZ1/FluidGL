@tool
class_name World
extends Resource

@export var sections: Array[PackedScene]
@export var empty_section: PackedScene
@export_range(0,1) var empty_ratio: float = 0.3