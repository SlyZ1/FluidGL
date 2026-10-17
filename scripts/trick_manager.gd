extends Node

signal trick_performed(_name: String, _accuracy: float)
signal trick_ended
signal xp_changed(_new_val: int, _out_of: int)
signal tier_changed(_new_tier: int)

var _current_trick : String = ""
var _possible_tricks : Array[String] = []
var _tricks : TrickGraph
var _started := false
var _nb_tricks_since_grounded := 0

var _last_input := ""

const _xp_per_tier = [100, 200, 400, 800, 1600]
var _xp := 0
var _tier := 0

func _reset() -> void:
	_current_trick = ""
	_started = false
	_possible_tricks = _tricks.get_neighbours("")
	_nb_tricks_since_grounded = 0

func _ready() -> void:
	_tricks = TrickGraph.new("res://combo_graph.json")
	_reset()

func _process(delta: float) -> void:
	var fhyb := [Input.is_action_pressed("F"), Input.is_action_pressed("H"), Input.is_action_pressed("Y"), Input.is_action_pressed("B")]
	
	var input_string := "fhyb"
	for i : int in range(4):
		if fhyb[i]:
			input_string[i] = input_string[i].to_upper()

	if input_string == "FHyb" and not _started:
		_started = true
		$InputLag.start()
		print("GO !")

	if _last_input != input_string:
		$InputLag.start()
		_last_input = input_string

	if not (_started and $InputLag.is_stopped()):
		return
	
	for trick_name : String in _possible_tricks:
		var trick := _tricks.get_node(trick_name)
		if trick.input == input_string:
			var tot: float = $TrickTiming.wait_time
			var t: float = $TrickTiming.time_left
			
			var good_timing: float = (t - tot / 2) / tot # between [-0.5, 0.5]
			good_timing *= 5
			good_timing = exp(-good_timing * good_timing)
			_trick_performed(trick_name, good_timing)
			break

func _trick_performed(_name: String, _accuracy: float) -> void:
	print(_name)
	_nb_tricks_since_grounded += 1

	var increased_by: int = 50 * _nb_tricks_since_grounded
	_xp += increased_by

	while _xp >= _xp_per_tier[_tier] and _tier < _xp_per_tier.size() - 1:
		_xp -= _xp_per_tier[_tier]
		_tier += 1
		tier_changed.emit(_tier)

	xp_changed.emit(float(_xp) / _xp_per_tier[_tier], increased_by)
	
	_current_trick = _name
	_possible_tricks = _tricks.get_neighbours(_name)

	$TrickTiming.start(get_trick_duration(_name))

	trick_performed.emit(_name, _accuracy)

func _trick_ended() -> void:
	print("flop")
	trick_ended.emit()
	_reset()

func get_trick_duration(_name: String):
	return _tricks.get_node(_name).duration
