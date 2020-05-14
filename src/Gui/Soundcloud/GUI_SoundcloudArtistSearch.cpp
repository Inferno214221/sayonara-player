/* SC::GUI_ArtistSearch.cpp */

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

#include "GUI_SoundcloudArtistSearch.h"
#include "Components/Streaming/Soundcloud/SoundcloudLibrary.h"
#include "Components/Streaming/Soundcloud/SoundcloudDataFetcher.h"
#include "Gui/Soundcloud/ui_GUI_SoundcloudArtistSearch.h"

#include "Utils/globals.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language/Language.h"

#include "Gui/Utils/Icons.h"

struct SC::GUI_ArtistSearch::Private
{
	SC::Library*		library=nullptr;
	SC::DataFetcher*	fetcher=nullptr;

	MetaDataList		tracks;
	AlbumList			albums;
	ArtistList			searchedArtists;
	ArtistList			chosenArtists;
	ArtistId			currentArtistSoundcloudId;
};

SC::GUI_ArtistSearch::GUI_ArtistSearch(SC::Library* library, QWidget* parent) :
	Dialog(parent)
{
	ui = new Ui::GUI_SoundcloudArtistSearch();
	ui->setupUi(this);

	m = Pimpl::make<SC::GUI_ArtistSearch::Private>();
	m->library = library;
	m->fetcher = new SC::DataFetcher(this);

	connect(ui->btnSearch, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::searchClicked);
	connect(ui->btnClear, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::clearClicked);
	connect(ui->btnAdd, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::addClicked);
	connect(ui->btnCancel, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::close);

	connect(ui->lwArtists, &QListWidget::currentRowChanged, this, &SC::GUI_ArtistSearch::artistSelected);

	connect(m->fetcher, &SC::DataFetcher::sigArtistsFetched, this, &SC::GUI_ArtistSearch::artistsFetched);
	connect(m->fetcher, &SC::DataFetcher::sigExtArtistsFetched, this, &SC::GUI_ArtistSearch::artistsExtFetched);
	connect(m->fetcher, &SC::DataFetcher::sigPlaylistsFetched, this, &SC::GUI_ArtistSearch::albumsFetched);
	connect(m->fetcher, &SC::DataFetcher::sigTracksFetched, this, &SC::GUI_ArtistSearch::tracksFetched);

	clearClicked();

	languageChanged();
	skinChanged();
}

SC::GUI_ArtistSearch::~GUI_ArtistSearch() = default;

void SC::GUI_ArtistSearch::searchClicked()
{
	QString text = ui->leSearch->text();
	clearClicked();

	ui->leSearch->setText(text);

	if(text.size() <= 3) {
		ui->labStatus->setText(tr("Query too short"));
	}

	setPlaylistCountLabel(-1);
	setTrackCountLabel(-1);

	m->fetcher->searchArtists(text);
}

void SC::GUI_ArtistSearch::clearClicked()
{
	ui->lwArtists->clear();
	ui->lwPlaylists->clear();
	ui->lwTracks->clear();
	ui->leSearch->clear();
	ui->labStatus->clear();
	ui->labArtistCount->clear();
	ui->btnAdd->setEnabled(false);

	setPlaylistCountLabel(-1);
	setTrackCountLabel(-1);

	m->searchedArtists.clear();
	m->chosenArtists.clear();
	m->tracks.clear();
	m->albums.clear();
}

void SC::GUI_ArtistSearch::addClicked()
{
	if( m->tracks.size() > 0 &&
		m->chosenArtists.size() > 0)
	{
		m->library->insertTracks(m->tracks, m->chosenArtists, m->albums);
		close();
	}
}

void SC::GUI_ArtistSearch::closeClicked()
{
	close();
}

void SC::GUI_ArtistSearch::artistSelected(int index)
{
	ui->lwPlaylists->clear();
	ui->lwTracks->clear();

	setPlaylistCountLabel(-1);
	setTrackCountLabel(-1);

	m->tracks.clear();
	m->albums.clear();

	if(!Util::between(index, m->searchedArtists)) {
		return;
	}

	m->currentArtistSoundcloudId = m->searchedArtists[ ArtistList::Size(index) ].id();
	m->chosenArtists.clear();

	m->fetcher->getTracksByArtist(m->currentArtistSoundcloudId);
}

void SC::GUI_ArtistSearch::languageChanged()
{
	ui->retranslateUi(this);

	ui->btnAdd->setText(Lang::get(Lang::Add));
	ui->btnCancel->setText(Lang::get(Lang::Cancel));
}

void SC::GUI_ArtistSearch::skinChanged()
{
	ui->btnClear->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
	ui->btnSearch->setIcon(Gui::Icons::icon(Gui::Icons::Search));
}

void SC::GUI_ArtistSearch::artistsFetched(const ArtistList& artists)
{
	ui->lwArtists->clear();
	m->searchedArtists.clear();

	if(artists.size() == 0)
	{
		ui->labStatus->setText(tr("No artists found"));
		return;
	}

	else
	{
		ui->labArtistCount->setText( tr("Found %n artist(s)", "", artists.count()) );
		for(const Artist& artist: artists){
			ui->lwArtists->addItem(artist.name());
		}

		m->searchedArtists = artists;
	}
}

void SC::GUI_ArtistSearch::artistsExtFetched(const ArtistList &artists)
{
	m->chosenArtists = artists;
}


void SC::GUI_ArtistSearch::albumsFetched(const AlbumList& albums)
{
	ui->lwPlaylists->clear();

	for(const Album& album : albums){
		ui->lwPlaylists->addItem(album.name());
	}

	m->albums = albums;

	setPlaylistCountLabel(albums.count());
}


void SC::GUI_ArtistSearch::tracksFetched(const MetaDataList& tracks)
{
	ui->lwTracks->clear();

	for(const MetaData& md : tracks){
		ui->lwTracks->addItem(md.title());
	}

	m->tracks = tracks;

	ui->btnAdd->setEnabled(tracks.size() > 0);

	setTrackCountLabel(tracks.count());
}

void SC::GUI_ArtistSearch::setTrackCountLabel(int trackCount)
{
	if(trackCount >= 0) {
		ui->labTrackCount->setText(Lang::getWithNumber(Lang::NrTracks, trackCount));
	}

	ui->labTrackCount->setVisible(trackCount >= 0);
}

void SC::GUI_ArtistSearch::setPlaylistCountLabel(int playlistCount)
{
	if(playlistCount >= 0){
		ui->labPlaylistCount->setText( tr("%n playlist(s) found", "", playlistCount) );
	}

	ui->labPlaylistCount->setVisible(playlistCount >= 0);
}

