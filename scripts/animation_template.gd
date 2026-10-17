class_name Anim
extends Resource

@export var rot_axis: Vector3
@export var rot_angle: float
@export var impulse_front: float
@export var impulse_back: float
@export var curve: Curve

func to_quaternion() -> Quaternion:
	return Quaternion(rot_axis.normalized(), deg_to_rad(rot_angle))
