extends Node

@export var all_upgrades : Array[Upgrade]

signal on_process(dt: float)

func add_upgrade(upgrade: Upgrade) -> void:
    upgrade.initialize()
    PlayerState.inventory.push_back(upgrade)
    
# remove by name
func remove_upgrade(upgrade_name: String) -> void:
    for i in range(PlayerState.inventory.size()):
        if PlayerState.inventory[i].upgrade_name == upgrade_name:
            PlayerState.inventory[i].reset()
            PlayerState.inventory.remove_at(i)
            return

func _exit_tree() -> void:
    for upgrade in PlayerState.inventory:
        upgrade.reset() 

func _ready() -> void:
    for upgrade in PlayerState.inventory:
        upgrade.initialize()

func _process(delta: float) -> void:
    on_process.emit(delta)

func generate_random_upgrades() -> Array[Upgrade]:
    return [all_upgrades.pick_random(), all_upgrades.pick_random(), all_upgrades.pick_random()]