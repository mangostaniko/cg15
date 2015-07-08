#ifndef POISSONDISKSAMPLER_H
#define POISSONDISKSAMPLER_H

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/constants.hpp>

#include <vector>
#include <random>
#include <functional>

#include <iostream>

// Fast Poisson Disk Sampling in Arbitrary Dimensions
// http://people.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf

// Implementation adapted from https://github.com/corporateshark/poisson-disk-generator/blob/master/Poisson.cpp
// by Sergey Kosarevsky, 2014-2015
// based on http://devmag.org.za/2009/05/03/poisson-disk-sampling/

/* PoissonDiskSampler
 * sample area is mapped to a grid, each grid cell can contain at most one random floating point position,
 * which must however lie at a minimum distance to positions in neighbouring grid cells,
 * such that instead of pure random sampling where positions could all be in one place,
 * we get random samples distributed over the whole area via the grid (as in jittered grid sampling),
 * but also, positions of neighbouring grid cells are never too close together (that is the essential property of the algorithm).
 * starting form an initial position in a grid cell, random neighbours at a certain euclidean distance
 * are attempted to be found in other grid cells, until the desired number of samples is acquired.
 */
class PoissonDiskSampler
{
	PoissonDiskSampler();
	~PoissonDiskSampler();

	static std::random_device rd;
	static std::mt19937 gen;
	static std::uniform_real_distribution<float> distribution;

	static float randomFloat();

	template<typename Type>
	static Type popRandomVectorElem(std::vector<Type> &vect)
	{
		std::uniform_int_distribution<> indexDistribution(0, vect.size() - 1);
		auto rand = std::bind(indexDistribution, std::ref(gen));
		int randomIndex = rand();
		Type elem = vect[randomIndex];
		vect.erase(vect.begin() + randomIndex);
		return elem;
	}

	static glm::vec2 generateRandomNeighbour(const glm::vec2 &position, float minDist);

	static bool isInNeighbourhood(const glm::vec2 &position, const std::vector<std::vector<glm::vec2> > &grid, float minDist, float gridUnitLength);

public:

	/**
	 * @brief generate a sample of approximately poisson disk distributed positions
	 * @param sampleSize the number of positions to generate
	 * @param minDist the minimum distance between any positions (if this is too small, some areas might not get covered!)
	 * @param maxNeighboursToTry the number of positions attempted to be found for the current grid cell until the grid cell index is rejected
	 * @return the generated poisson disk distributed positions
	 */
	static std::vector<glm::vec2> generatePoissonSample(unsigned int sampleSize, float minDist, int maxNeighboursToTry = 30);

};

#endif // POISSONDISKSAMPLER_H
