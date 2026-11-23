extends Node

@export var _default_config: PlayerStateConfig

@export_group("Points")
@export var _points : int
@export var _pending_points : int

signal on_hp_changed(hp: int)
signal on_max_hp_changed(max_hp: int)

var inventory: Array[Upgrade]
var _config: PlayerStateConfig

func _ready() -> void:
	reset()

func reset() -> void:
	_points = 0  
	_pending_points = 0
	_config = _default_config.duplicate()


# ---------------- GET

func get_max_hp() -> int:
	return _config.max_hp

func get_hp() -> int:
	return _config.hp

func get_points() -> int:
	return _points

func get_pending_points() -> int:
	return _pending_points

func get_luck() -> float:
	return _config.luck

func get_gravity() -> float:
	return _config.gravity

func get_speed() -> float:
	return _config.speed

func get_bullet_time_scale() -> float:
	return _config.bullet_time_scale

func get_jump_height() -> float:
	return _config.jump_height


# --------------- SET

func add_hp(hp : int) -> void:
	set_hp(_config.hp + hp)

func set_hp(hp : int) -> void:
	_config.hp = clamp(hp, 0, get_max_hp())

func set_max_hp(max_hp : int) -> void:
	_config.max_hp = clamp(max_hp, 0, 60000) 

func set_points(points : int) -> void:
	_points = points

func set_pending_points(pending_points : int) -> void:
	_pending_points = pending_points

func set_luck(luck : float) -> void:
	_config.luck = luck

func set_gravity(gravity : float) -> void:
	_config.gravity = gravity

func set_speed(speed : float) -> void:
	_config.speed = speed

func set_bullet_time_scale(bullet_time_scale : float) -> void:
	_config.bullet_time_scale = bullet_time_scale

func set_jump_height(jump_height : float) -> void:
	_config.jump_height = jump_height
