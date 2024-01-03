/* SpectrumLabel.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "SpectrumLabel.h"
#include "Interfaces/AudioDataProvider.h"
#include <cmath>

#include <QPixmap>
#include <QPainter>
#include <QEvent>

struct SpectrumLabel::Private
{
	SpectrumDataProvider* dataProvider;

	Private(SpectrumDataProvider* dataProvider) :
		dataProvider(dataProvider) {}
};

SpectrumLabel::SpectrumLabel(SpectrumDataProvider* dataProvider, QWidget* parent) :
	QLabel(parent)
{
	m = Pimpl::make<Private>(dataProvider);

	m->dataProvider->registerSpectrumReceiver(this);
	m->dataProvider->spectrumActiveChanged(true);
}

SpectrumLabel::~SpectrumLabel() = default;

void SpectrumLabel::setSpectrum(const std::vector<float>& spectrum)
{
	QPixmap pm(this->width(), this->height());
	pm.fill(QColor(0, 0, 0, 0));

	QPainter painter(&pm);

	double bass = (spectrum[1] + 75.0) / 75.0;
	double midTmp = (spectrum[spectrum.size() / 2] + 75.0) / 75.0;
	double highTmp = (spectrum[spectrum.size() - 2] + 76.0) / 75.0;

	double mid = std::pow(midTmp, 0.5);
	double high = std::pow(highTmp, 0.3) * 2.0;

	int w = this->width() / 3 - 3;
	int h = this->height();
	// left, top, width, height
	QRect bassRect(2, h - int(h * bass), w, h);
	QRect midRect(2 + w, h - int(h * mid), w, h);
	QRect highRect(2 + w + w, h - int(h * high), w, h);

	painter.setBrush(QColor(255, 255, 255));
	painter.drawRect(bassRect);
	painter.drawRect(midRect);
	painter.drawRect(highRect);

	this->setPixmap(pm);

	emit sigPixmapChanged();
}

bool SpectrumLabel::isActive() const
{
	return this->isVisible();
}
