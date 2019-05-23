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
#include <QProgressBar>

namespace Gui
{
	/**
	 * @brief The ProgressBar class
	 * @ingroup Widgets
	 */
	class ProgressBar :
			public QProgressBar
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
			~ProgressBar();

			void set_position(ProgressBar::Position o);

		protected:
			void showEvent(QShowEvent* e) override;
	};
}

#endif // SAYONARALOADINGBAR_H
