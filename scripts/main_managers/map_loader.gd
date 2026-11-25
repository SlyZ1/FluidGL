extends Node3D

class_name MapLoader

@export var cam: CustomCamera
@export var stop_duration: float = 1.5
@export var spawn_distance: float = 300
@export var kill_distance: float = 60
@export var angle: float = 0
@export var speed: float = 35
@export_range(0,1) var speed_increase_rate: float = 0.002
@export var sections: Array[PackedScene]
@export var empty_section: PackedScene
@export_range(0,1) var empty_ratio: float = 0.3
@export var portal : PackedScene

var ready_to_restart := false

var speed_vector: Vector3 = Vector3.ZERO

var _speed_xp_accel: float = 1
var _speed: float = 0

var _spawned_sections: Array[Section] = []
# Section "fantome" pour le prochain spawn
var _next_spawn: Section

var waiting_for_restart := false
var previous_spawn_index: int = -1

@export var nbSectionsBeforePortal := 14
var remainingSectionsBeforePortal := nbSectionsBeforePortal
signal portal_entered

static var instance: MapLoader

func _enter_tree() -> void:
	if instance and instance != self:
		push_warning("Plusieurs instances de MapLoader dans la scène")
	instance = self

func _exit_tree() -> void:
	if instance == self:
		instance = null
	TimeManager.on_substep.disconnect(_control_speed)

func _ready() -> void:
	_next_spawn = _new_section(-20 * Vector3.RIGHT, true)
	_control_speed(0.5)
	_control_spawn()
	_control_spawn(true)
	XpManager.points_changed.connect(_on_points_changed)
	GameManager.instance.on_spawn.connect(_on_start)
	TimeManager.on_substep.connect(_control_speed)
	_speed = speed 

func _process(_delta: float) -> void:
	rotation_degrees.z = angle
	_control_spawn()

func _control_spawn(should_spawn_empty := false):
	# Spawn le prochain fantome
	#if waiting_for_restart: return
	if should_spawn_empty:
		_next_spawn = _new_section(_spawned_sections.back().snap_front.global_position, true)
	elif _next_spawn == null && _spawned_sections.size() > 0:
		_next_spawn = _new_section(_spawned_sections.back().snap_front.global_position, false)

	# Check si le fantome est valide
	var custom_spawn_dist: float = spawn_distance * max(cam.get_distance_ratio() / 1.5, 1)
	if _next_spawn && (_next_spawn.snap_back.global_position.length_squared() <= custom_spawn_dist * custom_spawn_dist || should_spawn_empty):
		_next_spawn.visible = true
		_spawned_sections.append(_next_spawn)
		_next_spawn = null
		
	# Check si le plus vieux est hors cam
	var custom_kill_dist: float = kill_distance * max(cam.get_distance_ratio() / 2, 1)
	var front_section: Section = _spawned_sections.front()
	if (front_section 
		and front_section.snap_front.global_position.length_squared() > custom_kill_dist * custom_kill_dist
		and front_section.snap_front.global_position.dot(Vector3(1,0,0)) < 0
	):
		front_section.queue_free()
		_spawned_sections.pop_front()

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
	new_section.snap(snap_to, self)
	add_child(new_section)
	new_section.visible = false

	remainingSectionsBeforePortal -= 1
	if remainingSectionsBeforePortal <= 0:
		remainingSectionsBeforePortal = nbSectionsBeforePortal # c'est temporaire, en vrai ce serait a RunManager de decider
		spawnPortal(new_section)

	return new_section

func spawnPortal(new_section : Section) -> void:
	var newPortal = portal.instantiate()
	new_section.add_child(newPortal)
	newPortal.global_position = new_section.global_position
	newPortal.position.z = 6
	if randi_range(0, 1) == 1:
		newPortal.position.z *= -1
	newPortal.scale *= 80 # nsm je comprends rien a la scale de godot
	newPortal.get_node("Hitbox").area_entered.connect(_on_portal_entered)

func _on_portal_entered(_other : Area3D):
	portal_entered.emit()

func _control_speed(delta: float):
	for section: Section in _spawned_sections:
		section.position -= _speed * _speed_xp_accel * Vector3.RIGHT * delta
	if _next_spawn: _next_spawn.position -= _speed * _speed_xp_accel * Vector3.RIGHT * delta
	if _spawned_sections.size() > 0: speed_vector = _speed * _spawned_sections[0].global_basis.x * _speed_xp_accel

func _on_points_changed(_points_added: int, _total_points: int):
	_speed_xp_accel = 1 + float(_total_points) / 1000 * speed_increase_rate
	
func stop() -> void:
	await create_tween().tween_property(self, "_speed", 0.0, stop_duration).finished
	
func _on_start() -> void:
	_speed = speed

func restart_skip(duration: float) -> void:
	_previous_skip_t = 0
	_skip_dt = 0
	_control_spawn(true)
	var distance_to_skip: float = _spawned_sections.back().snap_front.global_position.x 
	distance_to_skip += _spawned_sections.back().snap_back.global_position.x
	distance_to_skip /= 2
	var tween: Tween = create_tween()
	tween.tween_method(_skip.bind(distance_to_skip), 0.0, 1.0, duration).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_IN_OUT)
	await tween.finished

var _previous_skip_t: float
var _skip_dt: float
func _skip(t: float, distance_to_skip: float) -> void:
	_skip_dt = t - _previous_skip_t
	_previous_skip_t = t
	for section: Section in _spawned_sections:
		section.position -= distance_to_skip * Vector3.RIGHT * _skip_dt
	if _next_spawn: _next_spawn.position -= distance_to_skip * Vector3.RIGHT * _skip_dt
	_control_spawn()
