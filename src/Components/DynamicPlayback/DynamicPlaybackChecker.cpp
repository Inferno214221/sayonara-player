/* DynamicPlaybackCheckerImpl.cpp */
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
#include "DynamicPlaybackChecker.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Utils/Library/LibraryInfo.h"

#include <QList>

namespace
{
	class DynamicPlaybackCheckerImpl :
		public DynamicPlaybackChecker
	{
		public:
			DynamicPlaybackCheckerImpl(Library::InfoAccessor* libraryInfoAccessor) :
				m_libraryInfoAccessor {libraryInfoAccessor} {}

			~DynamicPlaybackCheckerImpl() override = default;

			bool isDynamicPlaybackPossible() const override
			{
				return (!m_libraryInfoAccessor->allLibraries().isEmpty());
			}

		private:
			Library::InfoAccessor* m_libraryInfoAccessor;
	};
}

DynamicPlaybackChecker* DynamicPlaybackChecker::create(Library::InfoAccessor* libraryInfoAccessor)
{
	return new DynamicPlaybackCheckerImpl(libraryInfoAccessor);
}

