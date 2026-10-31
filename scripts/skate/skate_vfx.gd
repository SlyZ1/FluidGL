extends Sprite3D
class_name SkateVFX

func _ready() -> void:
	TrickManager.trick_performed.connect(_on_trick_performed)

func set_trick_vfx(t : float) -> void:
	material_override.set_shader_parameter("imp", t)

func _on_trick_performed(_name: String, _accuracy: float) -> void:
	if _name in ["catch", "instable_saut"]: return
	var tw:Tween = create_tween()
	visible = true
	tw.tween_method(set_trick_vfx, 0., 1., 0.3).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	await tw.finished
	visible = false
