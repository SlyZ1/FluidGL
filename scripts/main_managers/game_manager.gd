extends Node
class_name GameManager

@export var map_loader: MapLoader
@export var camera: CustomCamera
@export_group("Restart")
@export var restart_transition_duration: float = 2
@export_group("Skate")
@export var skate: SkatePhysics
@export var skate_ragdoll: RigidBody3D

signal on_stop
signal on_death
signal on_spawn
signal on_respawn
signal on_ready_to_respawn
signal on_pause(paused: bool)

static var auto_start: bool = false

var game_started: bool = false
var game_paused: bool = false

# run state
var _ready_to_respawn: bool = false

static var instance: GameManager
	
func set_active(node: Node3D, active: bool) -> void:
	node.process_mode = Node.PROCESS_MODE_INHERIT if active else Node.PROCESS_MODE_DISABLED
	node.visible = active

func _ready() -> void:
	Engine.max_fps = 120
	on_death.connect(AudioManager.on_global_stop)
	on_death.connect(TrickManager.reset)
	on_death.connect(XpManager.reset)
	on_stop.connect(XpManager.on_stop)

	set_active(skate, false)
	set_active(skate_ragdoll, false)
	if auto_start: spawn()

func _enter_tree() -> void:
	if instance and instance != self:
		push_warning("Plusieurs instances de GameManager dans la scène")
	instance = self

func _exit_tree() -> void:
	if instance == self:
		instance = null
		
func death(impulse_direction: Vector3 = Vector3.ZERO) -> void:
	if not game_started: return

	PlayerState.add_hp(-1)
	AudioManager.play(SoundEffectSettings.SOUND_EFFECT_TYPE.DEATH)
	game_started = false

	skate_ragdoll.global_transform = skate.global_transform
	skate.reset()
	set_active(skate, false)
	set_active(skate_ragdoll, true)
	camera.skate = skate_ragdoll
	
	skate_ragdoll.linear_velocity = Vector3.ZERO
	skate_ragdoll.angular_velocity = Vector3.ZERO
	var impulse: Vector3 = impulse_direction if impulse_direction != Vector3.ZERO else (Vector3.UP + Vector3.FORWARD).normalized() * 12
	skate_ragdoll.apply_central_impulse(impulse)
	var axe := Vector3(randf_range(-1, 1), randf_range(-1, 1), randf_range(-1, 1)).normalized()
	skate_ragdoll.apply_torque_impulse(axe * 10)
	
	on_death.emit()
	await map_loader.stop()
	await get_tree().create_timer(0.5).timeout

	if PlayerState.get_hp() <= 0:
		PlayerState.reset()
		on_stop.emit()
		get_tree().reload_current_scene()
		return

	skate.reset()
	skate_ragdoll.process_mode = Node.PROCESS_MODE_DISABLED
	create_tween().tween_property(skate_ragdoll, "global_position", skate.global_position, restart_transition_duration).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_IN_OUT)
	create_tween().tween_property(skate_ragdoll, "global_rotation", skate.global_rotation, restart_transition_duration).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_IN_OUT)
	await map_loader.restart_skip(restart_transition_duration)
	_ready_to_respawn = true

	camera.skate = skate
	set_active(skate, true)
	set_active(skate_ragdoll, false)

	on_ready_to_respawn.emit()

func spawn() -> void:
	if game_started: return

	game_started = true
	set_active(skate, true)
	set_active(skate_ragdoll, false)
	on_spawn.emit()

func _handle_pause() -> void:
	if Input.is_action_just_pressed("pause"):
		game_paused = !game_paused
		on_pause.emit(game_paused)
		Engine.time_scale = 0 if game_paused else 1

func _restart_logic() -> void:
	if _ready_to_respawn and Input.is_action_pressed("F") and Input.is_action_pressed("H"):
		_ready_to_respawn = false
		on_respawn.emit()
		spawn()

func _process(_dt: float) -> void:
	_handle_pause()
	_restart_logic()
	if Input.is_action_just_pressed("fullscreen"): toggle_fullscreen()

var _fullscreen: bool = false
func toggle_fullscreen():
	_fullscreen = not _fullscreen
	if _fullscreen:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN)
	else:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_WINDOWED)
