/* SettingRegistry.cpp */

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

#include "SettingRegistry.h"
#include "Settings.h"

#include "Utils/Macros.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/EqualizerSetting.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Library/LibraryNamespaces.h"

#include "Utils/RawShortcutMap.h"

#include <QDir>
#include <QSize>
#include <QPoint>
#include <QThread>
#include <QLocale>
#include <type_traits>

template<typename KeyClass>
void registerSetting(const char* databaseKey, const typename KeyClass::Data& defaultValue)
{
	auto setting = new Setting<KeyClass>(databaseKey, defaultValue);
	Settings::instance()->registerSetting(setting);
}

template<typename KeyClass>
void registerSetting(const typename KeyClass::Data& defaultValue)
{
	auto setting = new Setting<KeyClass>(defaultValue);
	Settings::instance()->registerSetting(setting);
}

bool SettingRegistry::init()
{
	registerSetting<Set::LFM_Login>("LastFM_login", StringPair("", ""));
	registerSetting<Set::LFM_Username>("lfm_username", QString());
	registerSetting<Set::LFM_Password>("lfm_password", QString());
	registerSetting<Set::LFM_ScrobbleTimeSec>("lfm_scrobble_time", 10);
	registerSetting<Set::LFM_Active>("LastFM_active", false);
	registerSetting<Set::LFM_Corrections>("lfm_corrections", false);
	registerSetting<Set::LFM_ShowErrors>("lfm_show_errors", false);
	registerSetting<Set::LFM_SessionKey>("lfm_session_key", QString());

	registerSetting<Set::Eq_Last>("eq_last", 0);
	registerSetting<Set::Eq_Gauss>("EQ_Gauss", true);

	{
		registerSetting<Set::Lib_ColStateArtists>("lib_col_state_artists", QByteArray());
		registerSetting<Set::Lib_ColStateAlbums>("lib_col_state_albums", QByteArray());
		registerSetting<Set::Lib_ColStateTracks>("lib_col_state_tracks", QByteArray());
	}

	registerSetting<Set::Lib_LiveSearch>("lib_live_search", true);
	registerSetting<Set::Lib_Sorting>("lib_sortings", Library::Sortings());
	registerSetting<Set::Lib_Path>("library_path", QString());
	registerSetting<Set::Lib_Show>("show_library", true);
	registerSetting<Set::Lib_CurPlugin>("last_lib_plugin", QString("local_library"));

	registerSetting<Set::Lib_SplitterStateArtist>("splitter_state_artist", QByteArray());

	{
		QByteArray splitter_genres;
		SettingConverter::fromString("0,0,0,255,0,0,0,1,0,0,0,2,0,0,2,65,0,0,0,186,1,255,255,255,255,1,0,0,0,1,0",
		                             splitter_genres);
		registerSetting<Set::Lib_SplitterStateGenre>("splitter_state_genre", splitter_genres);
	}

	{
		QByteArray splitter_tracks;
		SettingConverter::fromString("0,0,0,255,0,0,0,1,0,0,0,2,0,0,1,150,0,0,1,48,1,255,255,255,255,1,0,0,0,2,0",
		                             splitter_tracks);
		registerSetting<Set::Lib_SplitterStateTrack>("splitter_state_track", splitter_tracks);
	}

	registerSetting<Set::Lib_SplitterStateDate>("splitter_state_date", QByteArray());
	registerSetting<Set::Lib_OldWidth>("lib_old_width", 0);
	registerSetting<Set::Lib_DC_DoNothing>("lib_dc_do_nothing", false);
	registerSetting<Set::Lib_DC_PlayIfStopped>("lib_dc_play_if_stopped", true);
	registerSetting<Set::Lib_DC_PlayImmediately>("lib_dc_play_immediately", false);
	registerSetting<Set::Lib_DD_DoNothing>("lib_dd_do_nothing", true);
	registerSetting<Set::Lib_DD_PlayIfStoppedAndEmpty>("lib_dd_play_if_stopped_and_empty", false);
	registerSetting<Set::Lib_SearchMode>("lib_search_mode", static_cast<int>(Library::CaseInsensitve));
	registerSetting<Set::Lib_AutoUpdate>("lib_auto_update", false);
	registerSetting<Set::Lib_ShowAlbumArtists>("lib_show_album_artists", true);
	registerSetting<Set::Lib_ShowAlbumCovers>("lib_show_album_covers", false);
	registerSetting<Set::Lib_ViewType>("lib_view_style", ::Library::ViewType::Standard);
	registerSetting<Set::Lib_CoverZoom>("lib_cover_zoom", 100);
	registerSetting<Set::Lib_CoverShowUtils>("lib_cover_show_utils", false);
	registerSetting<Set::Lib_CoverShowArtist>("lib_cover_show_artist", true);
	registerSetting<Set::Lib_CoverOrigPMCache>("lib_cover_orig_pm_cache", 1);
	registerSetting<Set::Lib_CoverScaledPMCache>("lib_cover_scaled_pm_cache", 3);
	registerSetting<Set::Lib_GenreTree>("lib_show_genre_tree", true);
	registerSetting<Set::Lib_LastIndex>("lib_last_idx", -1);
	registerSetting<Set::Lib_AllLibraries>("lib_all_libraries", QList<Library::Info>()); // deprecated
	registerSetting<Set::Lib_UseViewClearButton>("lib_view_clear_button", false);
	registerSetting<Set::Lib_ShowFilterExtBar>("lib_show_filter_ext_bar", true);
	registerSetting<Set::Lib_SortIgnoreArtistArticle>("lib_ignore_artist_article", false);
	registerSetting<Set::Lib_HeaderAutoResizeArtists>("lib_header_auto_resize_artists", true);
	registerSetting<Set::Lib_HeaderAutoResizeAlbums>("lib_header_auto_resize_albums", true);
	registerSetting<Set::Lib_HeaderAutoResizeTracks>("lib_header_auto_resize_tracks", true);
	registerSetting<Set::Lib_SearchStringLength>("lib_search_string_length", 3);

#ifdef Q_OS_WIN
	registerSetting<Set::Lib_FontBold>("lib_font_bold", false);
#else
	registerSetting<Set::Lib_FontBold>("lib_font_bold", true);
#endif

	registerSetting<Set::Dir_ShowTracks>("dir_show_tracks", true);
	registerSetting<Set::Dir_SplitterDirFile>("dir_splitter_dir_file", QByteArray());
	registerSetting<Set::Dir_SplitterTracks>("dir_splitter_tracks", QByteArray());
	registerSetting<Set::Dir_TagToFilenameExpression>("dir_tag_to_filename_expression", QString());

	registerSetting<Set::Player_Version>("player_version", QString(SAYONARA_VERSION));
	registerSetting<Set::Player_Language>("player_language", QLocale().name());
	registerSetting<Set::Player_Style>("player_style", 1);
	registerSetting<Set::Player_ControlStyle>("player_control_style", 1);
	registerSetting<Set::Player_FadingCover>("player_fading_cover", true);
	registerSetting<Set::Player_Geometry>("player_geometry", QByteArray());
	registerSetting<Set::Player_Fullscreen>("player_fullscreen", false);
	registerSetting<Set::Player_Maximized>("player_maximized", false);
	registerSetting<Set::Player_ShownPlugin>("shown_plugin", QString());
	registerSetting<Set::Player_OneInstance>("only_one_instance", true);
	registerSetting<Set::Player_Min2Tray>("min_to_tray", false);
	registerSetting<Set::Player_StartInTray>("start_in_tray", false);
	registerSetting<Set::Player_ShowTrayIcon>("show_tray_icon", true);
	registerSetting<Set::Player_514Fix>("514_fix", true);
	registerSetting<Set::Player_NotifyNewVersion>("notify_new_version", true);

	{
		QByteArray splitter_state_player;
		SettingConverter::fromString("0,0,0,255,0,0,0,1,0,0,0,2,0,0,1,82,0,0,3,72,0,0,0,0,4,1,0,0,0,1,0",
		                             splitter_state_player);
		registerSetting<Set::Player_SplitterState>("splitter_state_player", splitter_state_player);
	}

	registerSetting<Set::Player_SplitterControls>("player_splitter_controls", QByteArray());
	registerSetting<Set::Player_PrivId>("player_priv_id", QByteArray());
	registerSetting<Set::Player_PublicId>("player_pub_id", QByteArray());
	registerSetting<Set::Player_ForceNativeDirDialog>("player_native_dir_dialog", false);

	registerSetting<Set::PL_LoadSavedPlaylists>("load_saved_playlists", false);
	registerSetting<Set::PL_LoadTemporaryPlaylists>("load_temporary_playlists", true);
	registerSetting<Set::PL_LoadLastTrack>("load_last_track", true);
	registerSetting<Set::PL_RememberTime>("remember_time", true);
	registerSetting<Set::PL_StartPlaying>("start_playing", false);
	registerSetting<Set::PL_StartPlayingWorkaround_Issue263>("start_playing_wa_263", false);
	registerSetting<Set::PL_LastTrack>("last_track", -1);
	registerSetting<Set::PL_LastTrackBeforeStop>("last_track_before_stop", -1);
	registerSetting<Set::PL_LastPlaylist>("last_playlist", -1);
	registerSetting<Set::PL_Mode>("playlist_mode", Playlist::Mode());
	registerSetting<Set::PL_ShowNumbers>("show_playlist_numbers", true);
	registerSetting<Set::PL_ShowBottomBar>("show_bottom_bar", true);
	registerSetting<Set::PL_EntryLook>("playlist_look", QString("*%title%* - %artist%"));
	registerSetting<Set::PL_ShowClearButton>("playlist_show_clear_button", false);
	registerSetting<Set::PL_RememberTrackAfterStop>("playlist_remember_track_after_stop", false);
	registerSetting<Set::PL_ShowCovers>("playlist_show_covers", false);
	registerSetting<Set::PL_ShowRating>("playlist_show_rating", false);
	registerSetting<Set::PL_CurrentTrackCustomColorStandard>("playlist_current_track_custom_color_standard", false);
	registerSetting<Set::PL_CurrentTrackColorStringStandard>("playlist_current_track_custom_color_string_standard",
	                                                         QString());
	registerSetting<Set::PL_CurrentTrackCustomColorDark>("playlist_current_track_custom_color_dark", true);
	registerSetting<Set::PL_CurrentTrackColorStringDark>("playlist_current_track_custom_color_string_dark",
	                                                     QString("#6f91cc"));
	registerSetting<Set::PL_JumpToCurrentTrack>("playlist_jump_to_current_track", true);
	registerSetting<Set::PL_PlayTrackAfterSearch>("playlist_play_track_after_search", false);

	registerSetting<Set::Notification_Show>("show_notifications", true);
	registerSetting<Set::Notification_Timeout>("notification_timeout", 5000);
	registerSetting<Set::Notification_Name>("notification_name", QString("DBus"));

	registerSetting<Set::Engine_Name>("sound_engine", QString());
	registerSetting<Set::Engine_SoundFileExtensions>("sound_file_extensions", QStringList());
	registerSetting<Set::Engine_CurTrackPos_s>("last_track_pos", 0);
	registerSetting<Set::Engine_Vol>("volume", 50);
	registerSetting<Set::Engine_Mute>("mute", false);
	registerSetting<Set::AudioConvert_NumberThreads>("convert_number_threads", QThread::idealThreadCount());
	registerSetting<Set::AudioConvert_PreferredConverter>("convert_preferred_converter", "ogg");
	registerSetting<Set::AudioConvert_QualityLameVBR>("convert_quality_lame_vbr", 7);
	registerSetting<Set::AudioConvert_QualityLameCBR>("convert_quality_lame_cbr", 192);
	registerSetting<Set::AudioConvert_QualityOgg>("convert_quality_ogg", 7);
	registerSetting<Set::Engine_CovertTargetPath>("convert_target_path", QDir::homePath());
	registerSetting<Set::Engine_ShowLevel>("show_level", false);
	registerSetting<Set::Engine_ShowSpectrum>("show_spectrum", false);
	registerSetting<Set::Engine_SpectrumBins>("spectrum_bins", 70);

	registerSetting<Set::Engine_SR_Active>("streamripper", false);
	registerSetting<Set::Engine_SR_Warning>("streamripper_warning", true);
	registerSetting<Set::Engine_SR_Path>("streamripper_path", QDir::homePath());
	registerSetting<Set::Engine_SR_SessionPath>("streamripper_session_path", true);
	registerSetting<Set::Engine_SR_SessionPathTemplate>("streamripper_session_path_template", QString());
	registerSetting<Set::Engine_SR_AutoRecord>("streamripper_auto_recording", false);
	registerSetting<Set::Engine_CrossFaderActive>("crossfader_active", false);
	registerSetting<Set::Engine_CrossFaderTime>("crossfader_time", 2000);
	registerSetting<Set::Engine_Pitch>("engine_pitch", 440);
	registerSetting<Set::Engine_PreservePitch>("engine_preserve_pitch", false);
	registerSetting<Set::Engine_SpeedActive>("engine_speed_active", false);
	registerSetting<Set::Engine_Speed>("engine_speed", 1.0f);
	registerSetting<Set::Engine_Sink>("engine_sink", QString("auto"));
	registerSetting<Set::Engine_AlsaDevice>("engine_alsa_device", QString(""));
	registerSetting<Set::Engine_BufferSizeMS>("engine_buffer_size_ms", 500);

	registerSetting<Set::Spectrum_Style>("spectrum_style", 0);
	registerSetting<Set::Level_Style>("level_style", 0);

	registerSetting<Set::Broadcast_Active>("broadcast_active", false);
	registerSetting<Set::Broadcast_Prompt>("broadcast_prompt", false);
	registerSetting<Set::Broadcast_Port>("broadcast_port", 54054);

	registerSetting<Set::Remote_Active>("remote_control_active", false);
	registerSetting<Set::Remote_Port>("remote_control_port", {54055});
	registerSetting<Set::Remote_Discoverable>("remote_discoverable", false);
	registerSetting<Set::Remote_DiscoverPort>("remote_discoverport", 54056);

	registerSetting<Set::Stream_NewTab>("stream_new_tab", true);
	registerSetting<Set::Stream_ShowHistory>("stream_show_history", true);
	registerSetting<Set::Stream_SearchWindowSize>("stream_search_window_size", QSize());

	registerSetting<Set::Lyrics_Server>("lyrics_server", {});
	registerSetting<Set::Lyrics_Zoom>("lyrics_zoom", 100);

	registerSetting<Set::Cover_Server>("cover_server",
	                                   QStringList {"discogs", "audioscrobbler", "amazon", "allmusic", "google"});
	registerSetting<Set::Cover_FetchFromWWW>("cover_fetch_from_www", true);
	registerSetting<Set::Cover_SaveToDB>("cover_save_to_db", true);
	registerSetting<Set::Cover_SaveToSayonaraDir>("cover_save_to_sayonara_dir", false);
	registerSetting<Set::Cover_StartSearch>("cover_start_search_automatically", true);
	registerSetting<Set::Cover_SaveToLibrary>("cover_save_to_library", false);
	registerSetting<Set::Cover_TemplatePath>("cover_template_path", QString("Cover.jpg"));

	registerSetting<Set::Icon_Theme>("icon_theme", QString());
	registerSetting<Set::Icon_ForceInDarkTheme>("icon_force_in_dark_theme", false);

	registerSetting<Set::Proxy_Active>("proxy_active", false);
	registerSetting<Set::Proxy_Hostname>("proxy_hostname", QString());
	registerSetting<Set::Proxy_Port>("proxy_port", 3128);
	registerSetting<Set::Proxy_Username>("proxy_username", QString());
	registerSetting<Set::Proxy_Password>("proxy_password", QString());
	registerSetting<Set::Proxy_SavePw>("proxy_save_pw", false);

	registerSetting<Set::Speed_LastTab>("speed_last_tab", 0);

	registerSetting<Set::InfoDialog_Size>("info_dialog_size", {4, 3});
	registerSetting<Set::AlternativeCovers_Size>("alternative_covers_size", QSize());

	registerSetting<Set::Settings_Revision>("settings_version", 0);
	registerSetting<Set::Logger_Level>("logger_level", 0);

	registerSetting<SetNoDB::MP3enc_found>(true);
	registerSetting<SetNoDB::Pitch_found>(true);
	registerSetting<SetNoDB::Soundcloud_AuthToken>(QString());

	SetSetting(Set::Player_Version, SAYONARA_VERSION);

	return true;
}

QList<SettingKey> SettingRegistry::undeployableKeys()
{
	return QList<SettingKey> {
		SettingKey::Player_Version,
		SettingKey::Player_Language,

		SettingKey::Player_PublicId,
		SettingKey::Player_PrivId,

		SettingKey::AudioConvert_NumberThreads,

		SettingKey::Engine_CovertTargetPath,
		SettingKey::Engine_SR_Path,

		SettingKey::Soundcloud_AuthToken,

		SettingKey::MP3enc_found,
		SettingKey::Pitch_found
	};
}
