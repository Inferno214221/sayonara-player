/* GUI_LevelPainter.cpp */

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


#include "GUI_LevelPainter.h"
#include "VisualColorStyleChooser.h"
#include "Gui/Plugins/ui_GUI_LevelPainter.h"

#include "Interfaces/AudioDataProvider.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include <QPainter>
#include <QBrush>

#include <cstring>
#include <array>
#include <vector>
#include <atomic>

static const size_t Channels = 2;

using Step = uint_fast8_t;

using ChannelArray = std::array<float, Channels>;
using ChannelSteps = std::vector<Step>;
using StepArray = std::array<ChannelSteps, Channels>;

struct GUI_LevelPainter::Private
{
	ChannelArray level;
	StepArray steps;
	float* expFunctionLookupTable = nullptr;
	LevelDataProvider* dataProvider;

	std::atomic_flag lock = ATOMIC_FLAG_INIT;

	Private(LevelDataProvider* dataProvider) :
		dataProvider(dataProvider)
	{}

	void resizeSteps(int n_rects)
	{
		for(size_t c = 0; c < level.size(); c++)
		{
			steps[c].resize(size_t(n_rects));
			std::fill(steps[c].begin(), steps[c].end(), 0);
		}
	}

	void initLookupTable()
	{
		size_t n = 40;
		expFunctionLookupTable = new float[n];
		for(size_t i = 0; i < n; i++)
		{
			expFunctionLookupTable[i] = -(i / 40.0f) + 1.0f;
		}
	}

	float scale(float value)
	{
		int v = int(-value);

		// [-39, 0]
		int idx = std::min(v, 39);
		idx = std::max(0, idx);

		return expFunctionLookupTable[idx];
	}

	void setLevel(float left, float right)
	{
		level[0] = scale(left);
		level[1] = scale(right);
	}

	void decreaseStep(size_t channel, size_t step)
	{
		steps[channel][step] = steps[channel][step] - 1;
	}

	void setStep(size_t channel, size_t step, Step value)
	{
		steps[channel][step] = value;
	}
};

GUI_LevelPainter::GUI_LevelPainter(LevelDataProvider* dataProvider, PlayManager* playManager, QWidget* parent) :
	VisualPlugin(playManager, parent),
	Engine::LevelReceiver()
{
	m = Pimpl::make<Private>(dataProvider);
	SetSetting(Set::Engine_ShowLevel, false);
}

GUI_LevelPainter::~GUI_LevelPainter()
{
	m->dataProvider->unregisterLevelReceiver(this);

	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_LevelPainter::initUi()
{
	if(isUiInitialized())
	{
		return;
	}

	m->initLookupTable();

	setupParent(this, &ui);
}

void GUI_LevelPainter::finalizeInitialization()
{
	VisualPlugin::initUi();

	m->resizeSteps(currentStyle().n_rects);
	m->setLevel(0, 0);

	PlayerPlugin::Base::finalizeInitialization();
	m->dataProvider->registerLevelReceiver(this);

	reload();
}

QString GUI_LevelPainter::name() const
{
	return "Level";
}

QString GUI_LevelPainter::displayName() const
{
	return tr("Level");
}

bool GUI_LevelPainter::isActive() const
{
	return this->isVisible();
}

void GUI_LevelPainter::retranslate()
{
	ui->retranslateUi(this);
}

void GUI_LevelPainter::setLevel(float left, float right)
{
	if(!isUiInitialized() || !isVisible())
	{
		return;
	}

	if(m->lock.test_and_set())
	{
		return;
	}

	m->setLevel(left, right);

	stop_fadeout_timer();
	update();

	m->lock.clear();
}

void GUI_LevelPainter::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e)

	QPainter painter(this);

	ColorStyle style = currentStyle();
	int n_rects = style.n_rects;
	int border_x = style.hor_spacing;
	int border_y = style.ver_spacing;
	int n_fading_steps = style.n_fading_steps;
	int h_rect = style.rect_height;
	int w_rect = style.rect_width;

	int y = 10;
	size_t num_zero = 0;
	int x_init = (w_rect + border_x);

	for(size_t c = 0; c < Channels; c++)
	{
		size_t n_colored_rects = size_t(n_rects * m->level[c]);

		QRect rect(0, y, w_rect, h_rect);

		for(size_t r = 0; r < size_t(n_rects); r++)
		{
			if(r < n_colored_rects)
			{
				if(!style.style[r].contains(-1))
				{
					spLog(Log::Debug, this) << "Style does not contain -1";
				}

				painter.fillRect(rect, style.style[r].value(-1));

				m->setStep(c, r, Step(n_fading_steps - 1));
			}

			else
			{
				if(!style.style[r].contains(m->steps[c][r]))
				{
					spLog(Log::Debug, this) << "2 Style does not contain " << m->steps[c][r] << ", " << c << ", " << r;
				}

				painter.fillRect(rect, style.style[r].value(m->steps[c][r]));

				if(m->steps[c][r] > 0)
				{
					m->decreaseStep(c, r);
				}

				if(m->steps[c][r] == 0)
				{
					num_zero++;
				}
			}

			rect.translate(x_init, 0);
		}

		if(num_zero == Channels * size_t(n_rects))
		{
			// all rectangles where fade out
			stop_fadeout_timer();
		}

		y += h_rect + border_y;
	}
}

void GUI_LevelPainter::doFadeoutStep()
{
	for(float& l : m->level)
	{
		l -= 2.0f;
	}

	update();
}

void GUI_LevelPainter::update_style(int new_index)
{
	SetSetting(Set::Level_Style, new_index);
	m_ecsc->reload(width(), height());

	m->resizeSteps(currentStyle().n_rects);

	update();
}

void GUI_LevelPainter::reload()
{
	ColorStyle style = currentStyle();
	int new_height = style.rect_height * 2 + style.ver_spacing + 12;

	setMinimumHeight(0);
	setMaximumHeight(100);

	setMinimumHeight(new_height);
	setMaximumHeight(new_height);

	if(isVisible())
	{
		emit sigReload(this);
	}
}

void GUI_LevelPainter::showEvent(QShowEvent* e)
{
	SetSetting(Set::Engine_ShowLevel, true);
	m->dataProvider->levelActiveChanged(true);
	VisualPlugin::showEvent(e);
}

void GUI_LevelPainter::closeEvent(QCloseEvent* e)
{
	SetSetting(Set::Engine_ShowLevel, false);
	m->dataProvider->levelActiveChanged(false);
	VisualPlugin::closeEvent(e);
}

void GUI_LevelPainter::hideEvent(QHideEvent* e)
{
	VisualPlugin::hideEvent(e);
}

QWidget* GUI_LevelPainter::widget()
{
	return this;
}

bool GUI_LevelPainter::hasSmallButtons() const
{
	return true;
}

ColorStyle GUI_LevelPainter::currentStyle() const
{
	return m_ecsc->get_color_scheme_level(currentStyleIndex());
}

int GUI_LevelPainter::currentStyleIndex() const
{
	return GetSetting(Set::Level_Style);
}
