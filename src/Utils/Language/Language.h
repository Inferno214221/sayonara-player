/* Language.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QLocale>

/**
 * @brief The LanguageString class
 * @ingroup Language
 */
class LanguageString :
	public QString
{
	public:
		LanguageString(const QString& other);
		LanguageString(const LanguageString& other) = default;

		LanguageString& operator=(const QString& other);
		LanguageString& operator=(const LanguageString& other) = default;

		LanguageString& toFirstUpper();

		LanguageString& space();

		LanguageString& question();

		LanguageString& triplePt();
};

class Lang :
	public QObject
{
	Q_OBJECT

	public:
		enum Term
		{
			About = 0,
			Action,
			Actions,
			Activate,
			Active,
			Add,
			AddArtist,
			AddTab,
			Album,
			AlbumArtist,
			AlbumArtists,
			Albums,
			All,
			Append,
			Application,
			Apply,
			Artist,
			Artists,
			Ascending,
			Automatic,
			Bitrate,
			Bookmarks,
			Broadcast,
			By,
			Cancel,
			CannotFindLame,
			CaseInsensitive,
			Comment,
			Continue,
			Covers,
			Clear,
			ClearSelection,
			Close,
			CloseOthers,
			CloseTab,
			CreateDirectory,
			Created,
			CreateNewLibrary,
			DarkMode,
			Date,
			Days,
			DaysShort,
			Default,
			Delete,
			Descending,
			Directory,
			Directories,
			Disc,
			Duration,
			DurationShort,
			DynamicPlayback,
			Edit,
			EmptyInput,
			EnterName,
			EnterNewName,
			EnterUrl,
			Entries,
			Entry,
			Error,
			Fast,
			File,
			Filename,
			Files,
			Filesize,
			Filetype,
			Filter,
			First,
			Font,
			Fonts,
			Fulltext,
			GaplessPlayback,
			GB,
			Genre,
			Genres,
			Hide,
			Hours,
			HoursShort,
			IgnoreSpecialChars,
			IgnoreAccents,
			ImportDir,
			ImportFiles,
			Inactive,
			Info,
			InvalidChars,
			KB,
			Key_Find,
			Key_Delete,
			Key_Escape,
			Key_Control,
			Key_Alt,
			Key_Shift,
			Key_Backspace,
			Key_Tab,
			Library,
			LibraryPath,
			LibraryView,
			Listen,
			LiveSearch,
			Loading,
			LoadingArg,
			Logger,
			LogLevel,
			Lyrics,
			MB,
			Menu,
			Minimize,
			Minutes,
			MinutesShort,
			Missing,
			Modified,
			Months,
			MoveDown,
			MoveUp,
			MuteOn,
			MuteOff,
			Name,
			New,
			NextPage,
			NextTrack,
			No,
			NoAlbums,
			NumTracks,
			OK,
			On,
			Open,
			OpenDir,
			OpenFile,
			Or,
			Overwrite,
			Pause,
			Play,
			PlayingTime,
			PlayInNewTab,
			Playlist,
			Playlists,
			PlayNext,
			PlayPause,
			Plugin,
			Podcasts,
			Preferences,
			PreviousPage,
			PreviousTrack,
			PurchaseUrl,
			Quit,
			Radio,
			RadioStation,
			Rating,
			Really,
			Refresh,
			ReloadLibrary,
			Remove,
			Rename,
			Repeat1,
			RepeatAll,
			Replace,
			Reset,
			Retry,
			ReverseOrder,
			Sampler,
			Save,
			SaveAs,
			SaveToFile,
			ScanForFiles,
			SearchNoun,
			SearchVerb,
			SearchNext,
			SearchPrev,
			Second,
			Seconds,
			SecondsShort,
			SeekForward,
			SeekBackward,
			Show,
			ShowAlbumArtists,
			ShowCovers,
			ShowLibrary,
			Shuffle,
			ShufflePlaylist,
			Shutdown,
			SimilarArtists,
			SmartPlaylists,
			SortBy,
			Stop,
			Streams,
			StreamUrl,
			Success,
			Th,
			Third,
			Title,
			Track,
			TrackOn,
			TrackNo,
			Tracks,
			Tree,
			Undo,
			UnknownAlbum,
			UnknownArtist,
			UnknownTitle,
			UnknownGenre,
			UnknownYear,
			UnknownPlaceholder,
			Various,
			VariousAlbums,
			VariousArtists,
			VariousTracks,
			Version,
			VolumeDown,
			VolumeUp,
			Warning,
			Weeks,
			Year,
			Years,
			Yes,
			Zoom,
			NUMBER_OF_LANGUAGE_KEYS
		};

		enum TermNr
		{
			NrDirectories = 0,
			NrFiles,
			NrPlaylists,
			NrTracks,
			NrTracksFound,
			NUMBER_OF_LANGUAGE_PARAM_KEYS
		};

	public:
		Lang();
		~Lang();

		static LanguageString get(Lang::Term term, bool* ok = nullptr);

		static LanguageString getWithNumber(Lang::TermNr term, int param, bool* ok = nullptr);
};

#endif // LANGUAGE_H
