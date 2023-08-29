#include "test/Common/SayonaraTest.h"
#include "test/Common/PlayManagerMock.h"
#include "test/Common/FileSystemMock.h"

#include "Components/Engine/StreamRecorder/StreamRecorder.h"
#include "Components/Engine/PipelineExtensions/StreamRecordable.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingRegistry.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/FileUtils.h"

#include <QDateTime>
#include <QFile>

namespace SR = StreamRecorder;

namespace
{
	class PipelineMock :
		public PipelineExtensions::StreamRecordable
	{
		public:
			PipelineMock() = default;
			~PipelineMock() override = default;

			void setRecordingPath(const QString& targetPath) override
			{
				m_targetPath = targetPath;
			}

			void prepareForRecording() override
			{
				m_sessionActive = true;
			}

			void finishRecording() override
			{
				m_sessionActive = false;
			}

			bool sessionActive() const
			{
				return m_sessionActive;
			}

			[[nodiscard]] const QString& targetPath() const
			{
				return m_targetPath;
			}

		private:
			bool m_sessionActive {false};
			QString m_targetPath;
	};

	QString createWWWPath(const int i)
	{
		return QString("http://path%1.com").arg(i);
	}

	MetaData createTrack(const int i, const QString& filepath)
	{
		MetaData track;
		track.setTitle(QString("title%1").arg(i));
		track.setArtist(QString("artist%1").arg(i));
		track.setFilepath(filepath);

		return track;
	}

	void createFile(const QString& filename)
	{
		const auto data = QByteArray(25000, 'x');
		auto f = QFile(filename);
		f.open(QFile::WriteOnly);
		f.write(data);
		f.close();
	}

	QString todaysDateString()
	{
		const auto currentDate = QDate::currentDate();
		return QString("%1%2%3")
			.arg(currentDate.year())
			.arg(currentDate.month(), 2, 10, QChar('0')) // NOLINT(readability-magic-numbers)
			.arg(currentDate.day(), 2, 10, QChar('0')); // NOLINT(readability-magic-numbers)
	}

	QMap<QString, QStringList> createStructure(const QString& path)
	{
		return {{path, {}}};
	}

	struct TestEnv
	{
		Util::FileSystemPtr fileSystem;
		PlayManager* playManager;
		std::shared_ptr<PipelineMock> pipeline;
		StreamRecorder::StreamRecorder streamRecorder;

		explicit TestEnv(const QString& tempPath) :
			fileSystem {std::make_shared<Test::FileSystemMock>(createStructure(tempPath))},
			playManager {new PlayManagerMock()},
			pipeline {std::make_shared<PipelineMock>()},
			streamRecorder {StreamRecorder::StreamRecorder(playManager, fileSystem, pipeline, nullptr)} {}
	};
}

class StreamRecorderTest :
	public Test::Base
{
	Q_OBJECT

	public:
		StreamRecorderTest() :
			Test::Base("StreamRecorderTest")
		{
			SetSetting(Set::Engine_SR_Path, tempPath());
			SetSetting(SetNoDB::MP3enc_found, true);
			SetSetting(Set::Engine_SR_Active, true);
			SetSetting(Set::Engine_SR_SessionPath, true);
			SetSetting(Set::Engine_SR_SessionPathTemplate, QString("<y><m><d>/<tn> - <t>"));
		}

	private slots:

		[[maybe_unused]] void testTargetPathTemplate() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto failedIndex = -1;
			auto* settings = Settings::instance();
			const auto sessionPathTemplate = settings->get<Set::Engine_SR_SessionPathTemplate>();
			const auto errorCode = SR::Utils::validateTemplate(sessionPathTemplate, &failedIndex);

			QVERIFY(errorCode == SR::Utils::ErrorCode::OK);
			QVERIFY(failedIndex == -1);
		}

		[[maybe_unused]] void
		testIfRecordFlagIsOffFilenameIsEmpty() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto testEnv = TestEnv {tempPath()};
			struct TestCase
			{
				int number;
			};

			const auto testCases = std::array {
				TestCase {1},
				TestCase {2},
				TestCase {8}
			};

			QVERIFY(!testEnv.pipeline->sessionActive());
			QVERIFY(!testEnv.streamRecorder.isRecording());

			for(const auto& testCase: testCases)
			{
				testEnv.streamRecorder.changeTrack(createTrack(testCase.number, createWWWPath(testCase.number)));
				QVERIFY(testEnv.pipeline->targetPath().isEmpty());
			}
		}

		[[maybe_unused]] void testNonWWWFileStopsSession() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto testEnv = TestEnv {tempPath()};
			struct TestCase
			{
				int number;
				bool isWWW;
			};

			const auto testCases = std::array {
				TestCase {1, true},
				TestCase {2, true},
				TestCase {8, false}
			};

			testEnv.streamRecorder.record(true);
			for(const auto& testCase: testCases)
			{
				const auto filepath = testCase.isWWW
				                      ? createWWWPath(testCase.number)
				                      : tempPath("someFile.mp3");

				testEnv.streamRecorder.changeTrack(createTrack(testCase.number, filepath));
				QVERIFY(testCase.isWWW == testEnv.streamRecorder.isRecording());
				QVERIFY(testCase.isWWW == testEnv.pipeline->sessionActive());
			}
		}

		[[maybe_unused]] void
		ifFilesAreSavedProperlyIndexesAreCounted() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto testEnv = TestEnv {tempPath()};
			struct TestCase
			{
				int number;
				QString expectedFilename;
			};

			const auto testCases = std::array {
				TestCase {1, "0001 - title1.mp3"},
				TestCase {2, "0002 - title2.mp3"},
				TestCase {8, "0003 - title8.mp3"}
			};

			testEnv.streamRecorder.record(true);
			for(const auto& testCase: testCases)
			{
				const auto track = createTrack(testCase.number, createWWWPath(testCase.number));

				testEnv.streamRecorder.changeTrack(track);

				const auto expectedFilename =
					QString("%1/%2/%3")
						.arg(tempPath())
						.arg(todaysDateString())
						.arg(testCase.expectedFilename);

				createFile(expectedFilename);

				QVERIFY(testEnv.pipeline->targetPath() == expectedFilename);
				QVERIFY(testEnv.streamRecorder.isRecording());
			}
		}

		[[maybe_unused]] void
		ifFileIsNotSavedTheIndexStaysTheSame() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto testEnv = TestEnv {tempPath()};
			struct TestCase
			{
				int number;
				QString expectedFilename;
			};

			const auto testCases = std::array {
				TestCase {1, "0001 - title1.mp3"},
				TestCase {2, "0001 - title2.mp3"},
				TestCase {3, "0001 - title3.mp3"}
			};

			testEnv.streamRecorder.record(true);
			for(const auto& testCase: testCases)
			{
				const auto track = createTrack(testCase.number, createWWWPath(testCase.number));

				testEnv.streamRecorder.changeTrack(track);

				const auto expectedFilename =
					QString("%1/%2/%3")
						.arg(tempPath())
						.arg(todaysDateString())
						.arg(testCase.expectedFilename);

				QVERIFY(testEnv.pipeline->targetPath() == expectedFilename);
				QVERIFY(testEnv.streamRecorder.isRecording());
			}
		}

		[[maybe_unused]] void
		testIndexStartsOverAfterNextSession() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto testEnv = TestEnv {tempPath()};
			struct TestCase
			{
				int number;
				bool isLastTrackOfSession;
				QString expectedFilename;
			};

			const auto testCases = std::array {
				TestCase {1, false, "0001 - title1.mp3"},
				TestCase {2, false, "0002 - title2.mp3"},
				TestCase {3, true, "0003 - title3.mp3"},
				TestCase {8, false, "0001 - title8.mp3"},
				TestCase {9, false, "0002 - title9.mp3"}
			};

			testEnv.streamRecorder.record(true);
			for(const auto& testCase: testCases)
			{
				const auto track = createTrack(testCase.number, createWWWPath(testCase.number));

				testEnv.streamRecorder.changeTrack(track);

				const auto expectedFilename =
					QString("%1/%2/%3")
						.arg(tempPath())
						.arg(todaysDateString())
						.arg(testCase.expectedFilename);

				createFile(expectedFilename);

				QVERIFY(testEnv.pipeline->targetPath() == expectedFilename);
				QVERIFY(testEnv.streamRecorder.isRecording());

				if(testCase.isLastTrackOfSession)
				{
					testEnv.streamRecorder.record(false); // end session
					testEnv.streamRecorder.record(true); // start new session
				}
			}
		}

		[[maybe_unused]] void
		testSessionStopsAfterStoppedSignalFrom() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto testEnv = TestEnv {tempPath()};
			struct TestCase
			{
				int number;
				bool isLastTrackOfSession;
				QString expectedFilename;
			};

			const auto testCases = std::array {
				TestCase {1, false, "0001 - title1.mp3"},
				TestCase {2, false, "0002 - title2.mp3"},
				TestCase {3, true, "0003 - title3.mp3"},
				TestCase {8, false, "0001 - title8.mp3"},
				TestCase {9, false, "0002 - title9.mp3"}
			};

			testEnv.streamRecorder.record(true);
			for(const auto& testCase: testCases)
			{
				const auto track = createTrack(testCase.number, createWWWPath(testCase.number));

				testEnv.streamRecorder.changeTrack(track);

				const auto expectedFilename =
					QString("%1/%2/%3")
						.arg(tempPath())
						.arg(todaysDateString())
						.arg(testCase.expectedFilename);

				createFile(expectedFilename);

				QVERIFY(testEnv.pipeline->targetPath() == expectedFilename);
				QVERIFY(testEnv.streamRecorder.isRecording());

				if(testCase.isLastTrackOfSession)
				{
					testEnv.playManager->sigPlaystateChanged(PlayState::Stopped);
					testEnv.streamRecorder.record(true);
				}
			}
		}

		[[maybe_unused]] void
		testSessionDoesNotComeUpIfRecordIsNotTriggeredAgain() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto testEnv = TestEnv {tempPath()};
			struct TestCase
			{
				int number;
				bool isLastTrackOfSession;
				QString expectedFilename;
				bool isRecording;
			};

			const auto testCases = std::array {
				TestCase {1, false, "0001 - title1.mp3", true},
				TestCase {2, false, "0002 - title2.mp3", true},
				TestCase {3, true, "0003 - title3.mp3", true},
				TestCase {8, false, QString(), false},
				TestCase {9, false, QString(), false}
			};

			testEnv.streamRecorder.record(true);
			for(const auto& testCase: testCases)
			{
				const auto track = createTrack(testCase.number, createWWWPath(testCase.number));

				testEnv.streamRecorder.changeTrack(track);

				const auto expectedFilename = testCase.expectedFilename.isEmpty()
				                              ? QString {}
				                              : QString("%1/%2/%3")
					                              .arg(tempPath())
					                              .arg(todaysDateString())
					                              .arg(testCase.expectedFilename);

				createFile(expectedFilename);

				const auto targetPath = testEnv.pipeline->targetPath();
				QVERIFY(testEnv.pipeline->targetPath() == expectedFilename);
				QVERIFY(testEnv.streamRecorder.isRecording() == testCase.isRecording);

				if(testCase.isLastTrackOfSession)
				{
					testEnv.playManager->sigPlaystateChanged(PlayState::Stopped);
				}
			}
		}
};

QTEST_GUILESS_MAIN(StreamRecorderTest)

#include "StreamRecorderTest.moc"

