/* GUI_Broadcast.h */

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

#ifndef GUI_BROADCAST_H
#define GUI_BROADCAST_H

#include "Gui/Plugins/PlayerPluginBase.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_Broadcast)

class PlayManager;
class RawAudioDataProvider;

class GUI_Broadcast :
		public PlayerPlugin::Base
{
	Q_OBJECT
	PIMPL(GUI_Broadcast)
	UI_CLASS(GUI_Broadcast)

public:
	explicit GUI_Broadcast(PlayManager* playManager, RawAudioDataProvider* audioDataProvider, QWidget* parent=nullptr);
	~GUI_Broadcast() override;

	QString name() const override;
	QString displayName() const override;

private slots:
	void connectionEstablished(const QString& ip);
	void connectionClosed(const QString& ip);
	void canListenChanged(bool b);

	void dismissClicked();
	void dismissAllClicked();
	void currentIndexChanged(int idx);
	void retry();
	void mp3EncoderFound();

private:
	void dismissAt(int idx);
	void setStatusLabel();

	void retranslate() override;
	void initUi() override;

	bool checkDismissVisible() const;
	bool checkDismissAllVisible() const;
	void updateDismissButtons();

	void startServer();
};

#endif // GUI_BROADCAST_H
