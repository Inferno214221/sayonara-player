/* GUI_CoverEdit.cpp */

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

#include "GUI_CoverEdit.h"
#include "Gui/TagEdit/ui_GUI_CoverEdit.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Tagging/Editor.h"
#include "Components/Tagging/CoverEditor.h"

#include "Utils/Language/Language.h"

#include <QMap>
#include <QPixmap>

namespace
{
	void refreshAllCheckboxText(QCheckBox* checkbox, int count)
	{
		const auto text = QString("%1 (%2 %3)")
			.arg(Lang::get(Lang::All))
			.arg(count)
			.arg(Lang::get(Lang::Tracks));

		checkbox->setText(text);
	}
}

struct GUI_CoverEdit::Private
{
	Tagging::CoverEditor* coverEditor;

	Private(Tagging::Editor* tagEditor, QObject* parent) :
		coverEditor(new Tagging::CoverEditor(tagEditor, parent)) {}
};

GUI_CoverEdit::GUI_CoverEdit(Tagging::Editor* tagEditor, QWidget* parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>(tagEditor, this);

	ui = new Ui::GUI_CoverEdit();
	ui->setupUi(this);

	const auto style =
		QString("min-width: %1ex; min-height: %1ex; max-width: %1ex; max-height: %1ex;").arg(20);

	ui->labCoverOriginal->setStyleSheet(style);
	ui->btnCoverReplacement->setSilent(true);
	ui->btnCoverReplacement->setStyleSheet(style);

	connect(ui->cbCoverAll, &QCheckBox::toggled, this, &GUI_CoverEdit::btnAllToggled);
	connect(ui->btnSearch, &QPushButton::clicked, ui->btnCoverReplacement, &Gui::CoverButton::trigger);
	connect(ui->cbReplace, &QPushButton::toggled, this, &GUI_CoverEdit::replaceToggled);
	connect(ui->btnCoverReplacement, &Gui::CoverButton::sigCoverChanged, this, &GUI_CoverEdit::coverChanged);

	languageChanged();

	reset();
}

GUI_CoverEdit::~GUI_CoverEdit() = default;

void GUI_CoverEdit::reset()
{
	ui->cbCoverAll->setEnabled(false);
	ui->cbCoverAll->setChecked(false);
	ui->btnCoverReplacement->setEnabled(true);
	ui->btnSearch->setEnabled(true);

	m->coverEditor->reset();
}

void GUI_CoverEdit::updateTrack(const int index)
{
	m->coverEditor->updateTrack(index);
}

void GUI_CoverEdit::setCurrentIndex(const int index)
{
	m->coverEditor->setCurrentIndex(index);
}

void GUI_CoverEdit::refreshCurrentTrack()
{
	if(isVisible())
	{
		refreshOriginalCover();
		refreshReplacementCover();
	}
}

void GUI_CoverEdit::refreshOriginalCover()
{
	const auto size = QSize(height() / 2, height() / 2);
	ui->labCoverOriginal->setFixedSize(size);

	const auto pixmap = m->coverEditor->currentOriginalCover();
	if(pixmap.isNull())
	{
		ui->labCoverOriginal->clear();
		ui->labCoverOriginal->setText(tr("File has no cover"));
	}

	else
	{
		const auto pixmapScaled = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		ui->labCoverOriginal->setPixmap(pixmapScaled);
		ui->labCoverOriginal->setText(QString());
	}
}

void GUI_CoverEdit::refreshReplacementCover()
{
	const auto coverLocation = m->coverEditor->currentCoverLocation();
	const auto size = QSize(height() / 2, height() / 2);
	ui->btnCoverReplacement->setFixedSize(size);

	if(coverLocation.isValid())
	{
		const auto currentReplacementCover = m->coverEditor->currentReplacementCover();
		ui->cbReplace->setChecked(!currentReplacementCover.isNull());
		ui->btnCoverReplacement->setCoverLocation(coverLocation);
	}

	else
	{
		ui->cbCoverAll->setEnabled(false);
		ui->cbReplace->setEnabled(false);
		ui->cbReplace->setChecked(false);
	}
}

void GUI_CoverEdit::coverChanged()
{
	const auto currentReplacementCover = m->coverEditor->currentReplacementCover();
	const auto hasReplacementCover = !currentReplacementCover.isNull();
	const auto isCoverForAllAvailable = m->coverEditor->isCoverForAllAvailable();
	const auto pixmap = hasReplacementCover
	                    ? currentReplacementCover
	                    : ui->btnCoverReplacement->pixmap();

	const auto isValidPixmap = !pixmap.isNull();

	ui->cbReplace->setChecked(hasReplacementCover);
	ui->cbCoverAll->setEnabled(isValidPixmap);

	ui->btnSearch->setEnabled(!isCoverForAllAvailable);
	ui->btnCoverReplacement->setEnabled(!isCoverForAllAvailable);
	ui->cbReplace->setEnabled(!isCoverForAllAvailable);
}

void GUI_CoverEdit::replaceToggled(bool b)
{
	const auto pixmap = b
	                    ? ui->btnCoverReplacement->pixmap()
	                    : QPixmap();

	m->coverEditor->replaceCurrentCover(pixmap);
}

void GUI_CoverEdit::btnAllToggled(bool b)
{
	const auto pixmap = b
	                    ? ui->btnCoverReplacement->pixmap()
	                    : QPixmap();

	m->coverEditor->replaceCoverForAll(pixmap);

	ui->btnCoverReplacement->setEnabled(!b);
	ui->btnSearch->setEnabled(!b);
	ui->cbReplace->setEnabled(!b);

	if(!b)
	{
		auto currentReplacementCover = m->coverEditor->currentReplacementCover();
		ui->cbReplace->setChecked(!currentReplacementCover.isNull());
	}

	else
	{
		ui->cbReplace->setChecked(true);
	}
}

void GUI_CoverEdit::showEvent(QShowEvent* event)
{
	Gui::Widget::showEvent(event);
	refreshCurrentTrack();
}

void GUI_CoverEdit::languageChanged()
{
	refreshAllCheckboxText(ui->cbCoverAll, m->coverEditor->count());

	ui->labOriginal->setText(tr("Original"));
	ui->btnSearch->setText(Lang::get(Lang::SearchVerb));
	ui->cbReplace->setText(Lang::get(Lang::Replace));
}
