/* GUI_AlternativeCovers.h */

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


/*
 * GUI_AlternativeCovers.h
 *
 *  Created on: Jul 1, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef GUI_ALTERNATE_COVERS_H_
#define GUI_ALTERNATE_COVERS_H_

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"
#include <QPixmap>

namespace Cover
{
	class Location;
}

UI_FWD(GUI_AlternativeCovers)

/**
 * @brief The GUI_AlternativeCovers class
 * @ingroup GuiCovers
 */

class GUI_AlternativeCovers :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_AlternativeCovers)
	UI_CLASS(GUI_AlternativeCovers)

	signals:
		void sigCoverChanged(const Cover::Location& cl);

	public:
		explicit GUI_AlternativeCovers(const Cover::Location& cl, bool silent, QWidget* parent);
		~GUI_AlternativeCovers() override;

		void setCoverLocation(const Cover::Location& cl);

	public slots:
		void start();
		void stop();

	private:
		void initUi();
		void reset();
		void reloadCombobox();
		void initSaveToLibrary();

	private slots:
		void okClicked();
		void applyClicked();
		void searchClicked();
		void openFileDialog();

		void coverPressed(const QModelIndex& idx);
		void coverLookupStarted();
		void coverLookupFinished(bool);
		void coverFound(const QPixmap& cover);

		void readyForProgressbar();

		void coverServersChanged();
		void autostartToggled(bool b);
		void rbAutosearchToggled(bool b);
		void wwwActiveChanged();

		void searchTextEdited(const QString& text);

	protected:
		void showEvent(QShowEvent* e) override;
		void resizeEvent(QResizeEvent* e) override;
		void languageChanged() override;
};

#endif /* GUI_ALTERNATE_COVERS_H_ */
