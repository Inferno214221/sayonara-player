/* GUI_Spectrum.cpp */

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

#include "GUI_Spectrum.h"
#include "Gui/Plugins/ui_GUI_Spectrum.h"

#include "Interfaces/AudioDataProvider.h"

#include "VisualColorStyleChooser.h"

#include "Utils/globals.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QPainter>
#include <QList>
#include <QTimer>

#include <cstring>
#include <cmath>
#include <algorithm>
#include <mutex>

using Step = uint_fast8_t;
using BinSteps = std::vector<Step>;
using StepArray = std::vector<BinSteps>;

struct GUI_Spectrum::Private
{
	std::atomic_flag locked = ATOMIC_FLAG_INIT;

	std::vector<float> spec;
	StepArray steps;
	SpectrumDataProvider* dataProvider;
	float* logarithmLookupTable = nullptr;

	explicit Private(SpectrumDataProvider* dataProvider) :
		dataProvider(dataProvider) {}

	void initLookupTable(int bins)
	{
		logarithmLookupTable = new float[bins];

		// make bass value smaller than high frequencies
		// log_lu[0] = 0.0.01666667
		// log_lu[50] = 0.37930765
		for(int i = 0; i < bins; i++)
		{
			logarithmLookupTable[i] = (std::pow(10.0f, (i / 140.0f) + 1.0f) / 8.0f) / 75.0f;
		}
	}

	void setSpectrum(const std::vector<float>& spectrum)
	{
		// s: [-75, 0]
		// s[i] + 75: [0, 75]
		// scaling factor of ( / 75.0) is in log_lu

		spec.clear();
		spec.reserve(spectrum.size());

		for(size_t i = 0; i < spectrum.size(); i++)
		{
			float f = (spectrum[i] + 75.0) * logarithmLookupTable[i];
			spec.push_back(f);
		}
	}

	void resizeSteps(int binCount, int rects)
	{
		binCount = std::max(50, binCount);

		if(binCount != (int) steps.size())
		{
			steps.resize(binCount);
		}

		for(BinSteps& step: steps)
		{
			step.resize(rects);
			std::fill(step.begin(), step.end(), 0);
		}
	}
};

GUI_Spectrum::GUI_Spectrum(SpectrumDataProvider* dataProvider, PlayManager* playManager, QWidget* parent) :
	VisualPlugin(playManager, parent),
	Engine::SpectrumDataReceiver()
{
	m = Pimpl::make<Private>(dataProvider);

	SetSetting(Set::Engine_ShowSpectrum, false);
}

GUI_Spectrum::~GUI_Spectrum()
{
	m->dataProvider->unregisterSpectrumReceiver(this);

	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_Spectrum::initUi()
{
	if(isUiInitialized())
	{
		return;
	}

	setupParent(this, &ui);
}

void GUI_Spectrum::finalizeInitialization()
{
	VisualPlugin::initUi();

	int bins = GetSetting(Set::Engine_SpectrumBins);

	m->initLookupTable(bins);
	m->resizeSteps(bins, currentStyle().n_rects);
	m->spec.resize((size_t) bins, -100.0f);

	PlayerPlugin::Base::finalizeInitialization();
	m->dataProvider->registerSpectrumReceiver(this);

	update();
}

QString GUI_Spectrum::name() const
{
	return "Spectrum";
}

QString GUI_Spectrum::displayName() const
{
	return tr("Spectrum");
}

bool GUI_Spectrum::isActive() const
{
	return GetSetting(Set::Engine_ShowSpectrum);
}

void GUI_Spectrum::retranslate() {}

void GUI_Spectrum::setSpectrum(const std::vector<float>& spectrum)
{
	if(!isUiInitialized() || !isVisible())
	{
		return;
	}

	m->setSpectrum(spectrum);

	stop_fadeout_timer();
	update();
}

void GUI_Spectrum::doFadeoutStep()
{
	for(auto it = m->spec.begin(); it != m->spec.end(); it++)
	{
		*it = (*it - 1.5f);
	}

	update();
}

void GUI_Spectrum::update_style(int new_index)
{
	if(!isUiInitialized())
	{
		return;
	}

	if(m->locked.test_and_set())
	{
		spLog(Log::Debug, this) << "Cannot update stylde";
		return;
	}

	m_ecsc->reload(width(), height());
	SetSetting(Set::Spectrum_Style, new_index);

	int bins = GetSetting(Set::Engine_SpectrumBins);
	m->resizeSteps(bins, currentStyle().n_rects);

	update();

	m->locked.clear();
}

void GUI_Spectrum::showEvent(QShowEvent* e)
{
	SetSetting(Set::Engine_ShowSpectrum, true);
	m->dataProvider->spectrumActiveChanged(true);
	VisualPlugin::showEvent(e);
}

void GUI_Spectrum::closeEvent(QCloseEvent* e)
{
	VisualPlugin::closeEvent(e);
	SetSetting(Set::Engine_ShowSpectrum, false);
	m->dataProvider->spectrumActiveChanged(false);
}

void GUI_Spectrum::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e)

	QPainter painter(this);

	float widget_height = height() * 1.0f;

	ColorStyle style = currentStyle();
	int n_rects = style.n_rects;
	int n_fading_steps = style.n_fading_steps;
	int h_rect = int((widget_height / n_rects) - style.ver_spacing);
	int border_y = style.ver_spacing;
	int border_x = style.hor_spacing;

	const int ninety = 35;
	const int offset = 0;
	int n_zero = 0;
	int x = 3;

	int w_bin = ((width() + 10) / (ninety - offset)) - border_x;

	// run through all bins
	for(int i = offset; i < ninety + 1; i++)
	{
		// if this is one bar, how tall would it be?
		int h = int(m->spec.at(i) * widget_height);

		// how many colored rectangles would fit into this bar?
		int colored_rects = h / (h_rect + border_y) - 1;

		colored_rects = std::max(colored_rects, 0);

		// we start from bottom with painting
		int y = int(widget_height - h_rect);

		// run vertical

		QRect rect(x, y, w_bin, h_rect);
		QColor col;
		for(int r = 0; r < n_rects; r++)
		{
			// 100%
			if(r < colored_rects)
			{
				col = currentStyle().style[r].value(-1);
				m->steps[i][r] = n_fading_steps;
			}

				// fading out
			else
			{
				col = currentStyle().style[r].value(m->steps[i][r]);

				if(m->steps[i][r] > 0)
				{
					m->steps[i][r]--;
				}

				else
				{
					n_zero++;
				}
			}

			painter.fillRect(rect, col);

			rect.translate(0, -(h_rect + border_y));
		}

		x += w_bin + border_x;
	}

	if(n_zero == (ninety - offset) * n_rects)
	{
		stop_fadeout_timer();
	}
}

QWidget* GUI_Spectrum::widget()
{
	return ui->lab;
}

bool GUI_Spectrum::hasSmallButtons() const
{
	return false;
}

ColorStyle GUI_Spectrum::currentStyle() const
{
	return m_ecsc->get_color_scheme_spectrum(currentStyleIndex());
}

int GUI_Spectrum::currentStyleIndex() const
{
	return GetSetting(Set::Spectrum_Style);
}
