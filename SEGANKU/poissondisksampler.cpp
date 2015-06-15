#include "poissondisksampler.h"

std::random_device PoissonDiskSampler::rd;
std::mt19937 PoissonDiskSampler::gen(rd());
std::uniform_real_distribution<float> PoissonDiskSampler::distribution(0.0, 1.0);

float PoissonDiskSampler::randomFloat()
{
	auto rand = std::bind(distribution, std::ref(gen));
	return static_cast<float>(float(rand()));
}

glm::vec2 PoissonDiskSampler::generateRandomNeighbour(const glm::vec2 &position, float minDist)
{
	// random radius in range [minDist, 2*minDist]
	// random angle in range [0, 2*pi]
	float radius = (1.0f + randomFloat()) * minDist;
	float angle = 2 * 3.141592653589f * randomFloat();

	return glm::vec2(position.x + radius*cos(angle), position.y + radius*sin(angle));
}

bool PoissonDiskSampler::isInNeighbourhood(const glm::vec2 &position, const std::vector<std::vector<glm::vec2> > &grid, float minDist, float gridUnitLength)
{
	glm::vec2 gridPos(int(position.x / gridUnitLength), int(position.y / gridUnitLength));

	// number of adjacent grid cells to check for neighbouring points
	const int D = 5;

	// scan the neighbourhood of the point in the grid
	for (unsigned int i = gridPos.x - D; i < gridPos.x + D; ++i) {
		for (unsigned int j = gridPos.y - D; j < gridPos.y + D; ++j) {

			if (i >= 0 && i < grid.size() && j >= 0 && j < grid[0].size()) {

				glm::vec2 neighbourPosition = grid[i][j];
				if (glm::distance(neighbourPosition, position) < minDist) {
					return true;
				}
			}
		}
	}


	return false;
}

std::vector<glm::vec2> PoissonDiskSampler::generatePoissonSample(unsigned int sampleSize, float minDist, int maxNeighboursToTry)
{
	std::vector<glm::vec2> samplePositions;
	std::vector<glm::vec2> processPositions;

	// define grid
	float gridUnitLength = minDist/sqrt(2);
	std::vector<std::vector<glm::vec2> > grid;
	grid.resize(int(ceil(1.0f/gridUnitLength)));
	for (auto i = grid.begin(); i != grid.end(); ++i) { i->resize(int(ceil(1.0f/gridUnitLength))); }

	glm::vec2 initialPosition = glm::vec2(randomFloat(), randomFloat());
	samplePositions.push_back(initialPosition);
	processPositions.push_back(initialPosition);
	grid[int(initialPosition.x / gridUnitLength)][int(initialPosition.y / gridUnitLength)] = initialPosition;

	// generate new points for each point in the queue
	while (!processPositions.empty() && samplePositions.size() < sampleSize) {

		glm::vec2 position = popRandomVectorElem<glm::vec2>(processPositions);

		for (int i = 0; i < maxNeighboursToTry; ++i) {

			glm::vec2 newPosition = generateRandomNeighbour(position, minDist);

			if (newPosition.x >= 0 && newPosition.x <= 1 && newPosition.y >= 0 && newPosition.y <= 1
			    && !isInNeighbourhood(newPosition, grid, minDist, gridUnitLength)) {

				processPositions.push_back(newPosition);
				samplePositions.push_back(newPosition);
				grid[int(initialPosition.x / gridUnitLength)][int(initialPosition.y / gridUnitLength)] = newPosition;
				continue;
			}
		}
	}

	return samplePositions;
}
