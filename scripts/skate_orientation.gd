extends MeshInstance3D

@export var ray_back: RayCast3D
@export var ray_front: RayCast3D
@export_range(0.5, 2) var ground_offset: float

var velocity_y: float = 0
var previous_position: Vector3 = Vector3.ZERO
var grounded: bool = false

func _ready() -> void:
	previous_position = global_position

func _integrate(dt: float) -> void:
	#if not grounded: velocity_y += -20 * dt
	global_position += Vector3.UP * velocity_y * dt
	
func _grounded_logic(one_collide: bool, target_position: Vector3) -> void:
	if (one_collide and global_position.distance_to(target_position) <= ground_offset): 
			if not grounded: velocity_y = 0
			grounded = true
	elif not one_collide: grounded = false

func _ground_snap(dt: float) -> void:
	var both_collide: bool = ray_back.is_colliding() and ray_front.is_colliding()
	var one_collide: bool = ray_back.is_colliding() or ray_front.is_colliding()
	
	var target_position: Vector3 = Vector3.ZERO
	if one_collide:
		var point_back: Vector3 = ray_back.get_collision_point() if ray_back.is_colliding() else ray_front.get_collision_point()
		var point_front: Vector3 = ray_front.get_collision_point() if ray_front.is_colliding() else ray_back.get_collision_point() 
		var normal: Vector3 = (ray_back.get_collision_normal() + ray_front.get_collision_normal()) / 2
		var direction: Vector3 = point_back - point_front 
		if direction.length() < 0.01: direction = Vector3.RIGHT
		
		var scale: Vector3 = global_basis.get_scale()
		global_basis = Basis.looking_at(direction.normalized(), normal) * Basis.from_scale(scale)
		target_position = (point_back + point_front).y / 2 * Vector3.UP
		if grounded: 
			var theta: float = acos(normal.dot(-MapLoader.s_speed.normalized()))
			velocity_y = MapLoader.s_speed.length() / tan(theta)
			velocity_y = clamp(velocity_y, 0, 100)
			global_position = target_position + ground_offset * normal
	
	_grounded_logic(one_collide, target_position)

func _process(dt: float) -> void:
	_integrate(dt)
	_ground_snap(dt)
	previous_position = global_position
