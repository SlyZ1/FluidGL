extends Resource
class_name PlayerStateConfig

@export_group("Health")
@export var max_hp : int = 6:
	set(val):
		max_hp = val
		PlayerState.on_max_hp_changed.emit(val)
@export var hp : int = 6:
	set(val):
		hp = val
		PlayerState.on_hp_changed.emit(val)

@export_group("Luck")
@export var luck : float

@export_group("Skate")
@export var gravity : float
@export var speed : float
@export var jump_height : float
@export var bullet_time_scale : float