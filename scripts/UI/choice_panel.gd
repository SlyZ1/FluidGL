extends Control

@export var anim_speed : float

@export var choice_title : Label
@export var bg : ColorRect
@export var rect1 : Control
@export var rect2 : Control
@export var rect3 : Control
@export var unsync : Timer

func _ready() -> void:
	choice_title.scale = Vector2.ZERO

func set_background_anim(t : float) -> void:
	bg.material.set_shader_parameter("tt", t)

func display_choice(choice1 : Upgrade, choice2 : Upgrade, choice3 : Upgrade) -> void:
	visible = true

	var bg_anim_speed := 2 * anim_speed
	var delay_btw_choices := 0.25 * anim_speed

	create_tween().tween_method(set_background_anim, 0., 1., bg_anim_speed)
	unsync.start(0.33 * bg_anim_speed)
	await unsync.timeout

	rect1.appear(0, anim_speed, choice1)
	rect2.appear(1 * delay_btw_choices, anim_speed, choice2)
	rect3.appear(2 * delay_btw_choices, anim_speed, choice3)

	unsync.start(3 * delay_btw_choices)
	await unsync.timeout
	
	create_tween().set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_ELASTIC).tween_property(choice_title, "scale", Vector2.ONE, anim_speed)
