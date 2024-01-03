/* MergeData.h */

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



#ifndef MERGEDATA_H
#define MERGEDATA_H

#include "Utils/Pimpl.h"
#include "Utils/SetFwd.h"
#include "Utils/typedefs.h"

namespace Library
{
	class MergeData
	{
		PIMPL(MergeData)

		public:
			MergeData(const Util::Set<Id>& sourceIds, Id targetId, LibraryId libraryId);
			MergeData(const MergeData& other);
			~MergeData();

			MergeData& operator=(const MergeData& other);

			[[nodiscard]] bool isValid() const;
			[[nodiscard]] Util::Set<Id> sourceIds() const;
			[[nodiscard]] Id targetId() const;
			[[nodiscard]] LibraryId libraryId() const;
	};
}

#endif // MERGEDATA_H
