// clazy:excludeall=non-pod-global-static

/* Icons.cpp */

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

#include "Icons.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"

#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QIcon>
#include <QMap>
#include <QPair>

using namespace Gui;

static char* s_standard_theme=nullptr;
static bool s_force_standard_icons=false;

using P=QPair<QString, QString>;

static const QMap<Icons::IconName, QPair<QString, QString>> s_icon_map =
{
	{Icons::AudioFile,		P("audio-x-generic", "cd.png")},
	{Icons::Append,			P("list-add", "append")},
	{Icons::Backward,		P("media-skip-backward", "bwd")},
	{Icons::Clear,			P("edit-clear", "")},
	{Icons::Close,			P("window-close", "")},
	{Icons::Delete,			P("edit-delete", "")},
	{Icons::Dynamic,		P("dynamic", "dynamic")},
	{Icons::Edit,			P("edit-copy", "")},
	{Icons::Exit,			P("application-exit", "")},
	{Icons::File,			P("text-x-generic", "")},
	{Icons::FileManager,	P("system-file-manager", "")},
	{Icons::Folder,			P("folder", "")},
	{Icons::FolderOpen,		P("folder-open", "")},
	{Icons::Forward,		P("media-skip-forward", "fwd")},
	{Icons::Gapless,		P("gapless", "gapless")},
	{Icons::ImageFile,		P("image-x-generic", "")},
	{Icons::Info,			P("dialog-information", "")},
	{Icons::LocalLibrary,	P("audio-x-generic", "append")},
	{Icons::Lyrics,			P("format-justify-left", "")},
	{Icons::New,			P("document-new", "")},
	{Icons::Next,			P("media-skip-forward", "fwd")},
	{Icons::Open,			P("document-open", "")},
	{Icons::Pause,			P("media-playback-pause", "pause")},
	{Icons::Play,			P("media-playback-start", "play")},
	{Icons::PlaySmall,		P("media-playback-start", "")},
	{Icons::PlayBorder,		P("media-playback-start", "")},
	{Icons::PlaylistFile,	P("text-x-generic", "")},
	{Icons::Preferences,	P("applications-system", "")},
	{Icons::Previous,		P("media-skip-backward", "bwd")},
	{Icons::Record,			P("media-record", "rec")},
	{Icons::Refresh,		P("view-refresh", "")},
	{Icons::Remove,			P("list-remove", "")},
	{Icons::Rename,			P("edit-copy", "")},
	{Icons::Repeat1,		P("rep_1", "rep_1")},
	{Icons::RepeatAll,		P("rep_all", "rep_all")},
	{Icons::Save,			P("document-save", "")},
	{Icons::SaveAs,			P("document-save-as", "")},
	{Icons::Search,			P("edit-find", "")},
	{Icons::Shuffle,		P("shuffle", "shuffle")},
	{Icons::Shutdown,		P("system-shutdown", "")},
	{Icons::Star,			P("star.png", "star.png")},
	{Icons::StarDisabled,	P("star_disabled.png", "star_disabled.png")},
	{Icons::Stop,			P("media-playback-stop", "stop")},
	{Icons::Table,			P("format-justify-fill", "")},
	{Icons::Undo,			P("edit-undo", "")},
	{Icons::Vol1,			P("audio-volume-low", "")},
	{Icons::Vol2,			P("audio-volume-medium", "")},
	{Icons::Vol3,			P("audio-volume-high", "")},
	{Icons::VolMute,		P("audio-volume-muted", "")},
};


#ifdef Q_OS_WIN
QString get_win_icon_name(const QString& name)
{
	QString icon_name = QString(":/IconsWindows/") + name + ".png";
	return icon_name;
}
#endif

QIcon Icons::icon(Icons::IconName spec, Icons::IconMode mode)
{
	QString std_name = s_icon_map[spec].first;
	QString dark_name = s_icon_map[spec].second;

	QList<QIcon> icons
	{
		QIcon::fromTheme(std_name),			// from theme
		Gui::Util::icon(std_name, Gui::Util::MintY),			// new icons
		Gui::Util::icon(dark_name, Gui::Util::NoTheme)			// old icons
	};

	int index = 0;
	bool is_dark = Style::is_dark();

	if(mode == Icons::IconMode::ForceStdIcon)
	{
		index = 0;
	}

	else if(mode == Icons::IconMode::ForceSayonaraIcon)
	{
		index = 2;
	}

	else
	{
		if(is_dark)
		{
			index = 1;
		}

		else
		{
			index = 0;
		}
	}

	QIcon icon = icons[index];
	if(icon.isNull())
	{
		index = 2;
		icon = icons[index];
	}

	return icon;
}


QIcon Icons::icon(IconName spec)
{
	change_theme();

	if(s_force_standard_icons){
		return icon(spec, IconMode::ForceStdIcon);
	}

	else {
		return icon(spec, IconMode::Automatic);
	}
}

void Icons::change_theme()
{
	QString theme = GetSetting(Set::Icon_Theme);

	QIcon::setThemeName(theme);
}

QPixmap Icons::pixmap(Icons::IconName spec)
{
	if(s_force_standard_icons){
		return pixmap(spec, IconMode::ForceStdIcon);
	}

	return pixmap(spec, IconMode::Automatic);
}

QPixmap Icons::pixmap(Icons::IconName spec, Icons::IconMode mode)
{
	QString std_name = s_icon_map[spec].first;
	QString dark_name = s_icon_map[spec].second;

	QPixmap pm;

	if(mode == IconMode::ForceSayonaraIcon){
		pm = Gui::Util::pixmap(dark_name, Gui::Util::NoTheme);
	}

	else if(mode == IconMode::ForceStdIcon)
	{
		QIcon icon = QIcon::fromTheme(std_name);
		if(!icon.isNull())
		{
			QList<QSize> sizes = icon.availableSizes();
			::Util::Algorithm::sort(sizes, [](QSize sz1, QSize sz2){
				return (sz1.width() < sz2.width());
			});

			QSize sz(32, 32);
			if(!sizes.isEmpty())
			{
				sz = sizes.last();
			}

			pm = icon.pixmap(sz);
		}
	}

	else {
		pm = Gui::Util::pixmap(std_name, Gui::Util::MintY);
	}

	if(pm.isNull())
	{
		if(!Style::is_dark())
		{
	#ifdef Q_OS_WIN
			pm = QIcon(get_win_icon_name(std_name)).pixmap(QSize(32,32));
	#else
			pm = QIcon::fromTheme(std_name).pixmap(QSize(32,32));
	#endif
		}

		else
		{
	#ifdef Q_OS_WIN
			pm = QIcon(get_win_icon_name(std_name)).pixmap(QSize(32,32));
	#else
			pm = Gui::Util::pixmap(dark_name, Gui::Util::MintY);
	#endif
		}
	}

	if(!pm.isNull())
	{
		return pm;
	}

	return Gui::Util::pixmap(dark_name, Gui::Util::NoTheme);
}

void Icons::set_standard_theme(const QString& name)
{
	s_standard_theme = strdup(name.toLocal8Bit().data());
}

QString Icons::standard_theme()
{
	return QString(s_standard_theme);
}

void Icons::force_standard_icons(bool b)
{
	s_force_standard_icons = b;
}
