extends Node

@export var xp_per_tiers : Array[int] = [100, 200, 400, 800, 1600]

signal points_changed(_points_added: int, _total_points: int)
signal xp_changed(_xp_added: int, _tier_progression: float, _tier: int)
signal tier_changed(_new_tier: int)

var _tier := 0
var _points := 0
var _xp := 0.0
var _xp_buffer := 0.0


func add_buffered_xp(xp: float) -> void:
	_xp_buffer += xp * (_tier + 1)

func validate_buffered_xp() -> void:
	_add_xp(_xp_buffer)
	_xp_buffer = 0

func get_points() -> int:
	return _points



func _process(dt: float) -> void:
	var decrease := (pow(_tier, 3) * 0.3 + 1) * 100 * dt
	decrease = max(0, decrease)
	_add_xp(-decrease)



func _add_xp(xp_added: float):
	var points_added: int = int(max(xp_added, 0))

	_xp += xp_added
	_points += points_added

	_update_tier()

	var tier_progression: float = _xp / xp_per_tiers[_tier]
	tier_progression = clamp(tier_progression, 0, 1)

	if abs(xp_added) > 0: xp_changed.emit(xp_added, tier_progression, _tier)
	if points_added > 0: points_changed.emit(points_added, _points)


func _update_tier() -> void:
	if _xp <= 0 and _tier > 0:
		_tier -= 1
		_xp += xp_per_tiers[_tier] - 1
		tier_changed.emit(_tier)
	
	_xp = clamp(_xp, 0.0, xp_per_tiers.back())
	
	if _xp >= xp_per_tiers[_tier] and _tier < xp_per_tiers.size() - 1:
		_xp -= xp_per_tiers[_tier]
		_tier += 1
		tier_changed.emit(_tier)


func _on_restart() -> void:
	_points = 0
	_xp = 0
	_xp_buffer = 0
	_tier = 0