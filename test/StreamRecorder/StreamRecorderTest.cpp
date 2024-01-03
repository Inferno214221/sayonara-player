/* StreamRecorderTest.cpp
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
#include "test/Common/FileSystemMock.h"
#include "test/Common/TaggingMocks.h"

#include "Components/Engine/StreamRecorder/StreamRecorder.h"
#include "Components/Engine/PipelineExtensions/StreamRecordable.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingRegistry.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/FileUtils.h"

#include <QDateTime>
#include <QFile>
#include <utility>

namespace SR = StreamRecorder;

namespace
{
	class PipelineMock :
		public PipelineExtensions::StreamRecordable
	{
		public:
			explicit PipelineMock(Util::FileSystemPtr fileSystem) :
				m_fileSystem {std::move(fileSystem)} {};
			~PipelineMock() override = default;

			void setRecordingPath(const QString& targetPath) override
			{
				const auto StreamedData = QByteArray {};

				m_targetPath = targetPath;
				if(m_isSaveEnabled)
				{
					m_fileSystem->writeFile(StreamedData, targetPath);
				}
			}

			void prepareForRecording() override
			{
				m_sessionActive = true;
			}

			void finishRecording() override
			{
				m_sessionActive = false;
				m_targetPath.clear();
			}

			[[nodiscard]] bool sessionActive() const
			{
				return m_sessionActive;
			}

			[[nodiscard]] const QString& targetPath() const
			{
				return m_targetPath;
			}

			void toggleSaveFile(const bool b)
			{
				m_isSaveEnabled = b;
			}

		private:
			bool m_isSaveEnabled {true};
			bool m_sessionActive {false};
			QString m_targetPath;
			Util::FileSystemPtr m_fileSystem;
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

	QString todaysDateString()
	{
		const auto currentDate = QDate::currentDate();
		return QString("%1%2%3")
			.arg(currentDate.year())
			.arg(currentDate.month(), 2, 10, QChar('0')) // NOLINT(readability-magic-numbers)
			.arg(currentDate.day(), 2, 10, QChar('0')); // NOLINT(readability-magic-numbers)
	}

	QMap<QString, QStringList> createFileStructure(const QString& path)
	{
		return {{path, {}}};
	}

	struct TestEnv
	{
		Util::FileSystemPtr fileSystem;
		Tagging::TagWriterPtr tagWriter;
		std::shared_ptr<PipelineMock> pipeline;
		StreamRecorder::StreamRecorder streamRecorder;

		explicit TestEnv(const QString& tempPath) :
			fileSystem {std::make_shared<Test::FileSystemMock>(createFileStructure(tempPath))},
			tagWriter {std::make_shared<Test::TagWriterMock>()},
			pipeline {std::make_shared<PipelineMock>(fileSystem)},
			streamRecorder {StreamRecorder::StreamRecorder(fileSystem, tagWriter, pipeline, nullptr)} {}
	};

	void createOrUpdateSession(TestEnv& testEnv, const bool create, const MetaData& track)
	{
		if(create)
		{
			if(testEnv.streamRecorder.isRecording())
			{
				testEnv.streamRecorder.endSession();
			}

			testEnv.streamRecorder.startNewSession(track);
		}

		else
		{
			testEnv.streamRecorder.updateMetadata(track);
		}
	}
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
			constexpr const auto trackNumbers = std::array {1, 2, 3};

			QVERIFY(!testEnv.pipeline->sessionActive());
			QVERIFY(!testEnv.streamRecorder.isRecording());

			for(const auto& trackNumber: trackNumbers)
			{
				const auto track = createTrack(trackNumber, createWWWPath(trackNumber));
				testEnv.streamRecorder.updateMetadata(track);
				QVERIFY(testEnv.pipeline->targetPath().isEmpty());
			}
		}

		[[maybe_unused]] void testNonWWWFileStopsSession() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto testEnv = TestEnv {tempPath()};
			struct TestCase
			{
				int number;
				bool isFirstTrack;
				bool isWWW;
			};

			const auto testCases = std::array {
				TestCase {1, true, true},
				TestCase {2, false, true},
				TestCase {8, false, false}
			};

			for(const auto& testCase: testCases)
			{
				const auto filepath = testCase.isWWW
				                      ? createWWWPath(testCase.number)
				                      : tempPath("someFile.mp3");
				const auto track = createTrack(testCase.number, filepath);

				createOrUpdateSession(testEnv, testCase.isFirstTrack, track);

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
				bool isFirstTrack;
				QString expectedFilename;
			};

			const auto testCases = std::array {
				TestCase {1, true, "0001 - title1.mp3"},
				TestCase {2, false, "0002 - title2.mp3"},
				TestCase {8, false, "0003 - title8.mp3"}
			};

			for(const auto& testCase: testCases)
			{
				const auto track = createTrack(testCase.number, createWWWPath(testCase.number));
				const auto expectedFilename =
					QString("%1/%2/%3")
						.arg(tempPath())
						.arg(todaysDateString())
						.arg(testCase.expectedFilename);

				createOrUpdateSession(testEnv, testCase.isFirstTrack, track);

				QVERIFY(testEnv.pipeline->targetPath() == expectedFilename);
				QVERIFY(testEnv.streamRecorder.isRecording());
			}
		}

		[[maybe_unused]] void
		ifFileIsNotSavedTheIndexStaysTheSame() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto testEnv = TestEnv {tempPath()};
			testEnv.pipeline->toggleSaveFile(false);

			struct TestCase
			{
				int number;
				bool isFirstTrack;
				QString expectedFilename;
			};

			const auto testCases = std::array {
				TestCase {1, true, "0001 - title1.mp3"},
				TestCase {2, false, "0001 - title2.mp3"},
				TestCase {3, false, "0001 - title3.mp3"}
			};

			for(const auto& testCase: testCases)
			{
				const auto track = createTrack(testCase.number, createWWWPath(testCase.number));
				createOrUpdateSession(testEnv, testCase.isFirstTrack, track);

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
				bool isFirstTrackOfSession;
				QString expectedFilename;
			};

			const auto testCases = std::array {
				TestCase {1, true, "0001 - title1.mp3"},
				TestCase {2, false, "0002 - title2.mp3"},
				TestCase {3, false, "0003 - title3.mp3"},
				TestCase {8, true, "0001 - title8.mp3"},
				TestCase {9, false, "0002 - title9.mp3"}
			};

			for(const auto& testCase: testCases)
			{
				const auto track = createTrack(testCase.number, createWWWPath(testCase.number));
				createOrUpdateSession(testEnv, testCase.isFirstTrackOfSession, track);

				const auto expectedFilename =
					QString("%1/%2/%3")
						.arg(tempPath())
						.arg(todaysDateString())
						.arg(testCase.expectedFilename);

				QVERIFY(testEnv.pipeline->targetPath() == expectedFilename);
				QVERIFY(testEnv.streamRecorder.isRecording());
			}
		}
};

QTEST_GUILESS_MAIN(StreamRecorderTest)

#include "StreamRecorderTest.moc"

