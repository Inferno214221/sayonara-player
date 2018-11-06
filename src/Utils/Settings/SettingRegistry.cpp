/* SettingRegistry.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "SettingRegistry.h"
#include "Settings.h"

#include "Utils/Macros.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/EqualizerPresets.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Library/LibraryInfo.h"

#include "Utils/RawShortcutMap.h"

#include <QDir>
#include <QSize>
#include <QPoint>
#include <QApplication>
#include <QFont>
#include <type_traits>

			/* Show tree view of genres */

template<typename KeyClass>
void register_setting(const char* db_key, const typename KeyClass::Data& default_value)
{
	auto setting = new Setting<KeyClass>(db_key, default_value);

	Settings::instance()->register_setting( setting );
}

template<typename KeyClass>
void register_setting(const typename KeyClass::Data& default_value)
{
	auto setting = new Setting<KeyClass>(default_value);

	Settings::instance()->register_setting( setting );
}

bool SettingRegistry::init()
{
	register_setting<Set::LFM_Login>("LastFM_login", StringPair("", ""));
	register_setting<Set::LFM_Username>("lfm_username", QString());
	register_setting<Set::LFM_Password>("lfm_password", QString());
	register_setting<Set::LFM_ScrobbleTimeSec>("lfm_scrobble_time", 10);
	register_setting<Set::LFM_Active>( "LastFM_active", false );
	register_setting<Set::LFM_Corrections>( "lfm_corrections", false );
	register_setting<Set::LFM_ShowErrors>( "lfm_q.show_errors", false );
	register_setting<Set::LFM_SessionKey>( "lfm_session_key", QString() );

	register_setting<Set::Eq_Last>( "eq_last", 0);
	register_setting<Set::Eq_List>( "EQ_list", EQ_Setting::get_defaults() );
	register_setting<Set::Eq_Gauss>( "EQ_Gauss", true );

	BoolList shown_cols(10); shown_cols.assign(10, true);
	register_setting<Set::Lib_ColsTitle>( "lib_shown_cols_title", shown_cols );
	register_setting<Set::Lib_ColsArtist>( "lib_shown_cols_artist", shown_cols );
	register_setting<Set::Lib_ColsAlbum>( "lib_shown_cols_album", shown_cols );
	register_setting<Set::Lib_LiveSearch>( "lib_live_search", true );
	register_setting<Set::Lib_Sorting>( "lib_sortings", Library::Sortings() );
	register_setting<Set::Lib_Path>( "library_path", QString() );
	register_setting<Set::Lib_Show>( "show_library", true );
	register_setting<Set::Lib_CurPlugin >("last_lib_plugin", QString("local_library"));
	register_setting<Set::Lib_SplitterStateArtist >("splitter_state_artist", QByteArray());
	register_setting<Set::Lib_SplitterStateGenre >("splitter_state_genre", QByteArray());
	register_setting<Set::Lib_SplitterStateTrack >("splitter_state_track", QByteArray());
	register_setting<Set::Lib_SplitterStateDate >("splitter_state_date", QByteArray());
	register_setting<Set::Lib_OldWidth >("lib_old_width", 0);
	register_setting<Set::Lib_DC_DoNothing >("lib_dc_do_nothing", true);
	register_setting<Set::Lib_DC_PlayIfStopped >("lib_dc_play_if_stopped", false);
	register_setting<Set::Lib_DC_PlayImmediately >("lib_dc_play_immediately", false);
	register_setting<Set::Lib_DD_DoNothing >("lib_dd_do_nothing", true);
	register_setting<Set::Lib_DD_PlayIfStoppedAndEmpty >("lib_dd_play_if_stopped_and_empty", false);
	register_setting<Set::Lib_SearchMode>( "lib_search_mode", (int) Library::CaseInsensitve);
	register_setting<Set::Lib_AutoUpdate>( "lib_auto_update", false);
	register_setting<Set::Lib_ShowAlbumArtists>( "lib_show_album_artists", false);
	register_setting<Set::Lib_ShowAlbumCovers>( "lib_show_album_covers", false);
	register_setting<Set::Lib_CoverZoom>( "lib_cover_zoom", 100);
	register_setting<Set::Lib_CoverShowUtils>( "lib_cover_show_utils", false);
	register_setting<Set::Lib_CoverShowArtist>("lib_cover_show_artist", true);
	register_setting<Set::Lib_GenreTree>( "lib_show_genre_tree", true);
	register_setting<Set::Lib_LastIndex>( "lib_last_idx", -1);
	register_setting<Set::Lib_AllLibraries>( "lib_all_libraries", QList<Library::Info>()); // deprecated
	register_setting<Set::Lib_UseViewClearButton>( "lib_view_clear_button", false);
	register_setting<Set::Lib_ShowFilterExtBar>("lib_show_filter_ext_bar", true);

#ifdef Q_OS_WIN
	register_setting<Set::Lib_FontBold >("lib_font_bold", false);
	register_setting<Set::Lib_FontSize >("lib_font_size", -1);
#else
	register_setting<Set::Lib_FontBold >("lib_font_bold", true);
	register_setting<Set::Lib_FontSize >("lib_font_size", -1);
#endif

	register_setting<Set::Dir_ShowTracks>( "dir_show_tracks", true);
	register_setting<Set::Dir_SplitterDirFile>( "dir_splitter_dir_file", QByteArray());
	register_setting<Set::Dir_SplitterTracks>( "dir_splitter_tracks", QByteArray());

	register_setting<Set::Player_Version>( "player_version", QString(SAYONARA_VERSION));
	register_setting<Set::Player_Language>( "player_language", QString("en_US"));
	register_setting<Set::Player_Style>( "player_style", 0 );
	register_setting<Set::Player_ControlStyle>( "player_control_style", 0 );
	register_setting<Set::Player_FontName>( "player_font", QApplication::font().family() );
	register_setting<Set::Player_FontSize>( "player_font_size", QApplication::font().pointSize() );
	register_setting<Set::Player_Size>( "player_size", QSize(1200,800) );
	register_setting<Set::Player_Pos>( "player_pos", QPoint(50,50) );
	register_setting<Set::Player_Fullscreen>( "player_fullscreen", false );
	register_setting<Set::Player_Maximized>( "player_maximized", false );
	register_setting<Set::Player_ShownPlugin>( "shown_plugin", QString() );
	register_setting<Set::Player_OneInstance>( "only_one_instance", true );
	register_setting<Set::Player_Min2Tray>( "min_to_tray", false );
	register_setting<Set::Player_StartInTray>( "start_in_tray", false );
	register_setting<Set::Player_ShowTrayIcon>( "show_tray_icon", true );
	register_setting<Set::Player_NotifyNewVersion>( "notify_new_version", true );
	register_setting<Set::Player_SplitterState >("splitter_state_player", QByteArray());
	register_setting<Set::Player_Shortcuts>( "shortcuts", RawShortcutMap()); // deprecated
	register_setting<Set::Player_SplitterControls>("player_splitter_controls", QByteArray());
	register_setting<Set::Player_PrivId>("player_priv_id", QByteArray());
	register_setting<Set::Player_PublicId>("player_pub_id", QByteArray());

	register_setting<Set::PL_Playlist>( "playlist", QStringList() );
	register_setting<Set::PL_LoadSavedPlaylists>( "load_saved_playlists", false );
	register_setting<Set::PL_LoadTemporaryPlaylists>( "load_temporary_playlists", false );
	register_setting<Set::PL_LoadLastTrack>( "load_last_track", false );
	register_setting<Set::PL_RememberTime>( "remember_time", false );
	register_setting<Set::PL_StartPlaying>( "start_playing", false );
	register_setting<Set::PL_LastTrack>( "last_track", -1 );
	register_setting<Set::PL_LastTrackBeforeStop>( "last_track_before_stop", -1);
	register_setting<Set::PL_LastPlaylist>( "last_playlist", -1 );
	register_setting<Set::PL_Mode>( "playlist_mode", Playlist::Mode() );
	register_setting<Set::PL_ShowNumbers>( "show_playlist_numbers", true );
	register_setting<Set::PL_EntryLook>( "playlist_look", QString("*%title%* - %artist%"));
	register_setting<Set::PL_FontSize>( "playlist_font_size", -1);
	register_setting<Set::PL_ShowClearButton>( "playlist_show_clear_button", false);
	register_setting<Set::PL_RememberTrackAfterStop>( "playlist_remember_track_after_stop", false);
	register_setting<Set::PL_ShowCovers>( "playlist_show_covers", false);
	register_setting<Set::PL_ShowRating>( "playlist_show_rating", false);

	register_setting<Set::Notification_Show>( "show_notifications", true );
	register_setting<Set::Notification_Timeout>( "notification_timeout", 5000 );
	register_setting<Set::Notification_Name>( "notification_name", QString("DBus") );

	register_setting<Set::Engine_Name>( "sound_engine", QString() );
	register_setting<Set::Engine_CurTrackPos_s>( "last_track_pos", 0 );
	register_setting<Set::Engine_Vol>( "volume", 50 );
	register_setting<Set::Engine_Mute>( "mute", false );
	register_setting<Set::Engine_ConvertQuality>( "convert_quality", 0 );
	register_setting<Set::Engine_CovertTargetPath>( "convert_target_path", QDir::homePath() );
	register_setting<Set::Engine_ShowLevel>( "show_level", false);
	register_setting<Set::Engine_ShowSpectrum>( "show_spectrum", false);
	register_setting<Set::Engine_SpectrumBins>( "spectrum_bins", 70);

	register_setting<Set::Engine_SR_Active>( "streamripper", false );
	register_setting<Set::Engine_SR_Warning>( "streamripper_warning", true );
	register_setting<Set::Engine_SR_Path>( "streamripper_path", QDir::homePath() );
	register_setting<Set::Engine_SR_SessionPath>( "streamripper_session_path", true );
	register_setting<Set::Engine_SR_SessionPathTemplate>( "streamripper_session_path_template", QString());
	register_setting<Set::Engine_SR_AutoRecord>( "streamripper_auto_recording", false);
	register_setting<Set::Engine_CrossFaderActive>( "crossfader_active", false);
	register_setting<Set::Engine_CrossFaderTime>( "crossfader_time", 5000);
	register_setting<Set::Engine_Pitch>( "engine_pitch", 440);
	register_setting<Set::Engine_PreservePitch>( "engine_preserve_pitch", false);
	register_setting<Set::Engine_SpeedActive>( "engine_speed_active", false);
	register_setting<Set::Engine_Speed>( "engine_speed", 1.0f);
	register_setting<Set::Engine_Sink>( "engine_sink", QString("auto"));

	register_setting<Set::Spectrum_Style>( "spectrum_style", 0 );
	register_setting<Set::Level_Style>( "level_style", 0 );

	register_setting<Set::Broadcast_Active>( "broadcast_active", false );
	register_setting<Set::Broadcast_Prompt>( "broadcast_prompt", false );
	register_setting<Set::Broadcast_Port>( "broadcast_port", 54054 );

	register_setting<Set::Remote_Active>( "remote_control_active", false);
	register_setting<Set::Remote_Port>( "remote_control_port", 54055);

	register_setting<Set::Stream_NewTab>( "stream_new_tab", true);
	register_setting<Set::Stream_ShowHistory>( "stream_show_history", true);

	register_setting<Set::Lyrics_Server>( "lyrics_server", QString());
	register_setting<Set::Lyrics_Zoom>( "lyrics_zoom", 100);

	register_setting<Set::Cover_Server>( "cover_server", QStringList());
	register_setting<Set::Cover_LoadFromFile>( "cover_load_from_file", true);
	register_setting<Set::Cover_StartSearch>("cover_start_search_automatically", true);

	register_setting<Set::Icon_Theme>( "icon_theme", QString());
	register_setting<Set::Icon_ForceInDarkTheme>( "icon_force_in_dark_theme", false);

	register_setting<Set::Proxy_Active>( "proxy_active", false);
	register_setting<Set::Proxy_Hostname>( "proxy_hostname", QString());
	register_setting<Set::Proxy_Port>( "proxy_port", 3128);
	register_setting<Set::Proxy_Username>( "proxy_username", QString());
	register_setting<Set::Proxy_Password>( "proxy_password", QString());
	register_setting<Set::Proxy_SavePw>( "proxy_save_pw", false);

	register_setting<Set::Settings_Revision>("settings_version", 0);
	register_setting<Set::Logger_Level>( "logger_level", 0);

	register_setting<SetNoDB::MP3enc_found>( true );
	register_setting<SetNoDB::Pitch_found>( true );
	register_setting<SetNoDB::Player_Quit>( false );

	Settings::instance()->set<Set::Player_Version>(SAYONARA_VERSION);

	return true;
}

