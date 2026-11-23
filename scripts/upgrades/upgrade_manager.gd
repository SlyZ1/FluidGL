extends Node

@export var current_upgrades: Array[Upgrade]

signal on_process(dt: float)

func add_upgrade(upgrade: Upgrade) -> void:
    upgrade.initialize()
    current_upgrades.push_back(upgrade)
    
# remove by name
func remove_upgrade(upgrade_name: String) -> void:
    for i in range(current_upgrades.size()):
        if current_upgrades[i].upgrade_name == upgrade_name:
            current_upgrades[i].reset()
            current_upgrades.remove_at(i)
            return

func _exit_tree() -> void:
    for upgrade in current_upgrades:
        upgrade.reset() 


func add_hp(hp : int) -> void:
    set_hp(PlayerState._hp + hp)

func set_hp(hp : int) -> void:
    PlayerState._hp = clamp(hp, 0, 60000)

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

func _ready() -> void:
    for upgrade in current_upgrades:
        upgrade.initialize()

func _process(delta: float) -> void:
    on_process.emit(delta)