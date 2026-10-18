extends Node3D

@export var capteur_back: ShapeCast3D
@export var capteur_front: ShapeCast3D
@export var capteur_mesh: ShapeCast3D
@export var capteur_ground: ShapeCast3D
@export var mesh_instance: Node3D

signal on_fall()
signal on_grounded()
signal on_jump()
signal on_rail()
signal off_rail()

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

var current_rail: Path3D = null
var prev_rail: Path3D = null


func _limit_rotation(direction: Vector3) -> Vector3:
	var up: Vector3 = MapLoader.up()

	var horizontal: Vector3 = direction - up * direction.dot(up)
	var longueur_h: float = horizontal.length()
	if longueur_h < 0.001:
		return direction

	var angle: float = atan2(direction.dot(up), longueur_h)
	var max_rad: float = deg_to_rad(30)
	if angle >= 0 && angle <= max_rad:
		return direction

	angle = clamp(angle, -0, max_rad)
	return (horizontal / longueur_h * cos(angle) + up * sin(angle)) * direction.length()

func apply_impulse(impulse_back: float, impulse_front: float) -> void:
	velocity_back += impulse_back
	velocity_front += impulse_front
	impulsed_timer = 0.1
	
func apply_rot(rot: Basis) -> void:
	var p_back_offset: Vector3 = capteur_back.global_position - global_position
	var p_front_offset: Vector3 = capteur_front.global_position - global_position
	p_back_offset = rot * p_back_offset
	p_front_offset = rot * p_front_offset
	set_pos(p_back_offset + global_position, p_front_offset + global_position)
		
func set_rot(rot: Basis) -> void:
	mesh_instance.basis = rot

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
	$RailCooldown.start()
	current_rail = null

func _integrate(dt: float) -> void:
	var acc: float = -20 * dt
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

	var echelle: Vector3 = global_basis.get_scale()
	var dir_locale: Vector3 = (capteur_front.position - capteur_back.position).normalized()

	var rot: Basis = Basis.looking_at(direction.normalized(), normal) \
			* Basis.looking_at(dir_locale, Vector3.UP).inverse()
	global_basis = rot * Basis.from_scale(echelle)

	var milieu_local: Vector3 = (capteur_front.position + capteur_back.position) / 2.0
	global_position = (back + front) / 2.0 - global_basis * milieu_local
			
func _corriger_roue(capteur: ShapeCast3D, p: Vector3) -> float:
	capteur.force_shapecast_update()
	if not capteur.is_colliding():
		return -INF
	var rayon: float = (capteur.shape as SphereShape3D).radius * capteur.global_basis.get_scale().x
	var g: Vector3 = capteur.get_collision_point(0)
	var n: Vector3 = capteur.get_collision_normal(0)
	if n.y < 0.1:
		return -INF
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
		velocity_back = max(velocity_back, 0.0)

	var y_front: float = _corriger_roue(capteur_front, p_front)
	grounded_front = p_front.y <= y_front + 0.01
	if grounded_front:
		p_front.y = y_front
		corrige = true
		var n_front: Vector3 = capteur_front.get_collision_normal(0)
		normal += n_front
		var theta_front: float = acos(n_front.dot(-MapLoader.s_speed.normalized()))
		velocity_front = MapLoader.s_speed.length() / tan(theta_front)
		velocity_front = max(velocity_front, 0.0)

	if corrige:
		set_pos(p_back, p_front, normal.normalized())
		
func touches_path(which: Path3D, threshold: float) -> bool:
	var local_pos = which.to_local(global_position)
	var closest_point = which.curve.get_closest_point(local_pos)
	var distance = local_pos.distance_to(closest_point)
	return distance <= threshold

func _find_rail() -> void:
	current_rail = null
	for sec in MapLoader.instance._spawned_sections:
		for rl in sec.rails:
			if touches_path(rl, 1):
				current_rail = rl
	if current_rail and not is_grounded():
		TrickManager._current_trick = ""
		TrickManager._possible_tricks = TrickManager._tricks.get_neighbours("")

func get_zy_from_x(target_x: float) -> Vector2:
	if not current_rail: return Vector2.ZERO
	current_rail.curve.bake_interval = 0.1
	
	var tf = current_rail.global_transform
	var pts = current_rail.curve.get_baked_points()
	if pts.is_empty(): return Vector2.ZERO

	for i in range(pts.size()):
		pts[i] = tf * pts[i]
		
	var asc = pts[0].x <= pts[-1].x
	var low = 0
	var high = pts.size() - 1
	
	while low <= high:
		var mid = (low + high) >> 1
		if (pts[mid].x < target_x) == asc: low = mid + 1
		else: high = mid - 1
		
	var idx = clamp(low, 1, pts.size() - 1)
	var p1 = pts[idx - 1]
	var p2 = pts[idx]
	var t = 0.0 if p1.x == p2.x else (target_x - p1.x) / (p2.x - p1.x)
	
	return Vector2(lerp(p1.z, p2.z, t), lerp(p1.y, p2.y, t)) # Returns global Vector2(Z, Y)

func _process(dt: float) -> void:
	_integrate(dt)
	_ground_snap(dt)

	#if velocity_back + velocity_front < 0:
	if $RailCooldown.is_stopped():
		_find_rail()

	if current_rail and not prev_rail:
		on_rail.emit()
	if not current_rail and prev_rail:
		if impulsed_timer < 0: reset_velocities()
		off_rail.emit()

	prev_rail = current_rail
	
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
			
	if current_rail and $RailCooldown.is_stopped():
		#print("Rail")
		var zy = get_zy_from_x(global_position.x)
		global_position.z = zy.x
		global_position.y = zy.y
		
	_is_grounded = capteur_ground.is_colliding()
	
	previous_grounded_back = grounded_back
	previous_grounded_front = grounded_front
	impulsed_timer -= dt
	fall_timer -= dt
	
	if Input.is_action_just_pressed("B"): MapLoader.instance.stop()


func _on_rail_cooldown_timeout() -> void:
	_find_rail()
