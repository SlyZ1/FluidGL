extends Node

class_name RailController

@export var lane_snapper: LaneSnapper
@export var anim_controller: AnimController
@export var rail_xp_rate: float = 1
@export var rail_z_thresh: float = 2.5
@export var rail_y_thresh: float = 0.75
@export var rail_y_offset: float = -0.3
@export var rail_entry_duration: float = 0.15
@export var rail_slide_angles: Vector3 = Vector3(-20, 90, 0)
@export var rail_pose_duration: float = 0.12
@export var rail_suivre_direction: bool = true
@export var rail_seuil_courbure: float = 0.02
@export var rail_duree_changement_cote: float = 0.25

var _rail_offset: Vector2 = Vector2.ZERO
var _rail_offset_weight: float = 0.0
var _rail_tween: Tween
var _rail_cote: float = 1.0
var _rail_pose_weight: float = 0.0
var _rail_pose_cible: Quaternion = Quaternion.IDENTITY
var _rail_weight_tween: Tween
var _rail_cote_cible: float = 1.0

var _parent: Node3D

var _current_rail: Path3D
var _prev_rail: Path3D

var is_on_rail: bool
var _exit_rail_timer: float = 0

func _ready() -> void:
	_parent = get_parent()

func touches_path(rail: Path3D) -> float:
	var zyt: Vector3 = get_zy_from_x(rail, _parent.global_position.x)
	if zyt.z == -1:
		return -INF
	var dy: float = _parent.global_position.y - zyt.y
	var dz: float = abs(_parent.global_position.z - zyt.x) 
	if dz <= rail_z_thresh and dy <= rail_y_thresh and dy >= -2:
		return zyt.z
	else:
		return -INF

func _find_rail() -> void:
	_current_rail = null
	var closest_t: float = -INF
	for sec in MapLoader.instance._spawned_sections:
		for rl in sec.rails:
			var current_t := touches_path(rl)
			if closest_t < current_t and current_t <= 2:
				_current_rail = rl
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
		angles.y -= atan(_rail_pente(_current_rail, _parent.global_position.x))
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
	if _current_rail:
		var zyt := get_zy_from_x(_current_rail, _parent.global_position.x)
		if zyt.z > 1 or zyt.z < 0: 
			# When Fall from rail
			$"../../SFX/Rail".stop()
			if zyt.z > 1 and _prev_rail: _parent.reset_velocities()
			_current_rail = null
			_prev_rail = _current_rail
			return
			
		if not TrickManager.get_catch_pressed():
			# When arrive on rail without catch
			if not _prev_rail: 
				$"../../SFX/Rail".stop()
				if _exit_rail_timer > 0: 
					_current_rail = null
					return
				MapLoader.instance.stop()
			# When jump from rail
			else: 
				$"../../SFX/Rail".stop()
				_parent.reset_velocities()
				var impulse := _rail_vitesse_verticale(_current_rail, _parent.global_position.x)
				impulse = min(impulse, 40)
				_parent.apply_impulse(impulse, impulse)
				_current_rail = null
				_prev_rail = _current_rail
				_exit_rail_timer = 0.1
				return
				
		if not _prev_rail:
			$"../../SFX/Rail".play(0.)
			TrickManager.add_trick_count()
			TrickManager.set_possible_tricks(["instable_saut"])
			_rail_offset = Vector2(_parent.global_position.z - zyt.x, _parent.global_position.y - zyt.y)
			_rail_offset_weight = 1.0
			if _rail_tween and _rail_tween.is_valid():
				_rail_tween.kill()
			_rail_tween = create_tween()
			_rail_tween.tween_property(self, "_rail_offset_weight", 0.0, rail_entry_duration) \
					.set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
					
			lane_snapper.kill_tween()
				
		TrickManager._xp_buffer += rail_xp_rate * delta
				
		is_on_rail = true
				
		_parent.reset_orientation()
		_parent.global_position.z = zyt.x + _rail_offset.x * _rail_offset_weight
		_parent.global_position.y = zyt.y + _rail_offset.y * _rail_offset_weight
			
	if _current_rail == null and _prev_rail != null:
		_parent.reset_velocities()
			
	_prev_rail = _current_rail
	
func _process(dt: float) -> void:
	if _exit_rail_timer > 0: _exit_rail_timer -= dt
	
	var etait_sur_rail: bool = _prev_rail != null
	_rail_logic(dt)
	var est_sur_rail: bool = _current_rail != null
	var inversion: float = -1.0 if false else 1.0

	if est_sur_rail:
		var k: float = _rail_courbure(_current_rail, _parent.global_position.x)
		if absf(k) > rail_seuil_courbure:
			_rail_cote_cible = signf(k) * inversion

		if not etait_sur_rail:
			if absf(k) <= rail_seuil_courbure:
				_rail_cote_cible = _rail_cote_interieur(_current_rail) * inversion
			_rail_cote = _rail_cote_cible
			_tween_rail_weight(1.0)

		_rail_cote = move_toward(_rail_cote, _rail_cote_cible, dt * 2.0 / rail_duree_changement_cote)
		_rail_pose_cible = _calc_rail_pose()

	elif etait_sur_rail:
		_tween_rail_weight(0.0)

	anim_controller.set_default_trick_rotation(Quaternion.IDENTITY.slerp(_rail_pose_cible, _rail_pose_weight))
	lane_snapper.toggle(not is_on_rail)
