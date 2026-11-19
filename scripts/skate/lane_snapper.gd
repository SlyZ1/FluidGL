extends Node

class_name LaneSnapper

@export var skate_physics: SkatePhysics
@export var lane_position: float = 5

var _lane_tween: Tween
var _snap: bool = true
var _current_lane: float = -1

func _process(_delta: float) -> void:
	if (Input.is_action_just_pressed("switch_lane")):
		_change_lane(-_current_lane)

	# Snap To Lane
	if (abs(skate_physics.global_position.z - lane_position) > 0.1 
		and (_lane_tween == null or !_lane_tween.is_valid() or !_lane_tween.is_running())
		and _snap
	):
		_change_lane(-1 if skate_physics.global_position.z <= 0 else 1)
		
func toggle(snap: bool) -> void:
	_snap = snap
	if not snap: kill_tween()
		
func kill_tween() -> void:
	if _lane_tween and _lane_tween.is_valid():
		_lane_tween.kill()

func _change_lane(lane: float, force: bool = false) -> void:
	if not force and skate_physics.is_grounded(): return
	_current_lane = lane
	_lane_tween = create_tween()
	_lane_tween.tween_property(skate_physics, "global_position:z", lane * lane_position, 0.4) \
			   .set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)