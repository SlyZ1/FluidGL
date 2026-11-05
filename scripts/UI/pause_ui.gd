extends CanvasLayer

func _ready() -> void:
	GameManager.instance.on_pause.connect(_on_pause)

func _on_pause(paused: bool) -> void:
	visible = !paused