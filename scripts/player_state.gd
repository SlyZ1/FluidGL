extends Node

@export var _hp : int
@export var _points : int
@export var _pending_points : int
@export var _luck : float

@export var _gravity : float
@export var _speed : float
@export var _bullet_time_scale : float
@export var _jump_height : float

var inventory : Array[Upgrade]


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