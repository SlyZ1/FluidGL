class_name InputPanel
extends Control

@export var quest_color : Color = Color.RED
@export var correct_color : Color = Color.GREEN
@export var passive_color : Color = Color.WHITE_SMOKE
@export var roundness: int = 4

var _last_input : String

func set_input_ref(input: String) -> void:
	_last_input = input
	_update_with_color(input, quest_color)
	for i in range(6):
		get_node(input[i].to_upper()).modulate = quest_color if input[i] == input[i].to_upper() else passive_color

func _update_with_color(input: String, clr: Color) -> void:
	for i in range(6):
		get_node(input[i].to_upper()).modulate = clr if input[i] == input[i].to_upper() else passive_color

func _ready():
	set_input_ref("rfVYhn")

func correct():
	_update_with_color(_last_input, correct_color)

func incorrect():
	_update_with_color(_last_input, quest_color)
