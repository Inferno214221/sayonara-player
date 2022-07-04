/* TimeSpanChooser.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "TimeSpanChooser.h"
#include "Utils/Language/Language.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QFrame>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>

namespace
{
	QWidget* createHorizontalLine()
	{
		auto* line = new QFrame();
		line->setFrameShape(QFrame::Shape::HLine);

		return line;
	}

	void createSection(const Lang::Term text, const int row, QGridLayout* gridLayout, QSpinBox* spinBox)
	{
		auto* label = new QLabel(Lang::get(text));
		gridLayout->addWidget(label, row, 0);
		gridLayout->addWidget(spinBox, row, 1);
	}

	QSpinBox* createSpinBox(const int value, const int maxValue)
	{
		auto* spinBox = new QSpinBox();
		spinBox->setValue(value);
		spinBox->setMinimum(0);
		spinBox->setMaximum(maxValue);

		return spinBox;
	}
}

using SmartPlaylists::TimeSpan;

struct TimeSpanChooser::Private
{
	QSpinBox* sbYears;
	QSpinBox* sbMonths;
	QSpinBox* sbDays;

	explicit Private(const TimeSpan& value) :
		sbYears {createSpinBox(value.years, 9)}, // NOLINT(readability-magic-numbers)
		sbMonths {createSpinBox(value.months, 12)}, // NOLINT(readability-magic-numbers)
		sbDays {createSpinBox(value.days, 30)} {} // NOLINT(readability-magic-numbers)
};

TimeSpanChooser::TimeSpanChooser(const TimeSpan& value, QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>(value);

	auto* layout = new QVBoxLayout {};
	auto* gridLayout = new QGridLayout {};

	createSection(Lang::Years, 0, gridLayout, m->sbYears);
	createSection(Lang::Months, 1, gridLayout, m->sbMonths);
	createSection(Lang::Days, 2, gridLayout, m->sbDays);

	layout->addLayout(gridLayout);
	layout->addWidget(createHorizontalLine());

	auto* buttonBox = new QDialogButtonBox();
	buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	layout->addWidget(buttonBox);

	setLayout(layout);
	setWindowTitle(tr("Create time span"));
	setModal(true);
}

TimeSpanChooser::~TimeSpanChooser() = default;

int TimeSpanChooser::days() const
{
	return SmartPlaylists::timeSpanToDays({
		m->sbYears->value(),
		m->sbMonths->value(),
		m->sbDays->value()
	});
}
