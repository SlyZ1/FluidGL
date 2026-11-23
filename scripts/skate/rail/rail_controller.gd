extends Node
class_name RailController

@export var skate_point_per_second: float = 200
@export_group("Components")
@export var skate_physics: SkatePhysics
@export var anim_controller: AnimController
@export var lane_snapper: LaneSnapper
@export_group("Rail Detection")
@export var rail_z_thresh: float = 2.5
@export var rail_y_thresh_enter: float = 1.5
@export var rail_y_thresh_die: float = 0.75
@export var rail_y_thresh_neg: float = -2
@export var rail_y_offset: float = -0.3
@export var rail_exit_time: float = 0.1
@export_group("Rail Animation")
@export var rail_pose_duration = 0.12
@export var rail_slide_angles: Vector3 = Vector3(-20, 90, 0)
@export var rail_suivre_direction: bool = true
@export var rail_seuil_courbure: float = 0.02
@export var rail_duree_changement_cote: float = 0.25

var is_on_rail: bool = false

var current_rail: Path3D
var _current_state: RailControllerState

var _rail_pose_weight: float
var _rail_pose_cible: Quaternion
var _rail_cote: float
var _rail_cote_cible: float
var _rail_weight_tween: Tween

enum RailState { GROUNDED, FREE, GRINDING };

func _ready() -> void:
	GameManager.instance.on_death.connect(_reset)
	TimeManager.on_substep.connect(_step)
	_current_state = _new_state(RailState.GROUNDED)

func _exit_tree() -> void:
	TimeManager.on_substep.disconnect(_step)

func _new_state(state: RailState) -> RailControllerState:
	match state:
		RailState.FREE: return RailFreeState.new()
		RailState.GRINDING: return RailGrindingState.new()
		RailState.GROUNDED: return RailGroundedState.new()
	return RailGroundedState.new()

func _step(dt: float) -> void:
	if not GameManager.instance.game_started: 
		return
	var state: RailState = _current_state.check_transitions(skate_physics, dt)
	if state != _current_state.state():
		_current_state.exit(self)
		_current_state = _new_state(state)
		_current_state.enter(self)

func _process(_delta: float) -> void:
	anim_controller.add_default_trick_rotation(Quaternion.IDENTITY.slerp(_rail_pose_cible, _rail_pose_weight))

func _update_rail(new_rail: Path3D) -> void:
	current_rail = new_rail

func _reset() -> void:
	_current_state.exit(self)
	_current_state = _new_state(RailState.FREE)
	_current_state.enter(self)
	current_rail = null
	is_on_rail = false
	_rail_pose_weight = 0
	if _rail_weight_tween: _rail_weight_tween.kill()


# -------------------- UTILITIES --------------------


func _baked(rail: Path3D) -> PackedVector3Array:
	if not rail.has_meta(&"baked"):
		rail.set_meta(&"baked", rail.curve.get_baked_points())  
	return rail.get_meta(&"baked")

func _get_point_on_rail(rail: Path3D, pos_x: float) -> Vector3:
	if rail == null:
		return Vector3.INF
	var points: PackedVector3Array = _baked(rail)
	if points.size() < 2: 
		return Vector3.INF

	# local space transformation
	var rail_transform: Transform3D = rail.global_transform
	var x_projector: Vector3 = Vector3(rail_transform.basis.x.x, rail_transform.basis.y.x, rail_transform.basis.z.x)
	var local_pos_x: float = pos_x - rail_transform.origin.x

	# out of bounds check
	var start_x: float = x_projector.dot(points[0])
	var end_x: float = x_projector.dot(points[-1])
	if local_pos_x < min(start_x, end_x) or local_pos_x > max(start_x, end_x):
		return Vector3.INF

	# dichotomy
	var ascendant: bool = start_x <= end_x
	var low: int = 0
	var high: int = points.size() - 1
	while low <= high:
		var mid := (low + high) >> 1
		if (x_projector.dot(points[mid]) < local_pos_x) == ascendant:
			low = mid + 1
		else:
			high = mid - 1

	var index: int = clamp(low, 1, points.size() - 1)
	var local_a: Vector3 = points[index - 1]
	var local_b: Vector3 = points[index]
	var local_xa: float = x_projector.dot(local_a)
	var local_xb: float = x_projector.dot(local_b)
	var t: float = 0.0 if is_equal_approx(local_xa, local_xb) else (local_pos_x - local_xa) / (local_xb - local_xa)

	var p: Vector3 = rail_transform * local_a.lerp(local_b, t)
	p += rail_y_offset * Vector3.UP
	return p

func _touches_rail(rail: Path3D, pos: Vector3) -> float:
	var rail_point: Vector3 = _get_point_on_rail(rail, pos.x)
	if rail_point == Vector3.INF:
		return INF

	var dy: float = pos.y - rail_point.y
	var dz: float = abs(pos.z - rail_point.z) 
	if dz <= rail_z_thresh:
		return dy # dy <= rail_y_thresh_enter

	return INF



func _rail_vitesse_verticale(rail: Path3D, x: float) -> float:
	if rail == null: 
		return 0
	var h: float = 0.3
	var a: float = _get_point_on_rail(rail, x - h).y
	var b: float = _get_point_on_rail(rail, x + h).y
	if a == INF or b == INF: 
		return 0
	var pente: float = (b - a) / (2.0 * h)
	var v: Vector3 = MapLoader.instance.speed_vector
	return pente * v.x - v.y


func _rail_range(rail: Path3D) -> Vector2:
	var pts := _baked(rail)
	var tf := rail.global_transform
	var x0 := (tf * pts[0]).x
	var x1 := (tf * pts[-1]).x
	return Vector2(minf(x0, x1), maxf(x0, x1))

func _rail_courbure(rail: Path3D, x: float) -> float:
	if rail == null:
		return 0
	var h: float = 1.0
	var r: Vector2 = _rail_range(rail)
	if r.y - r.x < 2.0 * h:
		return 0.0 

	var xc: float = clamp(x, r.x + h, r.y - h)
	var a: float = _get_point_on_rail(rail, xc - h).z
	var b: float = _get_point_on_rail(rail, xc).z
	var c: float = _get_point_on_rail(rail, xc + h).z
	var pente: float = (c - a) / (2.0 * h)
	var d2: float = (c - 2.0 * b + a) / (h * h)
	return d2 / pow(1.0 + pente * pente, 1.5)

func _rail_pente(rail: Path3D, x: float) -> float:
	var h: float = 0.3
	var r: Vector2 = _rail_range(rail)
	var xa: float = clamp(x - h, r.x, r.y)
	var xb: float = clamp(x + h, r.x, r.y)
	if xb - xa < 1e-4:
		return 0.0
	var a: Vector3 = _get_point_on_rail(rail, xa)
	var b: Vector3 = _get_point_on_rail(rail, xb)
	return (b.z - a.z) / abs(xb - xa)

func _calc_rail_pose() -> Quaternion:
	var angles: Vector3 = rail_slide_angles * PI / 180.0
	angles.x *= _rail_cote
	if rail_suivre_direction:
		angles.y -= atan(_rail_pente(current_rail, skate_physics.global_position.x))
	return Quaternion.from_euler(angles)

func _rail_cote_interieur(rail: Path3D) -> float:
	var pts: PackedVector3Array = rail.curve.get_baked_points()
	if pts.size() < 3:
		return 1.0
	var tf: Transform3D = rail.global_transform
	var debut: Vector3 = tf * pts[0]
	var fin: Vector3 = tf * pts[-1]
	var milieu: Vector3 = tf * pts[pts.size() >> 1] # pts.size() / 2 sans warning
	var bombe: float = milieu.z - (debut.z + fin.z) / 2.0
	return -1.0 if bombe > 0.0 else 1.0

func _tween_rail_weight(cible: float) -> void:
	if _rail_weight_tween and _rail_weight_tween.is_valid():
		_rail_weight_tween.kill()
	_rail_weight_tween = create_tween()
	_rail_weight_tween.tween_property(self, "_rail_pose_weight", cible, rail_pose_duration)
	_rail_weight_tween.set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)


func _init_rail_pose_cible() -> void:
	var k: float = _rail_courbure(current_rail, skate_physics.global_position.x)
	if absf(k) > rail_seuil_courbure:
		_rail_cote_cible = signf(k)

	if absf(k) <= rail_seuil_courbure:
		_rail_cote_cible = _rail_cote_interieur(current_rail)
	_rail_cote = _rail_cote_cible


func _update_rail_pose_cile(dt: float) -> void:
	var k: float = _rail_courbure(current_rail, skate_physics.global_position.x)
	if absf(k) > rail_seuil_courbure:
		_rail_cote_cible = signf(k)

	_rail_cote = move_toward(_rail_cote, _rail_cote_cible, dt * 2.0 / rail_duree_changement_cote)
	_rail_pose_cible = _calc_rail_pose()
