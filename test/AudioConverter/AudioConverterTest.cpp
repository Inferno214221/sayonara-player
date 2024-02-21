/* AudioConverterTest.cpp
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

#include "test/Common/SayonaraTest.h"
#include "test/Common/PlayManagerMock.h"
#include "test/Common/PlaylistMocks.h"
#include "test/Common/FileSystemMock.h"

#include "Components/Converter/ConverterFactory.h"
#include "Components/Converter/OggConverter.h"
#include "Components/Converter/LameConverter.h"
#include "Components/Converter/OpusConverter.h"

#include "Utils/Playlist/CustomPlaylist.h"

// access working directory with Test::Base::tempPath("somefile.txt");

class AudioConverterTest :
	public Test::Base
{
	Q_OBJECT

	public:
		AudioConverterTest() :
			Test::Base("AudioConverterTest") {}

	private slots:
		void testFactory();
};

void AudioConverterTest::testFactory()
{
	// no files involved
	const auto fileSystem = std::make_shared<Test::FileSystemMock>();
	auto playlistHandler = new PlaylistHandlerMock(std::make_shared<PlayManagerMock>(), fileSystem);
	playlistHandler->createEmptyPlaylist(true);
	auto factory = ConverterFactory(playlistHandler);

	{
		auto* converter = factory.createConverter<ConverterFactory::ConvertType::OggVorbis>(320);
		QVERIFY(dynamic_cast<OggConverter*>(converter) != nullptr);
	}

	{
		auto* converter = factory.createConverter<ConverterFactory::ConvertType::Lame>(
			ConverterFactory::Bitrate::Constant, 320);
		QVERIFY(dynamic_cast<LameConverter*>(converter) != nullptr);
	}

	{
		auto* converter = factory.createConverter<ConverterFactory::ConvertType::OggOpus>(
			ConverterFactory::Bitrate::Constant, 320);
		QVERIFY(dynamic_cast<OpusConverter*>(converter) != nullptr);
	}
}

QTEST_GUILESS_MAIN(AudioConverterTest)

#include "AudioConverterTest.moc"
