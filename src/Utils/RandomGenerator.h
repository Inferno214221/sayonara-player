/* RandomGenerator.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include "Utils/Pimpl.h"
#include <algorithm>
#include <random>

class RandomGenerator
{
	private:
	PIMPL(RandomGenerator)

		RandomGenerator(const RandomGenerator& other);

	public:
		RandomGenerator();
		~RandomGenerator();

		void updateSeed();

		int getNumber(int min, int max);

		static int getRandomNumber(int min, int max);
		static std::mt19937 getGenerator();
};

namespace Util
{
	namespace Algorithm
	{
		template<class Container>
		void shuffle(Container& container)
		{
			std::shuffle(container.begin(), container.end(), RandomGenerator::getGenerator());
		}
	}
}

#endif // RANDOMGENERATOR_H

