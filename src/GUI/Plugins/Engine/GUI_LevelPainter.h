/* GUI_LevelPainter.h */

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

#ifndef GUI_LEVELPAINTER_H
#define GUI_LEVELPAINTER_H

#include "EnginePlugin.h"
#include "Components/Engine/Playback/SoundOutReceiver.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_LevelPainter)

class GUI_LevelPainter :
		public EnginePlugin,
		public LevelReceiver
{
    Q_OBJECT
	UI_CLASS(GUI_LevelPainter)
    PIMPL(GUI_LevelPainter)

public:
	explicit GUI_LevelPainter(QWidget *parent=nullptr);
	virtual ~GUI_LevelPainter();

	QString get_name() const override;
	QString get_display_name() const override;

public slots:
	void sl_update_style() override;

protected:
   void paintEvent(QPaintEvent* e) override;
   void showEvent(QShowEvent*) override;
   void closeEvent(QCloseEvent*) override;
   void hideEvent(QHideEvent* e) override;
   void init_ui() override;
   void retranslate_ui() override;

   QWidget* widget() override;
   bool has_small_buttons() const override;


protected slots:
    void do_fadeout_step() override;
	void set_level(float, float) override;

private:
    void reload();
};

#endif // GUI_LEVELPAINTER_H
