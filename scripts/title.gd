extends Node3D

@export var input_suggestion: Label
@export var gameplay_scene: PackedScene

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	GameManager.auto_start = false
	TrickManager.process_mode = Node.PROCESS_MODE_DISABLED
	$Background/UI.visible = false


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
	if Input.is_action_pressed("F") and Input.is_action_pressed("H"):
		TrickManager.process_mode = Node.PROCESS_MODE_ALWAYS
		get_tree().change_scene_to_packed(gameplay_scene)
		GameManager.auto_start = true

func _on_blink_timeout() -> void:
	input_suggestion.visible = not input_suggestion.visible
