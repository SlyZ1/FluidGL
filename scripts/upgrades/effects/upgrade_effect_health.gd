extends UpgradeEffect
class_name UpgradeEffectHealth

@export var hp_added: int

# protected
func initialize_logic() -> void:
	_on_apply.connect(UpgradeManager.add_hp.bind(hp_added))
	
func reset_logic() -> void:
	_on_apply.disconnect(UpgradeManager.add_hp.bind(hp_added))
