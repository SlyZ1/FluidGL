extends Node

@export_group("Physics Time")
@export var substep: float = 1.0 / 300.0
@export var max_steps_par_frame: int = 30

var _physics_step_accumulateur: float = 0


@export_group("Bullet Time")
@export_range(0, 10) var bullet_time_strength: float = 0.2
@export_range(0, 1) var bullet_min_timescale: float = 0.2 
@export var bullet_time_rewind_ratio: float = 4

var _bullet_time: float = 0
var _bullet_time_enabled: bool = false
var _bullet_timescale: float = 1


var _time_scale: float = 1

signal on_substep(dt: float)


func set_time_scale(time: float) -> void:
	_time_scale = time



func _ready() -> void:
	set_time_scale(1)
	TrickManager.trick_performed.connect(_on_trick_performed)
	TrickManager.on_reset.connect(_on_reset)
	TrickManager.on_catch_pressed.connect(_on_reset)

func _process(dt: float) -> void:
	_handle_bullet_time(dt)
	_run_physics_substeps(dt)


func _run_physics_substeps(dt: float) -> void:
	_physics_step_accumulateur += dt
	var steps: int = 0
	while _physics_step_accumulateur >= substep and steps < max_steps_par_frame:
		on_substep.emit(substep * _time_scale * _bullet_timescale)
		_physics_step_accumulateur -= substep
		steps += 1



func _handle_bullet_time(dt: float) -> void:
	if not _bullet_time_enabled: return
	_bullet_timescale = exp(-_bullet_time * bullet_time_strength)
	_bullet_timescale = max(_bullet_timescale, bullet_min_timescale)
	_bullet_time += dt

func _on_trick_performed(trick_name: String) -> void:
	if trick_name == TrickManager.jump_trick_name: return
	_bullet_time_enabled = true
	_bullet_time /= bullet_time_rewind_ratio

func _on_reset() -> void:
	_bullet_timescale = 1
	_bullet_time_enabled = false
	_bullet_time = 0