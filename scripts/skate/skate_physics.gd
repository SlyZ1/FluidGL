extends Node3D

class_name SkatePhysics

@export var capteur_back: ShapeCast3D
@export var capteur_front: ShapeCast3D
@export var capteur_mesh: ShapeCast3D
@export var capteur_ground: ShapeCast3D
@export var mesh_instance: Node3D
@export_group("Forces")
@export var gravity: float = 80
@export var mass: float = 1.0
@export var inertia: float = 0.05
@export var angular_drag: float = 0.01
@export_group("Rotation limit")
@export_range(0, 90) var max_deg_angle: float = 40
@export var limit_softness: float = 6
@export_group("Slope")
@export_range(0, 1) var slope_thresh: float = 0.3
@export_range(0, 1) var weak_slope_thresh: float = 0.8

signal on_fall()
signal on_grounded()
signal on_jump()

var _is_grounded: bool = false

var _impulsed_timer: float = 0
var _fall_timer: float = 0

var _y: float
var _theta: float

var _vy: float
var _omega: float

var _base_basis: Basis
var _centre_local: Vector3
var _centre_xz: Vector2
var _half_len: float

func _ready() -> void:
	GameManager.instance.on_substep.connect(_step)
	_init_state()

func apply_impulse(impulse: float, angular_impulse: float = 0) -> void:
	if not _is_grounded or not impulse < 0: _vy += impulse / mass
	_omega += angular_impulse / inertia
	if impulse > 0: _impulsed_timer = 0.1

func _apply_wheel_impulse(side: float, j: float) -> void:
	var levier := side * _half_len * cos(_theta)
	_vy += j / mass
	_omega += j * levier / inertia
	
func just_impulsed() -> bool:
	return _impulsed_timer > 0
	
func reset_orientation(dt: float = 1) -> void:
	#_theta = 0
	_omega = -_theta * 0.01 / dt

func reset_velocities() -> void:
	_vy = 0
	_omega = 0
		
func set_rot(rot: Basis) -> void:
	mesh_instance.basis = rot * Basis.from_scale(mesh_instance.basis.get_scale())

func is_grounded() -> bool:
	return capteur_ground.is_colliding()
	
func is_colliding() -> bool:
	return capteur_back.is_colliding() and capteur_front.is_colliding()
	
func is_mesh_colliding() -> bool:
	return capteur_mesh.is_colliding()
	
func get_normal() -> Vector3:
	var normal: Vector3 = Vector3.ZERO
	if capteur_back.is_colliding():
		var n_back: Vector3 = capteur_back.get_collision_normal(0)
		normal += n_back
	if capteur_front.is_colliding():
		var n_front: Vector3 = capteur_front.get_collision_normal(0)
		normal += n_front
	return normal.normalized()


# ------------------ PHYSICS LOGIC ------------------

func _init_state() -> void:
	_base_basis = global_basis
	_half_len = (capteur_back.global_position - capteur_front.global_position).length() * 0.5

	var mid: Vector3 = (capteur_back.global_position + capteur_front.global_position) / 2.0
	_centre_local = to_local(mid)
	_centre_xz = Vector2(mid.x, mid.z)
	_y = mid.y
	_theta = 0
	_vy = 0
	_omega = 0

func _apply_transform():
	global_basis = _base_basis * Basis(Vector3.RIGHT, -_theta)
	global_position.y = _y - (global_basis * _centre_local).y

func _get_most_up_normal(capteur: ShapeCast3D, inverse: bool = false) -> int:
	var result: int = 0
	var normal: Vector3 = capteur.get_collision_normal(0)
	for i in range(capteur.get_collision_count()):
		if (normal.y < capteur.get_collision_normal(i).y) == !inverse:
			normal = capteur.get_collision_normal(i)
			result = i
	return result
			
func _roue_snapped_position(capteur: ShapeCast3D) -> Vector2:
	capteur.force_shapecast_update()
	if not capteur.is_colliding():
		return Vector2(-INF, 0)
	var rayon: float = (capteur.shape as SphereShape3D).radius * capteur.global_basis.get_scale().x
	var i: int = _get_most_up_normal(capteur, true)
	var g: Vector3 = capteur.get_collision_point(i)
	var n: Vector3 = capteur.get_collision_normal(i)
	if n.y < -0.4:
		return Vector2(-INF, 0)
	if n.y < slope_thresh:
		GameManager.instance.stop((Vector3.LEFT * 2 + Vector3.UP) * 10)
		return Vector2(0, 0)
	var p: Vector3 = capteur.global_position
	var h: float = g.y + (rayon - n.x * (p.x - g.x) - n.z * (p.z - g.z)) / n.y # solving n dot (p - g) = r
	var v_sol: float = -n.dot(MapLoader.instance.speed_vector) / n.y
	return Vector2(h, v_sol)

func _contact_vitesse(side: float, sol: Vector2, dt: float) -> void:
	if sol.x == -INF: 
		return
	var y_pos: float = _y + side * _half_len * sin(_theta)
	var penetration: float = sol.x - y_pos
	if penetration < -0.01:
		return

	_vy = max(_vy, -20)

	var levier: float = side * _half_len * cos(_theta)
	var v_rot: float = _omega * levier
	var v_roue: float = _vy + v_rot
	var v_target: float = sol.y + minf(penetration, 0.0) / dt
	if v_roue >= v_target:
		return
	var k: float = 1 / mass + levier * levier / inertia
	var impulsion: float = (v_target - v_roue) / k
	_vy += impulsion / mass
	_omega += impulsion * levier / inertia

func _contact_position(side: float, y_ground: float) -> void:
	if y_ground == -INF:
		return
	var y_pos: float = _y + side * _half_len * sin(_theta)
	var penetration: float = y_ground - y_pos
	if penetration <= 0:
		return
	var levier: float = side * _half_len * cos(_theta)
	var k: float = 1 / mass + levier * levier / inertia
	var p := penetration * 0.8 / k
	_y += p / mass
	_theta += p * levier / inertia

func _limite_pitch(_dt: float) -> void:
	var pitch_max: float = deg_to_rad(max_deg_angle)
	var s: float = sign(_theta) if _theta != 0.0 else sign(_omega)
	var vers_exterieur: float = _omega * s
	if vers_exterieur <= 0.0:
		return
	var marge: float = max(pitch_max - abs(_theta), 0.0)
	var v_max: float = marge * limit_softness
	if vers_exterieur > v_max:
		_omega = v_max * s

func _grounded_logic() -> void:
	if capteur_ground.is_colliding():
		if !_is_grounded: 
			print("Grounded")
			on_grounded.emit()
	elif _is_grounded:
		if _impulsed_timer >= 0:
			AudioManager.play(SoundEffectSettings.SOUND_EFFECT_TYPE.JUMP)
			print("Jump")
			on_jump.emit()
		else:
			print("Fall")
			on_fall.emit()
	_is_grounded = capteur_ground.is_colliding()

func _step(dt: float) -> void:
	if !can_process():
		return 
	var sol_back: Vector2 = _roue_snapped_position(capteur_back)
	var sol_front: Vector2 = _roue_snapped_position(capteur_front)

	_vy -= gravity * dt
	_omega -= _omega * angular_drag * dt

	for _i in range(10):
		_contact_vitesse(-1, sol_back, dt)
		_contact_vitesse(1, sol_front, dt)

	_limite_pitch(dt)
	_y += _vy * dt
	_theta += _omega * dt

	for i in range(10):
		_contact_position(-1, sol_back.x)
		_contact_position(1, sol_front.x)

	_apply_transform()
	_grounded_logic()
	
	_impulsed_timer -= dt
	_fall_timer -= dt
