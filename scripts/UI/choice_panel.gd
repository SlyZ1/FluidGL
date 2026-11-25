extends Control

@export var anim_speed : float

@export var choice_title : Label
@export var bg : ColorRect
@export var rect1 : ColorRect
@export var rect2 : ColorRect
@export var rect3 : ColorRect
@export var unsync : Timer

func _ready() -> void:
	choice_title.scale = Vector2.ZERO
	rect1.scale = Vector2.ZERO
	rect2.scale = Vector2.ZERO
	rect3.scale = Vector2.ZERO

func set_background_anim(t : float) -> void:
	bg.material.set_shader_parameter("tt", t)

func display_choice(choice1 : Upgrade, choice2 : Upgrade, choice3 : Upgrade) -> void:
	visible = true

	var bg_anim_speed := 3 * anim_speed
	var delay_btw_choices := 0.25 * anim_speed

	create_tween().tween_method(set_background_anim, 0., 1., bg_anim_speed)
	unsync.start(0.5 * bg_anim_speed)
	await unsync.timeout
	
	create_tween().set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_ELASTIC).tween_property(rect1, "scale", Vector2.ONE, anim_speed)
	unsync.start(delay_btw_choices)
	await unsync.timeout
	
	create_tween().set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_ELASTIC).tween_property(rect2, "scale", Vector2.ONE, anim_speed)
	unsync.start(delay_btw_choices)
	await unsync.timeout
	
	create_tween().set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_ELASTIC).tween_property(rect3, "scale", Vector2.ONE, anim_speed)
	unsync.start(delay_btw_choices)
	await unsync.timeout
	
	create_tween().set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_ELASTIC).tween_property(choice_title, "scale", Vector2.ONE, anim_speed)
