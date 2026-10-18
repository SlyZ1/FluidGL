extends Node

class BasicTrick:
	var trick_name: String
	var q: Quaternion
	
	func _init(_trick_name: String) -> void:
		trick_name = _trick_name
		q = Quaternion.IDENTITY

var active_tricks: Array[BasicTrick]
var current_trick_duration: float = 0
var current_anim_duration: float = 0
var rail := false

@onready var rail_pose := Quaternion.IDENTITY

var parent: Node3D

func _ready() -> void:
	TrickManager.trick_performed.connect(_on_trick)
	TrickManager.trick_ended.connect(_on_trick_end)
	parent = get_parent()
	parent.on_fall.connect(TrickManager.on_airborne)
	parent.on_grounded.connect(TrickManager.on_ground)
	parent.on_rail.connect(_on_rail)
	parent.off_rail.connect(_off_rail)

func _process(delta: float) -> void:
	var current_rotation: Quaternion = rail_pose
	for trick in active_tricks:
		current_rotation *= trick.q
		current_rotation = current_rotation.normalized()

	if ((current_trick_duration - TrickManager.get_time_left() < 0.5 * current_anim_duration) 
		and parent.is_mesh_colliding() 
		and parent.velocity_back + parent.velocity_front < 0): 
		MapLoader.instance.stop()
	parent.set_rot(Basis(current_rotation))

func tween_axis_angle(object: Object, property: String, axis: Vector3, target_angle: float, duration: float, curve: Curve = null) -> Tween:
	var normalized_axis = axis.normalized()
	var tween = create_tween()
	
	tween.tween_method(
		func(progress: float):
			var curve_val = curve.sample(progress) if curve else progress
			var current_angle = target_angle * curve_val
			var q = Quaternion(normalized_axis, current_angle)
			object.set(property, q), 
			0.0, 1.0, duration)
	return tween

func _on_trick(_name: String, _accuracy: float) -> void:
	var _anim := TrickManager.get_resource(_name)
	var _duration := TrickManager.get_trick_duration(_name)
	current_trick_duration = _duration
	current_anim_duration = _anim.animation_duration
	
	if _anim.reset_velocities: parent.reset_velocities()
	get_parent().apply_impulse(_anim.impulse_back, _anim.impulse_front)

	var already_active := active_tricks.find_custom(func(trick): return trick.trick_name == _name)
	if abs(_anim.rot_angle) > 0.001:
		var trick := BasicTrick.new(_name)

		active_tricks.push_back(trick)
		tween_axis_angle(trick, "q", _anim.rot_axis, deg_to_rad(_anim.rot_angle), _anim.animation_duration, _anim.curve)

func _on_trick_end() -> void:
	active_tricks.clear()

func _on_rail() -> void:
	var target_quat := Quaternion.from_euler(Vector3(0, PI / 2.0, 0)) * Quaternion.from_euler(Vector3(-PI / 4.0, 0, 0))
	tween_axis_angle(self, "rail_pose", target_quat.get_axis(), target_quat.get_angle(), 0.16).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	rail = true

func _off_rail() -> void:
	create_tween().tween_property(self, "rail_pose", Quaternion.IDENTITY, 0.16).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	rail = false
