extends RailControllerState
class_name RailGroundedState

var _touches_rail: Callable

# protected
func exit(_rail_controller: RailController) -> void:
	pass

func enter(rail_controller: RailController) -> void:
	if not TrickManager.get_catch_pressed():
		GameManager.instance.death()
	_touches_rail = rail_controller._touches_rail

func state() -> RailController.RailState:
	return RailController.RailState.GROUNDED

func _find_rail(pos: Vector3) -> bool:
	for section in MapLoader.instance._spawned_sections:
		for rail in section.rails:
			var dy: float = _touches_rail.call(rail, pos)
			if rail and dy != INF and dy < 0: return true
	return false


func check_transitions(skate_physics: SkatePhysics, _dt: float) -> RailController.RailState:
	if not skate_physics.is_grounded(): return RailController.RailState.FREE

	if _find_rail(skate_physics.global_position): GameManager.instance.death()

	return state()
