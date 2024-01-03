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
		struct ActionPair
		{
			Lang::Term langTerm;

			Library::SortOrder sortOrderAsc;
			Library::SortOrder sortOrderDesc;

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
		QList<ActionPair> actions {
			{Lang::Term::Title,       Library::SortOrder::TrackTitleAsc,       Library::SortOrder::TrackTitleDesc},
			{Lang::Term::TrackNo,     Library::SortOrder::TrackNumAsc,         Library::SortOrder::TrackNumDesc},
			{Lang::Term::AlbumArtist, Library::SortOrder::TrackAlbumArtistAsc, Library::SortOrder::TrackAlbumArtistDesc},
			{Lang::Term::Artist,      Library::SortOrder::TrackArtistAsc,      Library::SortOrder::TrackArtistDesc},
			{Lang::Term::Album,       Library::SortOrder::TrackAlbumAsc,       Library::SortOrder::TrackAlbumDesc},
			{Lang::Term::Filename,    Library::SortOrder::TrackFilenameAsc,    Library::SortOrder::TrackFilenameDesc}};
	};

	SortingMenu::SortingMenu(QWidget* parent) :
		Gui::WidgetTemplate<QMenu> {parent},
		m {Pimpl::make<Private>()}
	{
		for(auto& action: m->actions)
		{
			addAction(action.actionAsc);
			addAction(action.actionDesc);

			connect(action.actionAsc, &QAction::triggered, this, [&]() {
				emit sigSortingTriggered(action.sortOrderAsc);
			});

			connect(action.actionDesc, &QAction::triggered, this, [&]() {
				emit sigSortingTriggered(action.sortOrderDesc);
			});
		}
	}

	SortingMenu::~SortingMenu() noexcept = default;

	void SortingMenu::languageChanged()
	{
		for(auto& actionPair: m->actions)
		{
			actionPair.actionAsc->setText(actionText(actionPair.langTerm, true));
			actionPair.actionDesc->setText(actionText(actionPair.langTerm, false));
		}
	}
}