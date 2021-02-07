#include "SayonaraTest.h"
#include "TestPlayManager.h"

#include "Components/Converter/ConverterFactory.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/PlaylistLoader.h"
#include "Components/Converter/OggConverter.h"
#include "Components/Converter/LameConverter.h"
#include "Components/Converter/OpusConverter.h"

#include "Utils/Playlist/CustomPlaylist.h"

// access working directory with Test::Base::tempPath("somefile.txt");

class DummyLoader : public Playlist::Loader
{
	private:
		QList<CustomPlaylist> m_playlists;

	public:
		~DummyLoader() override = default;

		int getLastPlaylistIndex() const override
		{
			return 0;
		}

		int getLastTrackIndex() const override
		{
			return 0;
		}

		const QList<CustomPlaylist>& playlists() const override
		{
			return m_playlists;
		}
};

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
	auto* playManager = new TestPlayManager(this);
	auto playlistHandler = new Playlist::Handler(playManager, std::make_shared<DummyLoader>());
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
