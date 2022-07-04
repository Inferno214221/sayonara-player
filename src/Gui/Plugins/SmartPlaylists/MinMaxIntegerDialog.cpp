/* MinMaxIntegerDialog.cpp */
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

#include "MinMaxIntegerDialog.h"
#include "StringValidator.h"
#include "InputField.h"

#include "Components/SmartPlaylists/SmartPlaylistCreator.h"
#include "Components/SmartPlaylists/TimeSpan.h"
#include "Utils/Language/Language.h"
#include "Utils/Widgets/WidgetTemplate.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

namespace
{
	struct Section
	{
		QWidget* widget {nullptr};
		QLabel* label {nullptr};
		InputField* inputField {nullptr};
		QLayout* layout {nullptr};
		StringValidator* stringValidator {nullptr};
	};

	using SmartPlaylistMap = QMap<SmartPlaylists::Type, std::shared_ptr<SmartPlaylist>>;

	SmartPlaylistMap createSmartPlaylists() noexcept
	{
		auto result = SmartPlaylistMap {};
		for(auto i = 0; i < static_cast<int>(SmartPlaylists::Type::NumEntries); i++)
		{
			const auto type = static_cast<SmartPlaylists::Type>(i);
			result.insert(type, SmartPlaylists::createFromType(type, -1, -1, -1));
		}

		return result;
	}

	const SmartPlaylistMap AllSmartPlaylists {createSmartPlaylists()};

	QWidget* createLine(QWidget* parent)
	{
		auto* frame = new QFrame(parent);
		frame->setFrameShape(QFrame::HLine);
		return frame;
	}

	QString toUserString(const SmartPlaylists::Type type, const int i)
	{
		return AllSmartPlaylists[type]->stringConverter()->intToUserString(i);
	}

	Section createSection(QWidget* parent)
	{
		auto* widget = new QWidget(parent);
		auto* lineEdit = new InputField(widget);
		auto* layout = new QHBoxLayout();
		auto* validator = new StringValidator(lineEdit);
		auto* label = new QLabel(widget);

		lineEdit->setValidator(validator);

		layout->setContentsMargins(2, 2, 2, 2);
		layout->addWidget(label);
		layout->addItem(
			new QSpacerItem(5, 5, QSizePolicy::Policy::MinimumExpanding)); // NOLINT(readability-magic-numbers)
		layout->addWidget(lineEdit);

		widget->setSizePolicy(widget->sizePolicy().horizontalPolicy(), QSizePolicy::Policy::Maximum);
		widget->setLayout(layout);

		return {widget, label, lineEdit, layout, validator};
	}

	void setSectionVisible(const Section& section, const bool b)
	{
		section.widget->setVisible(b);
	}

	QLabel* createTitleLabel(const QString& title)
	{
		auto* label = new QLabel(title);
		auto font = label->font();
		font.setBold(true);
		label->setFont(font);

		return label;
	}

	QComboBox* createTypeCombobox(const SmartPlaylists::Type type)
	{
		auto* comboBox = new QComboBox();
		for(auto it = AllSmartPlaylists.begin(); it != AllSmartPlaylists.end(); it++)
		{
			comboBox->addItem(it.value()->displayClassType(), static_cast<int>(it.key()));
		}
		comboBox->setCurrentIndex(static_cast<int>(type));

		return comboBox;
	}

	QString createDescription(const SmartPlaylists::Type type)
	{
		const auto& smartPlaylist = AllSmartPlaylists[type];
		return QObject::tr("Between %1 and %2")
			.arg(toUserString(type, smartPlaylist->minimumValue()))
			.arg(toUserString(type, smartPlaylist->maximumValue()));
	}

	bool checkText(InputField* lineEdit, const SmartPlaylists::Type type)
	{
		const auto& smartPlaylist = AllSmartPlaylists[type];
		const auto data = lineEdit->data();

		return data.has_value() &&
		       (data.value() >= smartPlaylist->minimumValue()) &&
		       (data.value() <= smartPlaylist->maximumValue());
	}

	void fillText(const Section& section, const SmartPlaylists::Type type, const int value)
	{
		section.inputField->setData(AllSmartPlaylists[type]->inputFormat(),
		                            AllSmartPlaylists[type]->stringConverter(),
		                            value);
	}

	void populate(const SmartPlaylists::Type type, QLabel* labDescription, const std::pair<Section, Section>& sections)
	{
		labDescription->setText(createDescription(type));

		const auto& smartPlaylist = AllSmartPlaylists[type];
		fillText(sections.first, type, smartPlaylist->minimumValue());
		fillText(sections.second, type, smartPlaylist->maximumValue());

		const auto isSingleValue = smartPlaylist->isSingleValue();
		setSectionVisible(sections.second, !isSingleValue);

		if(isSingleValue)
		{
			sections.first.label->setText(QObject::tr("Number of tracks"));
		}

		else
		{
			sections.first.label->setText(QObject::tr("From"));
			sections.second.label->setText(QObject::tr("To"));
		}

		const auto stringConverter = smartPlaylist->stringConverter();
		sections.first.stringValidator->setStringConverter(stringConverter);
		sections.second.stringValidator->setStringConverter(stringConverter);
	}
}

struct MinMaxIntegerDialog::Private
{
	SmartPlaylists::Type type;
	QLabel* labelDescription {new QLabel()};
	std::pair<Section, Section> sections;
	QDialogButtonBox* buttonBox {new QDialogButtonBox({QDialogButtonBox::Ok | QDialogButtonBox::Cancel})};

	Private(const SmartPlaylists::Type type, QWidget* parent) :
		type {type},
		sections {createSection(parent), createSection(parent)} {}
};

MinMaxIntegerDialog::MinMaxIntegerDialog(const SmartPlaylists::Type type, QWidget* parent) :
	QDialog(parent),
	m {Pimpl::make<Private>(type, this)}
{
	setWindowTitle(Lang::get(Lang::SmartPlaylists));
	setLayout(new QVBoxLayout());

	connect(m->sections.first.inputField, &QLineEdit::textChanged, this, &MinMaxIntegerDialog::textChanged);
	connect(m->sections.second.inputField, &QLineEdit::textChanged, this, &MinMaxIntegerDialog::textChanged);

	connect(m->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	populate(m->type, m->labelDescription, m->sections);
}

MinMaxIntegerDialog::MinMaxIntegerDialog(QWidget* parent) :
	MinMaxIntegerDialog(*(AllSmartPlaylists.keyBegin()), parent)
{
	auto* comboBox = createTypeCombobox(m->type);
	connect(comboBox, combo_activated_int, this, &MinMaxIntegerDialog::currentIndexChanged);
	fillLayout(comboBox);
}

MinMaxIntegerDialog::MinMaxIntegerDialog(const std::shared_ptr<SmartPlaylist>& smartPlaylist, QWidget* parent) :
	MinMaxIntegerDialog(smartPlaylist->type(), parent)
{
	auto* labelTitle = createTitleLabel(smartPlaylist->displayClassType());
	fillLayout(labelTitle);

	fillText(m->sections.first, m->type, smartPlaylist->from());
	fillText(m->sections.second, m->type, smartPlaylist->to());
}

MinMaxIntegerDialog::~MinMaxIntegerDialog() = default;

void MinMaxIntegerDialog::fillLayout(QWidget* headerWidget)
{
	layout()->addWidget(headerWidget);
	layout()->addWidget(createLine(this));
	layout()->addWidget(m->labelDescription);
	layout()->addWidget(m->sections.first.widget);
	layout()->addWidget(m->sections.second.widget);
	layout()->addWidget(createLine(this));
	layout()->addWidget(m->buttonBox);
}

void MinMaxIntegerDialog::currentIndexChanged(const int /*currentIndex*/)
{
	auto* comboBox = dynamic_cast<QComboBox*>(sender());
	m->type = static_cast<SmartPlaylists::Type>(comboBox->currentData().toInt());

	populate(m->type, m->labelDescription, m->sections);
}

void MinMaxIntegerDialog::textChanged(const QString& /*text*/)
{
	auto* lineEdit = dynamic_cast<InputField*>(sender());
	const auto isValid = checkText(lineEdit, m->type);

	const auto styleSheet = isValid ? QString() : QString("color: red;");
	lineEdit->setStyleSheet(styleSheet);

	auto* okButton = m->buttonBox->button(QDialogButtonBox::Ok);
	okButton->setEnabled(isValid);
}

int MinMaxIntegerDialog::fromValue() const
{
	const auto value1 = m->sections.first.inputField->data().value();
	const auto value2 = m->sections.second.inputField->data().value();

	return (m->sections.second.widget->isHidden()) ? value1 : std::min(value1, value2);
}

int MinMaxIntegerDialog::toValue() const
{
	const auto value1 = m->sections.first.inputField->data().value();
	const auto value2 = m->sections.second.inputField->data().value();

	return (m->sections.second.widget->isHidden()) ? value1 : std::max(value1, value2);
}

SmartPlaylists::Type MinMaxIntegerDialog::type() const { return m->type; }
