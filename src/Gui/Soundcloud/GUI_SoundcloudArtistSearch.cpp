/* GUI_ArtistSearch.cpp */

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

#include <Components/Covers/CoverLookupAlternative.h>
#include "GUI_SoundcloudArtistSearch.h"
#include "Components/Covers/CoverLookup.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Streaming/Soundcloud/SoundcloudLibrary.h"
#include "Components/Streaming/Soundcloud/SoundcloudDataFetcher.h"
#include "Gui/Soundcloud/ui_GUI_SoundcloudArtistSearch.h"

#include "Utils/globals.h"
#include "Utils/Message/Message.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language/Language.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/EventFilter.h"

using SC::GUI_ArtistSearch;

namespace
{
	constexpr const auto IconSize = 150;

	QIcon standardIcon()
	{
		static auto standardIcon = QIcon(
			QPixmap(":/sc_icons/icon.png").scaled(IconSize, IconSize));
		return standardIcon;
	}

	void initListWidget(QListWidget* listWidget)
	{
		listWidget->setViewMode(QListView::ViewMode::IconMode);
		listWidget->setIconSize(QSize(IconSize, IconSize));
		listWidget->setSpacing(IconSize / 10);
		listWidget->setUniformItemSizes(true);
		listWidget->setResizeMode(QListView::ResizeMode::Adjust);
	}
}

struct GUI_ArtistSearch::Private
{
	SC::Library* library;
	SC::DataFetcher* fetcher;

	MetaDataList tracks;
	AlbumList albums;
	ArtistList searchedArtists;
	ArtistList chosenArtists;
	ArtistId currentArtistSoundcloudId {-1};

	Private(SC::Library* library, QWidget* parent) :
		library {library},
		fetcher {new SC::DataFetcher(parent)} {}
};

GUI_ArtistSearch::GUI_ArtistSearch(SC::Library* library, QWidget* parent) :
	Dialog(parent)
{
	m = Pimpl::make<GUI_ArtistSearch::Private>(library, this);

	connect(m->fetcher, &SC::DataFetcher::sigArtistsFetched, this, &GUI_ArtistSearch::artistsFetched);
	connect(m->fetcher, &SC::DataFetcher::sigExtArtistsFetched, this, &GUI_ArtistSearch::artistsExtFetched);
	connect(m->fetcher, &SC::DataFetcher::sigPlaylistsFetched, this, &GUI_ArtistSearch::albumsFetched);
	connect(m->fetcher, &SC::DataFetcher::sigTracksFetched, this, &GUI_ArtistSearch::tracksFetched);

	initUserInterface();
}

GUI_ArtistSearch::~GUI_ArtistSearch() = default;

void GUI_ArtistSearch::initUserInterface()
{
	ui = std::make_shared<Ui::GUI_SoundcloudArtistSearch>();
	ui->setupUi(this);

	initListWidget(ui->lwArtists);
	initListWidget(ui->lwPlaylists);

	const auto filterTypes = QList<QEvent::Type> {QEvent::FocusIn, QEvent::FocusOut};
	auto* focusFilter = new Gui::GenericFilter(filterTypes, ui->leSearch);
	ui->leSearch->installEventFilter(focusFilter);
	connect(focusFilter, &Gui::GenericFilter::sigEvent, this, &GUI_ArtistSearch::lineEditFocusEvent);

	connect(ui->btnSearch, &QPushButton::clicked, this, &GUI_ArtistSearch::searchClicked);
	connect(ui->btnClear, &QPushButton::clicked, this, &GUI_ArtistSearch::clearClicked);
	connect(ui->btnAdd, &QPushButton::clicked, this, &GUI_ArtistSearch::addClicked);
	connect(ui->btnCancel, &QPushButton::clicked, this, &GUI_ArtistSearch::close);
	connect(ui->lwArtists, &QListWidget::currentRowChanged, this, &GUI_ArtistSearch::artistSelected);

	clearClicked();
	languageChanged();
	skinChanged();

	ui->leSearch->setFocus();
}

void GUI_ArtistSearch::searchClicked()
{
	const auto text = ui->leSearch->text();
	clearClicked();

	ui->leSearch->setText(text);
	if(text.size() <= 3)
	{
		ui->labStatus->setText(tr("Query too short"));
	}

	else
	{
		m->fetcher->searchArtists(text);
	}
}

void GUI_ArtistSearch::clearClicked()
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

void GUI_ArtistSearch::addClicked()
{
	if(!m->tracks.isEmpty() && !m->chosenArtists.empty())
	{
		m->library->insertTracks(m->tracks, m->chosenArtists, m->albums);
		close();
	}
}

void GUI_ArtistSearch::artistSelected(int index)
{
	ui->lwPlaylists->clear();
	ui->lwTracks->clear();

	setPlaylistCountLabel(-1);
	setTrackCountLabel(-1);

	m->tracks.clear();
	m->albums.clear();

	if(!Util::between(index, m->searchedArtists))
	{
		return;
	}

	m->currentArtistSoundcloudId = m->searchedArtists[static_cast<ArtistList::size_type>(index)].id();
	m->chosenArtists.clear();

	m->fetcher->getTracksByArtist(m->currentArtistSoundcloudId);
}

void GUI_ArtistSearch::artistsFetched(const ArtistList& artists)
{
	ui->lwArtists->clear();
	m->searchedArtists.clear();

	if(artists.empty())
	{
		const auto searchString = QString("'%1'").arg(ui->leSearch->text());
		Message::info(tr("No artist named %1 found").arg(searchString));
		ui->labStatus->setText(tr("No artists found"));
		return;
	}

	else
	{
		ui->labArtistCount->setText(tr("Found %n artist(s)", "", artists.count()));

		auto i = 0;
		for(const auto& artist: artists)
		{
			ui->lwArtists->addItem(new QListWidgetItem(standardIcon(), artist.name()));

			if(!artist.coverDownloadUrls().isEmpty())
			{
				const auto coverLocation = Cover::Location::coverLocation(artist);
				startCoverLookup(coverLocation, ui->lwArtists, i);
			}

			i++;
		}

		m->searchedArtists = artists;
	}
}

void GUI_ArtistSearch::artistsExtFetched(const ArtistList& artists)
{
	m->chosenArtists = artists;
}

void GUI_ArtistSearch::albumsFetched(const AlbumList& albums)
{
	ui->lwPlaylists->clear();

	auto i = 0;
	for(const auto& album: albums)
	{
		if(album.id() > 0)
		{
			ui->lwPlaylists->addItem(new QListWidgetItem(standardIcon(), album.name()));
			if(!album.coverDownloadUrls().isEmpty())
			{
				const auto coverLocation = Cover::Location::coverLocation(album);
				startCoverLookup(coverLocation, ui->lwPlaylists, i);
			}

			i++;
		}
	}

	m->albums = albums;

	setPlaylistCountLabel(albums.count());
}

void
GUI_ArtistSearch::startCoverLookup(const Cover::Location& coverLocation, QListWidget* targetView, int affectedRow)
{
	auto* coverLookup = new Cover::Lookup(coverLocation, 1, this);
	coverLookup->ignoreCache();

	connect(coverLookup, &Cover::Lookup::sigCoverFound, this, [=](const auto& pixmap) {
		const auto icon = QIcon(pixmap.scaled(IconSize, IconSize));
		targetView->item(affectedRow)->setIcon(icon);
	});

	connect(coverLookup, &Cover::Lookup::sigFinished, this, [=](const auto /* success */) {
		coverLookup->deleteLater();
	});

	coverLookup->start();
}

void GUI_ArtistSearch::tracksFetched(const MetaDataList& tracks)
{
	m->tracks = tracks;
	ui->lwTracks->clear();

	for(const auto& track: m->tracks)
	{
		ui->lwTracks->addItem(track.title());
	}

	setTrackCountLabel(tracks.count());

	ui->btnAdd->setEnabled(!m->tracks.isEmpty());
}

void GUI_ArtistSearch::setTrackCountLabel(int trackCount)
{
	if(trackCount >= 0)
	{
		ui->labTrackCount->setText(Lang::getWithNumber(Lang::NrTracks, trackCount));
	}

	ui->labTrackCount->setVisible(trackCount >= 0);
}

void GUI_ArtistSearch::setPlaylistCountLabel(int playlistCount)
{
	if(playlistCount >= 0)
	{
		ui->labPlaylistCount->setText(tr("%n playlist(s) found", "", playlistCount));
	}

	ui->labPlaylistCount->setVisible(playlistCount >= 0);
}

void GUI_ArtistSearch::lineEditFocusEvent(const QEvent::Type type)
{
	if(type == QEvent::Type::FocusOut)
	{
		ui->btnAdd->setDefault(ui->btnAdd->isEnabled());
		ui->btnAdd->setDefault(!ui->btnAdd->isEnabled());
	}

	if(type == QEvent::Type::FocusIn)
	{
		ui->btnCancel->setDefault(false);
		ui->btnAdd->setDefault(false);
		ui->btnSearch->setDefault(true);
	}
}

void GUI_ArtistSearch::languageChanged()
{
	ui->retranslateUi(this);

	ui->btnAdd->setText(Lang::get(Lang::Add));
	ui->btnCancel->setText(Lang::get(Lang::Cancel));
	ui->labArtistHeader->setText(Lang::get(Lang::Artists));
	ui->labAlbumHeader->setText(Lang::get(Lang::Playlists));
	ui->labTrackHeader->setText(Lang::get(Lang::Tracks));
}

void GUI_ArtistSearch::skinChanged()
{
	ui->btnClear->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
	ui->btnSearch->setIcon(Gui::Icons::icon(Gui::Icons::Search));
}
