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
#include "Common/PlayManagerMock.h"
#include "Common/TestTracks.h"
#include "Common/FileSystemMock.h"

#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistChangeNotifier.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Database/Connector.h"
#include "Database/Playlist.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Settings/Settings.h"

#include <QSignalSpy>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	QPair<int, QString> getNextPlaylistInfo()
	{
		static int playlistId = 0;

		playlistId++;
		return {playlistId, QString("new%1").arg(playlistId)};
	}

	::Playlist::Playlist createTestPlaylist(int index, const QString& name)
	{
		return {index, name, new PlayManagerMock(), std::make_shared<Test::AllFilesAvailableFileSystem>()};
	}

	template<typename A, typename B, typename Comparator>
	bool isEqual(const A& a, const B& b, Comparator comp)
	{
		const auto& tracks1 = a.tracks();
		const auto& tracks2 = b.tracks();
		if(tracks1.count() != tracks2.count())
		{
			return false;
		}

		const auto tracksEqual =
			std::equal(tracks1.begin(), tracks1.end(), tracks2.begin(), [](const auto& track1, const auto& track2) {
				return track1.isEqual(track2);
			});

		return comp(a, b) && tracksEqual;
	}

	template<typename A, typename B>
	bool isEqual(const A& a, const B& b)
	{
		auto comparator = [](const auto& a, const auto& b) {
			return (
				(a.id() == b.id()) &&
				(a.name() == b.name()) &&
				(a.isTemporary() == b.isTemporary())
			);
		};

		return isEqual(a, b, comparator);
	}
} // namespace

class PlaylistDbInterfaceTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistDbInterfaceTest() :
			Test::Base("PlaylistDbInterfaceTest"),
			m_playlistConnector {DB::Connector::instance()->playlistConnector()}
		{
			SetSetting(Set::PL_LoadSavedPlaylists, true);
			SetSetting(Set::PL_LoadTemporaryPlaylists, true);
		}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testPlaylistIsChangedWhenAddingTracks()
		{
			struct TestCase
			{
				QPair<int, QString> playlistInfo;
				bool save;
				bool expectedChanged;
			};

			const auto testCases = std::array {
				TestCase {getNextPlaylistInfo(), true, false},
				TestCase {getNextPlaylistInfo(), false, true},

			};

			for(const auto& testCase: testCases)
			{
				const auto tracks = Test::createTracks();
				auto playlist = createTestPlaylist(testCase.playlistInfo.first, testCase.playlistInfo.second);
				playlist.createPlaylist(tracks);
				QVERIFY(playlist.wasChanged());

				if(testCase.save)
				{
					playlist.save();
				}

				QVERIFY(playlist.wasChanged() == testCase.expectedChanged);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testDatabasePlaylistIsEqualToOriginalPlaylist()
		{
			const auto pipelineInfo = getNextPlaylistInfo();

			const auto tracks = Test::createTracks();
			auto playlist = createTestPlaylist(pipelineInfo.first, pipelineInfo.second);
			playlist.createPlaylist(tracks);
			playlist.save();

			const auto dbPlaylist = m_playlistConnector->getPlaylistById(playlist.id(), true);
			QVERIFY(isEqual(playlist, dbPlaylist));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testAppendSingleTrackChangesPlaylist()
		{
			const auto pipelineInfo = getNextPlaylistInfo();

			auto playlist = createTestPlaylist(pipelineInfo.first, pipelineInfo.second);
			const auto track = Test::createTrack(0, "title", "artist", "album");

			playlist.save();
			::Playlist::appendTracks(playlist, MetaDataList {track}, Playlist::Reason::Undefined);

			QVERIFY(!isEqual(playlist, m_playlistConnector->getPlaylistById(playlist.id(), true)));
			playlist.save();

			QVERIFY(isEqual(playlist, m_playlistConnector->getPlaylistById(playlist.id(), true)));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testChangeNotifierIsFired()
		{
			using Playlist::ChangeNotifier;
			const auto playlistInfo = getNextPlaylistInfo();
			const auto newName = playlistInfo.second + " new";

			auto playlist = createTestPlaylist(playlistInfo.first, playlistInfo.second);
			{ // save temporary playlist
				auto* changeNotifier = ChangeNotifier::instance();
				auto spy = QSignalSpy(changeNotifier, &ChangeNotifier::sigPlaylistAdded);

				const auto answer = playlist.save();
				QVERIFY(answer == Util::SaveAsAnswer::Success);
				QVERIFY(spy.count() == 0);
			}

			{ // rename temporary playlist
				const auto oldName = playlist.name();
				auto* changeNotifier = ChangeNotifier::instance();
				auto spy = QSignalSpy(changeNotifier, &ChangeNotifier::sigPlaylistRenamed);

				const auto answer = playlist.rename(newName);
				QVERIFY(answer == Util::SaveAsAnswer::Success);
				QVERIFY(spy.count() == 1);
				QVERIFY(spy.value(0)[0].toInt() == playlist.id());
				QVERIFY(spy.value(0)[1].toString() == oldName);
				QVERIFY(spy.value(0)[2].toString() == playlist.name());
			}

			{ // delete temporary playlist
				const auto id = playlist.id();
				auto* changeNotifier = ChangeNotifier::instance();
				auto spy = QSignalSpy(changeNotifier, &ChangeNotifier::sigPlaylistDeleted);

				const auto success = playlist.deletePlaylist();
				QVERIFY(success);
				QVERIFY(spy.count() == 1);
				QVERIFY(spy.value(0).first().toInt() == id);
			}

			{ // save permanent playlist
				auto* changeNotifier = ChangeNotifier::instance();
				auto spy = QSignalSpy(changeNotifier, &ChangeNotifier::sigPlaylistAdded);

				const auto answer = playlist.saveAs(playlist.name());
				QVERIFY(answer == Util::SaveAsAnswer::Success);
				QVERIFY(spy.count() == 1);
				QVERIFY(spy.value(0)[0] == playlist.id());
				QVERIFY(spy.value(0)[1] == playlist.name());
			}

			{ // rename permanent playlist
				const auto oldName = playlist.name();
				auto* changeNotifier = ChangeNotifier::instance();
				auto spy = QSignalSpy(changeNotifier, &ChangeNotifier::sigPlaylistRenamed);

				const auto answer = playlist.rename(newName);
				QVERIFY(answer == Util::SaveAsAnswer::Success);
				QVERIFY(spy.count() == 1);
				QVERIFY(spy.value(0)[0].toInt() == playlist.id());
				QVERIFY(spy.value(0)[1].toString() == oldName);
				QVERIFY(spy.value(0)[2].toString() == playlist.name());
			}

			{ // delete permanent playlist
				const auto id = playlist.id();
				auto* changeNotifier = ChangeNotifier::instance();
				auto spy = QSignalSpy(changeNotifier, &ChangeNotifier::sigPlaylistDeleted);

				const auto success = playlist.deletePlaylist();
				QVERIFY(success);
				QVERIFY(spy.count() == 1);
				QVERIFY(spy.value(0).first().toInt() == id);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testDeletion()
		{
			const auto playlistInfo = getNextPlaylistInfo();
			auto playlist = createTestPlaylist(playlistInfo.first, playlistInfo.second);
			{ // save playlist
				const auto answer = playlist.save();
				QVERIFY(answer == Util::SaveAsAnswer::Success);
				QVERIFY(playlist.id() >= 0);

			}

			{ // delete playlist
				const auto success = playlist.deletePlaylist();
				QVERIFY(success);
				QVERIFY(playlist.id() < 0);
			}

			{ // rename playlist
				const auto newName = playlistInfo.second + " new";
				const auto answer = playlist.rename(newName);
				QVERIFY(answer == Util::SaveAsAnswer::OtherError);
				QVERIFY(playlist.id() < 0);
			}

			{ // save playlist again
				const auto answer = playlist.saveAs(playlist.name());
				QVERIFY(answer == Util::SaveAsAnswer::Success);
				QVERIFY(playlist.id() >= 0);
				QVERIFY(!playlist.isTemporary());
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testInsertAndSaveAsInvalidName()
		{
			const auto pipelineInfoOrg = getNextPlaylistInfo();
			const auto pipelineInfoOther = getNextPlaylistInfo();
			auto playlist = createTestPlaylist(pipelineInfoOrg.first, pipelineInfoOrg.second);
			auto otherPlaylist = createTestPlaylist(pipelineInfoOther.first, pipelineInfoOther.second);
			{ // save playlist
				const auto answer = playlist.save();
				QVERIFY(answer == Util::SaveAsAnswer::Success);
			}

			{ // save other playlist
				const auto answer = otherPlaylist.save();
				QVERIFY(answer == Util::SaveAsAnswer::Success);
			}

			{ // try to save playlist as other playlist's name
				const auto oldName = playlist.name();
				const auto newName = otherPlaylist.name();
				const auto answer = playlist.saveAs(newName);
				QVERIFY(answer == Util::SaveAsAnswer::NameAlreadyThere);
				QVERIFY(playlist.name() == oldName);

				const auto dbPlaylist = m_playlistConnector->getPlaylistById(playlist.id(), false);
				QVERIFY(isEqual(playlist, dbPlaylist));
			}

			{ // try to save as empty name
				const auto answer = playlist.saveAs(QString());
				QVERIFY(answer == Util::SaveAsAnswer::InvalidName);

				const auto dbPlaylist = m_playlistConnector->getPlaylistById(playlist.id(), false);
				QVERIFY(isEqual(playlist, dbPlaylist));
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testInsertAndRename()
		{
			const auto playlistInfo = getNextPlaylistInfo();
			auto playlist = createTestPlaylist(playlistInfo.first, playlistInfo.second);

			QVERIFY(playlist.id() < 0);
			QVERIFY(playlist.name() == playlistInfo.second);
			QVERIFY(playlist.isTemporary());

			{ // save
				const auto answer = playlist.save();
				QVERIFY(answer == Util::SaveAsAnswer::Success);
				QVERIFY(playlist.id() >= 0);
				QVERIFY(playlist.isTemporary());

				const auto dbPlaylist = m_playlistConnector->getPlaylistById(playlist.id(), false);
				QVERIFY(isEqual(playlist, dbPlaylist));
			}

			{ // rename
				const auto newName = playlistInfo.second + " renamed";
				const auto answer = playlist.rename(newName);
				QVERIFY(answer == Util::SaveAsAnswer::Success);
				QVERIFY(playlist.name() == newName);
				QVERIFY(playlist.isTemporary());

				const auto dbPlaylist = m_playlistConnector->getPlaylistById(playlist.id(), false);
				QVERIFY(isEqual(playlist, dbPlaylist));
			}

			{ // rename other playlist to same name
				const auto newName = playlistInfo.second + " renamed again";
				auto otherPlaylist = createTestPlaylist(0, newName);
				QVERIFY(otherPlaylist.save() == Util::SaveAsAnswer::Success);

				const auto answer = otherPlaylist.rename(playlist.name());
				QVERIFY(answer == Util::SaveAsAnswer::NameAlreadyThere);
				QVERIFY(otherPlaylist.name() == newName);

				const auto dbPlaylist = m_playlistConnector->getPlaylistById(otherPlaylist.id(), false);
				QVERIFY(isEqual(otherPlaylist, dbPlaylist));
			}

			{ // rename playlist which isn't saved yet
				const auto newName = playlistInfo.second + " and again renamed";
				auto otherPlaylist = createTestPlaylist(0, newName);
				const auto answer = otherPlaylist.rename("some name");
				QVERIFY(answer == Util::SaveAsAnswer::OtherError);
				QVERIFY(otherPlaylist.id() < 0);
				QVERIFY(otherPlaylist.name() == newName);
			}
		}

	private:
		DB::Playlist* m_playlistConnector;
};

QTEST_GUILESS_MAIN(PlaylistDbInterfaceTest)

#include "PlaylistDbInterfaceTest.moc"
