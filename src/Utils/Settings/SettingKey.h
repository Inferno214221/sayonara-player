/* SettingKey.h */

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

// clazy:excludeall=non-pod-global-static

#ifndef SETTINGKEY_H
#define SETTINGKEY_H

#include "Utils/typedefs.h"

enum class SettingKey :
	unsigned short
{
	AlternativeCovers_Size = 0,
	AudioConvert_NumberThreads,
	AudioConvert_PreferredConverter,
	AudioConvert_QualityLameCBR,
	AudioConvert_QualityLameVBR,
	AudioConvert_QualityOgg,
	Broadcast_Active,
	Broadcast_Port,
	Broadcast_Prompt,
	Cover_FetchFromWWW,
	Cover_SaveToDB,
	Cover_SaveToLibrary,
	Cover_SaveToSayonaraDir,
	Cover_Server,
	Cover_StartSearch,
	Cover_TemplatePath,
	Dir_ShowTracks,
	Dir_SplitterDirFile,
	Dir_SplitterTracks,
	Dir_TagToFilenameExpression,
	Engine_AlsaDevice,
	Engine_BufferSizeMS,
	Engine_CovertTargetPath,
	Engine_CrossFaderActive,
	Engine_CrossFaderTime,
	Engine_CurTrackPos_s,
	Engine_Mute,
	Engine_Name,
	Engine_Pitch,
	Engine_PreservePitch,
	Engine_SR_Active,
	Engine_SR_AutoRecord,
	Engine_SR_Path,
	Engine_SR_SessionPath,
	Engine_SR_SessionPathTemplate,
	Engine_SR_Warning,
	Engine_ShowLevel,
	Engine_ShowSpectrum,
	Engine_Sink,
	Engine_SoundFileExtensions,
	Engine_SpectrumBins,
	Engine_Speed,
	Engine_SpeedActive,
	Engine_Vol,
	Eq_Gauss,
	Eq_Last,
	Icon_ForceInDarkTheme,
	Icon_Theme,
	InfoDialog_Size,
	InhibitIdle,
	LFM_Active,
	LFM_Corrections,
	LFM_Login,
	LFM_Password,
	LFM_ScrobbleTimeSec,
	LFM_SessionKey,
	LFM_ShowErrors,
	LFM_Username,
	Level_Style,
	Lib_AllLibraries,
	Lib_AutoUpdate,
	Lib_ColStateAlbums,
	Lib_ColStateArtists,
	Lib_ColStateTracks,
	Lib_CoverOrigPMCache,
	Lib_CoverScaledPMCache,
	Lib_CoverShowArtist,
	Lib_CoverShowUtils,
	Lib_CoverZoom,
	Lib_CurPlugin,
	Lib_DC_DoNothing,
	Lib_DC_PlayIfStopped,
	Lib_DC_PlayImmediately,
	Lib_DD_DoNothing,
	Lib_DD_PlayIfStoppedAndEmpty,
	Lib_FontBold,
	Lib_GenreTree,
	Lib_HeaderAutoResizeAlbums,
	Lib_HeaderAutoResizeArtists,
	Lib_HeaderAutoResizeTracks,
	Lib_LastIndex,
	Lib_LiveSearch,
	Lib_OldWidth,
	Lib_Path,
	Lib_SearchMode,
	Lib_SearchStringLength,
	Lib_Show,
	Lib_ShowAlbumArtists,
	Lib_ShowAlbumCovers,
	Lib_ShowFilterExtBar,
	Lib_SortIgnoreArtistArticle,
	Lib_SortModeMask,
	Lib_Sorting,
	Lib_SplitterStateArtist,
	Lib_SplitterStateDate,
	Lib_SplitterStateGenre,
	Lib_SplitterStateTrack,
	Lib_UseViewClearButton,
	Lib_ViewType,
	Logger_Level,
	Lyrics_Server,
	Lyrics_Zoom,
	MP3enc_found,
	Notification_Name,
	Notification_Show,
	Notification_Timeout,
	PL_CreateFilesystemPlaylist,
	PL_CurrentTrackColorStringDark,
	PL_CurrentTrackColorStringStandard,
	PL_CurrentTrackCustomColorDark,
	PL_CurrentTrackCustomColorStandard,
	PL_EntryLook,
	PL_FilesystemPlaylistName,
	PL_JumpToCurrentTrack,
	PL_LastPlaylist,
	PL_LastTrack,
	PL_LastTrackBeforeStop,
	PL_LoadLastTrack,
	PL_LoadSavedPlaylists,
	PL_LoadTemporaryPlaylists,
	PL_Mode,
	PL_PlayTrackAfterSearch,
	PL_RememberTime,
	PL_RememberTrackAfterStop,
	PL_ShowBottomBar,
	PL_ShowClearButton,
	PL_ShowCovers,
	PL_ShowNumbers,
	PL_ShowRating,
	PL_SpecifyFileystemPlaylistName,
	PL_StartPlaying,
	PL_StartPlayingWorkaround_Issue263,
	Pitch_found,
	Player_514Fix,
	Player_ControlStyle,
	Player_FadingCover,
	Player_ForceNativeDirDialog,
	Player_Fullscreen,
	Player_Geometry,
	Player_Language,
	Player_Maximized,
	Player_Min2Tray,
	Player_NotifyNewVersion,
	Player_OneInstance,
	Player_PrivId,
	Player_PublicId,
	Player_ShowTrayIcon,
	Player_ShownPlugin,
	Player_SplitterControls,
	Player_SplitterState,
	Player_StartInTray,
	Player_Style,
	Player_Version,
	Proxy_Active,
	Proxy_Hostname,
	Proxy_Password,
	Proxy_Port,
	Proxy_SavePw,
	Proxy_Username,
	Remote_Active,
	Remote_DiscoverPort,
	Remote_Discoverable,
	Remote_Port,
	Settings_Revision,
	Soundcloud_AuthToken,
	Spectrum_Style,
	Speed_LastTab,
	Stream_NewTab,
	Stream_SearchWindowSize,
	Stream_ShowHistory,
	Stream_UpdateMetadata,
	Num_Setting_Keys
};

class QString;
class QStringList;
class QPoint;
class QSize;
class QByteArray;

class EqualizerSetting;
struct RawShortcutMap;

namespace Playlist
{
	class Mode;
}

namespace Library
{
	class Sortings;
	class Info;
	enum class ViewType :
		quint8;
}

template<typename DataType, SettingKey keyIndex>
struct SettingIdentifier
{
	using Data [[maybe_unused]] = DataType;
	const static SettingKey key = keyIndex;

	SettingIdentifier() = delete;
	~SettingIdentifier() = delete;
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CreateSetting(settingkey, type); \
    using settingkey = SettingIdentifier<type, SettingKey:: settingkey> // NOLINT(bugprone-macro-parentheses)

namespace Set
{   // persistent settings
	CreateSetting(AlternativeCovers_Size, QSize); // Size of Alternative Cover Dialog
	CreateSetting(AudioConvert_NumberThreads, int); // Number of threads
	CreateSetting(AudioConvert_PreferredConverter, QString); // Preferred Converter: ogg, lame cbr, lame vbr
	CreateSetting(AudioConvert_QualityLameCBR, int); // 64 - 320
	CreateSetting(AudioConvert_QualityLameVBR, int); // Lame Quality for variable bitrate 1-10
	CreateSetting(AudioConvert_QualityOgg, int); // 1 - 10
	CreateSetting(Broadcast_Active, bool); // is broadcast active?
	CreateSetting(Broadcast_Port, int); // broadcast port
	CreateSetting(Broadcast_Prompt, bool); // prompt when new connection arrives?
	CreateSetting(Cover_FetchFromWWW, bool); // Fetch covers from www
	CreateSetting(Cover_SaveToDB, bool); // Save covers to DB
	CreateSetting(Cover_SaveToLibrary, bool); // Save covers in library
	CreateSetting(Cover_SaveToSayonaraDir, bool); // Save covers in library
	CreateSetting(Cover_Server, QStringList); // Cover server
	CreateSetting(Cover_StartSearch, bool); // start alternative cover search automatically
	CreateSetting(Cover_TemplatePath, QString); // Name of cover file in library
	CreateSetting(Dir_ShowTracks, bool); // show tracks panel in directory view
	CreateSetting(Dir_SplitterDirFile, QByteArray); // Splitter state between dirs and files
	CreateSetting(Dir_SplitterTracks, QByteArray); // Splitter between upper and track view
	CreateSetting(Dir_TagToFilenameExpression, QString); // Last used expression when converting tags to filename
	CreateSetting(Engine_AlsaDevice, QString); // Specific alsa device
	CreateSetting(Engine_BufferSizeMS, int); // Buffer size for streaming
	CreateSetting(Engine_CovertTargetPath, QString); // last convert path
	CreateSetting(Engine_CrossFaderActive, bool); // crossfader, but not gapless active
	CreateSetting(Engine_CrossFaderTime, int); // crossfader overlap time
	CreateSetting(Engine_CurTrackPos_s, int); // position of track (used to load old position)
	CreateSetting(Engine_Mute, bool); // Muted/unmuted
	CreateSetting(Engine_Name, QString); // Deprecated: Engine name
	CreateSetting(Engine_Pitch, int); // hertz of a
	CreateSetting(Engine_PreservePitch, bool); // if yes, should pitch be preserved?
	CreateSetting(Engine_SR_Active, bool); // Streamripper active
	CreateSetting(Engine_SR_AutoRecord, bool); // streamripper automatic recording
	CreateSetting(Engine_SR_Path, QString); // streamripper paths
	CreateSetting(Engine_SR_SessionPath, bool); // create streamripper session path?
	CreateSetting(Engine_SR_SessionPathTemplate, QString); // streamripper session path templat
	CreateSetting(Engine_SR_Warning, bool); // streamripper warnings
	CreateSetting(Engine_ShowLevel, bool); // show level
	CreateSetting(Engine_ShowSpectrum, bool); // show spectrum
	CreateSetting(Engine_Sink, QString); // Alsa, pulseaudio
	CreateSetting(Engine_SoundFileExtensions, QStringList); // additional file extensions
	CreateSetting(Engine_SpectrumBins, int); // number of spectrum bins
	CreateSetting(Engine_Speed, float); // if yes, set speed
	CreateSetting(Engine_SpeedActive, bool); // is speed control active?
	CreateSetting(Engine_Vol, int); // Volume
	CreateSetting(Eq_Gauss, bool); // do curve, when changing eq setting
	CreateSetting(Eq_Last, int); // last equalizer index
	CreateSetting(Icon_ForceInDarkTheme, bool); // Current icon theme
	CreateSetting(Icon_Theme, QString); // Current icon theme
	CreateSetting(InfoDialog_Size, QSize); // Size of Info Dialog
	CreateSetting(InhibitIdle, bool); // Avoid suspend/hibernate while playing music
	CreateSetting(LFM_Active, bool); // is lastFM active?
	CreateSetting(LFM_Corrections, bool); // propose lfm corrections
	CreateSetting(LFM_Login, StringPair); // deprecated: 2-Tupel, username, password
	CreateSetting(LFM_Password, QString); // encrypted password
	CreateSetting(LFM_ScrobbleTimeSec, int); // time in sec when to scrobble
	CreateSetting(LFM_SessionKey, QString); // lfm session key
	CreateSetting(LFM_ShowErrors, bool); // get error message, if there are lfm problems
	CreateSetting(LFM_Username, QString); // usernam
	CreateSetting(Level_Style, int); // index of level style
	CreateSetting(Lib_AllLibraries, QList<Library::Info>); // deprecated
	CreateSetting(Lib_AutoUpdate, bool); // Automatic update of library
	CreateSetting(Lib_ColStateAlbums, QByteArray); // Header state of albums
	CreateSetting(Lib_ColStateArtists, QByteArray); // Header state of artists
	CreateSetting(Lib_ColStateTracks, QByteArray); // Header state of tracks
	CreateSetting(Lib_CoverOrigPMCache, int); // Original sized pixmap cache
	CreateSetting(Lib_CoverScaledPMCache, int); // Scaled sized pixmap cache
	CreateSetting(Lib_CoverShowArtist, bool); // Show artist name in cover view
	CreateSetting(Lib_CoverShowUtils, bool); // Show utils bar in cover view
	CreateSetting(Lib_CoverZoom, int); // Zoom of album cover view
	CreateSetting(Lib_CurPlugin, QString); // Current shown library plugin
	CreateSetting(Lib_DC_DoNothing, bool); // when double clicked, create playlist and do nothin
	CreateSetting(Lib_DC_PlayIfStopped, bool); // when double clicked, play if stopped
	CreateSetting(Lib_DC_PlayImmediately, bool); // when double clicked, play immediately
	CreateSetting(Lib_DD_DoNothing, bool); // when drag dropped, insert tracks and do nothing
	CreateSetting(Lib_DD_PlayIfStoppedAndEmpty, bool); // when drag dropped, play if playlist is empty and stopped
	CreateSetting(Lib_FontBold, bool); // bold fonts in library
	CreateSetting(Lib_GenreTree, bool); // Show tree view of genres
	CreateSetting(Lib_HeaderAutoResizeAlbums, bool); // resize columns automatically in albums
	CreateSetting(Lib_HeaderAutoResizeArtists, bool); // resize columns automatically in artists
	CreateSetting(Lib_HeaderAutoResizeTracks, bool); // resize columns automatically in tracks
	CreateSetting(Lib_LastIndex, int); // Last selected library
	CreateSetting(Lib_LiveSearch, bool); // library live search
	CreateSetting(Lib_OldWidth, int); // Old library width when hiding library
	CreateSetting(Lib_Path, QString); // deprecated
	CreateSetting(Lib_SearchMode, int); // Search mode in library. See
	CreateSetting(Lib_SearchStringLength, int); // minimum length of search string
	CreateSetting(Lib_Show, bool); // show library
	CreateSetting(Lib_ShowAlbumArtists, bool); // Show album artists instead of artists
	CreateSetting(Lib_ShowAlbumCovers, bool); // Show album cover view
	CreateSetting(Lib_ShowFilterExtBar, bool); // Show the file extension filter bar in track view
	CreateSetting(Lib_SortIgnoreArtistArticle, bool); // ignore article for artist
	CreateSetting(Lib_SortModeMask, int); // additional attributes applied to sorting
	CreateSetting(Lib_Sorting, Library::Sortings); // how to sort in lib
	CreateSetting(Lib_SplitterStateArtist, QByteArray); // Splitter state between artists and albums
	CreateSetting(Lib_SplitterStateDate, QByteArray); // Splitter state between tracks and genres
	CreateSetting(Lib_SplitterStateGenre, QByteArray); // Splitter state between tracks and genres
	CreateSetting(Lib_SplitterStateTrack, QByteArray); // Splitter state between artists and tracks
	CreateSetting(Lib_UseViewClearButton, bool); // Show clear button in single view
	CreateSetting(Lib_ViewType, Library::ViewType); // Standard view, CoverView, LibraryView
	CreateSetting(Logger_Level, int); // Also log development:
	CreateSetting(Lyrics_Server, QString); // Lyrics server
	CreateSetting(Lyrics_Zoom, int); // Zoom factor in lyrics window
	CreateSetting(Notification_Name, QString); // type of notifications: libnotify or empty for native baloons
	CreateSetting(Notification_Show, bool); // show notifications
	CreateSetting(Notification_Timeout, int); // notification timeout
	CreateSetting(PL_CurrentTrackColorStringDark, QString); // custom color in dark theme
	CreateSetting(PL_CurrentTrackColorStringStandard, QString); // custom color string in standard theme
	CreateSetting(PL_CurrentTrackCustomColorDark, bool); // use custom color in dark theme
	CreateSetting(PL_CurrentTrackCustomColorStandard, bool); // use custom color in standard theme
	CreateSetting(PL_EntryLook, QString); // formatting of playlist entry
	CreateSetting(PL_JumpToCurrentTrack, bool); // jump to current track when track changes
	CreateSetting(PL_LastPlaylist, int); // last Playlist id, where LastTrack has been played
	CreateSetting(PL_LastTrack, int); // last track idx in playlist
	CreateSetting(PL_LastTrackBeforeStop, int); // last track before stop
	CreateSetting(PL_LoadLastTrack, bool); // load last track on startup
	CreateSetting(PL_LoadSavedPlaylists, bool); // load saved playlists on startup
	CreateSetting(PL_LoadTemporaryPlaylists, bool); // load temporary playlists on startus
	CreateSetting(PL_CreateFilesystemPlaylist, bool); // create an extra playlist when loading files from file system
	CreateSetting(PL_SpecifyFileystemPlaylistName, bool); // when creating playlist from files, choose special name
	CreateSetting(PL_FilesystemPlaylistName, QString); // name of the file system playlist
	CreateSetting(PL_Mode, Playlist::Mode); // playlist mode: rep1, repAll, shuffle...
	CreateSetting(PL_PlayTrackAfterSearch, bool); // play track after search is done
	CreateSetting(PL_RememberTime, bool); // remember time of last track
	CreateSetting(PL_RememberTrackAfterStop, bool); // when stop button is pressed, remember last track index
	CreateSetting(PL_ShowBottomBar, bool); // Show bottom bar in playlist
	CreateSetting(PL_ShowClearButton, bool); // show clear button in playlist
	CreateSetting(PL_ShowCovers, bool); // Show covers in Playlist
	CreateSetting(PL_ShowNumbers, bool); // show numbers in playlist
	CreateSetting(PL_ShowRating, bool); // Show rating in playlist
	CreateSetting(PL_StartPlaying, bool); // start playing immediately when opening Sayonara
	CreateSetting(PL_StartPlayingWorkaround_Issue263,
	              bool); // https://gitlab.com/luciocarreras/sayonara-player/-/issues/263
	CreateSetting(Player_514Fix, bool); // https://bugs.archlinux.org/task/59451
	CreateSetting(Player_ControlStyle, int); // Big cover or not
	CreateSetting(Player_FadingCover, bool); // If cover buttons should fade
	CreateSetting(Player_ForceNativeDirDialog, bool); // Under some environments, native dir dialog is disabled
	CreateSetting(Player_Fullscreen, bool); // player fullscreen
	CreateSetting(Player_Geometry, QByteArray); // player geometry
	CreateSetting(Player_Language, QString); // language of player
	CreateSetting(Player_Maximized, bool); // player maximized
	CreateSetting(Player_Min2Tray, bool); // minimize Sayonara to tray
	CreateSetting(Player_NotifyNewVersion, bool); // check for new version on startup
	CreateSetting(Player_OneInstance, bool); // only one Sayonara instance is allowed
	CreateSetting(Player_PrivId, QByteArray); // Unique identifier
	CreateSetting(Player_PublicId, QByteArray); // Unique identifier
	CreateSetting(Player_ShowTrayIcon, bool); // Show/hide the tray icon
	CreateSetting(Player_ShownPlugin, QString); // current shown plugin in player, empty if none
	CreateSetting(Player_SplitterControls, QByteArray); // Splitter state between controls and playlist
	CreateSetting(Player_SplitterState, QByteArray); // spliter state between playlist and library
	CreateSetting(Player_StartInTray, bool); // start in tray
	CreateSetting(Player_Style, int); // dark or native: native = 0, dark = 1
	CreateSetting(Player_Version, QString); // Version string of player
	CreateSetting(Proxy_Active, bool); // Is proxy server active
	CreateSetting(Proxy_Hostname, QString); // Proxy Hostname/IP Address
	CreateSetting(Proxy_Password, QString); // Proxy Password
	CreateSetting(Proxy_Port, int); // Proxy Port 3128
	CreateSetting(Proxy_SavePw, bool); // Should password be saved
	CreateSetting(Proxy_Username, QString); // Proxy Username
	CreateSetting(Remote_Active, bool); // Remote control activated
	CreateSetting(Remote_DiscoverPort, int); // UDP port for remote discovering
	CreateSetting(Remote_Discoverable, bool); // broadcast is discoverable via UDP
	CreateSetting(Remote_Port, int); // Remote control port
	CreateSetting(Settings_Revision, int); // Version number of settings
	CreateSetting(Spectrum_Style, int); // index of spectrum style
	CreateSetting(Speed_LastTab, int); // Last tab selected int he speed/pitch plugin
	CreateSetting(Stream_NewTab, bool); // Open Streams in new tab
	CreateSetting(Stream_SearchWindowSize, QSize); // Size of the stream search dialog
	CreateSetting(Stream_ShowHistory, bool); // Show history when playing streams
	CreateSetting(Stream_UpdateMetadata, bool); // Update metadata (can be changed for each stream individually)
}

namespace SetNoDB
{   // non-persistent settings
	CreateSetting(MP3enc_found, bool); // is mp3 encoder available?
	CreateSetting(Pitch_found, bool); // is pitch element available?
	CreateSetting(Soundcloud_AuthToken, QString); // soundcloud authentication token
}
#endif // SETTINGKEY_H
