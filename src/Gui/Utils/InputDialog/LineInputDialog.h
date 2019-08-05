/* LineInputDialog.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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



#ifndef LINEINPUTDIALOG_H
#define LINEINPUTDIALOG_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

UI_FWD(LineInputDialog)

namespace Gui
{
	class Completer;
	class LineInputDialog :
		public Dialog
	{
		Q_OBJECT
		UI_CLASS(LineInputDialog)
		PIMPL(LineInputDialog)

		public:
			enum ReturnValue
			{
				Ok=0,
				Cancelled
			};

			LineInputDialog(const QString& window_title, const QString& info_text, const QString& input_text, QWidget* parent=nullptr);
			LineInputDialog(const QString& window_title, const QString& info_text, QWidget* parent=nullptr);
			~LineInputDialog();

			void set_header_text(const QString& text);
			void set_info_text(const QString& text);
			void set_completer_text(const QStringList& lst);

			ReturnValue return_value() const;
			QString text() const;
			void set_text(const QString& text);

			bool was_accepted() const;

		private slots:
			void ok_clicked();
			void cancel_clicked();

		protected:
			void showEvent(QShowEvent* e) override;
			void closeEvent(QCloseEvent* e) override;
	};
}

#endif // LINEINPUTDIALOG_H
