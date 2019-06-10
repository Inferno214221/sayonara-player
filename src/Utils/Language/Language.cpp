/* Language.cpp */

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

#include "Language.h"
#include "Utils.h"

#include <QDir>
#include <QRegExp>
#include <QStringList>
#include <QLocale>

LanguageString::LanguageString(const QString& str) :
	QString(str) {}

LanguageString LanguageString::toFirstUpper() const
{
	return LanguageString(Util::cvt_str_to_first_upper(*this));
}

LanguageString LanguageString::space() const
{
	LanguageString str = *this;
	return str + " ";
}

LanguageString LanguageString::question() const
{
	LanguageString str = *this;
	return str + "?";
}

LanguageString LanguageString::triplePt() const
{
	LanguageString str = *this;
	return str + "...";
}

Lang::Lang() {}

Lang::~Lang() {}

LanguageString Lang::get(Lang::Term term, bool* ok)
{
	if(ok){
		*ok = true;
	}

	Lang l;
	switch(term)
	{
		case About:
			return l.tr("About");
		case Action:
			return l.tr("Action");
		case Actions:
			return l.tr("Actions");
		case Activate:
			return l.tr("Activate");
		case Active:
			return l.tr("Active");
		case Add:
			return l.tr("Add");
		case AddTab:
			return l.tr("Add tab");
		case Album:
			return l.tr("Album");
		case AlbumArtists:
			return l.tr("Album artists");
		case Albums:
			return l.tr("Albums");
		case All:
			return l.tr("All");
		case Append:
			return l.tr("Append");
		case Application:
			return l.tr("Application");
		case Apply:
			return l.tr("Apply");
		case Artist:
			return l.tr("Artist");
		case Artists:
			return l.tr("Artists");
		case Ascending:
			return l.tr("Ascending");
		case Automatic:
			return l.tr("Automatic");
		case Bitrate:
			return l.tr("Bitrate");
		case Bookmarks:
			return l.tr("Bookmarks");
		case Broadcast:
			return l.tr("Broadcast");
		case By:
			// "Beat it" by "Michael Jackson"
			return l.tr("by");
		case Cancel:
			return l.tr("Cancel");
		case CannotFindLame:
			return l.tr("Cannot find Lame MP3 encoder");
		case Clear:
			return l.tr("Clear");
		case Close:
			return l.tr("Close");
		case CloseOthers:
			return l.tr("Close others");
		case CloseTab:
			return l.tr("Close tab");
		case Comment:
			return l.tr("Comment");
		case Continue:
			return l.tr("Continue");
		case Covers:
			return l.tr("Covers");
		case CreateNewLibrary:
			return l.tr("Create a new library");
		case DarkMode:
			return l.tr("Dark Mode");
		case Date:
			return l.tr("Date");
		case Days:
			return l.tr("Days");
		case DaysShort:
			// short form of day
			return l.tr("d");
		case Default:
			return l.tr("Default");
		case Delete:
			return l.tr("Delete");
		case Descending:
			return l.tr("Descending");
		case Directory:
			return l.tr("Directory");
		case Directories:
			return l.tr("Directories");
		case Disc:
			return l.tr("Disc");
		case Duration:
			return l.tr("Duration");
		case DurationShort:
			// short form of duration
			return l.tr("Dur.");
		case DynamicPlayback:
			return l.tr("Dynamic playback");
		case Edit:
			return l.tr("Edit");
		case EmptyInput:
			return l.tr("Empty input");
		case EnterName:
			return l.tr("Enter name");
		case EnterUrl:
			return l.tr("Enter URL");
		case Entry:
			return l.tr("Entry");
		case Entries:
			return l.tr("Entries");
		case Error:
			return l.tr("Error");
		case Fast:
			return l.tr("Fast");
		case File:
			return l.tr("File");
		case Filename:
			return l.tr("Filename");
		case Files:
			return l.tr("Files");
		case Filesize:
			return l.tr("Filesize");
		case Filetype:
			return l.tr("File type");
		case Filter:
			return l.tr("Filter");
		case First:
			return l.tr("1st");
		case Font:
			return l.tr("Font");
		case Fonts:
			return l.tr("Fonts");
		case Fulltext:
			return l.tr("Fulltext");
		case GaplessPlayback:
			return l.tr("Gapless playback");
		case Genre:
			return l.tr("Genre");
		case Genres:
			return l.tr("Genres");
		case Hours:
			return l.tr("Hours");
		case HoursShort:
			// short form of hours
			return l.tr("h");
		case ImportDir:
			return l.tr("Import directory");
		case ImportFiles:
			return l.tr("Import files");
		case Inactive:
			return l.tr("Inactive");
		case Info:
			return l.tr("Info");
		case Loading:
			return l.tr("Loading");
		case LoadingArg:
			return l.tr("Loading %1");
		case InvalidChars:
			return l.tr("Invalid characters");
		case Key_Find:
			return l.tr("Ctrl+f");
		case Key_Delete:
			return l.tr("Delete");
		case Key_Escape:
			return l.tr("Esc");
		case Key_Control:
			return l.tr("Ctrl");
		case Key_Alt:
			return l.tr("Alt");
		case Key_Shift:
			return l.tr("Shift");
		case Key_Backspace:
			return l.tr("Backspace");
		case Key_Tab:
			return l.tr("Tab");
		case Library:
			return l.tr("Library");
		case LibraryPath:
			return l.tr("Library path");
		case Listen:
			return l.tr("Listen");
		case LiveSearch:
			return l.tr("Live Search");
		case Logger:
			return l.tr("Logger");
		case Lyrics:
			return l.tr("Lyrics");
		case Menu:
			return l.tr("Menu");
		case Minimize:
			return l.tr("Minimize");
		case Minutes:
			return l.tr("Minutes");
		case MinutesShort:
			// short form of minutes
			return l.tr("m");
		case Missing:
			return l.tr("Missing");
		case Months:
			return l.tr("Months");
		case MuteOn:
			return l.tr("Mute");
		case MuteOff:
			return l.tr("Mute off");
		case Name:
			return l.tr("Name");
		case New:
			return l.tr("New");
		case NextPage:
			return l.tr("Next page");
		case NextTrack:
			return l.tr("Next track");
		case No:
			return l.tr("No");
		case None:
			return l.tr("None");
		case NumTracks:
			return QString("#") + l.tr("Tracks");
		case MoveDown:
			return l.tr("Move down");
		case MoveUp:
			return l.tr("Move up");
		case OK:
			return l.tr("OK");
		case On:
			// 5th track on "Thriller"
			return l.tr("on");
		case Open:
			return l.tr("Open");
		case OpenDir:
			return l.tr("Open directory");
		case OpenFile:
			return l.tr("Open file");
		case Or:
			return l.tr("or");
		case Overwrite:
			return l.tr("Overwrite");
		case Pause:
			return l.tr("Pause");
		case Play:
			return l.tr("Play");
		case PlayingTime:
			return l.tr("Playing time");
		case PlayInNewTab:
			return l.tr("Play in new tab");
		case Playlist:
			return l.tr("Playlist");
		case Playlists:
			return l.tr("Playlists");
		case PlayNext:
			return l.tr("Play next");
		case PlayPause:
			return l.tr("Play/Pause");
		case Plugin:
			return l.tr("Plugin");
		case Podcasts:
			return l.tr("Podcasts");
		case Preferences:
			return l.tr("Preferences");
		case PreviousPage:
			return l.tr("Previous page");
		case PreviousTrack:
			return l.tr("Previous track");
		case Quit:
			return l.tr("Quit");
		case Radio:
			return l.tr("Radio");
		case RadioStation:
			return l.tr("Radio Station");
		case Rating:
			return l.tr("Rating");
		case Really:
			return l.tr("Really");
		case Refresh:
			return l.tr("Refresh");
		case ReloadLibrary:
			return l.tr("Reload Library");
		case Remove:
			return l.tr("Remove");
		case Rename:
			return l.tr("Rename");
		case Repeat1:
			return l.tr("Repeat 1");
		case RepeatAll:
			return l.tr("Repeat all");
		case Replace:
			return l.tr("Replace");
		case Reset:
			return l.tr("Reset");
		case Retry:
			return l.tr("Retry");
		case Sampler:
			return l.tr("Sampler");
		case Shuffle:
			return l.tr("Shuffle");
		case Shutdown:
			return l.tr("Shutdown");
		case Save:
			return l.tr("Save");
		case SaveAs:
			return l.tr("Save as");
		case SaveToFile:
			return l.tr("Save to file");
		case SearchNoun:
			return l.tr("Search");			// the noun of search
		case SearchVerb:
			return l.tr("Search");			// the verb of the searching process
		case SearchNext:
			return l.tr("Search next");
		case SearchPrev:
			return l.tr("Search previous");
		case Second:
			return l.tr("2nd");
		case Seconds:
			return l.tr("Seconds");
		case SecondsShort:
			// short form of seconds
			return l.tr("s");
		case SeekBackward:
			return l.tr("Seek backward");
		case SeekForward:
			return l.tr("Seek forward");
		case Show:
			return l.tr("Show");
		case ShowAlbumArtists:
			return l.tr("Show Album Artists");
		case ShowCovers:
			return l.tr("Show Covers");
		case ShowLibrary:
			return l.tr("Show Library");
		case SimilarArtists:
			return l.tr("Similar artists");
		case SortBy:
			return l.tr("Sort by");			// for example "sort by year"
		case Stop:
			return l.tr("Stop");
		case Streams:
			return l.tr("Streams");
		case StreamUrl:
			return l.tr("Stream URL");
		case Success:
			return l.tr("Success");
		case Th:
			return l.tr("th");
		case Third:
			return l.tr("3rd");
		case Title:
			return l.tr("Title");
		case Track:
			return l.tr("Track");
		case TrackNo:
			return l.tr("Track number");
		case TrackOn:
			return l.tr("track on");
		case Tracks:
			return l.tr("Tracks");
		case Tree:
			return l.tr("Tree");
		case Undo:
			return l.tr("Undo");
		case UnknownPlaceholder:
			return l.tr("Unknown placeholder");
		case Various:
			return l.tr("Various");
		case VariousAlbums:
			return l.tr("Various albums");
		case VariousArtists:
			return l.tr("Various artists");
		case VariousTracks:
			return l.tr("Various tracks");
		case Version:
			return l.tr("Version");
		case VolumeDown:
			return l.tr("Volume down");
		case VolumeUp:
			return l.tr("Volume up");
		case Warning:
			return l.tr("Warning");
		case Weeks:
			return l.tr("Weeks");
		case Year:
			return l.tr("Year");
		case Years:
			return l.tr("Years");
		case Yes:
			return l.tr("Yes");
		case Zoom:
			return l.tr("Zoom");
		default:
			if(ok){
				*ok = false;
			}
			return QString();
	}
}

QString Lang::convert_old_lang(const QString& old_lang)
{
	QString tl = two_letter(old_lang);
	if(tl.count() >= 2)
	{
		QMap<QString, QLocale> languages = available_languages();
		for(const QString& four_letter : languages.keys())
		{
			if(four_letter.startsWith(tl)){
				return four_letter;
			}
		}
	}

	return "en_US";
}



QMap<QString, QLocale> Lang::available_languages()
{
	QMap<QString, QLocale> ret;

	const QList<QDir> directories
	{
		QDir(Util::share_path("translations")),
		QDir(Util::sayonara_path("translations"))
	};

	for(const QDir& d : directories)
	{
		if(!d.exists()) {
			continue;
		}

		const QStringList entries = d.entryList(QStringList{"*.qm"}, QDir::Files);

		for(const QString& entry : entries)
		{
			QString fl = four_letter(entry);
			if(!fl.isEmpty()){
				ret[fl] = QLocale(fl);
			}
		}
	}

	return ret;
}


QString Lang::two_letter(const QString& language_name)
{
	QRegExp re(".*lang_(.+)(_.*)*.qm");
	int idx = re.indexIn(language_name);
	if(idx < 0)
	{
		return QString();
	}

	return re.cap(1);
}

QString Lang::four_letter(const QString& language_name)
{
	QRegExp re(".*lang_(.+).qm");
	int idx = re.indexIn(language_name);
	if(idx < 0)
	{
		return QString();
	}

	QString ret = re.cap(1);
	if(ret.count() == 5)
	{
		return ret;
	}

	return QString();
}
