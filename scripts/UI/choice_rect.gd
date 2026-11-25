extends Control
class_name ChoiceRect

@export var anim_seed : float
@export var press_what_to_select : String

@export var background : ColorRect
@export var delay : Timer
@export var upgrade_name : Label
@export var rarity : Label
@export var description : Label
@export var pickme : Label

signal chose_me(which : String)

func _ready() -> void:
	background.scale = Vector2.ZERO
	background.material.set_shader_parameter("seed", anim_seed)
	pickme.text = "press " + press_what_to_select + " to select"

func appear(with_delay : float, duration : float, content : Upgrade) -> void:
	upgrade_name.text = content.upgrade_name
	description.text = content.description

	const rarities : Array[String] = ["Common", "Rare", "Epic", "Legendary"]
	rarity.text = rarities[int(content.rarity * (len(rarities) - 1))]

	if with_delay > 0:
		delay.start(with_delay)
		await delay.timeout
	create_tween().set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_ELASTIC).tween_property(background, "scale", Vector2.ONE, duration)

func disappear(with_delay : float, duration : float) -> void:
	if with_delay > 0:
		delay.start(with_delay)
		await delay.timeout
	create_tween().set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_QUINT).tween_property(background, "scale", Vector2.ZERO, duration)

func _on_flicker_timeout() -> void:
	pickme.visible = not pickme.visible

func _process(delta : float) -> void:
	if Input.is_action_just_pressed(press_what_to_select):
		chose_me.emit(press_what_to_select)