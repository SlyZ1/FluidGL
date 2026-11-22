extends Control

@export_group("Points")
@export var points: Label
@export var popup_template: Label
@export var popup_color: Color

@export_group("Tiers")
@export var tier_label: Label
@export var tier_fill: TierFill
@export var tier_letters: Array[String] = ["D", "C", "B", "A", "S"]

@export_group("Wows")
@export var wow_curve: Curve
@export var wow_strengh: float
@export var wow_duration: float
@export var wow_texts: Array[String]
@export var wow_colors: Array[Color]
@export var wow_sounds: Array[SoundEffectSettings.SOUND_EFFECT_TYPE]
@export var wow_template: Label

var _wow_size: Vector2
var _wow_scale: Vector2

var _tier_scale: Vector2
var _points_scale: Vector2

func _ready() -> void:
	XpManager.xp_changed.connect(_on_xp_changed)
	XpManager.tier_changed.connect(_on_tier_changed)
	TrickManager.on_nb_trick_increased.connect(_on_nb_tricks_increased)
	_wow_size = wow_template.size
	_wow_scale = wow_template.scale
	_tier_scale = tier_fill.scale
	_points_scale = points.scale

func points_popup(num_points: int) -> void:
	var label = popup_template.duplicate()
	label.text = "+" + str(num_points)
	label.add_theme_color_override("font_color", popup_color)
	popup_template.get_parent().add_child(label)
	
	var tween = create_tween().set_parallel(true)
	tween.tween_property(label, "position:y", label.position.y - 50, 0.6)\
		.set_trans(Tween.TRANS_QUAD)\
		.set_ease(Tween.EASE_OUT)
	tween.tween_property(label, "modulate:a", 0.2, 0.6)\
		.set_trans(Tween.TRANS_QUAD)\
		.set_ease(Tween.EASE_IN)

	await tween.finished
	label.queue_free()
	
func _maj_barre(_new_val: float) -> void:
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
	if not wow_sounds[index] == SoundEffectSettings.SOUND_EFFECT_TYPE.NO_SOUND:
		AudioManager.play(wow_sounds[index])
	label.add_theme_color_override("font_color", wow_colors[index])

	if (_wow_tween and _wow_tween.is_valid()):
		_wow_tween.kill()
	_wow_tween = label.create_tween()
	_wow_tween.tween_method(
		func(t: float) -> void:
			var k: float = wow_curve.sample(t) if wow_curve else 1.0
			label.scale = _wow_scale * (1.0 + k * wow_strengh),
		0.0, 1.0, wow_duration
	)

func _on_nb_tricks_increased(nb_tricks: int) -> void:
	spawn_wow(nb_tricks)

var previous_tier: int = -1
func _on_xp_changed(_xp_added: int, _tier_progression: float, _tier: int):
	points.text = "%.d" % XpManager.get_points()
	
	if previous_tier != _tier:
		tier_fill.set_remplissage(0 if previous_tier < _tier else 1)
		previous_tier = _tier

	if _xp_added > 0:
		points_popup(_xp_added)
		
		_maj_barre(_tier_progression)
		var tween = create_tween()
		tween.tween_property(tier_label, "scale", _tier_scale * 1.2, 0.05).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
		create_tween().tween_property(points, "scale", _points_scale * 1.2, 0.05).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	
		await tween.finished
		create_tween().tween_property(tier_label, "scale", _tier_scale, 0.1).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
		create_tween().tween_property(points, "scale", _points_scale, 0.1).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	else: tier_fill.set_remplissage(_tier_progression)


func _on_tier_changed(_new_tier: int):
	_new_tier = clamp(_new_tier, 0, tier_letters.size() - 1)
	tier_label.text = tier_letters[_new_tier]
