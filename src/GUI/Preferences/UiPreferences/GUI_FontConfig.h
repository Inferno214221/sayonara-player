
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

#ifndef GUI_FONTCONFIG_H
#define GUI_FONTCONFIG_H

#include "GUI/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_FontConfig)

class QFont;
class QFontDatabase;

class GUI_FontConfig :
		public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(GUI_FontConfig)
	PIMPL(GUI_FontConfig)

public:
	explicit GUI_FontConfig(QWidget* parent=nullptr);
	virtual ~GUI_FontConfig();

	bool commit();
	void revert();

	QString action_name() const;

protected:
	void language_changed() override;

protected slots:
	void default_clicked();
	void combo_fonts_changed(const QFont& font);
	void skin_changed() override;

private:
	QStringList available_font_sizes(const QString& font_name, const QString& style=QString());
	QStringList available_font_sizes(const QFont& font);

	void fill_sizes(const QStringList& sizes);
};

#endif // FONTCONFIG_H