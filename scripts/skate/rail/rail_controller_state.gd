class_name RailControllerState

# protected
func exit(_rail_controller: RailController) -> void:
    pass

func enter(_rail_controller: RailController) -> void:
    pass

func state() -> RailController.RailState:
    return RailController.RailState.GROUNDED

func check_transitions(_skate_physics: SkatePhysics, _dt: float) -> RailController.RailState:
    return state() 