/* GUI_StationSearcher.h */

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



#ifndef GUISTATIONSEARCHER_H
#define GUISTATIONSEARCHER_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Components/Streaming/StationSearcher/StationSearcher.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_StationSearcher)

class GUI_StationSearcher :
		public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_StationSearcher)
	UI_CLASS(GUI_StationSearcher)

signals:
	void sigStreamSelected(const QString& name, const QString& url, bool save);

public:
	GUI_StationSearcher(QWidget* parent=nullptr);
	~GUI_StationSearcher() override;

private:
	QAbstractButton* okButton();
	void checkOkButton();
	void clearStations();
	void clearStreams();
	void changeMode(StationSearcher::Mode mode);
	void setupCoverButton(const RadioStation& station);
	void initLineEdit();

private slots:
	void searchClicked();
	void searchPreviousClicked();
	void searchNextClicked();
	void okClicked();

	void searchTextChanged(const QString& text);
	void stationsFetched();

	void currentStationChanged();

protected:
	void showEvent(QShowEvent* e) override;
	void closeEvent(QCloseEvent* e) override;

	void languageChanged() override;
	void skinChanged() override;
};


#endif // STATIONSEARCHER_H
