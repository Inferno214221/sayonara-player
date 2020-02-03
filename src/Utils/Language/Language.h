/* Language.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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
class LanguageString : public QString
{
public:
	LanguageString(const QString& str);

	/**
	 * @brief Convert first character to upper case and rest to lower case
	 */
	LanguageString toFirstUpper() const;

	/**
	 * @brief appends a space
	 */
	LanguageString space() const;

	/**
	 * @brief appends question mark
	 */
	LanguageString question() const;

	/**
	 * @brief Appends triple points
	 */
	LanguageString triplePt() const;

	LanguageString& operator=(const LanguageString& other);
};

/**
 * @brief The Lang class
 * @ingroup Language
 */
class Lang :
		public QObject
{
	Q_OBJECT

public:

	/**
	 * @brief An enum for the most common translation files.
	 * use with Lang::get()
	 */
	enum Term
	{
		About=0,
		Action,
		Actions,
		Activate,
		Active,
		Add,
		AddTab,
		Album,
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
		Comment,
		Continue,
		Covers,
		Clear,
		Close,
		CloseOthers,
		CloseTab,
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
		Genre,
		Genres,
		Hide,
		Hours,
		HoursShort,
		ImportDir,
		ImportFiles,
		Inactive,
		Info,
		InvalidChars,
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
		Listen,
		LiveSearch,
		Loading,
		LoadingArg,
		Logger,
		Lyrics,
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
		Shutdown,
		SimilarArtists,
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
		NrDirectories=0,
		NrFiles,
		NrPlaylists,
		NrTracks,
		NrTracksFound,
		NUMBER_OF_LANGUAGE_PARAM_KEYS
	};


public:
	Lang();
	~Lang();

	/**
	 * @brief Fetch translation by enum
	 * @param enum term describing the source word
	 * @param ok optional pointer for return value
	 * @return translated text
	 */
	static LanguageString get(Lang::Term term, bool* ok=nullptr);

	static LanguageString get_with_number(Lang::TermNr term, int param, bool* ok=nullptr);

	/**
	 * @brief Converts two letter into four letter
	 * @param two letter language filename
	 * @return four letter key if available, en_GB per default
	 */
	static QString convert_old_lang(const QString& old_lang);


	/**
	 * @brief Returns all languages located in user path and all
	 * languages in sayonara path
	 * @return map with four letter key as key and the locale as value
	 */
	static QMap<QString, QLocale> available_languages();

	/**
	 * @brief Returns the two letter representation of a language
	 * @param language_name e.g. ...lang_DE_de.qm.
	 * @return DE for the example above
	 */
	static QString two_letter(const QString& language_name);

	/**
	 * @brief Returns the four letter representation of a language
	 * @param language_name e.g. ...lang_DE_de.qm.
	 * @return DE for the example above
	 */
	static QString four_letter(const QString& language_name);
};

#endif // LANGUAGE_H
