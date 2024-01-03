/* SpectrumLabel.h
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

#ifndef SPECTRUMLABEL_H
#define SPECTRUMLABEL_H

#include "Interfaces/Engine/AudioDataReceiver.h"

#include "Utils/Pimpl.h"

#include <QLabel>
#include <vector>

class SpectrumDataProvider;
class SpectrumLabel :
	public QLabel,
	public Engine::SpectrumDataReceiver
{
	Q_OBJECT
	PIMPL(SpectrumLabel)

	signals:
		void sigPixmapChanged();

	public:
		SpectrumLabel(SpectrumDataProvider* dataProvider, QWidget* parent);
		~SpectrumLabel() override;

		void setSpectrum(const std::vector<float>& spectrum) override;
		bool isActive() const override;
};

#endif // SPECTRUMLABEL_H
