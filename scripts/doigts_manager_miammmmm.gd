extends Node

@export var deux_doigts: Node3D
@export var deux_doigts_up: Node3D

var initial_rot_finger: Basis
var initial_rot_finger_up: Basis

const ROT_RAIL := Basis(Vector3.UP, PI / 2.0)

func _ready() -> void:
	initial_rot_finger = deux_doigts.global_basis
	initial_rot_finger_up = deux_doigts_up.global_basis

func _process(delta: float) -> void:
	var fh := TrickManager.get_catch_pressed()
	deux_doigts.visible = fh
	deux_doigts_up.visible = not fh
	deux_doigts.visible = fh
	deux_doigts_up.visible = not fh
	
	var rot: Basis = ROT_RAIL if AnimController.is_on_rail else Basis.IDENTITY
	deux_doigts.global_basis = rot * initial_rot_finger
	deux_doigts_up.global_basis = rot * initial_rot_finger_up
