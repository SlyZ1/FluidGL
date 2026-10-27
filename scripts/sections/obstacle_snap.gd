@tool
extends Node3D

@export var snap_x: bool = true
@export var snap_y: bool = true
@export var snap_z: bool = true
@export var snap_settings: SnapSettings

func _notification(what: int) -> void:
	if not Engine.is_editor_hint():
		return
	match what:
		NOTIFICATION_APPLICATION_FOCUS_IN:
			set_notify_local_transform(true)
		NOTIFICATION_ENTER_TREE:
			set_notify_local_transform(true)
		NOTIFICATION_LOCAL_TRANSFORM_CHANGED:
			set_notify_local_transform(false)
			_on_moved()
			set_notify_local_transform(true)

func _on_moved() -> void:
	if snap_settings == null: return
	if snap_z:
		position.z -= snap_settings.lane_size * (snap_settings.num_lanes % 2 + 1)
		var lane: int = round(position.z / (snap_settings.lane_size * 2))
		position.z = lane * (snap_settings.lane_size * 2)
		position.z += snap_settings.lane_size * (snap_settings.num_lanes % 2 + 1)
		
	if snap_y:
		position.y = round(position.y / snap_settings.size_y) * snap_settings.size_y
		
	if snap_x:
		position.x -= snap_settings.size_x * 0.5
		position.x = round(position.x / snap_settings.size_x) * snap_settings.size_x
		position.x += snap_settings.size_x * 0.5
