#include "light.h"


Light::Light(const glm::mat4 &modelMatrix_, glm::vec3 endPos, glm::vec3 startCol, glm::vec3 endCol, float seconds)
	: SceneObject(modelMatrix_), endPosition(endPos), startColor(startCol), endColor(endCol), currentColor(startCol), 
	daySectionDuration(seconds), timePassed(0.f), noonColor(0.51f, 0.74f, 0.96f), nightColor(0.1f, 0.1f, 0.1f)
{
	startPosition = getLocation();
	direction = glm::normalize(endPosition - getLocation());
	distance = glm::abs(glm::length(endPosition - getLocation()));
	dayTime = NIGHT;
}

void Light::update(float timeDelta)
{
	timePassed += timeDelta;

	float t = (timePassed / daySectionDuration);

	/*
		Following if statements represent the daily cycle from Midnight to Midnight
		Color and Position are interpolated according to time passed, Position has to be reset to start 
		at Midnight
	*/
	if (dayTime == 0) {
		currentColor = startColor * (1.f - t) + t * noonColor;

		if (timePassed > daySectionDuration) {
			timePassed = 0.f;
			dayTime = AFTERNOON;
		}
	}
	else if (dayTime == 1) {
		currentColor = noonColor * (1.f - t) + t * endColor;

		if (timePassed > daySectionDuration) {
			timePassed = 0.f;
			dayTime = EVENING;
			
		}
	}
	else if (dayTime == 2) {
		currentColor = endColor * (1.f - t) + t * nightColor;
		
		if (timePassed > daySectionDuration) {
			timePassed = 0.f;
			dayTime = NIGHT;
			setLocation(startPosition);
		}
	}
	else if (dayTime == 3) {
		currentColor = nightColor * (1.f - t) + t * startColor;

		if (timePassed > daySectionDuration) {
			timePassed = 0.f;
			dayTime = MORNING;
		}
	}

	t = timeDelta / (daySectionDuration * 2.f);
	glm::vec3 transDist = direction * (distance * t);
	translate(transDist, SceneObject::RIGHT);
	
}


glm::vec3 Light::getColor() const
{
	return currentColor;
}