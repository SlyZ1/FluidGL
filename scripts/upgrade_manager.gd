extends Node

# func add_upgrade(tres_path : String) -> void:
#     var upgrade : Upgrade = load(tres_path)
#     upgrade.initialize()
#     PlayerState.inventory.push_back(upgrade)

func set_hp(hp : int) -> void:
    PlayerState._hp = hp

func set_points(points : int) -> void:
    PlayerState._points = points

func set_pending_points(pending_points : int) -> void:
    PlayerState._pending_points = pending_points

func set_luck(luck : float) -> void:
    PlayerState._luck = luck

func set_gravity(gravity : float) -> void:
    PlayerState._gravity = gravity

func set_speed(speed : float) -> void:
    PlayerState._speed = speed

func set_bullet_time_scale(bullet_time_scale : float) -> void:
    PlayerState._bullet_time_scale = bullet_time_scale

func set_jump_height(jump_height : float) -> void:
    PlayerState._jump_height = jump_height