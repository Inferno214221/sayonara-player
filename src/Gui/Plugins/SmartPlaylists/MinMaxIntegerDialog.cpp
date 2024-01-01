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

#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/SmartPlaylists/SmartPlaylistCreator.h"
#include "Components/SmartPlaylists/TimeSpan.h"
#include "Gui/Utils/GuiUtils.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Widgets/WidgetTemplate.h"

#include <QCheckBox>
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
		QLabel* label {nullptr};
		InputField* inputField {nullptr};
		StringValidator* stringValidator {nullptr};
	};

	std::shared_ptr<SmartPlaylist> createDummySmartPlaylist(const SmartPlaylists::Type type)
	{
		auto smartPlaylist = SmartPlaylists::createFromType(type, -1, {-1, -1, -1, -1, -1}, true, -1, nullptr);
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

	Section createSection(const int row, QWidget* parent, QGridLayout* gridLayout)
	{
		auto* label = new QLabel(parent);
		auto* lineEdit = new InputField(parent);
		auto* validator = new StringValidator(lineEdit);

		lineEdit->setValidator(validator);

		gridLayout->addWidget(label, row, 0);
		gridLayout->addWidget(lineEdit, row, 1);

		return {label, lineEdit, validator};
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

	void fillText(const Section& section, const std::shared_ptr<SmartPlaylist>& smartPlaylist, const int index)
	{
		const auto value = smartPlaylist->value(index);
		section.inputField->setData(smartPlaylist->inputFormat(), smartPlaylist->stringConverter(), value);
	}

	QGridLayout* createGridLayout()
	{
		auto* gridLayout = new QGridLayout();

		gridLayout->setVerticalSpacing(5); // NOLINT(readability-magic-numbers)
		gridLayout->setColumnStretch(0, 2);
		gridLayout->setColumnStretch(1, 3);

		return gridLayout;
	}

	QWidget* replaceSectionWidget(QWidget* oldWidget, QLayout* layout)
	{
		auto* sectionWidget = new QWidget();
		sectionWidget->setLayout(createGridLayout());
		layout->replaceWidget(oldWidget, sectionWidget);
		oldWidget->deleteLater();

		return sectionWidget;
	}

	QList<Section> createSections(const SmartPlaylists::Type type, QWidget* sectionWidget, QGridLayout* gridLayout)
	{
		auto sections = QList<Section> {};
		const auto smartPlaylist = createDummySmartPlaylist(type);
		for(int i = 0; i < smartPlaylist->count(); i++)
		{
			const auto section = createSection(i, sectionWidget, gridLayout);
			sections << section;
		}

		gridLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
		                    smartPlaylist->count(), 0);

		return sections;
	}

	void populateLibraryComboBox(Library::InfoAccessor* libraryManager, QComboBox* comboBox)
	{
		comboBox->clear();
		comboBox->addItem(QObject::tr("All libraries"), -1);

		const auto libraryInfos = libraryManager->allLibraries();
		for(const auto& libraryInfo: libraryInfos)
		{
			const auto iLibraryId = static_cast<int>(libraryInfo.id()); // NOLINT(cert-str34-c)
			comboBox->addItem(libraryInfo.name(), QVariant::fromValue(iLibraryId));
		}

		comboBox->setCurrentIndex(0);
	}

	void populate(const SmartPlaylists::Type type, QLabel* labDescription, QCheckBox* cbShuffle,
	              const QList<Section>& sections)
	{
		const auto description = createDescription(type);
		labDescription->setText(description);

		const auto smartPlaylist = createDummySmartPlaylist(type);
		const auto stringConverter = smartPlaylist->stringConverter();

		for(int i = 0; i < smartPlaylist->count(); i++)
		{
			fillText(sections[i], smartPlaylist, i);
			sections[i].label->setText(smartPlaylist->text(i));
			sections[i].stringValidator->setStringConverter(stringConverter);
		}

		cbShuffle->setVisible(smartPlaylist->isRandomizable());
	}
}

struct MinMaxIntegerDialog::Private
{
	SmartPlaylists::Type type;
	QLabel* labelDescription {new QLabel()};
	QWidget* sectionWidget {new QWidget()};
	QCheckBox* cbShuffle {new QCheckBox(Lang::get(Lang::ShufflePlaylist))};
	QComboBox* comboType;
	QComboBox* comboLibraries {new QComboBox()};

	QList<Section> sections;
	QDialogButtonBox* buttonBox {new QDialogButtonBox({QDialogButtonBox::Ok | QDialogButtonBox::Cancel})};

	explicit Private(const SmartPlaylists::Type type) :
		type {type},
		comboType {createTypeCombobox(type)}
	{
		auto* gridLayout = createGridLayout();

		sections = createSections(type, sectionWidget, gridLayout);
		sectionWidget->setLayout(gridLayout);
	}
};

MinMaxIntegerDialog::MinMaxIntegerDialog(const std::shared_ptr<SmartPlaylist>& smartPlaylist,
                                         Library::InfoAccessor* libraryManager, const EditMode editMode,
                                         QWidget* parent) :
	QDialog(parent),
	m {Pimpl::make<Private>(smartPlaylist->type())}
{
	setWindowTitle(Lang::get(Lang::SmartPlaylists));
	setLayout(new QVBoxLayout());
	setMinimumWidth(Gui::Util::textWidth(this, "Hallo") * 12); // NOLINT(readability-magic-numbers)

	connectTextFieldChanges(m->sections);
	connect(m->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(m->comboType, combo_activated_int, this, &MinMaxIntegerDialog::currentIndexChanged);

	populate(m->type, m->labelDescription, m->cbShuffle, m->sections);
	populateLibraryComboBox(libraryManager, m->comboLibraries);

	fillLayout(libraryManager->count());

	m->comboType->setEnabled(editMode == EditMode::New);
	m->cbShuffle->setChecked(smartPlaylist->isRandomized());
	m->cbShuffle->setVisible(smartPlaylist->isRandomizable());
	m->comboLibraries->setCurrentIndex(m->comboLibraries->findData(smartPlaylist->libraryId()));
}

MinMaxIntegerDialog::MinMaxIntegerDialog(Library::InfoAccessor* libraryManager, QWidget* parent) :
	MinMaxIntegerDialog(createDummySmartPlaylist(SmartPlaylists::Type::Rating), libraryManager, EditMode::New,
	                    parent) {}

MinMaxIntegerDialog::MinMaxIntegerDialog(const std::shared_ptr<SmartPlaylist>& smartPlaylist,
                                         Library::InfoAccessor* libraryManager, QWidget* parent) :
	MinMaxIntegerDialog(smartPlaylist, libraryManager, EditMode::Edit, parent)
{
	for(int i = 0; i < smartPlaylist->count(); i++)
	{
		fillText(m->sections[i], smartPlaylist, i);
	}
}

MinMaxIntegerDialog::~MinMaxIntegerDialog() = default;

QWidget* createLabel(const QString& l, QWidget* parent)
{
	auto* label = new QLabel(l, parent);
	label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	return label;
}

void MinMaxIntegerDialog::fillLayout(const int libraryCount)
{
	auto* gridWidget = new QWidget(this);
	auto* gridLayout = createGridLayout();

	gridLayout->addWidget(createLabel(tr("Category"), this), 0, 0);
	gridLayout->addWidget(m->comboType, 0, 1);
	if(libraryCount > 1)
	{
		gridLayout->addWidget(createLabel(Lang::get(Lang::Library), this), 1, 0);
		gridLayout->addWidget(m->comboLibraries, 1, 1);
	}
	gridWidget->setLayout(gridLayout);

	layout()->addWidget(gridWidget);
	layout()->addWidget(createLine(this));
	layout()->addWidget(m->labelDescription);
	layout()->addWidget(m->sectionWidget);
	layout()->addWidget(m->cbShuffle);
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

	auto* gridLayout = dynamic_cast<QGridLayout*>(m->sectionWidget->layout());
	m->sections = createSections(m->type, m->sectionWidget, gridLayout);
	connectTextFieldChanges(m->sections);

	populate(m->type, m->labelDescription, m->cbShuffle, m->sections);
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

bool MinMaxIntegerDialog::isRandomized() const { return m->cbShuffle->isChecked(); }

SmartPlaylists::Type MinMaxIntegerDialog::type() const { return m->type; }

LibraryId MinMaxIntegerDialog::libraryId() const
{
	const auto libraryId = m->comboLibraries->currentData().toInt();
	return static_cast<LibraryId>(libraryId);
}