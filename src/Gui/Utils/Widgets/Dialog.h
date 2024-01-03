/* Dialog.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef SAYONARADIALOG_H
#define SAYONARADIALOG_H

#include "WidgetTemplate.h"
#include <QDialog>

class QAbstractButton;
class QSize;
namespace Gui
{
	/**
	 * @brief Dialog with Settings connection. Also contains triggers for language_changed() and skin_changed(). Emits sig_closed() when closed.
	 * @ingroup Widgets
	 */
	class Dialog :
		public Gui::WidgetTemplate<QDialog>
	{
		Q_OBJECT

		signals:
			/**
			 * @brief emitted when closed
			 */
			void sigClosed();

		public:
			explicit Dialog(QWidget* parent = nullptr);
			virtual ~Dialog() override;

			void resizeRelative(QWidget* widget, double percent, const QSize& maxSize);
			bool isAccepted() const;

		protected:
			virtual void closeEvent(QCloseEvent* e) override;
			virtual void paintEvent(QPaintEvent* e) override;
	};
}

#endif // SAYONARADIALOG_H
