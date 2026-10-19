extends Node3D

class_name MapLoader

@export var spawn_distance = 20
@export var kill_distance = 20
@export var angle: float = -15
@export var speed: float = 1
@export_range(0,1) var speed_increase_rate: float = 1
@export var container: Node3D
@export var sections: Array[PackedScene]
@export var empty_section: PackedScene
@export var skate: Node3D
@export var ragdoll: RigidBody3D
@export var substep: float = 1.0 / 300.0
@export var max_steps_par_frame: int = 30
@export_range(0,1) var empty_ratio: float = 0.3

@onready var freezed_skate : Node3D
@onready var initial_skate_pos :Vector3 = $Skate.global_position

signal on_stop

func set_active(node: Node3D, active: bool) -> void:
	node.visible = active
	node.process_mode = Node.PROCESS_MODE_INHERIT if active else Node.PROCESS_MODE_DISABLED

var is_running: bool = true
var speed_ref : float
var ready_to_restart := false

var _step_accumulateur: float = 0

static var s_speed: Vector3 = Vector3.ZERO
static var s_angle: float = 0

var _speed_xp_accel: float = 1

var _spawned_sections: Array[Section] = []
# Section "fantome" pour le prochain spawn
var _next_spawn: Section

var waiting_for_restart := false
var previous_spawn_index: int = -1

static var instance: MapLoader

func _enter_tree() -> void:
	if instance and instance != self:
		push_warning("Plusieurs instances de MapLoader dans la scène")
	instance = self

func _exit_tree() -> void:
	if instance == self:
		instance = null

func _ready() -> void:
	_next_spawn = _new_section(Vector3.ZERO, true)
	TrickManager.trick_performed.connect(_on_trick)
	TrickManager.trick_ended.connect(_reset_quest)
	TrickManager.trick_catched.connect(_on_catch)
	TrickManager.xp_changed.connect(_on_xp_changed)
	set_active(ragdoll, false)
	set_active(skate, true)
	freezed_skate = $Skate.duplicate()
	freezed_skate.process_mode = Node.PROCESS_MODE_DISABLED
	
func _on_xp_changed(_new_val: int, _tier: int, _out_of: int, _total_points: int, _with_popup: bool):
	_speed_xp_accel = 1 + float(_total_points) / 1000 * speed_increase_rate

func stop(impulse_direction: Vector3 = Vector3.ZERO, no_ragdoll: bool = false) -> void:
	on_stop.emit()
	is_running = false
	if not no_ragdoll:
		ragdoll.global_transform = skate.global_transform
		set_active(ragdoll, true)
		
		var impulse: Vector3 = impulse_direction if impulse_direction != Vector3.ZERO else (Vector3.UP + Vector3.FORWARD).normalized() * 12
		ragdoll.apply_central_impulse(impulse)
		
		var axe := Vector3(randf_range(-1, 1), randf_range(-1, 1), randf_range(-1, 1)).normalized()
		ragdoll.apply_torque_impulse(axe * 10)
		set_active(skate, false)
		
		$RestartLag.start()

	else:
		skate.process_mode = Node.PROCESS_MODE_DISABLED

	speed_ref = speed
	create_tween().tween_property(self, "speed", 0.0, 2.0)

static func right() -> Vector3:
	return Basis.from_euler(Vector3(0, 0, deg_to_rad(s_angle))) * Vector3.RIGHT
	
static func up() -> Vector3:
	return Basis.from_euler(Vector3(0, 0, deg_to_rad(s_angle))) * Vector3.UP

func _new_section(snap_to: Vector3, empty := false) -> Section:
	var index: int = randi() % sections.size()
	for i in range(10):
		if index == previous_spawn_index:
			index = randi() % sections.size()
		else: break
	previous_spawn_index = index
	var scene: PackedScene = sections[index]
	
	empty = empty || randf() < empty_ratio
	
	var new_section: Section = (scene.instantiate() if not empty else empty_section.instantiate()) as Section
	container.add_child(new_section)
	new_section.snap(snap_to)
	new_section.visible = false
	return new_section

func _control_spawn(should_spawn_empty := false):
	# Spawn le prochain fantome
	#if waiting_for_restart: return
	if should_spawn_empty:
		_next_spawn = _new_section(_spawned_sections.back().snap_front.global_position, true)
	elif _next_spawn == null && _spawned_sections.size() > 0:
		_next_spawn = _new_section(_spawned_sections.back().snap_front.global_position, false)

	# Check si le fantome est valide
	if _next_spawn && (_next_spawn.snap_back.global_position.length_squared() <= spawn_distance * spawn_distance || should_spawn_empty):
		_next_spawn.visible = true
		_spawned_sections.append(_next_spawn)
		_next_spawn = null
		
	# Check si le plus vieux est hors cam
	var front_section: Section = _spawned_sections.front()
	if (front_section 
		and front_section.snap_front.global_position.length_squared() > kill_distance * kill_distance
		and front_section.snap_front.global_position.dot(Vector3(1,0,0)) < 0
	):
		front_section.queue_free()
		_spawned_sections.pop_front()

func _control_speed(delta: float):
	for section: Section in _spawned_sections:
		section.position -= speed * _speed_xp_accel * Vector3.RIGHT * delta
	if _next_spawn: _next_spawn.position -= speed * _speed_xp_accel * Vector3.RIGHT * delta
	if _spawned_sections.size() > 0: s_speed = speed * _spawned_sections[0].global_basis.x * _speed_xp_accel

@export var quests: Array[Quest]
var _current_quest: Quest = null
var _quest_progression := 0

func _on_trick(_name: String, _accuracy: float) -> void:
	if not _current_quest: return

	if _name == _current_quest.steps[_quest_progression]:
		get_node("UI/Input" + str(_quest_progression + 1)).correct()
		_quest_progression += 1
		TrickManager._update_xp(_current_quest.reward, 0)
		if _quest_progression >= len(_current_quest.steps):
			_move_quests(680)
			_current_quest = null
			_quest_progression = 0
		$NewQuest.start()
		$QuestEnd.stop()
		# TODO : féliciter le joueur
	else:
		_reset_quest()
		
var appear_tw : Tween
		
func _move_quests(target_y: float) -> void:
	for i in range(len(_current_quest.steps)):
		var whom = get_node("UI/Input" + str(i + 1))
		get_tree().create_tween().tween_property(whom, "position:y", target_y, 1.4).set_trans(Tween.TRANS_ELASTIC).set_ease(Tween.EASE_OUT).set_delay(0.3 * i)

func _reset_quest() -> void:
	get_node("UI/Input" + str(_quest_progression + 1)).incorrect()
	_quest_progression = 0

func _on_catch(_progression: float, _dt: float) -> void:
	_reset_quest()

func _on_new_quest_timeout() -> void:
	_current_quest = quests.pick_random()
	print(_current_quest.reward)
	for i in range(len(_current_quest.steps)):
		var input : String = TrickManager._tricks.get_node(_current_quest.steps[i]).input
		get_node("UI/Input" + str(i + 1)).set_input_ref(input)
	_move_quests(550)
	$QuestEnd.start()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	if Input.is_action_just_pressed("pause"):
		if not $UI/PauseLayer.visible:
			Engine.time_scale = 0
			$UI/PauseLayer.visible = true
		else:
			Engine.time_scale = 1
			$UI/PauseLayer.visible = false
	s_angle = angle
	container.rotation_degrees.z = angle
	_control_spawn()
	
	_step_accumulateur += delta
	var steps: int = 0
	while _step_accumulateur >= substep and steps < max_steps_par_frame:
		_control_speed(substep)
		if skate and skate.can_process() and skate.has_method("step"):
			skate.step(substep)
		_step_accumulateur -= substep
		steps += 1
	if ready_to_restart and TrickManager.catch_pressed():
		waiting_for_restart = false
		ready_to_restart = false
		
		print("RESTART")
		TrickManager._points = 0
		TrickManager._xp = 0
		TrickManager._xp_buffer = 0
		TrickManager._tier = 0
		get_tree().change_scene_to_file("res://scenes/skate_prototype.tscn")
		#_control_spawn(true)
		#_control_spawn(true)
		#_control_spawn(true)
		##var d : float = (_spawned_sections.back().snap_back.global_position).distance_to(Vector3($Camera3D.global_position.x, 0, $Camera3D.global_position.z))
			#
		#var tw1:Tween = create_tween()
		#tw1.tween_property(self, "speed", 7 * speed_ref, 1.0).set_trans(Tween.TRANS_QUAD)
		#await tw1.finished
		#var tw2:Tween = create_tween()
		#tw2.tween_property(self, "speed", speed_ref, 1.0).set_trans(Tween.TRANS_QUAD)
		#await tw2.finished
		#
		## TODO : spawn skate 2
		#skate.reset_orientation()
		#skate.reset_velocities()
		#skate.global_position = initial_skate_pos + 3 * Vector3.UP
		#skate.process_mode = Node.PROCESS_MODE_INHERIT
		#print(skate.name)
		#set_active(ragdoll, false)
		#ragdoll.global_position = skate.global_position
		#set_active(skate, true)
		#is_running = true
		
	if waiting_for_restart and not TrickManager.catch_pressed():
		ready_to_restart = true

func _on_quest_end_timeout() -> void:
	_move_quests(680)
	_current_quest = null
	_quest_progression = 0
	$NewQuest.start()

func _on_holdfh_timeout() -> void:
	$UI/Label2.visible = not $UI/Label2.visible

func _on_restart_lag_timeout() -> void:
	waiting_for_restart = true
	$UI/Label2/Timer.start()
