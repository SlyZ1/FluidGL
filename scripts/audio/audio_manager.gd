extends Node
class_name CustomAudioManager

@export var sound_effect_settings: Array[SoundEffectSettings]

var _sound_effects_dict: Dictionary[SoundEffectSettings.SOUND_EFFECT_TYPE, SoundEffectSettings]
var _repeated_sound_effects_dicts: Dictionary[SoundEffectSettings.SOUND_EFFECT_TYPE, AudioStreamPlayer]

func _ready() -> void:
	for sound_effect in sound_effect_settings:
		_sound_effects_dict[sound_effect.type] = sound_effect

func stop_repeated_sound_effect(type: SoundEffectSettings.SOUND_EFFECT_TYPE):
	if _sound_effects_dict.has(type):
		var sound_effect := _sound_effects_dict[type]

		if sound_effect.repeat_until_stopped and _repeated_sound_effects_dicts.has(type):
			_repeated_sound_effects_dicts[type].queue_free()
			_repeated_sound_effects_dicts.erase(type)
	else:
		push_error("Tu n'as pas encore ajouté au dictionnaire cet audio!!")

func play(type: SoundEffectSettings.SOUND_EFFECT_TYPE) -> void:
	if _sound_effects_dict.has(type):
		var sound_settings := _sound_effects_dict[type]
		
		sound_settings.sound_effect.loop = sound_settings.repeat_until_stopped

		var new_audio = AudioStreamPlayer.new()
		add_child(new_audio)

		new_audio.stream = sound_settings.sound_effect
		new_audio.volume_db = sound_settings.volume
		new_audio.pitch_scale = sound_settings.pitch_scale
		new_audio.pitch_scale += (2 * randf() - 1) * sound_settings.pitch_randomness

		if sound_settings.repeat_until_stopped: 
			stop_repeated_sound_effect(type)
			_repeated_sound_effects_dicts[type] = new_audio
		else: new_audio.finished.connect(new_audio.queue_free)

		new_audio.play(sound_settings.start_offset)
	else:
		push_error("Tu n'as pas encore ajouté au dictionnaire cet audio!!")

func on_global_stop() -> void:
	for type in _sound_effects_dict.keys():
		stop_repeated_sound_effect(type)
