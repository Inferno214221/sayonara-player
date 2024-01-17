/* SortingMenu.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "SortingMenu.h"

#include "Utils/Language/Language.h"
#include "Utils/Library/Sortorder.h"

#include <QAction>

namespace Playlist
{
	namespace
	{
		using Library::TrackSortorder;

		struct SortingAction
		{
			Lang::Term langTerm;

			TrackSortorder sortOrderAsc;
			TrackSortorder sortOrderDesc;

			QAction* actionAsc {new QAction()};
			QAction* actionDesc {new QAction()};
		};

		QString actionText(const Lang::Term term, const bool isAscending)
		{
			const auto ascTerm = isAscending
			                     ? Lang::get(Lang::Term::Ascending)
			                     : Lang::get(Lang::Term::Descending);
			return QString("%1 (%2)")
				.arg(Lang::get(term))
				.arg(ascTerm);
		}
	}

	struct SortingMenu::Private
	{
		QList<SortingAction> sortingActions {
			{Lang::Term::Title,       TrackSortorder::TitleAsc,       TrackSortorder::TitleDesc},
			{Lang::Term::TrackNo,     TrackSortorder::TrackNumberAsc, TrackSortorder::TrackNumberDesc},
			{Lang::Term::AlbumArtist, TrackSortorder::AlbumArtistAsc, TrackSortorder::AlbumArtistDesc},
			{Lang::Term::Artist,      TrackSortorder::ArtistAsc,      TrackSortorder::ArtistDesc},
			{Lang::Term::Album,       TrackSortorder::AlbumAsc,       TrackSortorder::AlbumDesc},
			{Lang::Term::Filename,    TrackSortorder::FilenameAsc,    TrackSortorder::FilenameDesc}};
	};

	SortingMenu::SortingMenu(QWidget* parent) :
		Gui::WidgetTemplate<QMenu> {parent},
		m {Pimpl::make<Private>()}
	{
		for(auto& sortingAction: m->sortingActions)
		{
			addAction(sortingAction.actionAsc);
			addAction(sortingAction.actionDesc);

			connect(sortingAction.actionAsc, &QAction::triggered, this, [&]() {
				emit sigSortingTriggered(sortingAction.sortOrderAsc);
			});

			connect(sortingAction.actionDesc, &QAction::triggered, this, [&]() {
				emit sigSortingTriggered(sortingAction.sortOrderDesc);
			});
		}
	}

	SortingMenu::~SortingMenu() noexcept = default;

	void SortingMenu::languageChanged()
	{
		for(auto& sortingAction: m->sortingActions)
		{
			sortingAction.actionAsc->setText(actionText(sortingAction.langTerm, true));
			sortingAction.actionDesc->setText(actionText(sortingAction.langTerm, false));
		}
	}
}