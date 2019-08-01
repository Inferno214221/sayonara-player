/* Language.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

class LanguageString : public QString
{
public:
	LanguageString(const QString& str);
	LanguageString toFirstUpper() const;
	LanguageString space() const;
	LanguageString question() const;
	LanguageString triplePt() const;

	LanguageString& operator=(const LanguageString& other);
};

class Lang :
		public QObject
{
	Q_OBJECT

public:
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

public:
	Lang();
	~Lang();

	static LanguageString get(Lang::Term term, bool* ok=nullptr);


	/*
	 * Eg: sayonara_lang_de.qm -> sayonara_lang_de_DE.qm
	 */
	static QString convert_old_lang(const QString& old_lang);
	static QMap<QString, QLocale> available_languages();

	static QString two_letter(const QString& language_name);
	static QString four_letter(const QString& language_name);
};

#endif // LANGUAGE_H
