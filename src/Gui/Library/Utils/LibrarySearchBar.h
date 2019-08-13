/* LibrarySearchBar.h */

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

#ifndef LIBRARYSEARCHBAR_H
#define LIBRARYSEARCHBAR_H

#include <QLineEdit>
#include "Utils/Pimpl.h"
#include "Utils/Library/Filter.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"

namespace Library
{
	/**
	 * @brief The searchbar has a special context menu which allows
	 * to select the search mode. A special search mode is the
	 * invalid genre mode set by set_invalid_genre_mode, which actually
	 * is an empty string. But this wouldn't make any sense
	 * @ingroup GuiLibrary
	 */
	class  SearchBar : public Gui::WidgetTemplate<QLineEdit>
	{
		Q_OBJECT
		PIMPL(SearchBar)

		using Parent=Gui::WidgetTemplate<QLineEdit>;

		signals:
			void sig_current_mode_changed();
			void sig_text_changed(const QString& text);

		public:
			SearchBar(QWidget* parent=nullptr);
			~SearchBar() override;

			/**
			 * @brief this method does not set the genre
			 * mode implicitly. You also have to use set_mode().
			 * The invalid genre mode searchs for an empty genre.
			 * This is used to fetch tracks which do not have
			 * a genre.
			 */
			void set_invalid_genre_mode(bool b);

			/**
			 * @brief if the current state is the invalid genre
			 * mode
			 */
			bool has_invalid_genre_mode() const;

			/**
			 * @brief Sets the supported modes.
			 * See Library::Filter::Mode
			 */
			void set_modes(const QList<Filter::Mode>& modes);

			/**
			 * @brief returns supported modes.
			 * See Library::Filter::Mode
			 */
			QList<Filter::Mode> modes() const;

			/**
			 * @brief If mode is Filter::Mode::Genre but the
			 * genre should be empty, also use set_invalid_genre_mode()
			 */
			void set_current_mode(Filter::Mode mode);

			/**
			 * @brief fast toggling between modes by using
			 * arrow up key
			 */
			void set_previous_mode();

			/**
			 * @brief fast toggling between modes by using
			 * arrow down key
			 */
			void set_next_mode();

			/**
			 * @brief current selected mode
			 */
			Filter::Mode current_mode() const;

		protected:
			void init_context_menu();
			void keyPressEvent(QKeyEvent* e) override;
			void language_changed() override;
			void skin_changed() override;

		private slots:
			void text_changed(const QString& text);
			void search_shortcut_pressed();

			void livesearch_changed();
			void livesearch_triggered(bool b);
	};
}

#endif // LIBRARYSEARCHBAR_H
