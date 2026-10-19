extends Node3D

@export var capteur_back: ShapeCast3D
@export var capteur_front: ShapeCast3D
@export var capteur_mesh: ShapeCast3D
@export var capteur_ground: ShapeCast3D
@export var mesh_instance: Node3D
@export var gravity: float = 20
@export_range(0, 1) var slope_thresh: float = 0.3

signal on_fall()
signal on_grounded()
signal on_jump()

var velocity_back: float = 0
var velocity_front: float = 0
var grounded_back: bool = false
var grounded_front: bool = false
var previous_grounded_back: bool = false
var previous_grounded_front: bool = false
var is_jumping: bool = false

var _is_grounded: bool = false

var impulsed_timer: float = 0
var fall_timer: float = 0

var _scale: Vector3
var _mesh_scale: Vector3

func _ready() -> void:
	_scale = global_basis.get_scale()
	_mesh_scale = mesh_instance.basis.get_scale()

func _limit_rotation(direction: Vector3) -> Vector3:
	var up: Vector3 = MapLoader.up()

	var horizontal: Vector3 = direction - up * direction.dot(up)
	var longueur_h: float = horizontal.length()
	if longueur_h < 0.001:
		return direction

	var angle: float = atan2(direction.dot(up), longueur_h)
	var max_rad: float = deg_to_rad(40)
	if abs(angle) <= max_rad:
		return direction

	angle = clamp(angle, -0, max_rad)
	return (horizontal / longueur_h * cos(angle) + up * sin(angle)) * direction.length()

func apply_impulse(impulse_back: float, impulse_front: float) -> void:
	velocity_back += impulse_back
	velocity_front += impulse_front
	impulsed_timer = 0.1
	
func just_impulsed() -> bool:
	return impulsed_timer > 0
	
func apply_rot(rot: Basis) -> void:
	var p_back_offset: Vector3 = capteur_back.global_position - global_position
	var p_front_offset: Vector3 = capteur_front.global_position - global_position
	p_back_offset = rot * p_back_offset
	p_front_offset = rot * p_front_offset
	set_pos(p_back_offset + global_position, p_front_offset + global_position)
	
func reset_orientation() -> void:
	var p_back = capteur_back.global_position
	var p_front = capteur_front.global_position
	var center = (p_back + p_front) / 2.0
	p_back.y = center.y
	p_front.y = center.y
	set_pos(p_back, p_front)
		
func set_rot(rot: Basis) -> void:
	mesh_instance.basis = rot * Basis.from_scale(_mesh_scale)

func is_grounded() -> bool:
	return capteur_ground.is_colliding()
	
func is_colliding() -> bool:
	return capteur_back.is_colliding() and capteur_front.is_colliding()
	
func is_mesh_colliding() -> bool:
	return capteur_mesh.is_colliding()
	
func get_normal() -> Vector3:
	var normal: Vector3 = Vector3.ZERO
	if grounded_back:
		var n_back: Vector3 = capteur_back.get_collision_normal(0)
		normal += n_back
	if grounded_front:
		var n_front: Vector3 = capteur_front.get_collision_normal(0)
		normal += n_front
	return normal.normalized()

func reset_velocities() -> void:
	velocity_back = 0
	velocity_front = 0

func _integrate(dt: float) -> void:
	var acc: float = -80 * dt
	if not grounded_back: velocity_back += acc
	if not grounded_front: velocity_front += acc
		
	var p_back: Vector3 = capteur_back.global_position
	var p_front: Vector3 = capteur_front.global_position
	var ecart: Vector3 = p_front - p_back
	var longueur_h: float = Vector2(ecart.x, ecart.z).length()
	if longueur_h < 0.001:
		return

	var v_centre: float = (velocity_back + velocity_front) / 2.0
	var omega: float = (velocity_front - velocity_back) / longueur_h
	
	var centre: Vector3 = (p_back + p_front) / 2.0
	var axe: Vector3 = ecart.cross(Vector3.UP)
	if axe.length() > 0.001:
		var rot := Basis(axe.normalized(), omega * dt)
		global_basis = rot * global_basis
		global_position = centre + rot * (global_position - centre)

	global_position += Vector3.UP * v_centre * dt
	
	p_back = capteur_back.global_position
	p_front = capteur_front.global_position
	var d: Vector3 = p_front - p_back
	var d_lim: Vector3 = _limit_rotation(d)
	if not d.is_equal_approx(d_lim):
		var milieu: Vector3 = (p_back + p_front) / 2.0
		set_pos(milieu - d_lim / 2.0, milieu + d_lim / 2.0)
		velocity_back = v_centre
		velocity_front = v_centre
	
func set_pos(back: Vector3, front: Vector3, normal: Vector3 = Vector3.UP) -> void:
	var direction: Vector3 = _limit_rotation(front - back)
	if direction.length() < 0.001:
		return

	var dir_locale: Vector3 = (capteur_front.position - capteur_back.position).normalized()

	var rot: Basis = Basis.looking_at(direction.normalized(), normal) \
			* Basis.looking_at(dir_locale, Vector3.UP).inverse()
	global_basis = rot * Basis.from_scale(_scale)

	var milieu_local: Vector3 = (capteur_front.position + capteur_back.position) / 2.0
	global_position = (back + front) / 2.0 - global_basis * milieu_local
	
func _get_most_up_normal(capteur: ShapeCast3D) -> Vector3:
	var result: Vector3 = capteur.get_collision_normal(0)
	for i in range(1, capteur.get_collision_count()):
		if result.y > capteur.get_collision_normal(i).y:
			result = capteur.get_collision_normal(i)
	return result
			
func _corriger_roue(capteur: ShapeCast3D, p: Vector3) -> float:
	capteur.force_shapecast_update()
	if not capteur.is_colliding():
		return -INF
	var rayon: float = (capteur.shape as SphereShape3D).radius * capteur.global_basis.get_scale().x
	var g: Vector3 = capteur.get_collision_point(0)
	var n: Vector3 = _get_most_up_normal(capteur)
	if n.y < slope_thresh:
		MapLoader.instance.stop((Vector3.LEFT * 2 + Vector3.UP) * 10)
		return 0
	return g.y + (rayon - n.x * (p.x - g.x) - n.z * (p.z - g.z)) / n.y

func _ground_snap(dt: float) -> void:
	var p_back: Vector3 = capteur_back.global_position
	var p_front: Vector3 = capteur_front.global_position
	var corrige: bool = false
	
	capteur_back.force_shapecast_update()
	capteur_front.force_shapecast_update()
	
	var normal: Vector3 = Vector3.ZERO

	var y_back: float = _corriger_roue(capteur_back, p_back)
	grounded_back = p_back.y <= y_back + 0.01
	if grounded_back:
		p_back.y = y_back
		corrige = true
		var n_back: Vector3 = capteur_back.get_collision_normal(0)
		normal += n_back
		var theta_back: float = acos(n_back.dot(-MapLoader.s_speed.normalized()))
		velocity_back = MapLoader.s_speed.length() / tan(theta_back)
		velocity_back = max(velocity_back, -10.0)

	var y_front: float = _corriger_roue(capteur_front, p_front)
	grounded_front = p_front.y <= y_front + 0.01
	if grounded_front:
		p_front.y = y_front
		corrige = true
		var n_front: Vector3 = capteur_front.get_collision_normal(0)
		normal += n_front
		var theta_front: float = acos(n_front.dot(-MapLoader.s_speed.normalized()))
		velocity_front = MapLoader.s_speed.length() / tan(theta_front)
		velocity_front = max(velocity_front, -10.0)

	if corrige:
		set_pos(p_back, p_front, normal.normalized())

func step(dt: float) -> void:
	_integrate(dt)
	_ground_snap(dt)
	
	if capteur_ground.is_colliding():
		if !_is_grounded: 
			print("Grounded")
			on_grounded.emit()
	elif _is_grounded:
		if impulsed_timer >= 0:
			print("Jump")
			on_jump.emit()
		else:
			print("Fall")
			on_fall.emit()
		
	_is_grounded = capteur_ground.is_colliding()
	
	previous_grounded_back = grounded_back
	previous_grounded_front = grounded_front
	impulsed_timer -= dt
	fall_timer -= dt


@onready var initial_rot_finger = $"2doigts".global_rotation
@onready var initial_rot_finger_up = $"2doigts_up".global_rotation
func _process(delta: float) -> void:
	var fh := TrickManager.catch_pressed()
	$"2doigts".visible = fh
	$"2doigts_up".visible = not fh
	$"2doigts".visible = fh
	$"2doigts_up".visible = not fh
	$"2doigts".global_rotation = initial_rot_finger
	$"2doigts_up".global_rotation = initial_rot_finger_up
