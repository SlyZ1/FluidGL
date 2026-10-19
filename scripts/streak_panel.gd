extends Control

@export var tier_fill: TierFill
@export var points: Label
@export var popup_template: Label
@export var popup_color: Color

@export_group("Wow")
@export var wow_curve: Curve
@export var wow_strengh: float
@export var wow_duration: float
@export var wow_texts: Array[String]
@export var wow_colors: Array[Color]
@export var wow_template: Label

var _wow_size: Vector2
var _wow_scale: Vector2

var _tier_scale: Vector2
var _points_scale: Vector2

func _ready() -> void:
	TrickManager.trick_ended.connect(_on_trick_ended)
	TrickManager.xp_changed.connect(_on_xp_changed)
	TrickManager.tier_changed.connect(_on_tier_changed)
	#wow_template.visible = false
	_wow_size = wow_template.size
	_wow_scale = wow_template.scale
	_tier_scale = tier_fill.scale
	_points_scale = points.scale

func _process(delta: float) -> void:
	pass

func points_popup(points: int) -> void:
	var label = popup_template.duplicate()
	label.text = "+" + str(points)
	label.add_theme_color_override("font_color", popup_color)
	add_child(label)
	
	var tween = create_tween().set_parallel(true)
	tween.tween_property(label, "position:y", label.position.y - 50, 0.6)\
		.set_trans(Tween.TRANS_QUAD)\
		.set_ease(Tween.EASE_OUT)
	tween.tween_property(label, "modulate:a", 0.2, 0.6)\
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
			
var _wow_tween: Tween
var _wow_index: int = -1
func spawn_wow(index: int) -> void:
	index = clampi(index, 0, wow_texts.size() - 1)
	if index == _wow_index and _wow_tween:
		return
	_wow_index = index

	var label: Label = wow_template
	label.text = wow_texts[index]
	if not wow_texts[index] in ["", "..."]:
		var asp : AudioStreamPlayer = get_node("../../SFX/" + wow_texts[index]) as AudioStreamPlayer
		asp.play(0.05)
	label.add_theme_color_override("font_color", wow_colors[index])

	if (_wow_tween and _wow_tween.is_valid()):
		_wow_tween.kill()
	_wow_tween = label.create_tween()  # le tween meurt avec le label
	_wow_tween.tween_method(
		func(t: float) -> void:
			var k: float = wow_curve.sample(t) if wow_curve else 1.0
			label.scale = _wow_scale * (1.0 + k * wow_strengh),
			#label.modulate.a = 1.0 - smoothstep(0.75, 1.0, t),
		0.0, 1.0, wow_duration
	)

var previous_tier: int = -1
func _on_xp_changed(_new_val: float, _tier: int, _nb_tricks_since_grounded: int, _increased_by: int, _total_points: int, _with_popup: bool):
	_new_val = clamp(_new_val, 0, 1)
	points.text = "%.d" % _total_points
	
	spawn_wow(_nb_tricks_since_grounded)
	
	if previous_tier != _tier:
		tier_fill.set_remplissage(0 if previous_tier < _tier else 1)
		previous_tier = _tier
	if _with_popup:
		points_popup(_increased_by)
		
		_maj_barre(_new_val)
		var tween = create_tween()
		tween.tween_property($Tier, "scale", _tier_scale * 1.2, 0.05).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
		create_tween().tween_property(points, "scale", _points_scale * 1.2, 0.05).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	
		await tween.finished
		create_tween().tween_property($Tier, "scale", _tier_scale, 0.1).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
		create_tween().tween_property(points, "scale", _points_scale, 0.1).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	else: tier_fill.set_remplissage(_new_val)


func _on_tier_changed(_new_tier: int):
	$Tier.text = ["D", "C", "B", "A", "S"][_new_tier]
