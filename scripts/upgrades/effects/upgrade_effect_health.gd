extends UpgradeEffect
class_name UpgradeEffectHealth

@export var hp_added: int

# protected
func initialize_logic(upgrade_manager: UpgradeManager) -> void:
    _on_apply.connect(upgrade_manager.add_hp.bind(hp_added))

func reset_logic(upgrade_manager: UpgradeManager) -> void:
    _on_apply.disconnect(upgrade_manager.add_hp.bind(hp_added))
