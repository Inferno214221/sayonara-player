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

	connect(ui->btn_search, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::searchClicked);
	connect(ui->btn_add, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::addClicked);
	connect(ui->btn_cancel, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::close);
	connect(ui->btn_clear, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::clearClicked);

	connect(ui->list_artists, &QListWidget::currentRowChanged, this, &SC::GUI_ArtistSearch::artistSelected);

	connect(m->fetcher, &SC::DataFetcher::sigArtistsFetched, this, &SC::GUI_ArtistSearch::artistsFetched);
	connect(m->fetcher, &SC::DataFetcher::sigExtArtistsFetched, this, &SC::GUI_ArtistSearch::artistsExtFetched);
	connect(m->fetcher, &SC::DataFetcher::sigPlaylistsFetched, this, &SC::GUI_ArtistSearch::albumsFetched);
	connect(m->fetcher, &SC::DataFetcher::sigTracksFetched, this, &SC::GUI_ArtistSearch::tracksFetched);

	clearClicked();
}

SC::GUI_ArtistSearch::~GUI_ArtistSearch() {}

void SC::GUI_ArtistSearch::searchClicked()
{
	QString text = ui->le_search->text();
	clearClicked();

	ui->le_search->setText(text);

	if(text.size() <= 3){
		ui->lab_status->setText(tr("Query too short"));
	}

	setPlaylistCountLabel(-1);
	setTrackCountLabel(-1);

	m->fetcher->searchArtists(text);
}

void SC::GUI_ArtistSearch::clearClicked()
{
	ui->list_artists->clear();
	ui->list_playlists->clear();
	ui->list_tracks->clear();
	ui->le_search->clear();
	ui->lab_status->clear();
	ui->lab_artistCount->clear();
	ui->btn_add->setEnabled(false);

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
	ui->list_playlists->clear();
	ui->list_tracks->clear();

	setPlaylistCountLabel(-1);
	setTrackCountLabel(-1);

	m->tracks.clear();
	m->albums.clear();

	if(!Util::between(index, m->searchedArtists)) {
		return;
	}

	m->currentArtistSoundcloudId = m->searchedArtists[index].id();

	m->chosenArtists.clear();

	m->fetcher->getTracksByArtist(m->currentArtistSoundcloudId);
}

void SC::GUI_ArtistSearch::languageChanged()
{
	ui->retranslateUi(this);
}


void SC::GUI_ArtistSearch::artistsFetched(const ArtistList& artists)
{
	ui->list_artists->clear();
	m->searchedArtists.clear();

	if(artists.size() == 0)
	{
		ui->lab_status->setText(tr("No artists found"));
		return;
	}

	else
	{
		ui->lab_artistCount->setText( tr("Found %n artist(s)", "", artists.count()) );
		for(const Artist& artist: artists){
			ui->list_artists->addItem(artist.name());
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
	ui->list_playlists->clear();

	for(const Album& album : albums){
		ui->list_playlists->addItem(album.name());
	}

	m->albums = albums;

	setPlaylistCountLabel(albums.count());
}


void SC::GUI_ArtistSearch::tracksFetched(const MetaDataList& v_md)
{
	ui->list_tracks->clear();

	for(const MetaData& md : v_md){
		ui->list_tracks->addItem(md.title());
	}

	m->tracks = v_md;

	ui->btn_add->setEnabled(v_md.size() > 0);

	setTrackCountLabel(v_md.count());
}

void SC::GUI_ArtistSearch::setTrackCountLabel(int trackCount)
{
	if(trackCount >= 0) {
		ui->lab_trackCount->setText(Lang::getWithNumber(Lang::NrTracks, trackCount));
	}

	ui->lab_trackCount->setVisible(trackCount >= 0);
}

void SC::GUI_ArtistSearch::setPlaylistCountLabel(int playlistCount)
{
	if(playlistCount >= 0){
		ui->lab_playlistCount->setText( tr("%n playlist(s) found", "", playlistCount) );
	}

	ui->lab_playlistCount->setVisible(playlistCount >= 0);
}

