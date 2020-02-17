/* RandomGenerator.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Utils/RandomGenerator.h"
#include <random>
#include <chrono>

struct RandomGenerator::Private
{
	unsigned int seed;
	std::mt19937 generator;
};

RandomGenerator::RandomGenerator()
{
	m = Pimpl::make<RandomGenerator::Private>();
	updateSeed();
}

RandomGenerator::~RandomGenerator() {}

RandomGenerator::RandomGenerator(const RandomGenerator& other)
{
	(void) (other);
}


void RandomGenerator::updateSeed()
{
	m->seed = std::chrono::system_clock::now().time_since_epoch().count();
	m->generator = std::mt19937(m->seed);
}

int RandomGenerator::getNumber(int min, int max)
{
	std::uniform_int_distribution<int> d(min, max);
	return d(m->generator);
}

int RandomGenerator::getRandomNumber(int min, int max)
{
	RandomGenerator generator;
	return generator.getNumber(min, max);
}
