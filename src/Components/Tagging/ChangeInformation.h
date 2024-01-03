/* ChangeInformation.h
 *
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

#ifndef TAGGING_CHANGEINFORMATION_H
#define TAGGING_CHANGEINFORMATION_H

#include "Utils/Pimpl.h"

class MetaData;

namespace Tagging
{
	class ChangeInformation
	{
		PIMPL(ChangeInformation)

		public:
			ChangeInformation(const MetaData& track);
			~ChangeInformation();

			ChangeInformation(const ChangeInformation& other);
			ChangeInformation& operator=(const ChangeInformation& other);

			void update(const MetaData& track);
			void updateCover(const QPixmap& pm);

			/**
			 * @brief Overwrite original track with the modified one.
			 * This cannot be undone
			 */
			void apply();

			/**
			 * @brief Overwrite modified track with the original one.
			 */
			void undo();

			bool hasChanges() const;
			void setChanged(bool b);

			bool hasNewCover() const;
			QPixmap cover() const;

			const MetaData& currentMetadata() const;
			const MetaData& originalMetadata() const;

			MetaData& currentMetadata();
			MetaData& originalMetadata();
	};
}

#endif // TAGGING_CHANGEINFORMATION_H
