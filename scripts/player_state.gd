extends Node

@export var _max_hp : int = 6:
	set(val):
		_max_hp = val
		on_max_hp_changed.emit(val)
@export var _hp : int = 6:
	set(val):
		_hp = val
		on_hp_changed.emit(val)
@export var _points : int
@export var _pending_points : int
@export var _luck : float

@export var _gravity : float
@export var _speed : float
@export var _bullet_time_scale : float
@export var _jump_height : float

signal on_hp_changed(hp: int)
signal on_max_hp_changed(max_hp: int)

var inventory : Array[Upgrade]

func get_max_hp() -> int:
	return _max_hp

func get_hp() -> int:
	return _hp

func get_points() -> int:
	return _points

func get_pending_points() -> int:
	return _pending_points

func get_luck() -> float:
	return _luck

func get_gravity() -> float:
	return _gravity

func get_speed() -> float:
	return _speed

func get_bullet_time_scale() -> float:
	return _bullet_time_scale

func get_jump_height() -> float:
	return _jump_height

func reset() -> void:
	pass