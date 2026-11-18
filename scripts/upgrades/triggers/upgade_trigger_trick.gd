@tool
extends UpgradeTrigger
class_name UpgradeTriggerTrick

# public
@export var trick: String

func initialize_logic(_upgrade_manager: UpgradeManager) -> void:
	TrickManager.trick_performed.connect(_trick_performed)

func reset_logic(_upgrade_manager: UpgradeManager) -> void:
	TrickManager.trick_performed.disconnect(_trick_performed)

func _trick_performed(name: String):
	if name == trick:
		_enter() 

# protected
func enter_logic() -> bool:
	return true

func exit_logic() -> bool:
	return true

# private
func _init() -> void:
	exit_immediately = true

func _validate_property(property: Dictionary) -> void:
	if property.name == "exit_immediately":
		property.usage |= PROPERTY_USAGE_READ_ONLY # force exit_immediately = true