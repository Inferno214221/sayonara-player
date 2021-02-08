/* GUILibraryInfoBox.cpp

 * Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Michael Lugmair (Lucio Carreras),
 * Sep 2, 2012
 *
 */

#include "GUI_LibraryInfoBox.h"
#include "Gui/Library/ui_GUI_LibraryInfoBox.h"
#include "Gui/Utils/Icons.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Language/Language.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"

#include <QMap>
#include <QPixmap>

using Library::GUI_LibraryInfoBox;

struct GUI_LibraryInfoBox::Private
{
	Library::Info libraryInfo;

	Private(const Library::Info& libraryInfo) :
		libraryInfo {libraryInfo} {}
};

GUI_LibraryInfoBox::GUI_LibraryInfoBox(const Library::Info& libraryInfo, QWidget* parent) :
	Dialog(parent)
{
	m = Pimpl::make<Private>(libraryInfo);

	ui = new Ui::GUI_LibraryInfoBox();
	ui->setupUi(this);

	ui->lab_icon->setScaledContents(true);
	ui->lab_icon->setPixmap(
		Gui::Util::pixmap("logo.png", Gui::Util::NoTheme, QSize(24, 24), true)
	);
}

GUI_LibraryInfoBox::~GUI_LibraryInfoBox() = default;

void GUI_LibraryInfoBox::languageChanged()
{
	ui->retranslateUi(this);

	ui->lab_tracks->setText(Lang::get(Lang::Tracks).toFirstUpper());
	ui->lab_artists->setText(Lang::get(Lang::Artists));
	ui->lab_albums->setText(Lang::get(Lang::Albums));
	ui->lab_duration->setText(Lang::get(Lang::Duration));
	ui->lab_filesize_descr->setText(Lang::get(Lang::Filesize));

	ui->lab_name->setText(Lang::get(Lang::Library) + ": " + m->libraryInfo.name());

	this->setWindowTitle(Lang::get(Lang::Info));
}

void GUI_LibraryInfoBox::skinChanged()
{
	ui->lab_path->setText(Util::createLink( m->libraryInfo.path(), Style::isDark()));
	ui->lab_icon->setPixmap(Gui::Icons::pixmap(Gui::Icons::LocalLibrary));
}

void GUI_LibraryInfoBox::showEvent(QShowEvent* e)
{
	refresh();
	Dialog::showEvent(e);
}

void GUI_LibraryInfoBox::refresh()
{
	auto* db = DB::Connector::instance();
	auto* libraryDatabase = db->libraryDatabase(m->libraryInfo.id(), 0);

	MetaDataList tracks;
	AlbumList albums;
	ArtistList artists;

	libraryDatabase->getAllTracks(tracks);
	libraryDatabase->getAllAlbums(albums, false);
	libraryDatabase->getAllArtists(artists, false);

	auto nTracks = tracks.size();
	auto nAlbums = albums.size();
	auto nArtists = artists.size();
	MilliSeconds durationMs = 0;
	Filesize filesize = 0;

	for(const auto& track : tracks)
	{
		durationMs += track.durationMs();
		filesize += track.filesize();
	}

	const auto durationString = Util::msToString(durationMs, "$De $He $M:$S");
	const auto filesizeStr = Util::File::getFilesizeString(filesize);

	ui->lab_album_count->setText(QString::number(nAlbums));
	ui->lab_track_count->setText(QString::number(nTracks));
	ui->lab_artist_count->setText(QString::number(nArtists));
	ui->lab_duration_value->setText(durationString);
	ui->lab_filesize->setText(filesizeStr);
}
