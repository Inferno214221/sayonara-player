
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

#ifndef GUI_SHORTCUTENTRY_H
#define GUI_SHORTCUTENTRY_H

#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Widgets/Widget.h"
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
	void sigTestPressed(const QList<QKeySequence>& sequences);
	void sigSequenceEntered();

public:
	explicit GUI_ShortcutEntry(ShortcutIdentifier identifier, QWidget* parent=nullptr);
	~GUI_ShortcutEntry();

	QList<QKeySequence> sequences() const;
	void showSequenceError();

public slots:
	void commit();
	void clear();
	void revert();


private slots:
	void editClicked();
	void defaultClicked();
	void testClicked();

	void languageChanged() override;
	void skinChanged() override;
};

#endif // GUI_SHORTCUTENTRY_H
