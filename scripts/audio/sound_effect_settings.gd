extends Resource
class_name SoundEffectSettings

enum SOUND_EFFECT_TYPE{ 
	DEATH,
	JUMP,
	CATCH,
	ON_RAIL,
	POINTS_OK,
	POINTS_NICE,
	POINTS_GREAT,
	POINTS_AMAZING,
	POINTS_CRAZY,
	POINTS_EPIC,
	POINTS_LEGENDARY,
	POINTS_GODLIKE,
	POINTS_STOP_PLEASE,
	NO_SOUND,
}

@export var type: SOUND_EFFECT_TYPE
@export var sound_effect: AudioStreamMP3
@export_range(-40, 20) var volume: float = 0.0
@export_range(0.0, 4.0, .01) var pitch_scale: float = 1.0
@export_range(0.0, 1.0, .01) var pitch_randomness: float = 0.0
@export_range(0.0, 1000.0, 0.01, "hide_slider") var start_offset: float = 0.0
@export var repeat_until_stopped: bool