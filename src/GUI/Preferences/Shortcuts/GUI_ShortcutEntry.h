
/* Copyright (C) 2011-2017  Lucio Carreras
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

#ifndef GUI_SHORTCUTENTRY_H
#define GUI_SHORTCUTENTRY_H

#include "GUI/Utils/Shortcuts/Shortcut.h"
#include "GUI/Utils/Shortcuts/ShortcutHandler.h"
#include "GUI/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

#include <QKeySequence>

UI_FWD(GUI_ShortcutEntry)

/**
 * @brief The delegate class for displaying a shortcut.
 * @ingroup Shortcuts
 */

class GUI_ShortcutEntry :
		public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(GUI_ShortcutEntry)
	PIMPL(GUI_ShortcutEntry)

signals:
	/**
	 * @brief signal is emitted when the test button is pressed
	 * @param sequences list of sequences mapped to a specific shortcut
	 */
	void sig_test_pressed(const QList<QKeySequence>& sequences);

	void sig_sequence_entered();

public:
	explicit GUI_ShortcutEntry(ShortcutIdentifier identifier, QWidget* parent=nullptr);
	~GUI_ShortcutEntry();

	QList<QKeySequence> sequences() const;
	void show_sequence_error();

public slots:
	void commit();
	void clear();
	void revert();


private slots:
	void edit_clicked();
	void default_clicked();
	void test_clicked();

	void language_changed() override;
	void skin_changed() override;
};

#endif // GUI_SHORTCUTENTRY_H
