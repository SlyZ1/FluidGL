extends Node3D

@export_group("Scene Variables")
# Pour que ça s'update à chaque modif
@export var target: Node3D:
	set(val):
		target = val
		_update()
	
@export var skate: Node3D
		
@export_group("Position")
@export var distance: float:
	set(val):
		distance = val
		_update()
		
@export var angles: Vector2:
	set(val):
		angles = val
		_update()
		
@export var offset: Vector3 = Vector3.ZERO
		

@export_group("Dezoom")
@export var distance_multiplier: float = 2
@export var skate_min_height: float = 5
@export var lerp_speed_multiplier: float = 10 

@export_group("Impact")
@export_range(0, 1) var impact_strength: float = 2
@export var impact_curve: Curve
@export var impact_duration: float
@export var impact_max_combo: int

var _target_distance: float
var _actual_distance: float

var _impact_distance_multiplier: float = 1
var _impact_t: float = 0
var _impact_num_combo: int = 0

func _ready() -> void:
	_update()
	_target_distance = distance
	_actual_distance = distance
	MapLoader.instance.on_stop.connect(_on_stop)
	TrickManager.trick_validated.connect(_create_impact)

# Fonction pour rotate la cam
func _update() -> void:
	if not is_inside_tree() or target == null or not target.is_inside_tree():
		return
		
	var look_dir: Vector3 = Vector3.FORWARD * cos(angles.y) * cos(angles.x)
	look_dir += Vector3.RIGHT * sin(angles.x) * cos(angles.y)
	look_dir += Vector3.DOWN * sin(angles.y)
	
	global_position = target.global_position + offset + look_dir * _actual_distance * _impact_distance_multiplier
	global_basis = Basis.looking_at(
		-look_dir,
		Vector3.UP if look_dir.dot(Vector3.UP) < 0.999 else Vector3.FORWARD
	) 
	
func _handle_impact(dt: float) -> void:
	_impact_t += dt
	var strength := float(max(_impact_num_combo - 1, 0)) / impact_max_combo * impact_strength
	_impact_distance_multiplier = 1 - strength * (1 - impact_curve.sample(min(_impact_t, 1)))

func _process(delta: float) -> void:
	if skate: 
		var skate_offset: float = max(skate.global_position.y - skate_min_height, 0)
		_target_distance = distance + skate_offset * distance_multiplier
		
	_actual_distance = lerp(_actual_distance, _target_distance, 1 - exp(-delta * lerp_speed_multiplier))

	_handle_impact(delta)
	_update()
	
func _on_stop() -> void:
	_target_distance = distance
	lerp_speed_multiplier = 1
	skate = null
	
func _create_impact(_num_combo: int) -> void:
	_impact_t = 0
	_impact_num_combo = _num_combo
