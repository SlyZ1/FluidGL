extends Node3D

# Pour que ça s'update à chaque modif
@export var target: Node3D:
	set(val):
		target = val
		_update()
@export var distance: float:
	set(val):
		distance = val
		_update()
		
@export var angles: Vector2:
	set(val):
		angles = val
		_update()

func _ready() -> void:
	_update()

# Fonction pour rotate la cam
func _update() -> void:
	if not is_inside_tree() or target == null or not target.is_inside_tree():
		return
		
	var look_dir: Vector3 = Vector3.FORWARD * cos(angles.y) * cos(angles.x)
	look_dir += Vector3.RIGHT * sin(angles.x) * cos(angles.y)
	look_dir += Vector3.DOWN * sin(angles.y)
	
	global_position = target.global_position + look_dir * distance
	global_basis = Basis.looking_at(
		-look_dir,
		Vector3.UP if look_dir.dot(Vector3.UP) < 0.999 else Vector3.FORWARD
	)
