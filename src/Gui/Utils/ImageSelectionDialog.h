/* ImageSelectionDialog.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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



#ifndef IMAGESELECTIONDIALOG_H
#define IMAGESELECTIONDIALOG_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"

#include <QFileDialog>

namespace Gui
{
	/**
	 * @brief A selection dialog that displays an image and also its size
	 * @ingroup Gui
	 */
	class ImageSelectionDialog :
			public Gui::WidgetTemplate<QFileDialog>
	{
		Q_OBJECT
		PIMPL(ImageSelectionDialog)

	public:
		ImageSelectionDialog(const QString& dir, QWidget* parent=nullptr);
		~ImageSelectionDialog();

	private slots:
		void file_selected(const QString& file);

	protected:
		void showEvent(QShowEvent* e) override;
	};
}

#endif // IMAGESELECTIONDIALOG_H
