extends Node
class_name GameManager

@export var restart_lag_timer: Timer
@export_group("Skate")
@export var skate: SkatePhysics
@export var skate_ragdoll: RigidBody3D

signal on_stop
signal on_start
signal on_pause(paused: bool)

static var auto_start: bool = false

var game_started: bool = false
var game_paused: bool = false

# run state
var _ready_to_restart: bool = false
var _waiting_for_restart: bool = false

static var instance: GameManager
	
func set_active(node: Node3D, active: bool) -> void:
	node.visible = active
	node.process_mode = Node.PROCESS_MODE_INHERIT if active else Node.PROCESS_MODE_DISABLED

func _ready() -> void:
	on_stop.connect(AudioManager.on_global_stop)
	on_stop.connect(TrickManager.reset)

	set_active(skate, false)
	set_active(skate_ragdoll, false)
	if auto_start: start()

func _enter_tree() -> void:
	if instance and instance != self:
		push_warning("Plusieurs instances de GameManager dans la scène")
	instance = self

func _exit_tree() -> void:
	if instance == self:
		instance = null
		
func stop(impulse_direction: Vector3 = Vector3.ZERO, no_ragdoll: bool = false) -> void:
	if not game_started: return

	AudioManager.play(SoundEffectSettings.SOUND_EFFECT_TYPE.DEATH)
	on_stop.emit()
	game_started = false
	if not no_ragdoll:
		set_active(skate, false)

		skate_ragdoll.global_transform = skate.global_transform
		set_active(skate_ragdoll, true)
		var impulse: Vector3 = impulse_direction if impulse_direction != Vector3.ZERO else (Vector3.UP + Vector3.FORWARD).normalized() * 12
		skate_ragdoll.apply_central_impulse(impulse)
		var axe := Vector3(randf_range(-1, 1), randf_range(-1, 1), randf_range(-1, 1)).normalized()
		skate_ragdoll.apply_torque_impulse(axe * 10)

		restart_lag_timer.start()
	else:
		skate.process_mode = Node.PROCESS_MODE_DISABLED

func start() -> void:
	if game_started: return

	on_start.emit()
	game_started = true
	set_active(skate, true)
	set_active(skate_ragdoll, false)

func _handle_pause() -> void:
	if Input.is_action_just_pressed("pause"):
		game_paused = !game_paused
		on_pause.emit(game_paused)
		Engine.time_scale = 1 if game_paused else 0

func _restart_logic() -> void:
	if _ready_to_restart and TrickManager.get_catch_pressed():
		_waiting_for_restart = false
		_ready_to_restart = false
		
		TrickManager._points = 0
		TrickManager._xp = 0
		TrickManager._xp_buffer = 0
		TrickManager._tier = 0
		get_tree().reload_current_scene() # A CHANGER

	if _waiting_for_restart and not TrickManager.get_catch_pressed():
		_ready_to_restart = true

func _on_restart_lag_timeout() -> void:
	_waiting_for_restart = true

func _process(dt: float) -> void:
	_handle_pause()
	_restart_logic()
