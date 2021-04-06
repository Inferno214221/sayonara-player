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
#include "GUI_TagEdit.h"
#include "Gui/TagEdit/ui_GUI_CoverEdit.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Tagging/Editor.h"

#include "Utils/globals.h"
#include "Utils/Language/Language.h"
#include "Utils/Tagging/TaggingCover.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QMap>
#include <QPixmap>

using namespace Tagging;
using CoverPathMap = QMap<int, QPixmap>;

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
	CoverPathMap indexCoverMap;
	Editor* tagEdit;
	int currentIndex;

	Private(Editor* editor) :
		tagEdit(editor),
		currentIndex(0) {}
};

GUI_CoverEdit::GUI_CoverEdit(GUI_TagEdit* parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>(parent->editor());

	ui = new Ui::GUI_CoverEdit();
	ui->setupUi(this);

	const auto style =
		QString("min-width: %1ex; min-height: %1ex; max-width: %1ex; max-height: %1ex;").arg(20);

	ui->labCoverOriginal->setStyleSheet(style);

	ui->btnCoverReplacement->setSilent(true);
	ui->btnCoverReplacement->setStyleSheet(style);

	connect(m->tagEdit, &Editor::sigMetadataReceived, this, &GUI_CoverEdit::setMetadata);
	connect(ui->cbCoverAll, &QCheckBox::toggled, this, &GUI_CoverEdit::coverAllToggled);
	connect(ui->btnSearch, &QPushButton::clicked, ui->btnCoverReplacement, &Gui::CoverButton::trigger);
	connect(ui->cbReplace, &QPushButton::toggled, this, &GUI_CoverEdit::replaceToggled);
	connect(ui->btnCoverReplacement, &Gui::CoverButton::sigCoverChanged, this, &GUI_CoverEdit::coverChanged);

	languageChanged();

	reset();
}

GUI_CoverEdit::~GUI_CoverEdit() = default;

void GUI_CoverEdit::reset()
{
	ui->cbCoverAll->setChecked(false);
	showReplacementField(false);

	ui->btnCoverReplacement->setCoverLocation(Cover::Location());

	m->indexCoverMap.clear();
}

void GUI_CoverEdit::setMetadata([[maybe_unused]] const MetaDataList& tracks)
{
	refreshCurrentTrack();
	refreshAllCheckboxText(ui->cbCoverAll, tracks.count());
}

void GUI_CoverEdit::setCurrentIndex(int index)
{
	m->currentIndex = index;
}

QPixmap GUI_CoverEdit::selectedCover(int index) const
{
	if(!isCoverReplacementActive())
	{
		return QPixmap();
	}

	return (ui->cbCoverAll->isChecked())
	       ? m->indexCoverMap[m->currentIndex]
	       : m->indexCoverMap[index];
}

void GUI_CoverEdit::refreshCurrentTrack()
{
	if(!Util::between(m->currentIndex, m->tagEdit->count()))
	{
		return;
	}

	const auto track = m->tagEdit->metadata(m->currentIndex);
	setCover(track);

	if(!ui->cbCoverAll->isChecked())
	{
		bool has_replacement = m->tagEdit->hasCoverReplacement(m->currentIndex);
		ui->btnCoverReplacement->setChecked(has_replacement);
	}
}

void GUI_CoverEdit::showReplacementField(bool b)
{
	ui->btnCoverReplacement->setEnabled(b);
	ui->btnSearch->setEnabled(b);
	ui->cbCoverAll->setEnabled(b);
}

void GUI_CoverEdit::setCover(const MetaData& track)
{
	const auto size = QSize(this->height() / 2, this->height() / 2);
	ui->labCoverOriginal->setFixedSize(size);
	ui->btnCoverReplacement->setFixedSize(size);

	const auto hasCover = Tagging::Covers::hasCover(track.filepath());
	if(!hasCover)
	{
		ui->labCoverOriginal->clear();
		ui->labCoverOriginal->setText(tr("File has no cover"));
	}

	else
	{
		const auto pixmap = Tagging::Covers::extractCover(track.filepath());
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

	const auto coverLocation = Cover::Location::coverLocation(track);
	const auto isReplacementActive = coverLocation.isValid() && ui->cbReplace->isChecked();

	ui->btnCoverReplacement->setCoverLocation(coverLocation);
	ui->btnCoverReplacement->setEnabled(isReplacementActive && !ui->cbCoverAll->isChecked());
	ui->cbCoverAll->setEnabled(isReplacementActive);
}

void GUI_CoverEdit::replaceToggled(bool b)
{
	showReplacementField(b);
}

void GUI_CoverEdit::coverAllToggled(bool b)
{
	if(!b && Util::between(m->currentIndex, m->tagEdit->count()))
	{
		setCover(m->tagEdit->metadata(m->currentIndex));
	}

	ui->btnCoverReplacement->setEnabled(!b);
	ui->btnSearch->setEnabled(!b);
}

bool GUI_CoverEdit::isCoverReplacementActive() const
{
	return (ui->cbReplace->isChecked());
}

void GUI_CoverEdit::languageChanged()
{
	refreshAllCheckboxText(ui->cbCoverAll, m->tagEdit->count());
	ui->labOriginal->setText(tr("Original"));
	ui->btnSearch->setText(Lang::get(Lang::SearchVerb));
	ui->cbReplace->setText(Lang::get(Lang::Replace));
}

void GUI_CoverEdit::coverChanged()
{
	const auto pixmap = ui->btnCoverReplacement->pixmap();
	m->indexCoverMap[m->currentIndex] = pixmap;
}
