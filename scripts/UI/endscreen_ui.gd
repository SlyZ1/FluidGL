extends Node

@export var input_suggestion_label: Label
@export var input_suggestion_timer: Timer
@export var points_earned_label: Label

func _on_holdfh_timeout() -> void:
	input_suggestion_label.visible = not input_suggestion_label.visible

func _on_restart_lag_timeout() -> void:
	input_suggestion_timer.start()
	points_earned_label.text = "POINTS EARNED: %d" % TrickManager._points
