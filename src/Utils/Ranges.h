/* Ranges.h */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_RANGES_H
#define SAYONARA_PLAYER_RANGES_H

#include "Utils/Algorithm.h"

#include <utility>
#include <vector>

namespace Util
{
	using Range = std::pair<int, int>;
	using RangeList = std::vector<Range>;

	template<typename Element, template<typename> typename Container>
	Range getNextRange(const Container<Element>& ids, int startIndex)
	{
		if(startIndex >= ids.size())
		{
			return std::make_pair(-1, -1);
		}

		auto result = std::make_pair(startIndex, startIndex);

		auto currentIndex = startIndex;
		auto currentValue = ids[currentIndex];
		auto expectedValue = currentValue;

		while(currentValue == expectedValue)
		{
			result.second = currentIndex;

			currentIndex += 1;

			if(currentIndex == ids.size())
			{
				break;
			}

			expectedValue += 1;
			currentValue = ids[currentIndex];
		}

		return result;
	}

	template<typename Element, template<typename> typename Container>
	RangeList getRangesFromList(const Container<Element>& ids)
	{
		if(ids.empty())
		{
			return RangeList{};
		}

		RangeList result;
		auto p = getNextRange(ids, 0);
		while(p.first != -1)
		{
			result.push_back(p);
			p = getNextRange(ids, p.second + 1);
		}

		return result;
	}

	template<typename Element, template<typename> typename Container>
	auto prepareContainerForRangeCalculation(Container<Element> container) -> Container<Element>
	{
		std::sort(container.begin(), container.end());
		Util::Algorithm::remove_duplicates(container);

		return container;
	}
}

#endif //SAYONARA_PLAYER_RANGES_H
