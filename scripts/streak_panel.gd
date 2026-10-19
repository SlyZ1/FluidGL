extends Control

@export var tier_fill: TierFill
@export var popup_template: Label

func _ready() -> void:
	TrickManager.trick_ended.connect(_on_trick_ended)
	TrickManager.xp_changed.connect(_on_xp_changed)
	TrickManager.tier_changed.connect(_on_tier_changed)

func _process(delta: float) -> void:
	pass

func points_popup(points: int) -> void:
	var label = popup_template.duplicate()
	label.text = "+" + str(points)
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
	
var _xp_tween: Tween
func _maj_barre(_new_val: float) -> void:
	#if _xp_tween and _xp_tween.is_valid():
		#_xp_tween.kill()
	#_xp_tween = create_tween()
	create_tween().tween_method(tier_fill.set_remplissage, tier_fill.remplissage, _new_val, 0.2) \
			.set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)

var previous_tier: int = -1
func _on_xp_changed(_new_val: float, _tier: int, _increased_by: int, _total_points: int, _with_popup: bool):
	_new_val = clamp(_new_val, 0, 1)
	$Label.text = "%.d" % _total_points
	if previous_tier != _tier:
		tier_fill.set_remplissage(0 if previous_tier < _tier else 1)
		previous_tier = _tier
	if _with_popup:
		points_popup(_increased_by)
		
		_maj_barre(_new_val)
		var tween = create_tween()
		tween.tween_property($Tier, "scale", Vector2.ONE * 1.2, 0.05).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	
		await tween.finished
		create_tween().tween_property($Tier, "scale", Vector2.ONE, 0.1).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	else: tier_fill.set_remplissage(_new_val)


func _on_tier_changed(_new_tier: int):
	$Tier.text = ["D", "C", "B", "A", "S"][_new_tier]
