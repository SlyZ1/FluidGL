class_name BasicTrick

var trick_name: String
var _q: Quaternion
var _tween: Tween

func _init(_trick_name: String) -> void:
	trick_name = _trick_name
	_q = Quaternion.IDENTITY

func get_quaternion() -> Quaternion:
	return _q

func set_tween(anim: Anim, anim_controller: AnimController) -> void:
	_tween = _tween_axis_angle(anim_controller, anim.rot_axis, deg_to_rad(anim.rot_angle), anim.animation_duration, anim.curve)
	
func _tween_axis_angle(anim_controller: AnimController, axis: Vector3, target_angle: float, duration: float, curve: Curve = null) -> Tween:
	var normalized_axis: Vector3 = axis.normalized()
	var tween: Tween = anim_controller.create_tween()
	
	tween.tween_method(
		func(progress: float):
			var curve_val = curve.sample(progress) if curve else progress
			var current_angle = target_angle * curve_val
			var q = Quaternion(normalized_axis, current_angle)
			self.set("_q", q), 
			0.0, 1.0, duration)
	return tween