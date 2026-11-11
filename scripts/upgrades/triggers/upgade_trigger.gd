@tool
extends Resource
class_name UpgradeTrigger

# public
@export var exit_immediately: bool:
	set(v):
		exit_immediately = v
		notify_property_list_changed() # refresh l'ui pour un inspecteur dynamique

signal on_enter
signal on_exit

func is_triggered() -> bool:
	return _is_triggered

func initialize(_upgrade_manager: UpgradeManager) -> void:
	if _is_initialized: return
	_is_initialized = true
	initialize_logic(_upgrade_manager)

func reset(_upgrade_manager: UpgradeManager) -> void:
	if not _is_initialized: return
	_is_initialized = false
	reset_logic(_upgrade_manager)

# protected
func enter_logic() -> bool:
	return true

func exit_logic() -> bool:
	return true
	
func initialize_logic(_upgrade_manager: UpgradeManager) -> void:
	pass

func reset_logic(_upgrade_manager: UpgradeManager) -> void:
	pass

# private
var _is_triggered: bool = false
var _is_initialized: bool = false

func _enter() -> void:
	if _is_triggered: return
	if not enter_logic(): return
	
	_is_triggered = true
	on_enter.emit()
	if exit_immediately: _exit()

func _exit() -> void:
	if not _is_triggered: return
	if not exit_logic(): return

	_is_triggered = false
	on_exit.emit()