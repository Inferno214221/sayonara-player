#include "SayonaraTest.h"
#include "PlayManagerMock.h"
#include "PlaylistMocks.h"

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
	auto playlistHandler = new Playlist::Handler(new PlayManagerMock(), std::make_shared<PlaylistLoaderMock>());
	auto factory = ConverterFactory(playlistHandler);

	{
		auto* converter = factory.createConverter<ConverterFactory::ConvertType::OggVorbis>(320);
		QVERIFY(dynamic_cast<OggConverter*>(converter) != nullptr);
	}

	{
		auto* converter = factory.createConverter<ConverterFactory::ConvertType::Lame>(
		                                          ConverterFactory::Bitrate::Constant,
		                                          320);
		QVERIFY(dynamic_cast<LameConverter*>(converter) != nullptr);
	}

	{
		auto* converter = factory.createConverter<ConverterFactory::ConvertType::OggOpus>(
		                                          ConverterFactory::Bitrate::Constant,
		                                          320);
		QVERIFY(dynamic_cast<OpusConverter*>(converter) != nullptr);
	}
}

QTEST_GUILESS_MAIN(AudioConverterTest)

#include "AudioConverterTest.moc"
