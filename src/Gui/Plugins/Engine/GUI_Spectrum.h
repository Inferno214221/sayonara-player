/* GUI_Spectrum.h */

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

#ifndef GUI_SPECTRUM_H
#define GUI_SPECTRUM_H

#include "VisualPlugin.h"
#include "Utils/Pimpl.h"
#include "Interfaces/Engine/AudioDataReceiverInterface.h"

UI_FWD(GUI_Spectrum)

class GUI_Spectrum :
		public VisualPlugin,
		public Engine::SpectrumReceiver
{
	Q_OBJECT
	UI_CLASS(GUI_Spectrum)
	PIMPL(GUI_Spectrum)

	public:
		explicit GUI_Spectrum(QWidget* parent=nullptr);
		~GUI_Spectrum() override;

		QString name() const override;
		QString displayName() const override;
		bool isActive() const override;

	protected:
		void paintEvent(QPaintEvent* e) override;
		void showEvent(QShowEvent* e) override;
		void closeEvent(QCloseEvent* e) override;
		void initUi() override;
		void retranslate() override;

		QWidget*	widget() override;
		bool		hasSmallButtons() const override;
		ColorStyle	currentStyle() const override;
		int			currentStyleIndex() const override;
		void		finalizeInitialization() override;

	protected slots:
		void doFadeoutStep() override;

	public slots:
		void setSpectrum(const Engine::SpectrumList& spec) override;
		void update_style(int new_index) override;

	private:
		void activeChanged();
};

#endif // GUI_SPECTRUM_H
