/* TagFromPath.cpp */

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

#include "GUI_TagFromPath.h"

#include "Gui/TagEdit/ui_GUI_TagFromPath.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Message/Message.h"
#include "Utils/Language/Language.h"

#include <QDesktopServices>
#include <QMap>

using namespace Tagging;

struct GUI_TagFromPath::Private
{
	QString currentFilepath;
	QMap<TagName, ReplacedString> tagReplaceStringMap;
};

GUI_TagFromPath::GUI_TagFromPath(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::GUI_TagFromPath();
	ui->setupUi(this);

	showErrorFrame(false);

	connect(ui->btnApplyTag, &QPushButton::clicked, this, [&]() {
		clearInvalidFilepaths();
		emit sigApply();
	});

	connect(ui->btnApplyTagAll, &QPushButton::clicked, this, [&]() {
		clearInvalidFilepaths();
		emit sigApplyAll();
	});

	connect(ui->leTag, &QLineEdit::textChanged, this, &GUI_TagFromPath::tagTextChanged);
	connect(ui->btnTagHelp, &QPushButton::clicked, this, &GUI_TagFromPath::btnTagHelpClicked);

	initButtons();
	languageChanged();
}

GUI_TagFromPath::~GUI_TagFromPath() = default;

void GUI_TagFromPath::initButtons()
{
	connect(ui->btnTitle, &QPushButton::toggled, this, [&](const auto b) {
		btnChecked(ui->btnTitle, b, TagTitle);
	});

	connect(ui->btnArtist, &QPushButton::toggled, this, [&](const auto b) {
		btnChecked(ui->btnArtist, b, TagArtist);
	});

	connect(ui->btnAlbum, &QPushButton::toggled, this, [&](const auto b) {
		btnChecked(ui->btnAlbum, b, TagAlbum);
	});

	connect(ui->btnTrackNumber, &QPushButton::toggled, this, [&](const auto b) {
		btnChecked(ui->btnTrackNumber, b, TagTrackNum);
	});

	connect(ui->btnYear, &QPushButton::toggled, this, [&](const auto b) {
		btnChecked(ui->btnYear, b, TagYear);
	});

	connect(ui->btnDiscNumber, &QPushButton::toggled, this, [&](const auto b) {
		btnChecked(ui->btnDiscNumber, b, TagDisc);
	});
}

bool GUI_TagFromPath::checkIfAnyButtonIsChecked() const
{
	return (ui->btnAlbum->isChecked() ||
	        ui->btnArtist->isChecked() ||
	        ui->btnTitle->isChecked() ||
	        ui->btnYear->isChecked() ||
	        ui->btnDiscNumber->isChecked() ||
	        ui->btnTrackNumber->isChecked());
}

void GUI_TagFromPath::setFilepath(const QString& filepath)
{
	m->currentFilepath = filepath;

	if(ui->leTag->text().isEmpty() || !checkIfAnyButtonIsChecked())
	{
		ui->leTag->setText(filepath);
	}

	const auto expression = Tagging::Expression(ui->leTag->text(), filepath);
	setTagColors(expression.isValid());

	const auto tagType = Tagging::getTagType(filepath);
	const auto tagTypeString = Tagging::tagTypeToString(tagType);

	ui->labTagType->setText(tr("Tag") + ": " + tagTypeString);
}

void GUI_TagFromPath::reset()
{
	ui->leTag->clear();
	ui->leTag->setEnabled(true);
	ui->lvInvalidFilepaths->clear();

	ui->btnAlbum->setChecked(false);
	ui->btnArtist->setChecked(false);
	ui->btnTitle->setChecked(false);
	ui->btnYear->setChecked(false);
	ui->btnDiscNumber->setChecked(false);
	ui->btnTrackNumber->setChecked(false);
}

void GUI_TagFromPath::setTagColors(bool valid)
{
	const auto stylesheet = (valid)
	                        ? QStringLiteral("font-family: mono; font-size: 120%;")
	                        : QStringLiteral("font-family: mono; font-size: 120%; color: red;");

	ui->leTag->setStyleSheet(stylesheet);
	ui->btnApplyTag->setEnabled(valid);
	ui->btnApplyTagAll->setEnabled(valid);
}

void GUI_TagFromPath::tagTextChanged(const QString& tagString)
{
	const auto expression = Tagging::Expression(tagString, m->currentFilepath);
	setTagColors(expression.isValid());
}

void GUI_TagFromPath::clearInvalidFilepaths()
{
	showErrorFrame(false);
	ui->lvInvalidFilepaths->clear();
}

void GUI_TagFromPath::addInvalidFilepath(const QString& filepath)
{
	showErrorFrame(true);
	ui->lvInvalidFilepaths->addItem(filepath);
}

bool GUI_TagFromPath::replaceSelectedTagText(TagName tagName, bool buttonChecked)
{
	const auto textSelection = ui->leTag->textSelection();

	if(buttonChecked && (textSelection.selectionStart < 0))
	{
		Message::info(tr("Please select text first"));
		return false;
	}

	auto lineEditText = ui->leTag->text();
	const auto tagString = Tagging::tagNameToString(tagName);

	if(buttonChecked)
	{
		const auto selectedText = lineEditText.mid(textSelection.selectionStart, textSelection.selectionSize);

		lineEditText.replace(textSelection.selectionStart, textSelection.selectionSize, tagString);
		m->tagReplaceStringMap[tagName] = selectedText;

		ui->leTag->setText(lineEditText);
	}

	else
	{
		lineEditText.replace(tagString, m->tagReplaceStringMap[tagName]);
		m->tagReplaceStringMap.remove(tagName);

		ui->leTag->setText(lineEditText);
	}

	const auto expression = Tagging::Expression(lineEditText, m->currentFilepath);
	setTagColors(expression.isValid());

	return true;
}

void GUI_TagFromPath::btnChecked(QPushButton* btn, bool b, TagName tagName)
{
	ui->labTagFromPathWarning->setVisible(false);
	ui->lvInvalidFilepaths->setVisible(false);

	if(!replaceSelectedTagText(tagName, b))
	{
		btn->setChecked(false);
	}
}

void GUI_TagFromPath::showErrorFrame(bool b)
{
	ui->swTagFromPath->setCurrentIndex(b ? 0 : 1);
}

void GUI_TagFromPath::btnTagHelpClicked()
{
	const auto url = QUrl(QStringLiteral("https://sayonara-player.com/faq.php#tag-edit"));
	QDesktopServices::openUrl(url);
}

void GUI_TagFromPath::languageChanged()
{
	ui->retranslateUi(this);

	ui->btnTitle->setText(Lang::get(Lang::Title));
	ui->btnAlbum->setText(Lang::get(Lang::Album));
	ui->btnArtist->setText(Lang::get(Lang::Artist));
	ui->btnYear->setText(Lang::get(Lang::Year));
	ui->btnTrackNumber->setText(Lang::get(Lang::TrackNo).toFirstUpper());
	ui->labTagFromPathWarning->setText(Lang::get(Lang::Warning));

	ui->btnApplyTagAll->setText(Lang::get(Lang::Apply) + ": " + Lang::get(Lang::All).toFirstUpper());
	ui->btnApplyTag->setText(Lang::get(Lang::Apply) + ": " + Lang::get(Lang::Title).toFirstUpper());
}

QString GUI_TagFromPath::getRegexString() const
{
	return ui->leTag->text();
}
