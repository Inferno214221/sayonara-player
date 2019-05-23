/* VisualPlugin.h */

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

#ifndef ENGINEPLUGIN_H
#define ENGINEPLUGIN_H

#include "GUI_StyleSettings.h"
#include "VisualStyleTypes.h"
#include "Interfaces/PlayerPlugin/PlayerPluginBase.h"
#include "Components/PlayManager/PlayState.h"

#include "Utils/Pimpl.h"

#include <QTimer>
#include <QPushButton>

class VisualColorStyleChooser;

namespace Engine
{
	class Handler;
}

class VisualPlugin : public PlayerPlugin::Base
{
	Q_OBJECT
	PIMPL(VisualPlugin)

protected:
	VisualColorStyleChooser*	m_ecsc=nullptr;

	void				init_buttons(bool small);
	Engine::Handler*	engine() const;

	virtual void closeEvent(QCloseEvent* e) override;
	virtual void resizeEvent(QResizeEvent* e) override;
	virtual void mousePressEvent(QMouseEvent* e) override;
	virtual void enterEvent(QEvent* e) override;
	virtual void leaveEvent(QEvent* e) override;

	virtual QWidget*	widget()=0;
	virtual ColorStyle	current_style() const=0;
	virtual int			current_style_index() const=0;
	virtual bool		has_small_buttons() const=0;

	void stop_fadeout_timer();

private slots:
	void style_changed();


protected slots:
	virtual void config_clicked();
	virtual void next_clicked();
	virtual void prev_clicked();

	virtual void do_fadeout_step()=0;

	virtual void playstate_changed(PlayState play_state);
	virtual void played();
	virtual void paused();
	virtual void stopped();


public slots:
	virtual void update_style(int new_index)=0;
	virtual void update();
	virtual void init_ui() override;


public:
	explicit VisualPlugin(QWidget* parent=nullptr);
	virtual ~VisualPlugin();

	virtual bool is_title_shown() const override;
};

#endif // ENGINEPLUGIN_H


