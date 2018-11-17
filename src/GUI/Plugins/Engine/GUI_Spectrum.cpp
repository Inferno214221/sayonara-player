/* GUI_Spectrum.cpp */

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

#include "GUI_Spectrum.h"
#include "GUI/Plugins/ui_GUI_Spectrum.h"

#include "EngineColorStyleChooser.h"

#include "Components/Engine/Playback/PlaybackEngine.h"
#include "Components/Engine/EngineHandler.h"

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

using Step=uint_fast8_t;
using BinSteps=std::vector<Step>;
using StepArray=std::vector<BinSteps>;

struct GUI_Spectrum::Private
{
	std::atomic_flag	locked = ATOMIC_FLAG_INIT;
	SpectrumList		spec;
	StepArray			steps;
	float*				log_lu=nullptr;

	void init_lookup_table(int bins)
	{
		log_lu = new float[bins];

		// make bass value smaller than high frequencies
		// log_lu[0] = 0.0.01666667
		// log_lu[50] = 0.37930765
		for(int i=0; i<bins; i++)
		{
			log_lu[i] = (std::pow(10.0f, (i / 140.0f) + 1.0f) / 8.0f) / 75.0f;
		}
	}

	void set_spectrum(const SpectrumList& s)
	{
		// s: [-75, 0]
		// s[i] + 75: [0, 75]
		// scaling factor of ( / 75.0) is in log_lu

		spec.clear();
		spec.reserve(s.size());

		for(size_t i=0; i<s.size(); i++)
		{
			float f = (s[i] + 75.0) * log_lu[i];
			spec.push_back(f);
		}
	}

	void resize_steps(int n_bins, int rects)
	{
		n_bins = std::max(50, n_bins);

		if(n_bins != (int) steps.size()){
			steps.resize(n_bins);
		}

		for(BinSteps& bin_steps : steps)
		{
			bin_steps.resize(rects);
			std::fill(bin_steps.begin(), bin_steps.end(), 0);
		}
	}
};


GUI_Spectrum::GUI_Spectrum(QWidget *parent) :
	EnginePlugin(parent)
{
	m = Pimpl::make<Private>();

	_settings->set<Set::Engine_ShowSpectrum>(false);
}


GUI_Spectrum::~GUI_Spectrum()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}


void GUI_Spectrum::init_ui()
{
	if(is_ui_initialized()){
		return;
	}

	setup_parent(this, &ui);
}

void GUI_Spectrum::finalize_initialization()
{
	EnginePlugin::init_ui();

	int bins = _settings->get<Set::Engine_SpectrumBins>();

	m->init_lookup_table(bins);
	m->resize_steps(bins, current_style().n_rects);
	m->spec.resize((size_t) bins, -100.0f);

#pragma message "add spectrum receiver"

	PlayerPlugin::Base::finalize_initialization();

	update();
}


QString GUI_Spectrum::get_name() const
{
	return "Spectrum";
}


QString GUI_Spectrum::get_display_name() const
{
	return tr("Spectrum");
}


void GUI_Spectrum::retranslate_ui() {}

void GUI_Spectrum::set_spectrum(const SpectrumList& spec)
{
	if(!is_ui_initialized() || !isVisible()){
		return;
	}

	m->set_spectrum(spec);

	stop_fadeout_timer();
	update();
}


void GUI_Spectrum::do_fadeout_step()
{
	for(auto it=m->spec.begin(); it!= m->spec.end(); it++)
	{
		*it = (*it - 1.5f);
	}

	update();
}


void GUI_Spectrum::update_style(int new_index)
{
	if(!is_ui_initialized()){
		return;
	}

	if(m->locked.test_and_set()) {
		sp_log(Log::Debug, this) << "Cannot update stylde";
		return;
	}

	_ecsc->reload(width(), height());
	_settings->set<Set::Spectrum_Style>(new_index);

	int bins = _settings->get<Set::Engine_SpectrumBins>();
	m->resize_steps(bins, current_style().n_rects);

	update();

	m->locked.clear();
}


void GUI_Spectrum::showEvent(QShowEvent* e)
{
	_settings->set<Set::Engine_ShowSpectrum>(true);
	EnginePlugin::showEvent(e);
}


void GUI_Spectrum::closeEvent(QCloseEvent* e)
{
	_settings->set<Set::Engine_ShowSpectrum>(false);
	EnginePlugin::closeEvent(e);
}

void GUI_Spectrum::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e)

	QPainter painter(this);

	float widget_height = (float) height();

	ColorStyle style = current_style();
	int n_rects = style.n_rects;
	int n_fading_steps = style.n_fading_steps;
	int h_rect = (widget_height / n_rects) - style.ver_spacing;
	int border_y = style.ver_spacing;
	int border_x = style.hor_spacing;

	int x=3;
	int ninety = 35;
	int offset = 0;
	int n_zero = 0;

	if(ninety == 0) {
		return;
	}

	int w_bin = ((width() + 10) / (ninety - offset)) - border_x;

	// run through all bins
	for(int i=offset; i<ninety + 1; i++)
	{
		// if this is one bar, how tall would it be?
		int h =  m->spec[i] * widget_height;

		// how many colored rectangles would fit into this bar?
		int colored_rects = h / (h_rect + border_y) - 1 ;

		colored_rects = std::max(colored_rects, 0);

		// we start from bottom with painting
		int y = widget_height - h_rect;

		// run vertical

		QRect rect(x, y, w_bin, h_rect);
		QColor col;
		for(int r=0; r<n_rects; r++)
		{
			// 100%
			if( r < colored_rects)
			{
				col = current_style().style[r].value(-1);
				m->steps[i][r] = n_fading_steps;
			}

			// fading out
			else
			{
				col = current_style().style[r].value(m->steps[i][r]);

				if(m->steps[i][r] > 0) {
					m->steps[i][r]--;
				}

				else {
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

bool GUI_Spectrum::has_small_buttons() const
{
	return false;
}

ColorStyle GUI_Spectrum::current_style() const
{
	return _ecsc->get_color_scheme_spectrum(current_style_index());
}

int GUI_Spectrum::current_style_index() const
{
	return _settings->get<Set::Spectrum_Style>();
}
