extends Node

@export var input_suggestion_timer: Timer
@export var input_suggestion_label: Label
@export var points_earned_label: Label

func _ready() -> void:
	GameManager.instance.on_respawn.connect(_hide)
	GameManager.instance.on_ready_to_respawn.connect(_on_ready_to_restart)

func _on_holdfh_timeout() -> void:
	input_suggestion_label.visible = not input_suggestion_label.visible

func _on_ready_to_restart() -> void:
	input_suggestion_label.visible = true
	input_suggestion_timer.start()
	#points_earned_label.text = "POINTS EARNED: %d" % XpManager.get_points()

func _hide() -> void:
	points_earned_label.text = ""
	input_suggestion_label.visible = false
	input_suggestion_timer.stop()
