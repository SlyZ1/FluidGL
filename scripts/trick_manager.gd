extends Node

signal trick_performed(_name: String, _accuracy: float)
signal trick_ended
signal trick_catched(_progression: float, _dt: float)
signal trick_validated(_num_combo: int)

signal go_left
signal go_right

signal xp_changed(_new_val: int, _out_of: int, _total_points: int)
signal tier_changed(_new_tier: int)

var _current_trick : String = ""
var _possible_tricks : Array[String] = []
var _tricks : TrickGraph
var _started := false
var _nb_tricks_since_grounded := 0

var _last_input := ""
var _last_input_executed := ""

const _xp_per_tier = [100, 200, 400, 800, 1600]
var _xp := 0
var _points := 0
var _tier := 0

var bullet_time_curve : Curve = preload("res://prefabs/bullet_time_curve.tres")
const bullet_time_duration := 0.5
var bullet_timer : Timer

func _reset() -> void:
	_current_trick = ""
	_started = false
	_possible_tricks = _tricks.get_neighbours("")
	_nb_tricks_since_grounded = 0
	if not $TrickTiming.is_stopped():
		$TrickTiming.start()
		$TrickTiming.stop()

func _ready() -> void:
	_tricks = TrickGraph.new("res://combo_graph.json")
	bullet_timer = Timer.new()
	bullet_timer.one_shot = true
	bullet_timer.ignore_time_scale = true
	add_child(bullet_timer)
	_reset()
	
func _compute_input() -> String:
	var fhybrv := [Input.is_action_pressed("F"), Input.is_action_pressed("H"), Input.is_action_pressed("Y"), Input.is_action_pressed("B"), Input.is_action_pressed("R"), Input.is_action_pressed("V")]
	
	var input_string := "fhybrv"
	for i : int in range(6):
		if fhybrv[i]:
			input_string[i] = input_string[i].to_upper()
	return input_string
	
func catch_pressed() -> bool:
	var input_string := _compute_input()
	return input_string == "FHybrv"

func _process(delta: float) -> void:
	if MapLoader.instance: if not MapLoader.instance.is_running: return
	
	if not bullet_timer.is_stopped() and not _current_trick in ["instable_saut", "debut_saut"] and MapLoader.instance.is_running:
		Engine.time_scale = bullet_time_curve.sample((bullet_timer.wait_time - bullet_timer.time_left) / bullet_timer.wait_time)
	else:
		Engine.time_scale = 1
		
	var input_string := _compute_input()

	# Start tricks
	if catch_pressed() and not _started:
		_started = true
		$InputLag.start()
		print("GO !")

	# Set input lag when changing inputs
	if _last_input != input_string:
		$InputLag.start()
	_last_input = input_string
	
	# Applying input lag
	if not (_started and $InputLag.is_stopped()):
		return
		
	# If catching the skate
	if catch_pressed():
		var t: float = ($TrickTiming.wait_time - $TrickTiming.time_left) / $TrickTiming.wait_time
		trick_catched.emit(t, delta)
		_last_input_executed = ""
		return
		
	# If going left
	if input_string == "fhYbRv":
		print("Going Left")
		go_left.emit()
		return
	# If going right
	if input_string == "fhyBrV":
		print("Going Right")
		go_right.emit()
		return
		
	
	for trick_name : String in _possible_tricks:
		var trick := _tricks.get_node(trick_name)
		if trick.input == input_string:
			if trick.input == _last_input_executed:
				break
			_last_input_executed = trick.input
			var tot: float = $TrickTiming.wait_time
			var t: float = $TrickTiming.time_left
			
			var good_timing: float = (t - tot / 2) / tot # between [-0.5, 0.5]
			good_timing *= 5
			good_timing = exp(-good_timing * good_timing)
			_trick_performed(trick_name, good_timing)
			break
			
func get_time_left(stopped_val: float = -INF) -> float:
	if $TrickTiming.is_stopped():
		return stopped_val
	return $TrickTiming.time_left

func _trick_performed(_name: String, _accuracy: float) -> void:
	$InputLag.start()
	bullet_timer.stop()
	bullet_timer.start(bullet_time_duration)
	_nb_tricks_since_grounded += 1

	var increased_by: int = 50 * _nb_tricks_since_grounded
	_xp += increased_by
	_points += increased_by

	while _xp >= _xp_per_tier[_tier] and _tier < _xp_per_tier.size() - 1:
		_xp -= _xp_per_tier[_tier]
		_tier += 1
		tier_changed.emit(_tier)

	xp_changed.emit(float(_xp) / _xp_per_tier[_tier], increased_by, _points)
	
	_current_trick = _name
	_possible_tricks = _tricks.get_neighbours(_name)

	$TrickTiming.start(get_trick_duration(_name))

	trick_performed.emit(_name, _accuracy)
	trick_validated.emit(_nb_tricks_since_grounded)
	

func _trick_ended() -> void:
	if not _started: return
	print("trick ended")
	trick_ended.emit()
	_possible_tricks = []

func get_trick_duration(_name: String) -> float:
	return _tricks.get_node(_name).duration

func get_resource(_name : String) -> Anim:
	return load("res://tricks/" + _name + ".tres")
	
func on_airborne() -> void:
	_possible_tricks = _tricks.get_neighbours("instable_saut")
	_started = true
	
func on_ground() -> void:
	_reset()
	
func on_successful_catch() -> void: 
	#trick_validated.emit()
	$TrickTiming.stop()
