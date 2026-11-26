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

var _outline_tween: Tween

func _ready() -> void:
	_orientation_rot = Basis.from_euler(Vector3(0.0, deg_to_rad(orientation), 0.0))
	GameManager.instance.on_death.connect(reset)
	TrickManager.trick_performed.connect(_on_trick_performed)

func _process(_delta: float) -> void:
	if GameManager.instance.game_paused or not GameManager.instance.game_started: return

	var catching: bool = Input.is_action_pressed("F") and Input.is_action_pressed("H")
	var fh: bool = catching or skate_physics.is_grounded() or rail_controller.is_on_rail
	doigts_idle.visible = fh
	doigts_up.visible = not fh
	
	var rot: Basis = _rail_rot if rail_controller.is_on_rail else Basis.IDENTITY
	
	doigts_idle.basis = parent.global_basis.inverse() * _orientation_rot * rot
	doigts_up.basis = parent.global_basis.inverse() * _orientation_rot * rot

func _on_trick_performed(_name: String) -> void:
	if _outline_tween and _outline_tween.is_valid(): 
		_outline_tween.kill()

	if _name == TrickManager.jump_trick_name: 
		doigts_up.visual.material_overlay.stencil_outline_thickness = 0.3
		return

	doigts_up.visual.material_overlay.stencil_outline_thickness = 0.0
	_outline_tween = create_tween()
	_outline_tween.tween_property(doigts_up.visual.material_overlay, "stencil_outline_thickness", 0.3, 0.15)
	_outline_tween.set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)

func reset() -> void:
	if !is_inside_tree(): return
	if _outline_tween: _outline_tween.kill()
	doigts_up.visual.material_overlay.stencil_color = Color.WHITE
	doigts_up.visual.material_overlay.stencil_outline_thickness = 0.3
	doigts_idle.visible = false
	doigts_up.visible = true
	doigts_idle.basis = parent.global_basis.inverse() * _orientation_rot
	doigts_up.basis = parent.global_basis.inverse() * _orientation_rot