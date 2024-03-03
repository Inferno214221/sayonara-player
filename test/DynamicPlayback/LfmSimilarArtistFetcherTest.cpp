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
#include "Common/TestWebClientFactory.h"

#include "Components/DynamicPlayback/LfmSimilarArtistFetcher.h"
#include "Components/DynamicPlayback/ArtistMatch.h"

#include "Utils/FileUtils.h"

#include <QSignalSpy>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	QByteArray getSimilarArtistData()
	{
		constexpr const auto* filename = ":/test/metallica.similar_artists";
		auto data = QByteArray {};
		Util::File::readFileIntoByteArray(filename, data);

		return data;
	}
}

class LfmSimilarArtistFetcherTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LfmSimilarArtistFetcherTest() :
			Test::Base("LfmSimilarArtistFetcherTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testFetcherFinishes()
		{
			auto webClientFactory = std::make_shared<Test::WebClientFactory>();
			auto fetcher = DynamicPlayback::LfmSimilarArtistFetcher("Some artist", webClientFactory);
			auto spy = QSignalSpy(&fetcher, &DynamicPlayback::SimilarArtistFetcher::sigFinished);

			fetcher.start();

			auto* webClient = webClientFactory->clients()[0];
			webClient->fireData({});

			QCOMPARE(spy.count(), 1);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testDataLeadsToSuccess()
		{
			struct TestCase
			{
				QByteArray data;
				bool expectedValid;
			};

			const auto testCases = std::array {
				TestCase {{}, false},
				TestCase {{"#%Everything but vald exl"}, false},
				TestCase {getSimilarArtistData(), true}
			};

			for(const auto& testCase: testCases)
			{
				auto webClientFactory = std::make_shared<Test::WebClientFactory>();
				auto fetcher = DynamicPlayback::LfmSimilarArtistFetcher("Some artist", webClientFactory);
				auto spy = QSignalSpy(&fetcher, &DynamicPlayback::SimilarArtistFetcher::sigFinished);

				fetcher.start();

				auto* webClient = webClientFactory->clients()[0];
				webClient->fireData(testCase.data);

				QCOMPARE(spy.count(), 1);

				const auto artistMatch = fetcher.similarArtists();
				QCOMPARE(artistMatch.isValid(), testCase.expectedValid);
			}
		}
};

QTEST_GUILESS_MAIN(LfmSimilarArtistFetcherTest)

#include "LfmSimilarArtistFetcherTest.moc"
