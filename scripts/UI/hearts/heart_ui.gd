extends Control
class_name HeartUI

@export var heart_settings: HeartSettings

var sprite2D: Sprite2D

func _ready() -> void:
    sprite2D = get_child(0) as Sprite2D
    set_ui(2)


func set_ui(value: int) -> void:
    value = clamp(value, 0, 2)
    if value == 0:
        sprite2D.texture = heart_settings.empty_heart
    elif value == 1:
        sprite2D.texture = heart_settings.half_heart
    else:
        sprite2D.texture = heart_settings.full_heart