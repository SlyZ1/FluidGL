extends Node

@export_range(0, 1) var trick_cancel_end: float = 0.1
@export_range(0, 1) var trick_catchable_start: float = 0.7
@export_range(0, 1) var trick_ground_collision: float = 0.75
@export var catch_fall_speed: float = 20
@export var catch_start_speed: float = -1.5
@export var mesh_instance: MeshInstance3D

class BasicTrick:
	var trick_name: String
	var q: Quaternion
	var tween: Tween

	func _init(_trick_name: String) -> void:
		trick_name = _trick_name
		q = Quaternion.IDENTITY

var active_tricks: Array[BasicTrick]
var current_trick_duration: float = 0
var current_anim_duration: float = 0

var trick_catched: bool = true
var catch_pressed: bool = false
var rail := false

@onready var rail_pose := Quaternion.IDENTITY

var parent: Node3D

func _ready() -> void:
	TrickManager.trick_performed.connect(_on_trick)
	TrickManager.trick_ended.connect(_on_trick_end)
	TrickManager.trick_catched.connect(_on_trick_catched)
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
		
	if ((parent.is_grounded() or parent.is_mesh_colliding()) and not trick_catched):
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
	
func _confirm_trick():
	mesh_instance.scale = Vector3.ONE * 1.3
	create_tween().tween_property(mesh_instance, "scale", Vector3.ONE, 0.15).set_trans(Tween.TRANS_CIRC).set_ease(Tween.EASE_OUT_IN)

func _on_trick(_name: String, _accuracy: float) -> void:
	catch_pressed = false
	var _anim := TrickManager.get_resource(_name)
	var _duration := TrickManager.get_trick_duration(_name)
	current_trick_duration = _duration
	current_anim_duration = _anim.animation_duration
	
	if _anim.reset_velocities: parent.reset_velocities()
	get_parent().apply_impulse(_anim.impulse_back, _anim.impulse_front)
	
	_confirm_trick()

	var already_active := active_tricks.find_custom(func(trick): return trick.trick_name == _name)
	if abs(_anim.rot_angle) > 0.001:
		trick_catched = false
		var trick := BasicTrick.new(_name)

		active_tricks.push_back(trick)
		trick.tween = tween_axis_angle(trick, "q", _anim.rot_axis, deg_to_rad(_anim.rot_angle), _anim.animation_duration, _anim.curve)

func _on_trick_end() -> void:
	active_tricks.clear()

func _on_rail() -> void:
	var target_quat := Quaternion.from_euler(Vector3(0, PI / 2.0, 0)) * Quaternion.from_euler(Vector3(-PI / 4.0, 0, 0))
	tween_axis_angle(self, "rail_pose", target_quat.get_axis(), target_quat.get_angle(), 0.16).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	rail = true

func _off_rail() -> void:
	create_tween().tween_property(self, "rail_pose", Quaternion.IDENTITY, 0.16).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	rail = false
	
func _cancel_tricks() -> void:
	if active_tricks.is_empty():
		return

	var tween := create_tween().set_parallel(true)
	for trick in active_tricks:
		if trick.tween and trick.tween.is_valid():
			trick.tween.kill()

		var depart: Quaternion = trick.q
		tween.tween_method(
			func(t: float) -> void: trick.q = depart.slerp(Quaternion.IDENTITY, t),
			0.0, 1.0, 0.05
		)

	var a_retirer: Array[BasicTrick] = active_tricks.duplicate()
	tween.chain().tween_callback(func() -> void:
		for trick in a_retirer:
			active_tricks.erase(trick)
	)
	
func _catch_trick(_dt: float) -> void:
	if not catch_pressed:
		parent.velocity_back = min(parent.velocity_back, catch_start_speed)
		parent.velocity_front = min(parent.velocity_front, catch_start_speed)
		_confirm_trick()
		
	parent.apply_impulse(-_dt * catch_fall_speed, -_dt * catch_fall_speed)
	trick_catched = true
	catch_pressed = true
	
func _on_trick_catched(_progression: float, _dt: float) -> void:
	# On Cancel
	if _progression < trick_cancel_end:
		_cancel_tricks()
		return
		
	# Successful Catch
	if _progression > trick_catchable_start and not parent.is_grounded():
		print("Catched!")
		_cancel_tricks()
		_catch_trick(_dt)
		TrickManager.on_successful_catch()
		return
		
	# if already catched and grounded, nothing happens
	if trick_catched: return
		
	print(_progression)
	MapLoader.instance.stop((Vector3.DOWN + Vector3.BACK * 2).normalized() * 8)
