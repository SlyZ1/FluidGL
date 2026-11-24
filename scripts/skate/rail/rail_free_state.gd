extends RailControllerState
class_name RailFreeState

# private
var _touches_rail: Callable
var _rail_found: Path3D
var _previous_rail: Path3D

var _rail_y_thresh_enter: float
var _rail_y_thresh_die: float
var _rail_y_thresh_neg: float
var _rail_exit_timer: float

# protected
func exit(rail_controller: RailController) -> void:
	rail_controller.current_rail = _rail_found

func enter(rail_controller: RailController) -> void:
	_touches_rail = rail_controller._touches_rail
	_previous_rail = rail_controller.current_rail
	_rail_y_thresh_enter = rail_controller.rail_y_thresh_enter
	_rail_y_thresh_die = rail_controller.rail_y_thresh_die
	_rail_y_thresh_neg = rail_controller.rail_y_thresh_neg
	_rail_exit_timer = rail_controller.rail_exit_time

func state() -> RailController.RailState:
	return RailController.RailState.FREE

func _filter_rail(dy: float, rail: Path3D) -> Path3D:
	if (dy <= _rail_y_thresh_die and dy >= _rail_y_thresh_neg and not TrickManager.get_catch_pressed()):
		if rail == _previous_rail or _rail_exit_timer > 0:
			_previous_rail = rail
			return rail
		GameManager.instance.death()
		return null
	if dy <= _rail_y_thresh_enter and TrickManager.get_catch_pressed():
		return rail
	return null

func _find_rail(pos: Vector3) -> Path3D:
	for section in MapLoader.instance._spawned_sections:
		for rail in section.rail_containers:
			var dy: float = _touches_rail.call(rail, pos)
			var rail_found: Path3D = _filter_rail(dy, rail.path3D)
			if rail_found: return rail_found
	return null


func check_transitions(skate_physics: SkatePhysics, _dt: float) -> RailController.RailState:
	_rail_exit_timer -= _dt
	if skate_physics.is_grounded(): 
		return RailController.RailState.GROUNDED


	_rail_found = _find_rail(skate_physics.global_position)  
	if _rail_found == _previous_rail: return state()
	_previous_rail = null

	if _rail_found: return RailController.RailState.GRINDING

	return state()
