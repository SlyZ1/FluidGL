extends Node

func _ready() -> void:
	TrickManager.trick_performed.connect(_on_trick)

func _process(delta: float) -> void:
	pass

func tween_tour(noeud: Node3D, axe: Vector3, _angle: float, duree: float) -> Tween:
	var depart: Basis = noeud.basis
	var tween := noeud.create_tween()
	tween.tween_method(
		func(angle: float) -> void:
			noeud.basis = depart * Basis(axe.normalized(), angle), 0.0, deg_to_rad(_angle), duree)\
	.set_trans(Tween.TRANS_QUAD)\
	.set_ease(Tween.EASE_OUT)
	return tween

func _on_trick(_name: String, _accuracy: float) -> void:
	var _anim := TrickManager.get_resource(_name)
	var _duration := TrickManager.get_trick_duration(_name)
	
	tween_tour(get_parent(), _anim.rot_axis, _anim.rot_angle, _duration)
	
