/* LineInputDialog.h */

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

#ifndef LINEINPUTDIALOG_H
#define LINEINPUTDIALOG_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

UI_FWD(LineInputDialog)

namespace Gui
{
	class Completer;

	/**
	 * @brief Dialog for entering one line. Listen for the sig_closed() signal
	 * for the Gui::Dialog class and request the text value afterwards if LineInputDialog::ReturnValue
	 * is set to LineInputDialog::ReturnValue::Ok
	 * @ingroup Gui
	 */
	class LineInputDialog :
		public Dialog
	{
		Q_OBJECT
		UI_CLASS(LineInputDialog)
		PIMPL(LineInputDialog)

		public:
			enum ReturnValue
			{
				Ok = 0,
				Cancelled
			};

			LineInputDialog(const QString& title, const QString& infoText, const QString& inputText,
			                QWidget* parent = nullptr);
			LineInputDialog(const QString& title, const QString& infoText, QWidget* parent = nullptr);
			virtual ~LineInputDialog() override;

			void setHeaderText(const QString& text);
			void setInfoText(const QString& text);
			void setCompleterText(const QStringList& lst);

			/**
			 * @brief returns if the dialog was closes by cancel or ok button
			 * @return ReturnValue::Ok if ok button was pressed, ReturnValue::Cancelled else
			 */
			ReturnValue returnValue() const;

			/**
			 * @brief Return the entered text
			 * @return always returns the entered text no matter if Ok or Cancel was pressed
			 */
			QString text() const;

			/**
			 * @brief Prefill the QLineEdit widget
			 * @param text
			 */
			void setText(const QString& text);

			void setPlaceholderText(const QString& text);

			void showInfo(bool b, const QString& infoPrefix = QString());

			/**
			 * @brief Convenience method for return_value() method
			 * @return
			 */
			bool wasAccepted() const;
			void setInvalidChars(const QList<QChar>& chars);

		private slots:
			void okClicked();
			void cancelClicked();
			void textEdited(const QString& text);

		protected:
			void showEvent(QShowEvent* e) override;
			void closeEvent(QCloseEvent* e) override;

		public:
			static QString
			getRenameFilename(QWidget* parent, const QString& oldName, const QString& parentPath = QString());
			static QString getNewFilename(QWidget* parent, const QString& info, const QString& parentPath = QString());
	};
}

#endif // LINEINPUTDIALOG_H
