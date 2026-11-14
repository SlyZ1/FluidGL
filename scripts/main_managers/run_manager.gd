extends Node
class_name RunManager

var next_threshold := 500

@export var choicePanels : Array[ColorRect]

func _on_finish_line_crossed() -> void:
	print("Crossed ! Nb points : ", PlayerState.get_points(), " / ", next_threshold)
	# TODO : actuellement ya 0 points dans playerstate pcq faut que je clean TrickManager
	# en fait faudrait que TrickManager puisse donner des points au joueur
	# sauf qu'il a pas accès à UpdateManager pcq TrickManager est un autoload

	for ch in choicePanels:
		ch.scale = Vector2.ZERO
		ch.visible = true
		get_tree().create_tween().tween_property(ch, "scale", Vector2.ONE, 0.67)

