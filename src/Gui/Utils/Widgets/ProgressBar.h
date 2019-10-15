/* ProgressBar.h */

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


/* ProgressBar.h */

#ifndef SAYONARALOADINGBAR_H
#define SAYONARALOADINGBAR_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include <QProgressBar>

namespace Gui
{
	/**
	 * @brief Create a new Progress bar indicating progress of its parent widget
	 * Set the position. Every time when QWidget::show() or QWidget::setVisible(true)
	 * is called the progress bar is shown
	 * @ingroup Widgets
	 */
	class ProgressBar :
			public Gui::WidgetTemplate<QProgressBar>
	{
		Q_OBJECT
		PIMPL(ProgressBar)

		public:
			enum class Position
			{
				Top=0,
				Middle,
				Bottom
			};

		public:
			explicit ProgressBar(QWidget* parent);
			~ProgressBar() override;

			void set_position(ProgressBar::Position o);
			void refresh();

		protected:
			void showEvent(QShowEvent* e) override;
			void skin_changed() override;
	};
}

#endif // SAYONARALOADINGBAR_H
