extends Node3D


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	TrickManager.process_mode = Node.PROCESS_MODE_DISABLED
	$Background.set_active($Background.skate, false)
	$Background/Skate.visible = false
	$Background/UI.visible = false


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	if Input.is_action_pressed("F") and Input.is_action_pressed("H"):
		TrickManager.process_mode = Node.PROCESS_MODE_ALWAYS
		get_tree().change_scene_to_file("res://scenes/skate_prototype.tscn")
		


func _on_blink_timeout() -> void:
	$Control/Label2.visible = not $Control/Label2.visible
