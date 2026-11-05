class_name Upgrade
extends Resource

@export var upgrade_name : String
@export var description : String
@export_range(0, 1) var rarity : float


# ???
# @export var trigger : UpgradeTrigger
# @export var effect : UpgradeEffect

# func initialize() -> void:
#     trigger.connect(effect.fire)
