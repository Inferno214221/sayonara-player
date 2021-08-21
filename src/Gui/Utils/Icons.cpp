// clazy:excludeall=non-pod-global-static

/* Icons.cpp */

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

#include "Icons.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"

#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"

#include <QIcon>
#include <QMap>
#include <QPair>

using namespace Gui;

namespace
{
	auto standardTheme = QString();
	const QMap<Icons::IconName, QPair<QString, QString>> iconMap =
		{
			{Icons::AudioFile,    {"audio-x-generic",      "cd.png"}},
			{Icons::Append,       {"list-add",             "append"}},
			{Icons::Backward,     {"media-skip-backward",  "bwd"}},
			{Icons::Clear,        {"edit-clear",           ""}},
			{Icons::Close,        {"window-close",         ""}},
			{Icons::Delete,       {"edit-delete",          ""}},
			{Icons::Dynamic,      {"dynamic",              "dynamic"}},
			{Icons::Edit,         {"edit-copy",            ""}},
			{Icons::Exit,         {"application-exit",     ""}},
			{Icons::File,         {"text-x-generic",       ""}},
			{Icons::FileManager,  {"system-file-manager",  ""}},
			{Icons::Folder,       {"folder",               ""}},
			{Icons::FolderOpen,   {"folder-open",          ""}},
			{Icons::Forward,      {"media-skip-forward",   "fwd"}},
			{Icons::Gapless,      {"gapless",              "gapless"}},
			{Icons::Grid,         {"view-grid",            ""}},
			{Icons::ImageFile,    {"image-x-generic",      ""}},
			{Icons::Info,         {"dialog-information",   ""}},
			{Icons::LocalLibrary, {"audio-x-generic",      "append"}},
			{Icons::Logo,         {"logo.png",             ""}},
			{Icons::Lyrics,       {"format-justify-left",  ""}},
			{Icons::New,          {"document-new",         ""}},
			{Icons::Next,         {"media-skip-forward",   "fwd"}},
			{Icons::Open,         {"document-open",        ""}},
			{Icons::Pause,        {"media-playback-pause", "pause"}},
			{Icons::Play,         {"media-playback-start", "play"}},
			{Icons::PlaySmall,    {"media-playback-start", ""}},
			{Icons::PlayBorder,   {"media-playback-start", ""}},
			{Icons::PlaylistFile, {"text-x-generic",       ""}},
			{Icons::Preferences,  {"applications-system",  ""}},
			{Icons::Previous,     {"media-skip-backward",  "bwd"}},
			{Icons::Record,       {"media-record",         "rec"}},
			{Icons::Refresh,      {"view-refresh",         ""}},
			{Icons::Remove,       {"list-remove",          ""}},
			{Icons::Rename,       {"edit-copy",            ""}},
			{Icons::Repeat1,      {"rep_1",                "rep_1"}},
			{Icons::RepeatAll,    {"rep_all",              "rep_all"}},
			{Icons::Save,         {"document-save",        ""}},
			{Icons::SaveAs,       {"document-save-as",     ""}},
			{Icons::Search,       {"edit-find",            ""}},
			{Icons::Shuffle,      {"shuffle",              "shuffle"}},
			{Icons::Shutdown,     {"system-shutdown",      ""}},
			{Icons::Star,         {"star.png",             "star.png"}},
			{Icons::StarDisabled, {"star_disabled.png",    "star_disabled.png"}},
			{Icons::Stop,         {"media-playback-stop",  "stop"}},
			{Icons::Table,        {"format-justify-fill",  ""}},
			{Icons::Undo,         {"edit-undo",            ""}},
			{Icons::Vol1,         {"audio-volume-low",     ""}},
			{Icons::Vol2,         {"audio-volume-medium",  ""}},
			{Icons::Vol3,         {"audio-volume-high",    ""}},
			{Icons::VolMute,      {"audio-volume-muted",   ""}},
		};

}

QIcon Icons::icon(Icons::IconName spec, Icons::IconMode mode)
{
	const auto standardName = iconMap[spec].first;
	const auto darkName = iconMap[spec].second;
	const auto isDark = Style::isDark();

	QIcon icon;
	if(mode == Icons::IconMode::ForceStdIcon)
	{
		icon = Gui::Util::systemThemeIcon(standardName);
	}

	else if(mode == Icons::IconMode::ForceSayonaraIcon)
	{
		Gui::Util::icon(darkName, Gui::Util::NoTheme);
	}

	else
	{
		icon = (isDark)
			? Gui::Util::icon(standardName, Gui::Util::MintY)
			: Gui::Util::systemThemeIcon(standardName);
	}

	return (!icon.isNull())
	       ? icon
	       : Gui::Util::icon(darkName, Gui::Util::NoTheme);
}

QIcon Icons::icon(IconName spec)
{
	changeTheme();

	return (GetSetting(Set::Icon_ForceInDarkTheme))
	       ? icon(spec, IconMode::ForceStdIcon)
	       : icon(spec, IconMode::Automatic);
}

void Icons::changeTheme()
{
	const static auto standardTheme = QIcon::themeName();
	const auto settingTheme = GetSetting(Set::Icon_Theme);

	QIcon::setThemeName
		(
			(!settingTheme.isEmpty())
			? settingTheme
			: standardTheme
		);
}

QPixmap Icons::pixmap(Icons::IconName spec, const QSize& size)
{
	return (GetSetting(Set::Icon_ForceInDarkTheme))
	       ? pixmap(spec, size, IconMode::ForceStdIcon)
	       : pixmap(spec, size, IconMode::Automatic);
}

QPixmap Icons::pixmap(Icons::IconName spec, const QSize& size, Icons::IconMode mode)
{
	const auto standardName = iconMap[spec].first;
	const auto darkName = iconMap[spec].second;

	QPixmap pm;

	if((mode == IconMode::ForceSayonaraIcon) || (mode == IconMode::Automatic))
	{
		const auto themeSource = (mode == IconMode::ForceSayonaraIcon)
			? Gui::Util::NoTheme
			: Gui::Util::MintY;

		pm = Gui::Util::pixmap(darkName, themeSource);
	}

	else // if(mode == IconMode::ForceStdIcon)
	{
		const auto icon = QIcon::fromTheme(standardName);
		if(!icon.isNull())
		{
			pm = icon.pixmap(size);
		}
	}

	if(pm.isNull())
	{
		pm = (!Style::isDark())
		     ? QIcon::fromTheme(standardName).pixmap(size)
		     : Gui::Util::pixmap(darkName, Gui::Util::MintY);
	}

	if(!pm.isNull())
	{
		return pm;
	}

	return Gui::Util::pixmap(darkName, Gui::Util::NoTheme);
}

QString Icons::defaultSystemTheme()
{
	return standardTheme;
}

void Icons::setDefaultSystemTheme(const QString& themeName)
{
	standardTheme = themeName;
}
