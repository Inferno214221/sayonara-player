/* GUI_Stream.h */

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

#ifndef GUI_STREAM_H_
#define GUI_STREAM_H_

#include "AbstractStationPlugin.h"

UI_FWD(GUI_Stream)

class GUI_Stream :
	public Gui::AbstractStationPlugin
{
	Q_OBJECT
	UI_CLASS(GUI_Stream)
	PIMPL(GUI_Stream)

	public:
		explicit GUI_Stream(PlaylistCreator* playlistCreator, QWidget* parent=nullptr);
		~GUI_Stream() override;

		QString name() const override;
		QString displayName() const override;

	private:
		void initUi() override;
		void retranslate() override;
		QString titleFallbackName() const override;

	// GUI_AbstractStream interface
	protected:
		QComboBox* comboStream() override;
		QPushButton* btnPlay() override;
		Gui::MenuToolButton* btnMenu() override;
		AbstractStationHandler* streamHandler() const override;
		GUI_ConfigureStation* createConfigDialog() override;

		void skinChanged() override;

	private slots:
		void searchRadioTriggered();
		void streamSelected(const QString& name, const QString& url, bool save);
};

#endif /* GUI_STREAM_H_ */
