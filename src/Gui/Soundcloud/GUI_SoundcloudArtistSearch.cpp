/* SC::GUI_ArtistSearch.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

struct SC::GUI_ArtistSearch::Private
{
	SC::Library*		library=nullptr;
	SC::DataFetcher*	fetcher=nullptr;

	MetaDataList		v_md;
	AlbumList			albums;
	ArtistList			searched_artists;
	ArtistList			chosen_artists;
	uint64_t			cur_artist_sc_id;
};

SC::GUI_ArtistSearch::GUI_ArtistSearch(SC::Library* library, QWidget *parent) :
	Dialog(parent)
{
	ui = new Ui::GUI_SoundcloudArtistSearch();
	ui->setupUi(this);

	m = Pimpl::make<SC::GUI_ArtistSearch::Private>();
	m->library = library;
	m->fetcher = new SC::DataFetcher(this);

	connect(ui->btn_search, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::search_clicked);
	connect(ui->btn_add, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::add_clicked);
	connect(ui->btn_cancel, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::close);
	connect(ui->btn_clear, &QPushButton::clicked, this, &SC::GUI_ArtistSearch::clear_clicked);

	connect(ui->list_artists, &QListWidget::currentRowChanged, this, &SC::GUI_ArtistSearch::artist_selected);

	connect(m->fetcher, &SC::DataFetcher::sig_artists_fetched, this, &SC::GUI_ArtistSearch::artists_fetched);
	connect(m->fetcher, &SC::DataFetcher::sig_ext_artists_fetched, this, &SC::GUI_ArtistSearch::artists_ext_fetched);
	connect(m->fetcher, &SC::DataFetcher::sig_playlists_fetched, this, &SC::GUI_ArtistSearch::albums_fetched);
	connect(m->fetcher, &SC::DataFetcher::sig_tracks_fetched, this, &SC::GUI_ArtistSearch::tracks_fetched);

	clear_clicked();
}

SC::GUI_ArtistSearch::~GUI_ArtistSearch() {}

void SC::GUI_ArtistSearch::search_clicked()
{
	QString text = ui->le_search->text();
	clear_clicked();

	ui->le_search->setText(text);

	if(text.size() <= 3){
		ui->lab_status->setText(tr("Query too short"));
	}

	set_playlist_label(-1);
	set_tracks_label(-1);

	m->fetcher->search_artists(text);
}

void SC::GUI_ArtistSearch::clear_clicked()
{
	ui->list_artists->clear();
	ui->list_playlists->clear();
	ui->list_tracks->clear();
	ui->le_search->clear();
	ui->lab_status->clear();
	ui->lab_n_artists->clear();
	ui->btn_add->setEnabled(false);

	set_playlist_label(-1);
	set_tracks_label(-1);

	m->searched_artists.clear();
	m->chosen_artists.clear();
	m->v_md.clear();
	m->albums.clear();
}

void SC::GUI_ArtistSearch::add_clicked()
{
	if( m->v_md.size() > 0 &&
		m->chosen_artists.size() > 0)
	{
		m->library->insert_tracks(m->v_md, m->chosen_artists, m->albums);
		close();
	}
}

void SC::GUI_ArtistSearch::close_clicked()
{
	close();
}


void SC::GUI_ArtistSearch::artist_selected(int idx)
{
	ui->list_playlists->clear();
	ui->list_tracks->clear();

	set_playlist_label(-1);
	set_tracks_label(-1);

	m->v_md.clear();
	m->albums.clear();

	if(!Util::between(idx, m->searched_artists)) {
		return;
	}

	m->cur_artist_sc_id = m->searched_artists[idx].id();

	m->chosen_artists.clear();

	m->fetcher->get_tracks_by_artist(m->cur_artist_sc_id);
}

void SC::GUI_ArtistSearch::language_changed()
{
	ui->retranslateUi(this);
}


void SC::GUI_ArtistSearch::artists_fetched(const ArtistList& artists)
{
	ui->list_artists->clear();
	m->searched_artists.clear();

	if(artists.size() == 0){
		ui->lab_status->setText(tr("No artists found"));
		return;
	}

	else{
		ui->lab_n_artists->setText( tr("Found %n artist(s)", "", artists.size()) );
		for(const Artist& artist: artists){
			ui->list_artists->addItem(artist.name());
		}

		m->searched_artists = artists;
	}
}

void SC::GUI_ArtistSearch::artists_ext_fetched(const ArtistList &artists)
{
	m->chosen_artists = artists;
}


void SC::GUI_ArtistSearch::albums_fetched(const AlbumList& albums)
{
	ui->list_playlists->clear();

	for(const Album& album : albums){
		ui->list_playlists->addItem(album.name());
	}

	m->albums = albums;

	set_playlist_label(albums.size());
}


void SC::GUI_ArtistSearch::tracks_fetched(const MetaDataList& v_md)
{
	ui->list_tracks->clear();

	for(const MetaData& md : v_md){
		ui->list_tracks->addItem(md.title());
	}

	m->v_md = v_md;

	ui->btn_add->setEnabled(v_md.size() > 0);

	set_tracks_label(v_md.size());
}


void SC::GUI_ArtistSearch::set_tracks_label(int n_tracks)
{
	if(n_tracks >= 0){
		ui->lab_n_tracks->setText( tr("%n track(s) found", "", n_tracks) );
	}

	ui->lab_n_tracks->setVisible(n_tracks >= 0);
}

void SC::GUI_ArtistSearch::set_playlist_label(int n_playlists)
{
	if(n_playlists >= 0){
		ui->lab_n_playlists->setText( tr("%n playlist(s) found", "", n_playlists) );
	}

	ui->lab_n_playlists->setVisible(n_playlists >= 0);
}

