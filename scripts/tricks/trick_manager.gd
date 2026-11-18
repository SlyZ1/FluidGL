extends Node

@export var input_lag_timer: Timer
@export var trick_timing_timer: Timer
@export var jump_trick_name: String
# @export_group("Bullet time")
# @export var bullet_time_duration: float = 0.5
# @export_range(0, 1) var bullet_time_strength: float = 0.2
# @export var bullet_time_curve: Curve
# @export var bullet_timer : Timer

signal trick_performed(_name: String)
signal trick_ended
signal trick_catched(_progression: float, _dt: float)
signal trick_validated(_num_combo: int)
signal on_reset

signal on_catch_pressed

signal go_left
signal go_right

signal xp_changed(_new_val: float, _tier: int, _nb_tricks_since_grounded: int, _out_of: int, _total_points: int, _with_popup: bool)
signal tier_changed(_new_tier: int)

var _current_trick : String = ""
var _possible_tricks : Array[String] = []
var _tricks : TrickGraph
var _started := false
var _nb_tricks_since_grounded := 0
var _time_since_different_input: float

var _last_input := ""
var _last_input_executed := ""

const _xp_per_tier = [100, 200, 400, 800, 1600]
var _xp := 0.0
var _xp_buffer := 0.0
var _points := 0
var _tier := 0

var _catch_pressed: bool = true


func reset() -> void:
	_current_trick = ""
	_started = false
	_possible_tricks = _tricks.get_neighbours("")
	_nb_tricks_since_grounded = 0
	if not trick_timing_timer.is_stopped():
		trick_timing_timer.start()
		trick_timing_timer.stop()
	on_reset.emit()

func _ready() -> void:
	_tricks = TrickGraph.new("res://combo_graph.json")
	reset()
	
func _compute_input() -> String:
	var fhybrv := [Input.is_action_pressed("F"), Input.is_action_pressed("H"), Input.is_action_pressed("Y"), Input.is_action_pressed("B"), Input.is_action_pressed("R"), Input.is_action_pressed("V")]
	
	var input_string := "fhybrv"
	for i : int in range(6):
		if fhybrv[i]:
			input_string[i] = input_string[i].to_upper()
	return input_string

func get_catch_pressed() -> bool:
	return _catch_pressed
	
func _set_catch_pressed(input: String) -> void:
	if not _catch_pressed and input == "FHybrv":
		on_catch_pressed.emit()
	_catch_pressed = input == "FHybrv"

func get_progression() -> float:
	return (trick_timing_timer.wait_time - trick_timing_timer.time_left) / trick_timing_timer.wait_time

func _process(delta: float) -> void:
	var decrease := (_tier * _tier * _tier * 0.3 + 1) * 100 * delta
	_update_xp(0, max(0, decrease))

	_time_since_different_input += delta
	
	var input_string := _compute_input()
	_set_catch_pressed(input_string)

	if MapLoader.instance: if not MapLoader.instance.is_running: return
	if GameManager.instance: if not GameManager.instance.game_started: return
	
	# print(not bullet_timer.is_stopped(), " ", not _current_trick in [jump_trick_name])
	# if not bullet_timer.is_stopped() and not _current_trick in [jump_trick_name]:
	# 	print("aaaaaaaaaaaaaaa")
	# 	var bullet_curve_value: float = bullet_time_curve.sample((bullet_timer.wait_time - bullet_timer.time_left) / bullet_timer.wait_time)
	# 	Engine.time_scale = 1 - bullet_time_strength * (1 - bullet_curve_value)
	# else:
	# 	Engine.time_scale = 1
	
	# Start tricks
	if get_catch_pressed() and not _started:
		_started = true
		input_lag_timer.start()
	
	# Set input lag when changing inputs
	if _last_input != input_string:
		input_lag_timer.start()
		_time_since_different_input = 0.0
	_last_input = input_string

	# Applying input lag
	if not (_started and _time_since_different_input >= 0.032):
		return
		
	# If catching the skate
	if get_catch_pressed():
		var t: float = (trick_timing_timer.wait_time - trick_timing_timer.time_left) / trick_timing_timer.wait_time
		trick_catched.emit(t, delta)
		_last_input_executed = ""
		return
		
	# If going left
	if input_string == "fhYbRv":
		go_left.emit()
		return

	# If going right
	if input_string == "fhyBrV":
		go_right.emit()
		return
		
	for trick_name : String in _possible_tricks:
		var trick := _tricks.get_node(trick_name)
		if trick.input == input_string:
			if trick.input == _last_input_executed:
				break
			_last_input_executed = trick.input
			_trick_performed(trick_name)
			break

func get_time_left(stopped_val: float = -INF) -> float:
	if trick_timing_timer.is_stopped():
		return stopped_val
	return trick_timing_timer.time_left

func _update_xp(increased_by: float, decreased_by: float):
	var sum := increased_by - decreased_by
	_xp += sum
	_points += int(increased_by)

	if _xp <= 0 and _tier > 0:
		_xp -= 1
		_tier -= 1
		_xp += _xp_per_tier[_tier]
		tier_changed.emit(_tier)
		
	
	_xp = clamp(_xp, 0.0, _xp_per_tier.back())
	
	if _xp >= _xp_per_tier[_tier] and _tier < _xp_per_tier.size() - 1:
		_xp -= _xp_per_tier[_tier]
		_tier += 1
		tier_changed.emit(_tier)

	xp_changed.emit(_xp / _xp_per_tier[_tier], _tier, _nb_tricks_since_grounded, sum, _points, sum > 0)


func _trick_performed(_name: String) -> void:
	input_lag_timer.start()
	# bullet_timer.stop()
	# bullet_timer.start(bullet_time_duration)
	_nb_tricks_since_grounded += 1

	var increased_by: float = 50 * _nb_tricks_since_grounded * (_tier + 1)
	_xp_buffer += increased_by

	_current_trick = _name
	_possible_tricks = _tricks.get_neighbours(_name)

	trick_timing_timer.start(get_trick_duration(_name))

	trick_performed.emit(_name)
	trick_validated.emit(_nb_tricks_since_grounded)


func _trick_ended() -> void:
	if not _started: return
	trick_ended.emit()
	_possible_tricks = []
	
func set_possible_tricks(possible_tricks: Array[String]) -> void:
	_possible_tricks = possible_tricks
	trick_timing_timer.stop()

func get_possible_tricks() -> Array[String]:
	return _possible_tricks

func get_neighbours_tricks(trick_name: String) -> Array[String]:
	return _tricks.get_neighbours(trick_name)

func add_xp(xp: float) -> void:
	_xp_buffer += xp
	
func add_trick_count() -> void:
	_nb_tricks_since_grounded += 1

func get_trick_duration(_name: String) -> float:
	return _tricks.get_node(_name).duration

func get_resource(_name : String) -> Anim:
	return load("res://tricks/" + _name + ".tres") as Anim
	
func on_airborne() -> void:
	_possible_tricks = _tricks.get_neighbours(jump_trick_name)
	_started = true
	
func on_ground() -> void:
	reset()
	
func on_successful_catch() -> void: 
	#trick_validated.emit()
	trick_timing_timer.stop()
	
func validate_xp() -> void:
	_update_xp(_xp_buffer, 0)
	_xp_buffer = 0
