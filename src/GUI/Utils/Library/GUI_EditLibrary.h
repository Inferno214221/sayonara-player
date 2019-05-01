/* GUI_EditLibrary.h */

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

#ifndef GUI_EDITLIBRARY_H
#define GUI_EDITLIBRARY_H

#include "Utils/Pimpl.h"
#include "GUI/Utils/Widgets/Dialog.h"

UI_FWD(GUI_EditLibrary)

class GUI_EditLibrary :
		public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_EditLibrary)
	UI_CLASS(GUI_EditLibrary)

signals:
	void sig_accepted();
	void sig_recected();

public:
	explicit GUI_EditLibrary(const QString& name, const QString& path, QWidget *parent = 0);
	explicit GUI_EditLibrary(QWidget* parent=nullptr);
	~GUI_EditLibrary();

	/**
	 * @brief Same as GUI_EditLibrary(QWidget* parent=nullptr)
	 */
	void reset();

	enum class EditMode
	{
		New=0,
		Edit=1
	};

	QString name() const;
	QString path() const;

	bool has_name_changed() const;
	bool has_path_changed() const;

	EditMode edit_mode() const;

protected:
	void language_changed() override;
	void skin_changed() override;

private slots:
	void ok_clicked();
	void cancel_clicked();
	void choose_dir_clicked();
	void name_edited(const QString& text);
};

#endif // GUI_EDITLIBRARY_H
