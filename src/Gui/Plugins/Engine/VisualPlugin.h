/* VisualPlugin.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef ENGINEPLUGIN_H
#define ENGINEPLUGIN_H

#include "GUI_StyleSettings.h"
#include "VisualStyleTypes.h"
#include "Gui/Plugins/PlayerPluginBase.h"

#include "Utils/Pimpl.h"

#include <QTimer>
#include <QPushButton>

class EngineHandler;
class PlayManager;
class VisualColorStyleChooser;

class VisualPlugin :
	public PlayerPlugin::Base
{
	Q_OBJECT
	PIMPL(VisualPlugin)
	private:
		void set_button_sizes();
		void set_buttons_visible(bool b);

	protected:
		VisualColorStyleChooser* m_ecsc = nullptr;

		void init_buttons();

		virtual void showEvent(QShowEvent* e) override;
		virtual void closeEvent(QCloseEvent* e) override;
		virtual void resizeEvent(QResizeEvent* e) override;
		virtual void mousePressEvent(QMouseEvent* e) override;
		virtual void enterEvent(QEvent* e) override;
		virtual void leaveEvent(QEvent* e) override;

		virtual QWidget* widget() = 0;
		virtual ColorStyle currentStyle() const = 0;
		virtual int currentStyleIndex() const = 0;
		virtual bool hasSmallButtons() const = 0;

		void stop_fadeout_timer();

	private slots:
		void style_changed();

	protected slots:
		virtual void config_clicked();
		virtual void next_clicked();
		virtual void prev_clicked();

		virtual void doFadeoutStep() = 0;

		virtual void playstate_changed(PlayState play_state);
		virtual void played();
		virtual void paused();
		virtual void stopped();

	public slots:
		virtual void update_style(int new_index) = 0;
		virtual void update();
		virtual void initUi() override;

	public:
		explicit VisualPlugin(PlayManager* playManager, QWidget* parent = nullptr);
		virtual ~VisualPlugin();

		virtual bool hasTitle() const override;
};

#endif // ENGINEPLUGIN_H


