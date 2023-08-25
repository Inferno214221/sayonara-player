#include "Common/SayonaraTest.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"

#include <QDateTime>

#include <array>

using namespace StreamRecorder;

class StreamRecorderUtilsTest :
	public Test::Base
{
	Q_OBJECT

	public:
		StreamRecorderUtilsTest() :
			Test::Base("StreamRecorderUtilsTest") {}

	private slots:

		[[maybe_unused]] void testErrorCodes() // NOLINT(readability-convert-member-functions-to-static)
		{
			struct TestCase
			{
				QString expression;
				Utils::ErrorCode expectedError;
			};

			const auto testCases = std::array {
				TestCase {"<tn><t> <m>_<y>-/<ds>+df3-<min>_-<d>-/<dl>df<ar>", Utils::ErrorCode::OK},
				TestCase {"<t> <m>_<y>-/<ds>+df3-<min>-<d>-/<dl>df<ar>", Utils::ErrorCode::OK},
				TestCase {"<tn> <m>_<y>-/<ds>+df3-<min>-<d>-/<dl>df<ar>", Utils::ErrorCode::OK},
				TestCase {"<ar> - <t> - <<m>/bla.mp3", Utils::ErrorCode::BracketError},
				TestCase {"<t> <m>_<y>-/<fs>+df3-<min>_<t>-<d>-/<dl>df<a>", Utils::ErrorCode::UnknownTag},
				TestCase {"<m>_<y>-/<ds>:df3-<min>_<tn><t>-<d>-/<dl>df<ar>", Utils::ErrorCode::InvalidChars},
				TestCase {"<ar> - <t> - <<m>/bla.mp3", Utils::ErrorCode::BracketError},
				TestCase {"<m>_<y>-/<ds>+df3-<min>-<d>-/<dl>df<ar>", Utils::ErrorCode::MissingUniqueTag}
			};

			for(const auto& testCase: testCases)
			{
				int invalidIndex;
				QVERIFY(Utils::validateTemplate(testCase.expression, &invalidIndex) == testCase.expectedError);
			}
		}

		[[maybe_unused]] void filenameTest() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto title = QString {"MyTitle"};
			const auto artist = QString {"MyArtist"};

			auto track = MetaData {};
			track.setTitle(title);
			track.setArtist(artist);

			const auto streamRecorderPath = QString {"/path/to/nowhere"};
			const auto templ = QString {"<t> <m>_<y>-/<ds>+df3-<min>_<t>-<d>-/<dl>df<ar>"};

			const auto date = QDate::currentDate();
			const auto shortDayName = QLocale().dayName(date.dayOfWeek(), QLocale::ShortFormat);
			const auto longDayName = QLocale().dayName(date.dayOfWeek(), QLocale::LongFormat);

			const auto time = QTime::currentTime();

			int invalidIndex;
			const auto errorCode = Utils::validateTemplate(templ, &invalidIndex);
			QVERIFY(errorCode == Utils::ErrorCode::OK);

			const auto path = Utils::fullTargetPath(streamRecorderPath, templ, track, date, time);
			const auto expectedPath = streamRecorderPath + "/" +
			                          QString("%1 %2_%3-/%4+df3-%5_%6-%7-/%8df%9.mp3")
				                          .arg(title)
				                          .arg(date.month(), 2, 10, QChar('0'))
				                          .arg(date.year(), 4, 10, QChar('0'))
				                          .arg(shortDayName)
				                          .arg(time.minute(), 2, 10, QChar('0'))
				                          .arg(title)
				                          .arg(date.day(), 2, 10, QChar('0'))
				                          .arg(longDayName)
				                          .arg(artist);

			QVERIFY(path.filename == expectedPath);
		}
};

QTEST_GUILESS_MAIN(StreamRecorderUtilsTest)

#include "StreamRecorderUtilsTest.moc"
