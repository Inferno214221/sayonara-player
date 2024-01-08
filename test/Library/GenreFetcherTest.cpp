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
#include "Common/TaggingMocks.h"
#include "Components/Library/GenreFetcher.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Set.h"
#include "Utils/Algorithm.h"

#include <QSignalSpy>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	struct MetaDataTemplate
	{
		QString artist;
		QString album;
		QString title;
		QStringList genres;
	};

	MetaData createTrack(const MetaDataTemplate& trackTemplate)
	{
		auto track = MetaData(QString("/music/%1/%2/%3.mp3")
			                      .arg(trackTemplate.artist)
			                      .arg(trackTemplate.album)
			                      .arg(trackTemplate.title));
		track.setLibraryid(0);
		track.setGenres(trackTemplate.genres);

		return track;
	}

	class TestTagWriterMock :
		public Test::TagWriterMock
	{
		public:
			bool writeMetaData(const QString& filepath, const MetaData& track) override
			{
				m_tracks[filepath] = track;
				return TagWriterMock::writeMetaData(filepath, track);
			}

			bool writeChangedMetaDataOnly(const MetaData& oldTrack, const MetaData& newTrack) override
			{
				m_tracks[newTrack.filepath()] = newTrack;
				return TagWriterMock::writeChangedMetaDataOnly(oldTrack, newTrack);
			}

			bool updateMetaData(const MetaData& track) override
			{
				m_tracks[track.filepath()] = track;
				return TagWriterMock::updateMetaData(track);
			}

			[[nodiscard]] MetaDataList tracks() const
			{
				auto result = MetaDataList {};
				const auto tracks = m_tracks.values();
				for(const auto& track: tracks)
				{
					result.push_back(track);
				}

				return result;
			}

		private:
			QMap<QString, MetaData> m_tracks;

	};

	bool testMatchingGenres(const Util::Set<Genre>& genreList, const QStringList& genres)
	{
		if(genreList.count() != genres.count())
		{
			return false;
		}

		return std::all_of(genreList.begin(), genreList.end(), [&](const auto& genre) {
			return genres.contains(genre.name());
		});
	}
}

class GenreFetcherTest :
	public Test::Base
{
	Q_OBJECT

	public:
		GenreFetcherTest() :
			Test::Base("GenreFetcherTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testInsertGenreIntoEmptyTrack()
		{
			struct TestCase
			{
				QStringList genres;
				QString newGenre;
				QStringList expectedGenres;
			};

			const auto testCases = std::array {
				TestCase {{}, "Rock", {"Rock"}},
				TestCase {{"Blues"}, "Rock", {"Blues", "Rock"}}
			};

			for(const auto& testCase: testCases)
			{
				const auto tagWriter = std::make_shared<TestTagWriterMock>();
				auto genreFetcher = GenreFetcher(std::make_shared<Test::TagReaderMock>(), tagWriter);

				const auto track = createTrack({"artist", "album", "title1", testCase.genres});

				auto spy = QSignalSpy(&genreFetcher, &GenreFetcher::sigFinished);
				genreFetcher.applyGenreToMetadata(MetaDataList {track}, Genre {testCase.newGenre});
				spy.wait();

				const auto newTracks = tagWriter->tracks();

				const auto genres = newTracks[0].genres();
				QVERIFY(testMatchingGenres(genres, testCase.expectedGenres));
			}
		}
};

QTEST_GUILESS_MAIN(GenreFetcherTest)

#include "GenreFetcherTest.moc"
