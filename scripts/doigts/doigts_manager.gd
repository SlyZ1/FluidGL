extends Node

@export var doigts_idle: Doigt
@export var doigts_up: Doigt
@export var anim_controller: AnimController
@export var rail_controller: RailController
@export var skate_physics: SkatePhysics
@export var parent: Node3D
@export_range(-180, 180) var orientation: float = -90

var _orientation_rot: Basis
const _rail_rot: Basis = Basis(Vector3.UP, -PI / 2.0)

func _ready() -> void:
	_orientation_rot = Basis.from_euler(Vector3(0.0, deg_to_rad(orientation), 0.0))

func _process(_delta: float) -> void:
	var fh: bool = TrickManager.get_catch_pressed() or skate_physics.is_grounded()
	doigts_idle.visible = fh
	doigts_up.visible = not fh
	doigts_idle.visible = fh
	doigts_up.visible = not fh
	
	var rot: Basis = _rail_rot if rail_controller.is_on_rail else Basis.IDENTITY
	
	doigts_idle.basis = parent.global_basis.inverse() * _orientation_rot * rot
	doigts_up.basis = parent.global_basis.inverse() * _orientation_rot * rot
	
	if anim_controller.can_catch():
		doigts_up.visual.material_overlay.stencil_color = Color.WHITE
		doigts_up.visual.material_overlay.stencil_outline_thickness = 0.3
	else:
		doigts_up.visual.material_overlay.stencil_outline_thickness = 0.0
