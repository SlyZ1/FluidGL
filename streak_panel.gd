extends Control

func _ready() -> void:
	TrickManager.trick_ended.connect(_on_trick_ended)
	TrickManager.xp_changed.connect(_on_xp_changed)
	TrickManager.tier_changed.connect(_on_tier_changed)

func _process(delta: float) -> void:
	pass

func points_popup(points: int) -> void:
	var label = Label.new()
	label.text = "+" + str(points)
	label.position = Vector2(125, 150)
	add_child(label)
	
	var tween = create_tween().set_parallel(true)
	tween.tween_property(label, "position:y", label.position.y - 50, 0.6)\
		.set_trans(Tween.TRANS_QUAD)\
		.set_ease(Tween.EASE_OUT)
	tween.tween_property(label, "modulate:a", 0.0, 0.6)\
		.set_trans(Tween.TRANS_QUAD)\
		.set_ease(Tween.EASE_IN)

	await tween.finished
	label.queue_free()

func _on_trick_ended():
	pass

func _on_xp_changed(_new_val: float, _increased_by: int):
	points_popup(_increased_by)
	
	create_tween().tween_property($Background/XPSlot/XPBar, "scale:x", _new_val, 0.2).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	var tween = create_tween()
	tween.tween_property($Background, "scale", Vector2(1.1, 1.1), 0.1).set_trans(Tween.TRANS_QUAD)

	await tween.finished
	create_tween().tween_property($Background, "scale", Vector2(1.0, 1.0), 0.1).set_trans(Tween.TRANS_QUAD)


func _on_tier_changed(_new_tier: int):
	$Background/Tier.text = ["D", "C", "B", "A", "S"][_new_tier]
