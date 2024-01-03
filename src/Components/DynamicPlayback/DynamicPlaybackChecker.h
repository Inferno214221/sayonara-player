/* DynamicPlaybackCheckerImpl.h */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_DYNAMICPLAYBACKCHECKER_H
#define SAYONARA_PLAYER_DYNAMICPLAYBACKCHECKER_H

namespace Library
{
	class InfoAccessor;
}

class DynamicPlaybackChecker
{
	public:
		virtual ~DynamicPlaybackChecker() = default;
		virtual bool isDynamicPlaybackPossible() const = 0;

		static DynamicPlaybackChecker* create(Library::InfoAccessor* libraryInfoAccessor);
};

#endif //SAYONARA_PLAYER_DYNAMICPLAYBACKCHECKER_H
