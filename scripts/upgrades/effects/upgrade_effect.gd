extends Resource
class_name UpgradeEffect

# public
func apply() -> void:
    _on_apply.emit()

func initialize(upgrade_manager: UpgradeManager) -> void:
    if _is_initialized: return
    _is_initialized = true
    initialize_logic(upgrade_manager)

func reset(upgrade_manager: UpgradeManager) -> void:
    if not _is_initialized: return
    _is_initialized = false
    reset_logic(upgrade_manager)

func initialize_logic(_upgrade_manager: UpgradeManager) -> void:
    pass
    
func reset_logic(_upgrade_manager: UpgradeManager) -> void:
    pass

# private
signal _on_apply

var _is_initialized: bool = false