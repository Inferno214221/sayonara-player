#ifndef SETTINGKEYENUM_H
#define SETTINGKEYENUM_H

/**
 * @brief The SK namespace is used to access setting keys
 * @ingroup Settings
 */
enum class SettingKey : unsigned short
{
	LFM_Active=0,
	LFM_ScrobbleTimeSec,
	LFM_Login,
	LFM_Username,
	LFM_Password,
	LFM_Corrections,
	LFM_ShowErrors,
	LFM_SessionKey,

	Eq_Last,
	Eq_List,
	Eq_Gauss,

	Lib_Show,
	Lib_Path,
	Lib_ColsTitle,
	Lib_ColsArtist,
	Lib_ColsAlbum,
	Lib_LiveSearch,
	Lib_Sorting,
	Lib_CurPlugin,
	Lib_SplitterStateArtist,
	Lib_SplitterStateTrack,
	Lib_SplitterStateGenre,
	Lib_SplitterStateDate,
	Lib_OldWidth,
	Lib_DC_DoNothing,
	Lib_DC_PlayIfStopped,
	Lib_DC_PlayImmediately,
	Lib_DD_DoNothing,
	Lib_DD_PlayIfStoppedAndEmpty,
	Lib_FontSize,
	Lib_FontBold,
	Lib_SearchMode,
	Lib_AutoUpdate,
	Lib_ShowAlbumArtists,
	Lib_ShowAlbumCovers,
	Lib_CoverZoom,
	Lib_CoverShowUtils,
	Lib_CoverShowArtist,
	Lib_GenreTree,
	Lib_LastIndex,
	Lib_AllLibraries,				// deprecated
	Lib_UseViewClearButton,
	Lib_ShowFilterExtBar,

	Dir_ShowTracks,
	Dir_SplitterDirFile,
	Dir_SplitterTracks,

	Player_Version,
	Player_Language,
	Player_FontName,
	Player_FontSize,
	Player_Style,
	Player_ControlStyle,
	Player_Size,
	Player_Pos,
	Player_Fullscreen,
	Player_Maximized,
	Player_ShownPlugin,
	Player_OneInstance,
	Player_Min2Tray,
	Player_StartInTray,
	Player_ShowTrayIcon,
	Player_NotifyNewVersion,
	Player_SplitterState,
	Player_Shortcuts,
	Player_SplitterControls,
	Player_PrivId,
	Player_PublicId,

	PL_Playlist,
	PL_LoadSavedPlaylists,
	PL_LoadTemporaryPlaylists,
	PL_LoadLastTrack,
	PL_RememberTime,
	PL_StartPlaying,
	PL_LastTrack,
	PL_LastTrackBeforeStop,
	PL_LastPlaylist,
	PL_Mode,
	PL_ShowNumbers,
	PL_EntryLook,
	PL_FontSize,
	PL_ShowClearButton,
	PL_RememberTrackAfterStop,
	PL_ShowCovers,
	PL_ShowRating,

	Notification_Show,
	Notification_Timeout,
	Notification_Name,

	Engine_Name,
	Engine_Vol,
	Engine_Mute,
	Engine_ConvertQuality,
	Engine_CovertTargetPath,
	Engine_SpectrumBins,
	Engine_ShowSpectrum,
	Engine_ShowLevel,
	Engine_CurTrackPos_s,
	Engine_CrossFaderActive,
	Engine_CrossFaderTime,
	Engine_Pitch,
	Engine_PreservePitch,
	Engine_Speed,
	Engine_SpeedActive,
	Engine_Sink,

	Engine_SR_Active,
	Engine_SR_Warning,
	Engine_SR_Path,
	Engine_SR_SessionPath,
	Engine_SR_SessionPathTemplate,
	Engine_SR_AutoRecord,

	Spectrum_Style,
	Level_Style,

	Broadcast_Active,
	Broadcast_Prompt,
	Broadcast_Port,

	MP3enc_found,
	Pitch_found,
	Player_Quit,

	Remote_Active,
	Remote_Port,

	Stream_NewTab,
	Stream_ShowHistory,

	Lyrics_Zoom,
	Lyrics_Server,

	Cover_Server,
	Cover_LoadFromFile,
	Cover_StartSearch,
	Icon_Theme,
	Icon_ForceInDarkTheme,

	Proxy_Active,
	Proxy_Username,
	Proxy_Password,
	Proxy_Hostname,
	Proxy_Port,
	Proxy_SavePw,

	Logger_Level,
	Settings_Revision,

	Num_Setting_Keys
};


#endif // SETTINGKEYENUM_H
