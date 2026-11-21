extends Node

class_name LaneSnapper

@export var anim_controller: AnimController
@export var skate_physics: SkatePhysics

@export_group("Lane Transition")
@export var lane_settings: LaneSettings
@export var lane_position: float = 5
@export var lane_transition_duration: float = 0.4

@export_group("Coyotee Switch")
@export var coyotee_time: float = 0.1

@export_group("Drift")
@export_range(0, 180) var drift_angle: float = 50
@export var slerp_strength: float = 20


var _lane_tween: Tween
var _snap: bool = true
var _current_lane: float = -1

var _coyotee_timer: float = 0
var _wants_switch: bool = false

var _previous_z: float = 0
var _drift_rotation: Quaternion

func toggle(snap: bool) -> void:
	_snap = snap
	if not snap: kill_tween()
		
func kill_tween() -> void:
	if _lane_tween: _lane_tween.kill()

func _ready() -> void:
	GameManager.instance.on_death.connect(_reset)
	_previous_z = skate_physics.global_position.z

func _process(dt: float) -> void:
	_handle_coyotee_time(dt)
	if (_wants_switch): _change_lane(-_current_lane)

	_snap_closest_lane()
	_handle_drift(dt)
	_previous_z = skate_physics.global_position.z

func _snap_closest_lane() -> void:
	if _lane_tween and _lane_tween.is_running(): return

	var offset: float = skate_physics.global_position.z - lane_settings.center
	if abs(abs(offset) - lane_settings.size) > 0.1: 
		_change_lane(-1 if offset <= 0 else 1)

func _change_lane(lane: float) -> void:
	if not _snap: 
		_reset_coyotee()
		return
	if skate_physics.is_grounded(): return

	_reset_coyotee()
	_current_lane = lane
	_lane_tween = create_tween()
	_lane_tween.tween_property(
		skate_physics, 
		"global_position:z", 
		lane_settings.center + lane * lane_settings.size, 
		lane_transition_duration
	).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)

func _handle_coyotee_time(dt: float) -> void:
	if Input.is_action_just_pressed("switch_lane"): _coyotee_timer = coyotee_time
	_wants_switch = _coyotee_timer > 0
	_coyotee_timer -= dt

func _reset_coyotee() -> void:
	_wants_switch = false
	_coyotee_timer = 0

func _handle_drift(dt: float) -> void:
	var euler: Vector3 = Vector3.ZERO
	if _lane_tween and _lane_tween.is_running() and skate_physics.is_grounded():
		var z_speed_ref: float = 4 * lane_settings.size / lane_transition_duration
		var z_speed: float = (skate_physics.global_position.z - _previous_z) / dt
		var weight: float = clamp(z_speed / z_speed_ref, -1, 1)
		weight *= weight * sign(weight)
		euler = Vector3(0, drift_angle * weight, 0) * PI / 180

	_drift_rotation = Quaternion.from_euler(euler).slerp(_drift_rotation, exp(-slerp_strength * dt))
	anim_controller.add_default_trick_rotation(_drift_rotation)

func _reset() -> void:
	kill_tween()
	toggle(true)
	_reset_coyotee()
	_previous_z = skate_physics.global_position.z
	_drift_rotation = Quaternion.IDENTITY
