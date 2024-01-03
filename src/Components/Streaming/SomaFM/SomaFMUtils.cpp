/* SomaFMUtils.cpp
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

#include "SomaFMUtils.h"
#include "SomaFMStation.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/Fetcher/CoverFetcherUrl.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataSorting.h"

#include <QString>

void SomaFM::Utils::mapStationToMetadata(const SomaFM::Station& station, MetaDataList& tracks)
{
	const Cover::Location cl = station.coverLocation();
	const QList<Cover::Fetcher::Url> searchUrls = cl.searchUrls();

	QStringList coverUrls;
	Util::Algorithm::transform(searchUrls, coverUrls, [](auto url) {
		return url.url();
	});

	for(MetaData& md: tracks)
	{
		md.setCoverDownloadUrls(coverUrls);
		md.addCustomField("cover-hash", "", cl.hash());

		const QString filepath = md.filepath();
		md.setRadioStation(filepath, station.name());

		if(filepath.toLower().contains("mp3"))
		{
			md.setTitle(station.name() + " (mp3)");
		}

		else if(filepath.toLower().contains("aac"))
		{
			md.setTitle(station.name() + " (aac)");
		}
	}

	MetaDataSorting::sortMetadata(tracks, ::Library::SortOrder::TrackTitleAsc);
}
