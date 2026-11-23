@tool
extends UpgradeTrigger
class_name UpgradeTriggerOr

# public
@export var triggerA: UpgradeTrigger
@export var triggerB: UpgradeTrigger
 
func initialize_logic() -> void:
	triggerA.initialize()
	triggerB.initialize()
	triggerA.on_enter.connect(_enter)
	triggerB.on_enter.connect(_enter)
	triggerA.on_exit.connect(_exit)
	triggerB.on_exit.connect(_exit)

func reset_logic() -> void:
	triggerA.reset()
	triggerB.reset()
	triggerA.on_enter.disconnect(_enter)
	triggerB.on_enter.disconnect(_enter)
	triggerA.on_exit.disconnect(_exit)
	triggerB.on_exit.disconnect(_exit)

# protected
func enter_logic() -> bool:
	return true

func exit_logic() -> bool:
	return not triggerA.is_triggered() && not triggerB.is_triggered()