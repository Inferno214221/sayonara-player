/* ArtistInfo.h */

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

#ifndef SAYONARA_PLAYER_ARTISTINFO_H
#define SAYONARA_PLAYER_ARTISTINFO_H

#include "MetaDataInfo.h"
#include "Utils/Pimpl.h"

class ArtistInfo :
	public LibraryItemInfo
{
	PIMPL(ArtistInfo)

	public:
		explicit ArtistInfo(const MetaDataList& metaDataList);
		~ArtistInfo() override;

		[[nodiscard]] auto additionalData() const -> AdditionalInfo override;
		[[nodiscard]] auto coverLocation() const -> Cover::Location override;
		[[nodiscard]] auto header() const -> QString override;
		[[nodiscard]] auto subheader() const -> QString override;
};

#endif // SAYONARA_PLAYER_ARTISTINFO_H

