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

func initialize() -> void:
    trigger.initialize()
    for effect in effects:
        effect.initialize() 
        trigger.on_enter.connect(effect.apply)

func reset() -> void:
    trigger.reset()
    for effect in effects:
        effect.reset()
        trigger.on_enter.disconnect(effect.apply)