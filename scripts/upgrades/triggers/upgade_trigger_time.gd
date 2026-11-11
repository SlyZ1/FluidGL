@tool
extends UpgradeTrigger
class_name UpgradeTriggerTime

# public
@export var frequency: float
@export var duration: float:
	set(v):
		duration = clamp(v, 0, frequency)

func _validate_property(property: Dictionary) -> void:
	if property.name == "duration" and exit_immediately:
		property.usage &= ~PROPERTY_USAGE_EDITOR

func initialize_logic(_upgrade_manager: UpgradeManager) -> void:
	_upgrade_manager.on_process.connect(_custom_enter) 

func reset_logic(_upgrade_manager: UpgradeManager):
	_global_time_accumulated = 0.0
	_trigger_time_accumulated = 0.0
	_upgrade_manager.on_process.disconnect(_custom_enter)

# private
var _global_time_accumulated: float = 0
var _trigger_time_accumulated: float = 0

func _custom_enter(dt: float) -> void:
	_global_time_accumulated += dt
	while _global_time_accumulated > frequency:
		_global_time_accumulated -= frequency
		_enter()

	_trigger_time_accumulated += dt
	if not exit_immediately && _is_triggered && _trigger_time_accumulated > duration:
		_exit()

func exit_logic() -> bool:
	_trigger_time_accumulated = 0
	return true

func enter_logic() -> bool:
	_trigger_time_accumulated = 0
	return true