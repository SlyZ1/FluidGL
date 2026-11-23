extends Resource
class_name UpgradeEffect

# public
func apply() -> void:
    _on_apply.emit()

func initialize() -> void:
    if _is_initialized: return
    _is_initialized = true
    initialize_logic()

func reset() -> void:
    if not _is_initialized: return
    _is_initialized = false
    reset_logic()

func initialize_logic() -> void:
    pass
    
func reset_logic() -> void:
    pass

# private
signal _on_apply

var _is_initialized: bool = false