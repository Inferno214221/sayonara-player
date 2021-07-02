/* SearchBar.h */

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

#ifndef LIBRARYSEARCHBAR_H
#define LIBRARYSEARCHBAR_H

#include <QLineEdit>
#include "Utils/Pimpl.h"
#include "Utils/Library/Filter.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"

class QMenu;

namespace Gui
{
	class ContextMenuFilter;
}

namespace Library
{
	class SearchBar : public Gui::WidgetTemplate<QLineEdit>
	{
		Q_OBJECT
		PIMPL(SearchBar)

		using Parent=Gui::WidgetTemplate<QLineEdit>;

		signals:
			void sigCurrentModeChanged();

		public:
			SearchBar(QWidget* parent=nullptr);
			~SearchBar() override;

			void setModes(const QList<Filter::Mode>& modes);
			void setCurrentMode(Filter::Mode mode);

			void setGenre(const QString& text, bool invalidGenreMode=false);
			Filter updateFilter(const Filter& oldFilter) const;

		private slots:
			void currentTextChanged(const QString& text);
			void livesearchChanged();
			void livesearchTriggered(bool b);

		protected:
			bool event(QEvent* event) override;
			void keyPressEvent(QKeyEvent* keyEvent) override;
			void languageChanged() override;
			void skinChanged() override;

		private:
			QList<QAction*> initModeActions(const QList<Library::Filter::Mode>& modes);
			void initContextMenu();
	};
}

#endif // LIBRARYSEARCHBAR_H
