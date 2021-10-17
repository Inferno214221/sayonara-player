/* DirChooserDialog.h */

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

#ifndef SAYONARA_DIR_CHOOSER_DIALOG_H
#define SAYONARA_DIR_CHOOSER_DIALOG_H

#include <QFileDialog>

namespace Gui
{
	class DirectoryChooser : protected QFileDialog
	{
		Q_OBJECT

		protected:
			explicit DirectoryChooser(const QString& title = QString(), const QString& initialDirectory = QString(),
			                          bool enableMultiSelection = false, QWidget* parent = nullptr);
			~DirectoryChooser() override;

		public:
			static QString getDirectory(const QString& title = QString(), const QString& initialDirectory = QString(),
			                            bool resolveSymlinks = true, QWidget* parent = nullptr);

			static QStringList
			getDirectories(const QString& title = QString(), const QString& initialDirectory = QString(),
			               QWidget* parent = nullptr);
	};
}

#endif // SAYONARA_DIR_CHOOSER_DIALOG_H
