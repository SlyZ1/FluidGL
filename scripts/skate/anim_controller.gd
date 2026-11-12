extends Node

class_name AnimController

@export var skate_physics: SkatePhysics
@export_group("Tricks")
@export_range(0, 1) var trick_cancel_end: float = 0.1
@export_range(0, 1) var trick_catchable_start: float = 0.7
@export_range(0, 1) var trick_ground_collision: float = 0.75
@export var mesh_instance: Node3D
@export_group("Catch")
@export var catch_fall_speed: float = 20
@export var catch_start_speed: float = -1.5

class BasicTrick:
	var trick_name: String
	var q: Quaternion
	var tween: Tween

	func _init(_trick_name: String) -> void:
		trick_name = _trick_name
		q = Quaternion.IDENTITY

var _active_tricks: Array[BasicTrick]
var _current_trick_duration: float = 0
var _current_anim_duration: float = 0
var _default_trick_rotation: Quaternion = Quaternion.IDENTITY

var _trick_catched: bool = true
var _catch_pressed: bool = false
var _was_grounded_when_catched: bool = false

var _toggled: bool = true

var _mesh_instance_scale: Vector3

func _ready() -> void:
	_mesh_instance_scale = mesh_instance.scale
	TrickManager.trick_performed.connect(_on_trick_performed)
	TrickManager.trick_ended.connect(_on_trick_end)
	TrickManager.trick_catched.connect(_on_trick_catched)
	skate_physics.on_fall.connect(TrickManager.on_airborne)
	skate_physics.on_grounded.connect(TrickManager.on_ground)


func _process(_delta: float) -> void:
	if ((skate_physics.is_grounded() or skate_physics.is_mesh_colliding()) and not _trick_catched):
		GameManager.instance.stop()
	
	var current_rotation: Quaternion = _default_trick_rotation
	for trick in _active_tricks:
		current_rotation *= trick.q
		current_rotation = current_rotation.normalized()
	skate_physics.set_rot(Basis(current_rotation))

func toggle(value: bool) -> void:
	_toggled = value
	
func set_default_trick_rotation(rot: Quaternion) -> void:
	_default_trick_rotation = rot

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
	mesh_instance.scale = _mesh_instance_scale * 1.3
	create_tween().tween_property(mesh_instance, "scale", _mesh_instance_scale, 0.15).set_trans(Tween.TRANS_CIRC).set_ease(Tween.EASE_OUT_IN)

func _on_trick_performed(_name: String, _accuracy: float) -> void:
	_catch_pressed = false
	var _anim: Anim = TrickManager.get_resource(_name)
	var _duration: float = TrickManager.get_trick_duration(_name)
	_current_trick_duration = _duration
	_current_anim_duration = _anim.animation_duration
	
	if _anim.reset_velocities: skate_physics.reset_velocities()
	skate_physics.apply_impulse(_anim.impulse, _anim.angular_impulse)

	_confirm_trick()

	if abs(_anim.rot_angle) > 0.001:
		_trick_catched = false
		var trick := BasicTrick.new(_name)
		_active_tricks.push_back(trick)
		trick.tween = tween_axis_angle(trick, "q", _anim.rot_axis, deg_to_rad(_anim.rot_angle), _anim.animation_duration, _anim.curve)

func _on_trick_end() -> void:
	_active_tricks.clear()
	
func _cancel_tricks() -> void:
	if _active_tricks.is_empty():
		return

	var tween := create_tween().set_parallel(true)
	for trick in _active_tricks:
		if trick.tween and trick.tween.is_valid():
			trick.tween.kill()

		var depart: Quaternion = trick.q
		tween.tween_method(
			func(t: float) -> void: trick.q = depart.slerp(Quaternion.IDENTITY, t),
			0.0, 1.0, 0.05
		)

	var a_retirer: Array[BasicTrick] = _active_tricks.duplicate()
	tween.chain().tween_callback(func() -> void:
		for trick in a_retirer:
			_active_tricks.erase(trick)
	)
	
func _successfully_catched(dt: float) -> void:
	if not _catch_pressed:
		skate_physics.apply_impulse(catch_start_speed)
		#_confirm_trick()
		AudioManager.play(SoundEffectSettings.SOUND_EFFECT_TYPE.CATCH)
		if skate_physics.just_impulsed(): 
			TrickManager.validate_xp()
		
	if not _was_grounded_when_catched: skate_physics.reset_orientation(dt)
	skate_physics.apply_impulse(-dt * catch_fall_speed)
	_trick_catched = true
	_catch_pressed = true

func can_catch() -> bool:
	var prog := TrickManager.get_progression()
	return prog >= trick_cancel_end and prog > trick_catchable_start and not skate_physics.is_grounded()
	
func _on_trick_catched(_progression: float, _dt: float) -> void:
	if not _toggled:
		return

	# On Cancel
	if _progression < trick_cancel_end:
		_cancel_tricks()
		return
		
	#if (_progression > trick_catchable_start and skate_physics.is_grounded() and skate_physics.just_impulsed()):
		#TrickManager.validate_xp()

	if not _catch_pressed:
		_was_grounded_when_catched = false
	if skate_physics.is_grounded(): _was_grounded_when_catched = true
		
	# Successful Catch
	if _progression > trick_catchable_start and not skate_physics.is_grounded():
		_cancel_tricks()
		_successfully_catched(_dt)
		return
		
	# if already catched and grounded, nothing happens
	if _trick_catched: return
		
	GameManager.instance.stop((Vector3.DOWN + Vector3.BACK * 2).normalized() * 8)
