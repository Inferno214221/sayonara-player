/* Language.cpp */

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

#include "Language.h"
#include "Algorithm.h"
#include "StandardPaths.h"
#include "Utils.h"

#include <QDir>
#include <QRegExp>
#include <QStringList>
#include <QLocale>

LanguageString::LanguageString(const QString& other) :
	QString(other) {}

LanguageString& LanguageString::operator=(const QString& other)
{
	QString::operator=(other);
	return *this;
}

LanguageString& LanguageString::toFirstUpper()
{
	this->replace(0, 1, this->at(0).toUpper());
	return *this;
}

LanguageString& LanguageString::space()
{
	this->append(' ');
	return *this;
}

LanguageString& LanguageString::question()
{
	this->append('?');
	return *this;
}

LanguageString& LanguageString::triplePt()
{
	this->append("...");
	return *this;
}

Lang::Lang() = default;

Lang::~Lang() = default;

LanguageString Lang::get(const Lang::Term term, bool* ok)
{
	if(ok)
	{
		*ok = true;
	}

	switch(term)
	{
		case About:
			return QObject::tr("About");
		case Action:
			return QObject::tr("Action");
		case Actions:
			return QObject::tr("Actions");
		case Activate:
			return QObject::tr("Activate");
		case Active:
			return QObject::tr("Active");
		case Add:
			return QObject::tr("Add");
		case AddArtist:
			return QObject::tr("Add artist");
		case AddTab:
			return QObject::tr("Add tab");
		case Album:
			return QObject::tr("Album");
		case AlbumArtist:
			return QObject::tr("Album artist");
		case AlbumArtists:
			return QObject::tr("Album artists");
		case Albums:
			return QObject::tr("Albums");
		case All:
			return QObject::tr("All");
		case Append:
			return QObject::tr("Append");
		case Application:
			return QObject::tr("Application");
		case Apply:
			return QObject::tr("Apply");
		case Artist:
			return QObject::tr("Artist");
		case Artists:
			return QObject::tr("Artists");
		case Ascending:
			return QObject::tr("Ascending");
		case Automatic:
			return QObject::tr("Automatic");
		case Bitrate:
			return QObject::tr("Bitrate");
		case Bookmarks:
			return QObject::tr("Bookmarks");
		case Broadcast:
			return QObject::tr("Broadcast");
		case By:
			// "Beat it" by "Michael Jackson"
			return QObject::tr("by");
		case Cancel:
			return QObject::tr("Cancel");
		case CannotFindLame:
			return QObject::tr("Cannot find Lame MP3 encoder");
		case CaseInsensitive:
			return QObject::tr("Case insensitive");
		case Clear:
			return QObject::tr("Clear");
		case ClearSelection:
			return QObject::tr("Clear selection");
		case Close:
			return QObject::tr("Close");
		case CloseOthers:
			return QObject::tr("Close others");
		case CloseTab:
			return QObject::tr("Close tab");
		case Comment:
			return QObject::tr("Comment");
		case Continue:
			return QObject::tr("Continue");
		case Covers:
			return QObject::tr("Covers");
		case Created:
			return QObject::tr("Created");
		case CreateDirectory:
			return QObject::tr("Create new directory");
		case CreateNewLibrary:
			return QObject::tr("Create a new library");
		case DarkMode:
			return QObject::tr("Dark Mode");
		case Date:
			return QObject::tr("Date");
		case Days:
			return QObject::tr("Days");
		case DaysShort:
			// short form of day
			return QObject::tr("d");
		case Default:
			return QObject::tr("Default");
		case Delete:
			return QObject::tr("Delete");
		case Descending:
			return QObject::tr("Descending");
		case Directory:
			return QObject::tr("Directory");
		case Directories:
			return QObject::tr("Directories");
		case Disc:
			return QObject::tr("Disc");
		case Duration:
			return QObject::tr("Duration");
		case DurationShort:
			// short form of duration
			return QObject::tr("Dur.");
		case DynamicPlayback:
			return QObject::tr("Dynamic playback");
		case Edit:
			return QObject::tr("Edit");
		case EmptyInput:
			return QObject::tr("Empty input");
		case EnterName:
			return QObject::tr("Enter name");
		case EnterNewName:
			return QObject::tr("Please enter new name");
		case EnterUrl:
			return QObject::tr("Enter URL");
		case Entry:
			return QObject::tr("Entry");
		case Entries:
			return QObject::tr("Entries");
		case Error:
			return QObject::tr("Error");
		case Fast:
			return QObject::tr("Fast");
		case File:
			return QObject::tr("File");
		case Filename:
			return QObject::tr("Filename");
		case Files:
			return QObject::tr("Files");
		case Filesize:
			return QObject::tr("Filesize");
		case Filetype:
			return QObject::tr("File type");
		case Filter:
			return QObject::tr("Filter");
		case First:
			return QObject::tr("1st");
		case Font:
			return QObject::tr("Font");
		case Fonts:
			return QObject::tr("Fonts");
		case Fulltext:
			return QObject::tr("Fulltext");
		case GaplessPlayback:
			return QObject::tr("Gapless playback");
		case GB:
			return QObject::tr("GB");
		case Genre:
			return QObject::tr("Genre");
		case Genres:
			return QObject::tr("Genres");
		case Hide:
			return QObject::tr("Hide");
		case Hours:
			return QObject::tr("Hours");
		case HoursShort:
			// short form of hours
			return QObject::tr("h");
		case IgnoreAccents:
			return QObject::tr("Ignore accents");
		case IgnoreSpecialChars:
			return QObject::tr("Ignore special characters");
		case ImportDir:
			return QObject::tr("Import directory");
		case ImportFiles:
			return QObject::tr("Import files");
		case Inactive:
			return QObject::tr("Inactive");
		case Info:
			return QObject::tr("Info");
		case Loading:
			return QObject::tr("Loading");
		case LoadingArg:
			return QObject::tr("Loading %1");
		case InvalidChars:
			return QObject::tr("Invalid characters");
		case KB:
			return QObject::tr("KB");
		case Key_Find:
			return QObject::tr("Ctrl+f");
		case Key_Delete:
			return QObject::tr("Delete");
		case Key_Escape:
			return QObject::tr("Esc");
		case Key_Control:
			return QObject::tr("Ctrl");
		case Key_Alt:
			return QObject::tr("Alt");
		case Key_Shift:
			return QObject::tr("Shift");
		case Key_Backspace:
			return QObject::tr("Backspace");
		case Key_Tab:
			return QObject::tr("Tab");
		case Library:
			return QObject::tr("Library");
		case LibraryPath:
			return QObject::tr("Library path");
		case LibraryView:
			return QObject::tr("Library view type");
		case Listen:
			return QObject::tr("Listen");
		case LiveSearch:
			return QObject::tr("Live Search");
		case Logger:
			return QObject::tr("Logger");
		case LogLevel:
			return QObject::tr("Log level");
		case Lyrics:
			return QObject::tr("Lyrics");
		case MB:
			return QObject::tr("MB");
		case Menu:
			return QObject::tr("Menu");
		case Minimize:
			return QObject::tr("Minimize");
		case Minutes:
			return QObject::tr("Minutes");
		case MinutesShort:
			// short form of minutes
			return QObject::tr("m");
		case Missing:
			return QObject::tr("Missing");
		case Modified:
			return QObject::tr("Modified");
		case Months:
			return QObject::tr("Months");
		case MuteOn:
			return QObject::tr("Mute");
		case MuteOff:
			return QObject::tr("Mute off");
		case Name:
			return QObject::tr("Name");
		case New:
			return QObject::tr("New");
		case NextPage:
			return QObject::tr("Next page");
		case NextTrack:
			return QObject::tr("Next track");
		case No:
			return QObject::tr("No");
		case NoAlbums:
			return QObject::tr("No albums");
		case NumTracks:
			return QObject::tr("Tracks");
		case MoveDown:
			return QObject::tr("Move down");
		case MoveUp:
			return QObject::tr("Move up");
		case OK:
			return QObject::tr("OK");
		case On:
			// 5th track on "Thriller"
			return QObject::tr("on");
		case Open:
			return QObject::tr("Open");
		case OpenDir:
			return QObject::tr("Open directory");
		case OpenFile:
			return QObject::tr("Open file");
		case Or:
			return QObject::tr("or");
		case Overwrite:
			return QObject::tr("Overwrite");
		case Pause:
			return QObject::tr("Pause");
		case Play:
			return QObject::tr("Play");
		case PlayingTime:
			return QObject::tr("Playing time");
		case PlayInNewTab:
			return QObject::tr("Play in new tab");
		case Playlist:
			return QObject::tr("Playlist");
		case Playlists:
			return QObject::tr("Playlists");
		case PlayNext:
			return QObject::tr("Play next");
		case PlayPause:
			return QObject::tr("Play/Pause");
		case Plugin:
			return QObject::tr("Plugin");
		case Podcasts:
			return QObject::tr("Podcasts");
		case Preferences:
			return QObject::tr("Preferences");
		case PreviousPage:
			return QObject::tr("Previous page");
		case PreviousTrack:
			return QObject::tr("Previous track");
		case PurchaseUrl:
			return QObject::tr("Purchase Url");
		case Quit:
			return QObject::tr("Quit");
		case Radio:
			return QObject::tr("Radio");
		case RadioStation:
			return QObject::tr("Radio Station");
		case Rating:
			return QObject::tr("Rating");
		case Really:
			return QObject::tr("Really");
		case Refresh:
			return QObject::tr("Refresh");
		case ReloadLibrary:
			return QObject::tr("Reload Library");
		case Remove:
			return QObject::tr("Remove");
		case Rename:
			return QObject::tr("Rename");
		case Repeat1:
			return QObject::tr("Repeat 1");
		case RepeatAll:
			return QObject::tr("Repeat all");
		case Replace:
			return QObject::tr("Replace");
		case Reset:
			return QObject::tr("Reset");
		case Retry:
			return QObject::tr("Retry");
		case ReverseOrder:
			return QObject::tr("Reverse order");
		case Sampler:
			return QObject::tr("Sampler");
		case Shuffle:
			return QObject::tr("Shuffle");
		case ShufflePlaylist:
			return QObject::tr("Shuffle playlist");
		case Shutdown:
			return QObject::tr("Shutdown");
		case Save:
			return QObject::tr("Save");
		case SaveAs:
			return QObject::tr("Save as");
		case SaveToFile:
			return QObject::tr("Save to file");
		case ScanForFiles:
			return QObject::tr("Scan for audio files");
		case SearchNoun: // NOLINT(bugprone-branch-clone)
			return QObject::tr("Search");
		case SearchVerb:
			return QObject::tr("Search");
		case SearchNext:
			return QObject::tr("Search next");
		case SearchPrev:
			return QObject::tr("Search previous");
		case Second:
			return QObject::tr("2nd");
		case Seconds:
			return QObject::tr("Seconds");
		case SecondsShort:
			// short form of seconds
			return QObject::tr("s");
		case SeekBackward:
			return QObject::tr("Seek backward");
		case SeekForward:
			return QObject::tr("Seek forward");
		case Show:
			return QObject::tr("Show");
		case ShowAlbumArtists:
			return QObject::tr("Show Album Artists");
		case ShowCovers:
			return QObject::tr("Show Covers");
		case ShowLibrary:
			return QObject::tr("Show Library");
		case SimilarArtists:
			return QObject::tr("Similar artists");
		case SmartPlaylists:
			return QObject::tr("Smart Playlists");
		case SortBy:
			return QObject::tr("Sort by");            // for example "sort by year"
		case Stop:
			return QObject::tr("Stop");
		case Streams:
			return QObject::tr("Streams");
		case StreamUrl:
			return QObject::tr("Stream URL");
		case Success:
			return QObject::tr("Success");
		case Th:
			return QObject::tr("th");
		case Third:
			return QObject::tr("3rd");
		case Title:
			return QObject::tr("Title");
		case Track:
			return QObject::tr("Track");
		case TrackNo:
			return QObject::tr("Track number");
		case TrackOn:
			return QObject::tr("track on");
		case Tracks:
			return QObject::tr("Tracks");
		case Tree:
			return QObject::tr("Tree");
		case Undo:
			return QObject::tr("Undo");
		case UnknownAlbum:
			return QObject::tr("Unknown album");
		case UnknownArtist:
			return QObject::tr("Unknown artist");
		case UnknownTitle:
			return QObject::tr("Unknown title");
		case UnknownGenre:
			return QObject::tr("Unknown genre");
		case UnknownPlaceholder:
			return QObject::tr("Unknown placeholder");
		case UnknownYear:
		{
			[[maybe_unused]] const auto s = QObject::tr("Unknown year");
		}
			return {"-"};

		case Various:
			return QObject::tr("Various");
		case VariousAlbums:
			return QObject::tr("Various albums");
		case VariousArtists:
			return QObject::tr("Various artists");
		case VariousTracks:
			return QObject::tr("Various tracks");
		case Version:
			return QObject::tr("Version");
		case VolumeDown:
			return QObject::tr("Volume down");
		case VolumeUp:
			return QObject::tr("Volume up");
		case Warning:
			return QObject::tr("Warning");
		case Weeks:
			return QObject::tr("Weeks");
		case Year:
			return QObject::tr("Year");
		case Years:
			return QObject::tr("Years");
		case Yes:
			return QObject::tr("Yes");
		case Zoom:
			return QObject::tr("Zoom");
		case NUMBER_OF_LANGUAGE_KEYS:
			[[fallthrough]];
		default:
			if(ok)
			{
				*ok = false;
			}
			return QString();
	}
}

LanguageString Lang::getWithNumber(const TermNr term, const int param, bool* ok)
{
	if(ok)
	{
		*ok = true;
	}

	switch(term)
	{
		case NrDirectories:
			return (param == 0)
			       ? QObject::tr("No directories")
			       : QObject::tr("%n directory(s)", "", param);

		case NrFiles:
			return (param == 0)
			       ? QObject::tr("No files")
			       : QObject::tr("%n file(s)", "", param);

		case NrPlaylists:
			return (param == 0)
			       ? QObject::tr("No playlists")
			       : QObject::tr("%n playlist(s)", "", param);

		case NrTracks:
			return (param == 0)
			       ? QObject::tr("No tracks")
			       : QObject::tr("%n track(s)", "", param);

		case NrTracksFound:
			return (param == 0)
			       ? QObject::tr("No tracks found")
			       : QObject::tr("%n track(s) found", "", param);

		default:
			if(ok)
			{
				*ok = false;
			}

			return QString {};
	}
}
