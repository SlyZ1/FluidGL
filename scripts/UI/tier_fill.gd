extends Label

class_name TierFill

var remplissage: float = 0.0

func _ready() -> void:
	resized.connect(_maj_taille)
	_maj_taille()
	set_remplissage(remplissage)

func _maj_taille() -> void:
	material.set_shader_parameter("taille", size)

func set_remplissage(v: float) -> void:
	remplissage = clampf(v, 0.0, 1.0)
	remplissage = lerp(0.1, 0.9, remplissage)
	material.set_shader_parameter("remplissage", remplissage)
