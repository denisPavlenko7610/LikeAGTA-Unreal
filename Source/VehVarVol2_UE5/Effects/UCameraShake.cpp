#include "UCameraShake.h"

UCameraShake::UCameraShake()
{
	float amplitude = 0.35f;
	float frequency = 10.0f;
	
	RotOscillation.Pitch.Amplitude = amplitude;
	RotOscillation.Pitch.Frequency = frequency;

	RotOscillation.Yaw.Amplitude = amplitude;
	RotOscillation.Yaw.Frequency = frequency;

	RotOscillation.Roll.Amplitude = amplitude;
	RotOscillation.Roll.Frequency = frequency;
	
	OscillationDuration = 0.2f;
	OscillationBlendInTime = 0.1f;
	OscillationBlendOutTime = 0.1f;
}