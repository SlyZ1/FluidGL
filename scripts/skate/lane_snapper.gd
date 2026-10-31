extends Node

class_name LaneSnapper

@export var lane_position: float = 5

var _lane_tween: Tween
var _snap: bool = true

func _ready() -> void:
	TrickManager.go_left.connect(_on_go_left)
	TrickManager.go_right.connect(_on_go_right)

func _process(delta: float) -> void:
	# Snap To Lane
	if (abs(get_parent().global_position.z - lane_position) > 0.1 
		and (_lane_tween == null or !_lane_tween.is_valid() or !_lane_tween.is_running())
		and _snap
	):
		if get_parent().global_position.z <= 0: _on_go_left()
		else: _on_go_right()
		
func toggle(snap: bool) -> void:
	_snap = snap
		
func kill_tween() -> void:
	if _lane_tween and _lane_tween.is_valid():
		_lane_tween.kill()

func _on_go_left(force: bool = false) -> void:
	if not force and get_parent().is_grounded(): return
	_lane_tween = create_tween()
	_lane_tween.tween_property(get_parent(), "global_position:z", -lane_position, 0.4).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
	
func _on_go_right(force: bool = false) -> void:
	if not force and get_parent().is_grounded(): return
	_lane_tween = create_tween()
	_lane_tween.tween_property(get_parent(), "global_position:z", lane_position, 0.4).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
