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
using CoverPathMap=QMap<int, QPixmap>;

struct GUI_CoverEdit::Private
{
	Editor*				tagEdit=nullptr;
	CoverPathMap		indexCoverMap;
	int					currentIndex;

	Private(Editor* editor) :
		tagEdit(editor),
		currentIndex(0)
	{}
};

GUI_CoverEdit::GUI_CoverEdit(GUI_TagEdit* parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>(parent->editor());

	ui = new Ui::GUI_CoverEdit();
	ui->setupUi(this);

	const QString style =
		QString("min-width: %1ex; min-height: %1ex; max-width: %1ex; max-height: %1ex;").arg(20);

	ui->lab_coverOriginal->setStyleSheet(style);

	ui->btn_coverReplacement->setSilent(true);
	ui->btn_coverReplacement->setStyleSheet(style);

	connect(m->tagEdit, &Editor::sigMetadataReceived, this, &GUI_CoverEdit::setMetadata);
	connect(ui->cb_coverAll, &QCheckBox::toggled, this, &GUI_CoverEdit::coverAllToggled);
	connect(ui->btn_search, &QPushButton::clicked, ui->btn_coverReplacement, &Gui::CoverButton::trigger);
	connect(ui->cb_replace, &QPushButton::toggled, this, &GUI_CoverEdit::replaceToggled);
	connect(ui->btn_coverReplacement, &Gui::CoverButton::sigCoverChanged, this, &GUI_CoverEdit::coverChanged);

	languageChanged();

	reset();
}

GUI_CoverEdit::~GUI_CoverEdit() = default;

void GUI_CoverEdit::reset()
{
	ui->cb_coverAll->setChecked(false);
	showReplacementField(false);

	ui->btn_coverReplacement->setCoverLocation(Cover::Location());

	m->indexCoverMap.clear();
}

static void refresh_all_checkbox_text(QCheckBox* cb, int count)
{
	QString text = QString("%1 (%2 %3)")
		.arg(Lang::get(Lang::All))
		.arg(count)
		.arg(Lang::get(Lang::Tracks));

	cb->setText(text);
}

void GUI_CoverEdit::setMetadata(const MetaDataList& v_md)
{
	Q_UNUSED(v_md)

	refreshCurrentTrack();
	refresh_all_checkbox_text(ui->cb_coverAll, v_md.count());
}

void GUI_CoverEdit::setCurrentIndex(int index)
{
	m->currentIndex = index;
}

QPixmap GUI_CoverEdit::selectedCover(int index) const
{
	if(!isCoverReplacementActive()) {
		return QPixmap();
	}

	QPixmap pm;
	if(ui->cb_coverAll->isChecked()) {
		pm = m->indexCoverMap[m->currentIndex];
	}

	else {
		pm = m->indexCoverMap[index];
	}

	return pm;
}

void GUI_CoverEdit::refreshCurrentTrack()
{
	if(m->currentIndex < 0 || m->currentIndex >= m->tagEdit->count()) {
		return;
	}

	MetaData md = m->tagEdit->metadata(m->currentIndex);
	setCover(md);

	if(!ui->cb_coverAll->isChecked())
	{
		bool has_replacement = m->tagEdit->hasCoverReplacement(m->currentIndex);
		ui->btn_coverReplacement->setChecked(has_replacement);
	}
}

void GUI_CoverEdit::showReplacementField(bool b)
{
	ui->btn_coverReplacement->setEnabled(b);
	ui->btn_search->setEnabled(b);
	ui->cb_coverAll->setEnabled(b);
}


void GUI_CoverEdit::setCover(const MetaData& md)
{
	QSize sz(this->height() / 2, this->height() / 2);
	ui->lab_coverOriginal->setFixedSize(sz);
	ui->btn_coverReplacement->setFixedSize(sz);

	bool has_cover = Tagging::Covers::hasCover(md.filepath());

	if(!has_cover)
	{
		ui->lab_coverOriginal->clear();
		ui->lab_coverOriginal->setText(tr("File has no cover"));
	}

	else
	{
		QPixmap pm = Tagging::Covers::extractCover(md.filepath());
		if(pm.isNull())
		{
			ui->lab_coverOriginal->clear();
			ui->lab_coverOriginal->setText(tr("File has no cover"));
		}

		else
		{
			QPixmap pm_scaled = pm.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);

			ui->lab_coverOriginal->setPixmap(pm_scaled);
			ui->lab_coverOriginal->setText(QString());
		}
	}

	Cover::Location cl = Cover::Location::coverLocation(md);

	bool is_replacement_active = cl.isValid() && ui->cb_replace->isChecked();

	ui->btn_coverReplacement->setCoverLocation(cl);
	ui->btn_coverReplacement->setEnabled(is_replacement_active && !ui->cb_coverAll->isChecked());
	ui->cb_coverAll->setEnabled(is_replacement_active);
}

void GUI_CoverEdit::replaceToggled(bool b)
{
	showReplacementField(b);
}

void GUI_CoverEdit::coverAllToggled(bool b)
{
	if(!b)
	{
		if(Util::between(m->currentIndex, m->tagEdit->count()) ) {
			setCover(m->tagEdit->metadata(m->currentIndex));
		}
	}

	ui->btn_coverReplacement->setEnabled(!b);
	ui->btn_search->setEnabled(!b);
}

bool GUI_CoverEdit::isCoverReplacementActive() const
{
	return (ui->cb_replace->isChecked());
}

void GUI_CoverEdit::languageChanged()
{
	refresh_all_checkbox_text(ui->cb_coverAll, m->tagEdit->count());
	ui->lab_original->setText(tr("Original"));
	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	ui->cb_replace->setText(Lang::get(Lang::Replace));
}

void GUI_CoverEdit::coverChanged()
{
	QPixmap pm = ui->btn_coverReplacement->pixmap();
	m->indexCoverMap[m->currentIndex] = pm;
}
