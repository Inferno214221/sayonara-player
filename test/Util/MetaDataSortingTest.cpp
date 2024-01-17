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

#include "Common/SayonaraTest.h"
#include "Utils/MetaData/MetaDataSorting.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Algorithm.h"

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	using Modifier = std::function<void(MetaData&, const QString&)>;
	using Accessor = std::function<QString(const MetaData&)>;

	struct AccessAndModify
	{
		QString name;
		Accessor accessor;
		Modifier modifier;
		Library::TrackSortorder sortOrder;
	};

	AccessAndModify titleAccessor()
	{
		auto getter = [](const auto& track) -> QString {
			return track.title();
		};

		auto setter = [](auto& track, const auto& str) {
			track.setTitle(str);
		};

		return {"title", std::move(getter), std::move(setter), Library::TrackSortorder::TitleAsc};
	}

	AccessAndModify albumAccessor()
	{
		auto getter = [](const auto& track) -> QString {
			return track.album();
		};

		auto setter = [](auto& track, const auto& str) {
			track.setAlbum(str);
		};

		return {"album", std::move(getter), std::move(setter), Library::TrackSortorder::AlbumAsc};
	}

	AccessAndModify artistAccessor()
	{
		auto getter = [](const auto& track) -> QString {
			return track.artist();
		};

		auto setter = [](auto& track, const auto& str) {
			track.setArtist(str);
		};

		return {"artist", std::move(getter), std::move(setter), Library::TrackSortorder::ArtistAsc};
	}

	AccessAndModify albumArtistAccessor()
	{
		auto getter = [](const auto& track) -> QString {
			return track.albumArtist();
		};

		auto setter = [](auto& track, const auto& str) {
			track.setAlbumArtist(str);
		};

		return {"albumArtist", std::move(getter), std::move(setter), Library::TrackSortorder::AlbumArtistAsc};
	}

	QList<AccessAndModify> createTrackAcessors()
	{
		auto result = QList<AccessAndModify> {}
			<< titleAccessor()
			<< albumAccessor()
			<< artistAccessor()
			<< albumArtistAccessor();

		return result;
	}

	MetaData createTestTrack(const QString& str, const Modifier& setter)
	{
		auto track = MetaData();
		setter(track, str);
		return track;
	}

	MetaDataList createTestTracks(const Modifier& setter)
	{
		auto tracks = MetaDataList();

		tracks << createTestTrack("A Name 1", setter);
		tracks << createTestTrack("a name 2", setter);
		tracks << createTestTrack("The A Name 3", setter);
		tracks << createTestTrack("$$ A Name 4", setter);
		tracks << createTestTrack("Á Namé 5", setter);
		tracks << createTestTrack("A Name 6", setter);

		return tracks;
	}

	bool assertOrder(const MetaDataList& tracks, const QStringList& expectedNames, const Accessor& accessor)
	{
		for(auto i = 0; i < tracks.count(); i++)
		{
			if(expectedNames[i] != accessor(tracks[i]))
			{
				return false;
			}
		}

		return true;
	}
}

class MetaDataSortingTest :
	public Test::Base
{
	Q_OBJECT

	public:
		MetaDataSortingTest() :
			Test::Base("MetaDataSorting") {}

	private slots:
		[[maybe_unused]] void testSingleCombinations();
		[[maybe_unused]] void testAllCombinations();
};

[[maybe_unused]] void
MetaDataSortingTest::testSingleCombinations() // NOLINT(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
{
	const auto trackAccessors = createTrackAcessors();
	for(const auto& trackAccessor: trackAccessors)
	{
		auto tracks = createTestTracks(trackAccessor.modifier);

		MetaDataSorting::sortMetadata(tracks, trackAccessor.sortOrder, +MetaDataSorting::SortMode::None);
		QVERIFY(assertOrder(tracks, {
			"$$ A Name 4",
			"A Name 1",
			"A Name 6",
			"The A Name 3",
			"a name 2",
			"Á Namé 5",
		}, trackAccessor.accessor));

		MetaDataSorting::sortMetadata(tracks, trackAccessor.sortOrder, +MetaDataSorting::SortMode::CaseInsensitive);
		QVERIFY(assertOrder(tracks, {
			"$$ A Name 4",
			"A Name 1",
			"a name 2",
			"A Name 6",
			"The A Name 3",
			"Á Namé 5",
		}, trackAccessor.accessor));

		MetaDataSorting::sortMetadata(tracks, trackAccessor.sortOrder, +MetaDataSorting::SortMode::IgnoreArticle);
		if(trackAccessor.sortOrder == Library::TrackSortorder::ArtistAsc ||
		   trackAccessor.sortOrder == Library::TrackSortorder::AlbumArtistAsc)
		{
			QVERIFY(assertOrder(tracks, {
				"$$ A Name 4",
				"A Name 1",
				"The A Name 3", // 'The' is not ignored for artist columns
				"A Name 6",
				"a name 2",
				"Á Namé 5",
			}, trackAccessor.accessor));
		}

		else
		{
			QVERIFY(assertOrder(tracks, {
				"$$ A Name 4",
				"A Name 1",
				"A Name 6",
				"The A Name 3", // 'The' is ignored for non-artists columns
				"a name 2",
				"Á Namé 5",
			}, trackAccessor.accessor));
		}

		MetaDataSorting::sortMetadata(tracks, trackAccessor.sortOrder,
		                              +MetaDataSorting::SortMode::IgnoreDiacryticChars);
		QVERIFY(assertOrder(tracks, {
			"$$ A Name 4",
			"A Name 1",
			"Á Namé 5",
			"A Name 6",
			"The A Name 3",
			"a name 2",
		}, trackAccessor.accessor));

		MetaDataSorting::sortMetadata(tracks, trackAccessor.sortOrder, +MetaDataSorting::SortMode::IgnoreSpecialChars);
		QVERIFY(assertOrder(tracks, {
			"A Name 1",
			"$$ A Name 4",
			"A Name 6",
			"The A Name 3",
			"a name 2",
			"Á Namé 5",
		}, trackAccessor.accessor));
	}
}

[[maybe_unused]] void
MetaDataSortingTest::testAllCombinations() // NOLINT(readability-convert-member-functions-to-static)
{
	constexpr const auto mask = +MetaDataSorting::SortMode::CaseInsensitive |
	                            +MetaDataSorting::SortMode::IgnoreSpecialChars |
	                            +MetaDataSorting::SortMode::IgnoreDiacryticChars |
	                            +MetaDataSorting::SortMode::IgnoreArticle;
	const auto trackAccessors = createTrackAcessors();
	for(const auto& trackAccessor: trackAccessors)
	{
		auto tracks = createTestTracks(trackAccessor.modifier);

		MetaDataSorting::sortMetadata(tracks, trackAccessor.sortOrder, mask);

		if(trackAccessor.sortOrder == Library::TrackSortorder::ArtistAsc ||
		   trackAccessor.sortOrder == Library::TrackSortorder::AlbumArtistAsc)
		{
			QVERIFY(assertOrder(tracks, {
				"A Name 1",
				"a name 2",
				"The A Name 3", // 'The' is not ignored for artist columns
				"$$ A Name 4",
				"Á Namé 5",
				"A Name 6",
			}, trackAccessor.accessor));
		}

		else
		{
			QVERIFY(assertOrder(tracks, {
				"A Name 1",
				"a name 2",
				"$$ A Name 4",
				"Á Namé 5",
				"A Name 6",
				"The A Name 3", // 'The' is ignored when sorting a non-artist colum
			}, trackAccessor.accessor));
		}
	};
}

QTEST_GUILESS_MAIN(MetaDataSortingTest)

#include "MetaDataSortingTest.moc"
