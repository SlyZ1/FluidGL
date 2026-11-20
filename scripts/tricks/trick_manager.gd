extends Node

@export var jump_trick_name: String

signal trick_performed(_name: String)
signal trick_catched(_dt: float)
signal trick_validated(_num_combo: int)

signal on_reset
signal on_catch_pressed

signal on_nb_trick_increased(_nb_tricks: int)

var _current_trick : String = ""
var _possible_tricks : Array[String] = []
var _tricks : TrickGraph
var _started := false
var _nb_tricks_since_grounded := 0
var _time_since_different_input: float

var _last_input := ""
var _last_input_executed := ""

var _catch_pressed: bool = true


func reset() -> void:
	_catch_pressed = false
	_current_trick = ""
	_started = false
	_possible_tricks = []
	_nb_tricks_since_grounded = 0
	on_nb_trick_increased.emit(_nb_tricks_since_grounded)
	on_reset.emit()

func on_ground() -> void:
	reset()
	_possible_tricks = _tricks.get_neighbours("")

func add_trick_count() -> void:
	_nb_tricks_since_grounded += 1
	on_nb_trick_increased.emit(_nb_tricks_since_grounded)


func get_trick_duration(_name: String) -> float:
	return _tricks.get_node(_name).duration

func get_resource(_name : String) -> Anim:
	return load("res://tricks/" + _name + ".tres") as Anim

func set_possible_tricks(possible_tricks: Array[String]) -> void:
	_possible_tricks = possible_tricks

func on_airborne() -> void:
	_possible_tricks = _tricks.get_neighbours(jump_trick_name)
	_started = true



func _ready() -> void:
	_tricks = TrickGraph.new("res://combo_graph.json")
	reset()

func _process(dt: float) -> void:
	var input_string := _compute_input()
	_set_catch_pressed(input_string)
	_get_trick_performed(input_string, dt)


func _get_trick_performed(input_string: String, dt: float) -> void:
	if not GameManager.instance or not GameManager.instance.game_started: return

	if not _started:
		if get_catch_pressed():
			_started = true
			_time_since_different_input = 0
		else:
			return

	# Applying input lag
	_update_input_lag(input_string, dt)
	if _time_since_different_input < 0.032:
		return
		
	# If catching the skate
	if get_catch_pressed():
		_last_input_executed = ""
		trick_catched.emit(dt)
	else:
		_try_trick(input_string)

	
func _compute_input() -> String:
	var fhybrv := [
		Input.is_action_pressed("F"), 
		Input.is_action_pressed("H"),
		Input.is_action_pressed("Y"), 
		Input.is_action_pressed("B"), 
		Input.is_action_pressed("R"),
		Input.is_action_pressed("V")
	]
	
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

func _update_input_lag(input_string: String, dt: float) -> void:
	if _last_input != input_string:
		_time_since_different_input = 0.0
		_last_input = input_string
	else:
		_time_since_different_input += dt

func _try_trick(input_string: String) -> void:
	for trick_name : String in _possible_tricks:
		var trick := _tricks.get_node(trick_name)
		if trick.input == input_string:
			if trick.input == _last_input_executed:
				break
			_last_input_executed = trick.input
			_trick_performed(trick_name)
			break

func _trick_performed(_name: String) -> void:
	add_trick_count()
	XpManager.add_buffered_xp(50 * _nb_tricks_since_grounded)

	_current_trick = _name
	_possible_tricks = _tricks.get_neighbours(_name)

	trick_performed.emit(_name)
	trick_validated.emit(_nb_tricks_since_grounded)