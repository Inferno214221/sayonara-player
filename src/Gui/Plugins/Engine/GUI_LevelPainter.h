/* GUI_LevelPainter.h */

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

#ifndef GUI_LEVELPAINTER_H
#define GUI_LEVELPAINTER_H

#include "VisualPlugin.h"
#include "Interfaces/Engine/AudioDataReceiver.h"
#include "Utils/Pimpl.h"

class LevelDataProvider;

UI_FWD(GUI_LevelPainter)

class GUI_LevelPainter :
	public VisualPlugin,
	public Engine::LevelDataReceiver
{
	Q_OBJECT
	UI_CLASS(GUI_LevelPainter)
	PIMPL(GUI_LevelPainter)

	public:
		GUI_LevelPainter(LevelDataProvider* dataProvider, PlayManager* playManager, QWidget* parent = nullptr);
		~GUI_LevelPainter() override;

		QString name() const override;
		QString displayName() const override;
		bool isActive() const override;

	public slots:
		void update_style(int new_index) override;

	protected:
		void paintEvent(QPaintEvent* e) override;
		void showEvent(QShowEvent*) override;
		void closeEvent(QCloseEvent*) override;
		void hideEvent(QHideEvent* e) override;
		void initUi() override;
		void retranslate() override;

		QWidget* widget() override;
		bool hasSmallButtons() const override;
		ColorStyle currentStyle() const override;
		int currentStyleIndex() const override;
		void finalizeInitialization() override;

	protected slots:
		void doFadeoutStep() override;
		void setLevel(float, float) override;

	private:
		void reload();
};

#endif // GUI_LEVELPAINTER_H
