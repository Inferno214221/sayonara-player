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

#ifndef ARTISTINFO_H
#define ARTISTINFO_H

#include "MetaDataInfo.h"
#include "Utils/Pimpl.h"

/**
 * @brief The ArtistInfo class
 * @ingroup MetaDataHelper
 */
class ArtistInfo :
	public MetaDataInfo
{
	PIMPL(ArtistInfo)

private:
	void calcCoverLocation() override;
	void calcSubheader() override;
	void calcHeader() override;

	void calcSimilarArtists(Artist& artist);

public:
	explicit ArtistInfo(const MetaDataList& tracs);
	~ArtistInfo() override;

	QString additionalInfostring() const override;
	Cover::Location coverLocation() const override;
};

#endif // ARTISTINFO_H

