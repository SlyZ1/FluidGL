@tool
class_name Upgrade
extends Resource

@export_group("Properties")
@export var upgrade_name : String
@export_multiline() var description : String
@export_range(0, 1) var rarity : float

@export_group("Members")
@export var trigger : UpgradeTrigger
@export var effects : Array[UpgradeEffect]

func initialize(upgrade_manager: UpgradeManager) -> void:
    trigger.initialize(upgrade_manager)
    for effect in effects:
        effect.initialize(upgrade_manager)
        trigger.on_enter.connect(effect.apply)

func reset(upgrade_manager: UpgradeManager) -> void:
    trigger.reset(upgrade_manager)
    for effect in effects:
        effect.reset(upgrade_manager)
        trigger.on_enter.disconnect(effect.apply)