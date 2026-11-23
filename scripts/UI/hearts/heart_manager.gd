extends Control
class_name HeartManager

@export var hearts: Array[HeartUI]
@export var spacing: float

func _ready() -> void:
	PlayerState.on_hp_changed.connect(_on_hp_changed)
	PlayerState.on_max_hp_changed.connect(_on_max_hp_changed)
	PlayerState.set_max_hp(8)

func _on_max_hp_changed(max_hp: int) -> void:
	_reset_hearts()
	for i in range(1, max_hp >> 1):
		var heart: HeartUI = hearts[0].duplicate()
		add_child(heart)
		heart.position.y += spacing * i
		hearts.push_back(heart)
	_on_hp_changed(PlayerState.get_hp())

func _on_hp_changed(hp: int) -> void:
	hp = clamp(hp, 0, PlayerState.get_max_hp())
	var i: int = 0
	for heart in hearts:
		heart.set_ui(hp - i)
		i += 2

func _reset_hearts() -> void:
	for i in range(1, hearts.size()):
		hearts[i].queue_free()
	hearts = [hearts[0]]
