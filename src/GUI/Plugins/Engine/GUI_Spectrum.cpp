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

static std::mutex mtx;


using Step=uint_fast8_t;
using BinSteps=std::vector<Step>;
using StepArray=std::vector<BinSteps>;

struct GUI_Spectrum::Private
{
	SpectrumList	spec;
	StepArray		steps;
	float*			log_lu=nullptr;
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

	int bins = _settings->get<Set::Engine_SpectrumBins>();
	bins = std::max(50, bins);

	_cur_style_idx = _settings->get<Set::Spectrum_Style>();

	resize_steps(bins, _cur_style.n_rects);

	m->spec.resize((size_t) bins, -100.0f);

	m->log_lu = new float[bins];
	for(int i=0; i<bins; i++)
	{
		m->log_lu[i] = (std::pow(10.0f, (i / 140.0f) + 1.0f) / 8.0f) / 75.0f;
	}

	setup_parent(this, &ui);
	EnginePlugin::init_ui();


	_cur_style = _ecsc->get_color_scheme_spectrum(_cur_style_idx);

	Engine::Playback* playback_engine = engine()->get_playback_engine();
	if(playback_engine){
		playback_engine->add_spectrum_receiver(this);
	}

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

	m->spec = spec;

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


void GUI_Spectrum::resize_steps(int n_bins, int rects)
{
	if(!is_ui_initialized()){
		return;
	}

	if(n_bins != (int) m->steps.size()){
		m->steps.resize(n_bins);
	}

	for(BinSteps& bin_steps : m->steps)
	{
		bin_steps.resize(rects);
		std::fill(bin_steps.begin(), bin_steps.end(), 0);
	}
}



void GUI_Spectrum::sl_update_style()
{
	if(!is_ui_initialized()){
		return;
	}

	if(!mtx.try_lock()) {
		sp_log(Log::Debug, this) << "Cannot update stylde";
		return;
	}

	_ecsc->reload(width(), height());
	_cur_style = _ecsc->get_color_scheme_spectrum(_cur_style_idx);
	_settings->set<Set::Spectrum_Style>(_cur_style_idx);

	int bins = _settings->get<Set::Engine_SpectrumBins>();
	resize_steps(bins, _cur_style.n_rects);

	update();
	mtx.unlock();
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

	int n_rects = _cur_style.n_rects;
	int n_fading_steps = _cur_style.n_fading_steps;
	int h_rect = (widget_height / n_rects) - _cur_style.ver_spacing;
	int border_y = _cur_style.ver_spacing;
	int border_x = _cur_style.hor_spacing;

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
		// spec: [-75, 0]
		// f_scaled: [0, 75]
		// scaling factor of ( / 75.0) is in log_lu
		float f_scaled = (m->spec[i] + 75.0);
		float f = f_scaled * m->log_lu[i];

		// if this is one bar, how tall would it be?
		int h =  f * widget_height;

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
				col = _cur_style.style[r].value(-1);
				m->steps[i][r] = n_fading_steps;
			}

			// fading out
			else
			{
				col = _cur_style.style[r].value(m->steps[i][r]);

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
