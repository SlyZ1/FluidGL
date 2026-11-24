extends Sprite3D
class_name SkateVFX

var _tw: Tween

func _ready() -> void:
	TrickManager.trick_performed.connect(_on_trick_performed)
	GameManager.instance.on_death.connect(_reset)

func set_trick_vfx(t : float) -> void:
	material_override.set_shader_parameter("imp", t)

func _on_trick_performed(_name: String) -> void:
	if _name == TrickManager.jump_trick_name: return
	
	if _tw: _tw.kill()
	_tw = create_tween()
	visible = true
	_tw.tween_method(set_trick_vfx, 0., 1., 0.3).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	await _tw.finished
	visible = false

func _reset() -> void:
	set_trick_vfx(1.0)
	if _tw: _tw.kill()