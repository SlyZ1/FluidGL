extends Node

@export var doigts_idle: Doigt
@export var doigts_up: Doigt
@export var anim_controller: AnimController
@export var rail_controller: RailController

var initial_rot_finger: Basis
var initial_rot_finger_up: Basis

const ROT_RAIL := Basis(Vector3.UP, PI / 2.0)

func _ready() -> void:
	initial_rot_finger = doigts_idle.global_basis
	initial_rot_finger_up = doigts_up.global_basis

func _process(delta: float) -> void:
	var fh := TrickManager.get_catch_pressed()
	doigts_idle.visible = fh
	doigts_up.visible = not fh
	doigts_idle.visible = fh
	doigts_up.visible = not fh
	
	var rot: Basis = ROT_RAIL if rail_controller.is_on_rail else Basis.IDENTITY
	doigts_idle.global_basis = rot * initial_rot_finger
	doigts_up.global_basis = rot * initial_rot_finger_up
	
	if anim_controller.can_catch():
		doigts_up.visual.material_overlay.stencil_color = Color.WHITE
		doigts_up.visual.material_overlay.stencil_outline_thickness = 0.3 * (0.6 + 0.5 * sin(TrickManager.get_progression()))
	else:
		doigts_up.visual.material_overlay.stencil_outline_thickness = 0.0
