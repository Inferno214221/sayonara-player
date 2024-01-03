/* Splitter.h
 *
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

#ifndef SPLITTER_H
#define SPLITTER_H

#include "Utils/Pimpl.h"
#include <QSplitter>

namespace Gui
{
	class Splitter :
		public QSplitter
	{
		Q_OBJECT
		PIMPL(Splitter)

		signals:
			void sigResizeFinished();

		public:
			explicit Splitter(QWidget* parent=nullptr);
			~Splitter() override;

			void setHandleEnabled(bool b);
			bool isHandleEnabled() const;

		protected:
			QSplitterHandle* createHandle() override;
	};

	class SplitterHandle :
		public QSplitterHandle
	{
		Q_OBJECT

		signals:
			void sigResizeFinished();

		public:
			using QSplitterHandle::QSplitterHandle;
			void isPressed();

		protected:
			void mouseMoveEvent(QMouseEvent* e) override;
	};
} // namespace Gui

#endif // SPLITTER_H
