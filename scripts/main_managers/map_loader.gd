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
@export_range(0,1) var empty_ratio: float = 0.3

var is_running: bool = true
var ready_to_restart := false

var _step_accumulateur: float = 0

var speed_vector: Vector3 = Vector3.ZERO

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
	TrickManager.xp_changed.connect(_on_xp_changed)
	GameManager.instance.on_stop.connect(_on_stop)
	GameManager.instance.on_start.connect(_on_start)

func _on_xp_changed(_new_val: int, _tier: int, _nb_tricks_since_grounded: int, _out_of: int, _total_points: int, _with_popup: bool):
	_speed_xp_accel = 1 + float(_total_points) / 1000 * speed_increase_rate
	
func _on_stop() -> void:
	create_tween().tween_property(self, "speed", 0.01, 2.0)
	
func _on_start() -> void:
	_speed_xp_accel = 1

func right() -> Vector3:
	return Basis.from_euler(Vector3(0, 0, deg_to_rad(angle))) * Vector3.RIGHT
	
func up() -> Vector3:
	return Basis.from_euler(Vector3(0, 0, deg_to_rad(angle))) * Vector3.UP

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
	new_section.snap(snap_to, container)
	container.add_child(new_section)
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

func control_speed(delta: float):
	for section: Section in _spawned_sections:
		section.position -= speed * _speed_xp_accel * Vector3.RIGHT * delta
	if _next_spawn: _next_spawn.position -= speed * _speed_xp_accel * Vector3.RIGHT * delta
	if _spawned_sections.size() > 0: speed_vector = speed * _spawned_sections[0].global_basis.x * _speed_xp_accel

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
	container.rotation_degrees.z = angle
	_control_spawn()