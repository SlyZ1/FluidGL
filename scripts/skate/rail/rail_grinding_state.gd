extends RailControllerState
class_name RailGrindingState

# private
var _update_rail: Callable
var _update_rail_pose_cile: Callable
var _rail_vitesse_verticale: Callable
var _touches_rail: Callable
var _get_point_on_rail: Callable
var _current_rail: Path3D

var _skate_point_per_second: float

var _last_rail_point: Vector3

# protected
func exit(rail_controller: RailController) -> void:
	rail_controller.current_rail = null
	rail_controller.lane_snapper.toggle(true)
	rail_controller.anim_controller.toggle(true)
	rail_controller.skate_physics.toggle_linear(true)
	rail_controller.current_rail = _current_rail
	rail_controller.is_on_rail = false

	var skate_physics: SkatePhysics = rail_controller.skate_physics
	var v_rail: float = _rail_vitesse_verticale.call(_current_rail, skate_physics.global_position.x)
	skate_physics.apply_impulse(v_rail)

	if TrickManager.get_catch_pressed():
		TrickManager.on_airborne()

	rail_controller._tween_rail_weight(0.0)

func enter(rail_controller: RailController) -> void:
	_update_rail = rail_controller._update_rail
	_update_rail_pose_cile = rail_controller._update_rail_pose_cile
	_touches_rail = rail_controller._touches_rail
	_get_point_on_rail = rail_controller._get_point_on_rail
	_current_rail = rail_controller.current_rail
	_rail_vitesse_verticale = rail_controller._rail_vitesse_verticale
	_skate_point_per_second = rail_controller.skate_point_per_second

	rail_controller.lane_snapper.toggle(false)
	rail_controller.anim_controller.toggle(false)
	rail_controller.skate_physics.toggle_linear(false)
	rail_controller.skate_physics.reset_velocities()
	rail_controller.is_on_rail = true
	TrickManager.set_possible_tricks([TrickManager.jump_trick_name])
	TrickManager.add_trick_count()
	
	rail_controller._init_rail_pose_cible()
	rail_controller._tween_rail_weight(1.0)

func state() -> RailController.RailState:
	return RailController.RailState.GRINDING

func _find_rail(pos: Vector3) -> Path3D:
	for section in MapLoader.instance._spawned_sections:
		for rail in section.rails:
			if _touches_rail.call(rail, pos): return rail
	return null

func check_transitions(skate_physics: SkatePhysics, dt: float) -> RailController.RailState:
	if _current_rail == null: return RailController.RailState.FREE
	if not TrickManager.get_catch_pressed(): return RailController.RailState.FREE

	var rail_point: Vector3 = _get_point_on_rail.call(_current_rail, skate_physics.global_position.x)
	if rail_point == Vector3.INF: 
		var new_rail: Path3D = _find_rail(skate_physics.global_position)
		if new_rail:
			_current_rail = new_rail 
			_update_rail.call(_current_rail)
			return state()

		# on tolere un ptit gap
		if abs(skate_physics.global_position.x - _last_rail_point.x) < 1:
			_last_rail_point += MapLoader.instance.speed_vector * dt
			return state()

		return RailController.RailState.FREE

	_last_rail_point = rail_point

	XpManager.add_buffered_xp(_skate_point_per_second * dt)

	skate_physics.global_position = Vector3(skate_physics.global_position.x, rail_point.y, rail_point.z)
	skate_physics.reset_velocities(true, false)
	skate_physics.reset_orientation(dt, 0.05)
	skate_physics.set_global_y(rail_point.y)

	_update_rail_pose_cile.call(dt)

	return state()
