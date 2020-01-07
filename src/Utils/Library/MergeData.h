/* MergeData.h */

/* Copyright (C) 2011-2020 Lucio Carreras
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
	/**
	 * @brief Changes all metadata containing one of the source ids and replaces
	 * it with the target id. For example, 3 different artists are merged into one
	 * @ingroup Library
	 * @ingroup Helper
	 */
	class MergeData
	{
		PIMPL(MergeData)

		public:

			/**
			 * @brief Merges source_ids into target_id
			 * @param source ids
			 * @param target ids
			 * @param library_id, can be -1
			 */
			MergeData(const Util::Set<Id>& source_ids, Id target_id, LibraryId library_id);
			MergeData(const MergeData& other);
			~MergeData();

			MergeData& operator=(const MergeData& other);

			/**
			 * @brief target_id >= 0, source_ids.size() > 1, source_ids >= 0
			 * @return
			 */
			bool			is_valid() const;

			/**
			 * @brief getter for source_ids. See constructor
			 */
			Util::Set<Id>	source_ids() const;

			/**
			 * @brief getter for target_id. See constructor
			 */
			Id				target_id() const;

			/**
			 * @brief getter for library_d. See constructor
			 */
			LibraryId		library_id() const;
	};
}

#endif // MERGEDATA_H
