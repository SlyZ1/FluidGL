extends Node

class_name AnimController

@export var skate_physics: SkatePhysics
@export_group("Tricks")
@export_range(0, 1) var trick_ground_collision: float = 0.75
@export var mesh_instance: Node3D
@export_group("Catch")
@export var catch_fall_speed: float = 20
@export var catch_start_speed: float = -1.5

var _active_tricks: Array[BasicTrick]
var _current_trick_duration: float = 0
var _current_anim_duration: float = 0
var _default_trick_rotation: Quaternion = Quaternion.IDENTITY

var _trick_catched: bool = true
var _catch_pressed: bool = false
var _was_grounded_when_catched: bool = false

var _toggled: bool = true

var _mesh_instance_scale: Vector3



func toggle(value: bool) -> void:
	_toggled = value

func can_catch() -> bool:
	return not skate_physics.is_grounded()

func add_default_trick_rotation(rot: Quaternion) -> void:
	_default_trick_rotation *= rot



func _ready() -> void:
	_mesh_instance_scale = mesh_instance.scale
	TrickManager.trick_performed.connect(_on_trick_performed)
	TrickManager.trick_catched.connect(_on_trick_catched)
	skate_physics.on_fall.connect(TrickManager.on_airborne)
	skate_physics.on_grounded.connect(TrickManager.on_ground)
	GameManager.instance.on_death.connect(_reset)

var _was_grounded: bool
func _process(_delta: float) -> void:
	# if (skate_physics.is_grounded() and not _was_grounded) and not TrickManager.get_catch_pressed():
	# 	GameManager.instance.stop()
	_was_grounded = skate_physics.is_grounded()
	
	var current_rotation: Quaternion = _default_trick_rotation
	for trick in _active_tricks:
		current_rotation *= trick.get_quaternion()
		current_rotation = current_rotation.normalized()
	skate_physics.set_rot(Basis(current_rotation))
	_default_trick_rotation = Quaternion.IDENTITY
	


func _confirm_trick():
	mesh_instance.scale = _mesh_instance_scale * 1.3
	create_tween().tween_property(mesh_instance, "scale", _mesh_instance_scale, 0.15).set_trans(Tween.TRANS_CIRC).set_ease(Tween.EASE_OUT_IN)

func _on_trick_performed(_name: String) -> void:
	_catch_pressed = false
	var _anim: Anim = TrickManager.get_resource(_name)
	var _duration: float = TrickManager.get_trick_duration(_name)
	_current_trick_duration = _duration
	_current_anim_duration = _anim.animation_duration
	
	if _anim.reset_velocities: skate_physics.reset_velocities()
	skate_physics.apply_impulse(_anim.impulse, _anim.angular_impulse)

	if abs(_anim.rot_angle) > 0.001:
		_confirm_trick()
		_trick_catched = false
		var trick: BasicTrick = BasicTrick.new(_name)
		_active_tricks.push_back(trick)
		trick.set_tween(_anim, self)

	
func _on_trick_catched(_dt: float) -> void:
	if not _toggled:
		return
		
	if not _catch_pressed:
		_was_grounded_when_catched = false
	if skate_physics.is_grounded(): _was_grounded_when_catched = true
		
	# Successful Catch
	if not skate_physics.is_grounded():
		_successfully_catched(_dt)
		return
		
	# if already catched and grounded, nothing happens
	if _trick_catched:
		return
		
	GameManager.instance.death((Vector3.DOWN + Vector3.BACK * 2).normalized() * 8)

func _successfully_catched(dt: float) -> void:
	if not _catch_pressed:
		skate_physics.apply_impulse(catch_start_speed)
		AudioManager.play(SoundEffectSettings.SOUND_EFFECT_TYPE.CATCH)
		
	_active_tricks.clear()
	XpManager.validate_buffered_xp()
	if not _was_grounded_when_catched: skate_physics.smooth_reset_orientation(dt)
	skate_physics.apply_impulse(-dt * catch_fall_speed)
	_trick_catched = true
	_catch_pressed = true

func _reset() -> void:
	_active_tricks = []
	_trick_catched = true
	_catch_pressed = false
	_was_grounded_when_catched = false
	toggle(true)
	skate_physics.set_rot(Basis(Quaternion.IDENTITY))
