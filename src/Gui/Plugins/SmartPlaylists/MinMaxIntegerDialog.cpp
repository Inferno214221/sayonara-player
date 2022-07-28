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
#include "Utils/Algorithm.h"
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

	std::shared_ptr<SmartPlaylist> createDummySmartPlaylist(const SmartPlaylists::Type type)
	{
		auto smartPlaylist = SmartPlaylists::createFromType(type, -1, {-1, -1, -1, -1, -1});
		smartPlaylist->setValue(0, smartPlaylist->minimumValue());
		smartPlaylist->setValue(1, smartPlaylist->maximumValue());
		return smartPlaylist;
	}

	QWidget* createLine(QWidget* parent)
	{
		auto* frame = new QFrame(parent);
		frame->setFrameShape(QFrame::HLine);
		return frame;
	}

	QString toUserString(const SmartPlaylists::Type type, const int i)
	{
		const auto smartPlaylist = createDummySmartPlaylist(type);
		return smartPlaylist->stringConverter()->intToUserString(i);
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
			new QSpacerItem(100, 5, QSizePolicy::Policy::Expanding)); // NOLINT(readability-magic-numbers)
		layout->addWidget(lineEdit);

		widget->setSizePolicy(widget->sizePolicy().horizontalPolicy(), QSizePolicy::Policy::Maximum);
		widget->setLayout(layout);

		return {widget, label, lineEdit, layout, validator};
	}

	QLabel* createTitleLabel(const QString& title)
	{
		auto* label = new QLabel(title);
		auto font = label->font();
		font.setBold(true);
		label->setFont(font);

		return label;
	}

	QComboBox* createTypeCombobox(const SmartPlaylists::Type preselectedType)
	{
		auto* comboBox = new QComboBox();
		for(int i = 0; i < static_cast<int>(SmartPlaylists::Type::NumEntries); i++)
		{
			const auto type = static_cast<SmartPlaylists::Type>(i);
			const auto smartPlaylist = createDummySmartPlaylist(type);
			comboBox->addItem(smartPlaylist->displayClassType(), i);
		}

		comboBox->setCurrentIndex(static_cast<int>(preselectedType));

		return comboBox;
	}

	QString createDescription(const SmartPlaylists::Type type)
	{
		const auto smartPlaylist = createDummySmartPlaylist(type);
		return QObject::tr("Between %1 and %2")
			.arg(toUserString(type, smartPlaylist->minimumValue()))
			.arg(toUserString(type, smartPlaylist->maximumValue()));
	}

	bool checkText(InputField* lineEdit, const SmartPlaylists::Type type)
	{
		const auto smartPlaylist = createDummySmartPlaylist(type);
		const auto data = lineEdit->data();
		return data.has_value() &&
		       (data.value() >= smartPlaylist->minimumValue()) &&
		       (data.value() <= smartPlaylist->maximumValue());
	}

	void fillText(const Section& section, const SmartPlaylists::Type type, const int value)
	{
		const auto smartPlaylist = createDummySmartPlaylist(type);
		section.inputField->setData(smartPlaylist->inputFormat(), smartPlaylist->stringConverter(), value);
	}

	QWidget* replaceSectionWidget(QWidget* oldWidget, QLayout* layout)
	{
		auto* sectionWidget = new QWidget();
		sectionWidget->setLayout(new QVBoxLayout());
		layout->replaceWidget(oldWidget, sectionWidget);
		oldWidget->deleteLater();

		return sectionWidget;
	}

	QList<Section> createSections(const SmartPlaylists::Type type, QWidget* parent)
	{
		auto sections = QList<Section> {};
		const auto smartPlaylist = createDummySmartPlaylist(type);
		for(int i = 0; i < smartPlaylist->count(); i++)
		{
			const auto section = createSection(parent);
			sections << section;
			parent->layout()->addWidget(section.widget);
		}

		parent->layout()->addItem(
			new QSpacerItem(1, 1, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::MinimumExpanding));

		return sections;
	}

	void populate(const SmartPlaylists::Type type, QLabel* labDescription, const QList<Section>& sections)
	{
		const auto description = createDescription(type);
		labDescription->setText(description);

		const auto smartPlaylist = createDummySmartPlaylist(type);
		const auto stringConverter = smartPlaylist->stringConverter();

		for(int i = 0; i < smartPlaylist->count(); i++)
		{
			fillText(sections[i], type, smartPlaylist->value(i));
			sections[i].label->setText(smartPlaylist->text(i));
			sections[i].stringValidator->setStringConverter(stringConverter);
		}
	}
}

struct MinMaxIntegerDialog::Private
{
	SmartPlaylists::Type type;
	QLabel* labelDescription {new QLabel()};
	QWidget* sectionWidget {new QWidget()};
	QList<Section> sections;
	QDialogButtonBox* buttonBox {new QDialogButtonBox({QDialogButtonBox::Ok | QDialogButtonBox::Cancel})};

	explicit Private(const SmartPlaylists::Type type) :
		type {type}
	{
		sectionWidget->setLayout(new QVBoxLayout());
		sections = createSections(type, sectionWidget);
	}
};

MinMaxIntegerDialog::MinMaxIntegerDialog(const SmartPlaylists::Type type, QWidget* parent) :
	QDialog(parent),
	m {Pimpl::make<Private>(type)}
{
	setWindowTitle(Lang::get(Lang::SmartPlaylists));
	setLayout(new QVBoxLayout());

	connectTextFieldChanges(m->sections);
	connect(m->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	populate(m->type, m->labelDescription, m->sections);
}

MinMaxIntegerDialog::MinMaxIntegerDialog(QWidget* parent) :
	MinMaxIntegerDialog(static_cast<SmartPlaylists::Type>(0), parent)
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

	for(int i = 0; i < smartPlaylist->count(); i++)
	{
		fillText(m->sections[i], m->type, smartPlaylist->value(i));
	}
}

MinMaxIntegerDialog::~MinMaxIntegerDialog() = default;

void MinMaxIntegerDialog::fillLayout(QWidget* headerWidget)
{
	layout()->addWidget(headerWidget);
	layout()->addWidget(createLine(this));
	layout()->addWidget(m->labelDescription);
	layout()->addWidget(m->sectionWidget);
	layout()->addWidget(createLine(this));
	layout()->addWidget(m->buttonBox);
}

void MinMaxIntegerDialog::connectTextFieldChanges(const QList<Section>& sections) const
{
	for(const auto& section: sections)
	{
		connect(section.inputField, &QLineEdit::textChanged, this, &MinMaxIntegerDialog::textChanged);
	}
}

void MinMaxIntegerDialog::currentIndexChanged(const int /*currentIndex*/)
{
	auto* comboBox = dynamic_cast<QComboBox*>(sender());
	m->type = static_cast<SmartPlaylists::Type>(comboBox->currentData().toInt());

	m->sectionWidget = replaceSectionWidget(m->sectionWidget, layout());
	m->sections = createSections(m->type, m->sectionWidget);
	connectTextFieldChanges(m->sections);

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

QList<int> MinMaxIntegerDialog::values() const
{
	auto result = QList<int> {};
	Util::Algorithm::transform(m->sections, result, [](const auto& section) {
		return section.inputField->data().value();
	});

	return result;
}

SmartPlaylists::Type MinMaxIntegerDialog::type() const { return m->type; }
