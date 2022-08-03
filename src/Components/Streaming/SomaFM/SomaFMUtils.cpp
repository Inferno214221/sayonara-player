#include "SomaFMUtils.h"
#include "SomaFMStation.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/Fetcher/CoverFetcherUrl.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Algorithm.h"

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

	tracks.sort(::Library::SortOrder::TrackTitleAsc);
}
