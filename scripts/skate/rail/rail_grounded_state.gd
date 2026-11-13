extends RailControllerState
class_name RailGroundedState

# protected
func exit(_rail_controller: RailController) -> void:
    pass

func enter(_rail_controller: RailController) -> void:
    if not TrickManager.get_catch_pressed(): GameManager.instance.stop()

func state() -> RailController.RailState:
    return RailController.RailState.GROUNDED

func check_transitions(skate_physics: SkatePhysics, _dt: float) -> RailController.RailState:
    if not skate_physics.is_grounded(): return RailController.RailState.FREE

    return state()