extends Node

class_name AnimController

@export_group("Tricks")
@export_range(0, 1) var trick_cancel_end: float = 0.1
@export_range(0, 1) var trick_catchable_start: float = 0.7
@export_range(0, 1) var trick_ground_collision: float = 0.75
@export var mesh_instance: Node3D
@export_group("Lane")
@export var lane_position: float = 2.5
@export_group("Catch")
@export var catch_fall_speed: float = 20
@export var catch_start_speed: float = -1.5

@export_group("Rail")
@export var rail_xp_rate: float = 1
@export var rail_z_thresh: float = 0.4
@export var rail_y_thresh: float = 1
@export var rail_y_offset: float = 0.3
@export var rail_entry_duration: float = 0.15
@export var rail_slide_angles: Vector3 = Vector3(-20, 90, 0)
@export var rail_pose_duration: float = 0.12
@export var rail_suivre_direction: bool = true
@export var rail_seuil_courbure: float = 0.02
@export var rail_duree_changement_cote: float = 0.25

var _rail_offset: Vector2 = Vector2.ZERO
var _rail_offset_weight: float = 0.0
var _rail_tween: Tween
var rail_pose: Quaternion = Quaternion.IDENTITY
var _rail_cote: float = 1.0
var _rail_pose_weight: float = 0.0
var _rail_pose_cible: Quaternion = Quaternion.IDENTITY
var _rail_weight_tween: Tween
var _rail_cote_cible: float = 1.0

class BasicTrick:
	var trick_name: String
	var q: Quaternion
	var tween: Tween

	func _init(_trick_name: String) -> void:
		trick_name = _trick_name
		q = Quaternion.IDENTITY
		
static var is_on_rail: bool = false

var active_tricks: Array[BasicTrick]
var current_trick_duration: float = 0
var current_anim_duration: float = 0

var trick_catched: bool = true
var catch_pressed: bool = false

var parent: Node3D

var lane_tween: Tween

var current_rail: Path3D = null
var prev_rail: Path3D = null

var _mesh_instance_scale: Vector3

func _ready() -> void:
	_mesh_instance_scale = mesh_instance.scale
	TrickManager.trick_performed.connect(_on_trick)
	TrickManager.trick_ended.connect(_on_trick_end)
	TrickManager.trick_catched.connect(_on_trick_catched)
	TrickManager.go_left.connect(_on_go_left)
	TrickManager.go_right.connect(_on_go_right)
	parent = get_parent()
	parent.on_fall.connect(TrickManager.on_airborne)
	parent.on_grounded.connect(TrickManager.on_ground)
	
func touches_path(rail: Path3D) -> float:
	var zyt: Vector3 = get_zy_from_x(rail, parent.global_position.x)
	if zyt.z == -1:
		return -INF
	#var voie: float = lane_position * signf(parent.global_position.z)
	#if absf(zyt.x - voie) > lane_position:
		#return false
	var dy: float = parent.global_position.y - zyt.y
	var dz: float = abs(parent.global_position.z - zyt.x) 
	if dz <= rail_z_thresh and dy <= rail_y_thresh and dy >= -2:
		return zyt.z
	else:
		return -INF

func _find_rail() -> void:
	current_rail = null
	var closest_t: float = -INF
	for sec in MapLoader.instance._spawned_sections:
		for rl in sec.rails:
			var current_t := touches_path(rl)
			if closest_t < current_t and current_t <= 2:
				current_rail = rl
				closest_t = current_t
				
func get_zy_from_x(rail: Path3D, target_x: float) -> Vector3:
	if not rail: return Vector3(0,0,-1)
	#rail.curve.bake_interval = 0.1
	
	var tf = rail.global_transform
	var pts = rail.curve.get_baked_points()
	if pts.is_empty(): return Vector3(0,0,-1)

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
	return Vector3(lerp(p1.z, p2.z, t), lerp(p1.y, p2.y, t) - rail_y_offset, t) # Returns global Vector2(Z, Y)

func _rail_courbure(rail: Path3D, x: float) -> float:
	var h: float = 1.0
	var a: float = get_zy_from_x(rail, x - h).x
	var b: float = get_zy_from_x(rail, x).x
	var c: float = get_zy_from_x(rail, x + h).x
	var pente: float = (c - a) / (2.0 * h)
	var d2: float = (c - 2.0 * b + a) / (h * h)
	return d2 / pow(1.0 + pente * pente, 1.5)

func _rail_cote_interieur(rail: Path3D) -> float:
	var pts: PackedVector3Array = rail.curve.get_baked_points()
	if pts.size() < 3:
		return 1.0
	var tf: Transform3D = rail.global_transform
	var debut: Vector3 = tf * pts[0]
	var fin: Vector3 = tf * pts[-1]
	var milieu: Vector3 = tf * pts[pts.size() / 2]
	var bombe: float = milieu.z - (debut.z + fin.z) / 2.0
	return -1.0 if bombe > 0.0 else 1.0

func _rail_pente(rail: Path3D, x: float) -> float:
	var h: float = 0.3
	var a: Vector3 = get_zy_from_x(rail, x - h)
	var b: Vector3 = get_zy_from_x(rail, x + h)
	return (b.x - a.x) / (2.0 * h)

func _calc_rail_pose() -> Quaternion:
	var angles: Vector3 = rail_slide_angles * PI / 180.0
	angles.x *= _rail_cote  # inclinaison vers l'intérieur du virage
	if rail_suivre_direction:
		angles.y -= atan(_rail_pente(current_rail, parent.global_position.x))
	return Quaternion.from_euler(angles)

func _tween_rail_weight(cible: float) -> void:
	if _rail_weight_tween and _rail_weight_tween.is_valid():
		_rail_weight_tween.kill()
	_rail_weight_tween = create_tween()
	_rail_weight_tween.tween_property(self, "_rail_pose_weight", cible, rail_pose_duration).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)

func _rail_vitesse_verticale(rail: Path3D, x: float) -> float:
	var h: float = 0.3
	var a: float = get_zy_from_x(rail, x - h).y
	var b: float = get_zy_from_x(rail, x + h).y
	var pente: float = (b - a) / (2.0 * h)   # dy/dx
	var v: Vector3 = MapLoader.s_speed
	return pente * v.x - v.y

func _rail_logic(delta: float):
	_find_rail()
	is_on_rail = false
	if current_rail:
		var zyt := get_zy_from_x(current_rail, parent.global_position.x)
		if zyt.z > 1 or zyt.z < 0: 
			# When Fall from rail
			$"../../SFX/Rail".stop()
			if zyt.z > 1 and prev_rail: parent.reset_velocities()
			current_rail = null
			prev_rail = current_rail
			return
			
		if not TrickManager.catch_pressed():
			# When arrive on rail without catch
			if not prev_rail: 
				$"../../SFX/Rail".stop()
				if parent.just_impulsed(): 
					current_rail = null
					return
				MapLoader.instance.stop()
			# When jump from rail
			else: 
				$"../../SFX/Rail".stop()
				parent.reset_velocities()
				var impulse := _rail_vitesse_verticale(current_rail, parent.global_position.x)
				impulse = min(impulse, 40)
				parent.apply_impulse(impulse, impulse)
				current_rail = null
				prev_rail = current_rail
				return
				
		if not prev_rail:
			$"../../SFX/Rail".play(0.)
			TrickManager._nb_tricks_since_grounded += 1
			TrickManager._possible_tricks = ["instable_saut"]
			_rail_offset = Vector2(parent.global_position.z - zyt.x, parent.global_position.y - zyt.y)
			_rail_offset_weight = 1.0
			if _rail_tween and _rail_tween.is_valid():
				_rail_tween.kill()
			_rail_tween = create_tween()
			_rail_tween.tween_property(self, "_rail_offset_weight", 0.0, rail_entry_duration) \
					.set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
					
			if lane_tween and lane_tween.is_valid():
				lane_tween.kill()
				
		TrickManager._xp_buffer += rail_xp_rate * delta
				
				
		is_on_rail = true
				
		parent.reset_orientation()
		parent.global_position.z = zyt.x + _rail_offset.x * _rail_offset_weight
		parent.global_position.y = zyt.y + _rail_offset.y * _rail_offset_weight
			
	if current_rail == null and prev_rail != null:
		parent.reset_velocities()
			
	prev_rail = current_rail


func _process(delta: float) -> void:	
	if ((parent.is_grounded() or parent.is_mesh_colliding()) and not trick_catched):
		MapLoader.instance.stop()
		
	var etait_sur_rail: bool = prev_rail != null
	_rail_logic(delta)
	var est_sur_rail: bool = current_rail != null
	var inversion: float = -1.0 if false else 1.0

	if est_sur_rail:
		var k: float = _rail_courbure(current_rail, parent.global_position.x)
		if absf(k) > rail_seuil_courbure:
			_rail_cote_cible = signf(k) * inversion

		if not etait_sur_rail:
			if absf(k) <= rail_seuil_courbure:
				_rail_cote_cible = _rail_cote_interieur(current_rail) * inversion
			_rail_cote = _rail_cote_cible
			_tween_rail_weight(1.0)

		_rail_cote = move_toward(_rail_cote, _rail_cote_cible, delta * 2.0 / rail_duree_changement_cote)
		_rail_pose_cible = _calc_rail_pose()

	elif etait_sur_rail:
		_tween_rail_weight(0.0)

	rail_pose = Quaternion.IDENTITY.slerp(_rail_pose_cible, _rail_pose_weight)
	
	# Snap To Lane
	if (abs(parent.global_position.z - lane_position) > 0.1 
		and (lane_tween == null or !lane_tween.is_valid() or !lane_tween.is_running())
		and not current_rail
	):
		if parent.global_position.z <= 0: _on_go_left()
		else: _on_go_right()
	
	var current_rotation: Quaternion = rail_pose
	for trick in active_tricks:
		current_rotation *= trick.q
		current_rotation = current_rotation.normalized()
	parent.set_rot(Basis(current_rotation))

func tween_axis_angle(object: Object, property: String, axis: Vector3, target_angle: float, duration: float, curve: Curve = null) -> Tween:
	var normalized_axis = axis.normalized()
	var tween = create_tween()
	
	tween.tween_method(
		func(progress: float):
			var curve_val = curve.sample(progress) if curve else progress
			var current_angle = target_angle * curve_val
			var q = Quaternion(normalized_axis, current_angle)
			object.set(property, q), 
			0.0, 1.0, duration)
	return tween

func _confirm_trick():
	mesh_instance.scale = _mesh_instance_scale * 1.3
	create_tween().tween_property(mesh_instance, "scale", _mesh_instance_scale, 0.15).set_trans(Tween.TRANS_CIRC).set_ease(Tween.EASE_OUT_IN)

func set_trick_vfx(t : float) -> void:
	$"../TrickVFX".material_override.set_shader_parameter("imp", t)

func _on_trick(_name: String, _accuracy: float) -> void:
	catch_pressed = false
	var _anim :Anim = TrickManager.get_resource(_name)
	var _duration :float = TrickManager.get_trick_duration(_name)
	current_trick_duration = _duration
	current_anim_duration = _anim.animation_duration
	
	if _anim.reset_velocities: parent.reset_velocities()
	get_parent().apply_impulse(_anim.impulse_back, _anim.impulse_front)

	_confirm_trick()

	var already_active := active_tricks.find_custom(func(trick): return trick.trick_name == _name)
	if abs(_anim.rot_angle) > 0.001:
		trick_catched = false
		var trick := BasicTrick.new(_name)
		print(_name)
		active_tricks.push_back(trick)
		trick.tween = tween_axis_angle(trick, "q", _anim.rot_axis, deg_to_rad(_anim.rot_angle), _anim.animation_duration, _anim.curve)
	
	if not _name in ["catch", "instable_saut"]:
		var tw:Tween = create_tween()
		$"../TrickVFX".visible = true
		tw.tween_method(set_trick_vfx, 0., 1., 0.3).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
		await tw.finished
		$"../TrickVFX".visible = false

func _on_trick_end() -> void:
	active_tricks.clear()
	
func _cancel_tricks() -> void:
	if active_tricks.is_empty():
		return

	var tween := create_tween().set_parallel(true)
	for trick in active_tricks:
		if trick.tween and trick.tween.is_valid():
			trick.tween.kill()

		var depart: Quaternion = trick.q
		tween.tween_method(
			func(t: float) -> void: trick.q = depart.slerp(Quaternion.IDENTITY, t),
			0.0, 1.0, 0.05
		)

	var a_retirer: Array[BasicTrick] = active_tricks.duplicate()
	tween.chain().tween_callback(func() -> void:
		for trick in a_retirer:
			active_tricks.erase(trick)
	)
	
func _catch_trick(_dt: float) -> void:
	if not catch_pressed:
		parent.velocity_back = min(parent.velocity_back, catch_start_speed)
		parent.velocity_front = min(parent.velocity_front, catch_start_speed)
		#_confirm_trick()
		$"../../SFX/Catch".play(0.05)
		if parent.impulsed_timer <= 0: 
			TrickManager.validate_xp()
		
	parent.apply_impulse(-_dt * catch_fall_speed, -_dt * catch_fall_speed)
	trick_catched = true
	catch_pressed = true

func can_catch() -> bool:
	var prog := TrickManager.get_progression()
	return prog >= trick_cancel_end and prog > trick_catchable_start and not parent.is_grounded()
	
func _on_trick_catched(_progression: float, _dt: float) -> void:
	# On Cancel
	if _progression < trick_cancel_end:
		_cancel_tricks()
		return
		
	#if (_progression > trick_catchable_start and parent.is_grounded() and parent.impulsed_timer <= 0):
		#TrickManager.validate_xp()
		
	# Successful Catch
	if _progression > trick_catchable_start and not parent.is_grounded():
		_cancel_tricks()
		_catch_trick(_dt)
		return
		
	# if already catched and grounded, nothing happens
	if trick_catched: return
		
	print(_progression)
	MapLoader.instance.stop((Vector3.DOWN + Vector3.BACK * 2).normalized() * 8)
	
func _on_go_left(force: bool = false) -> void:
	if not force and parent.is_grounded(): return
	lane_tween = create_tween()
	lane_tween.tween_property(parent, "global_position:z", -lane_position, 0.4).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
	
func _on_go_right(force: bool = false) -> void:
	if not force and parent.is_grounded(): return
	lane_tween = create_tween()
	lane_tween.tween_property(parent, "global_position:z", lane_position, 0.4).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
