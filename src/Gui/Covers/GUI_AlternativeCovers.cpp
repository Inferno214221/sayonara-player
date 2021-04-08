/* GUI_AlternativeCovers.cpp */

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


/*
 * GUI_AlternativeCovers.cpp
 *
 *  Created on: Jul 1, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "GUI_AlternativeCovers.h"
#include "Gui/Covers/ui_GUI_AlternativeCovers.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookupAlternative.h"
#include "Components/Covers/CoverFetchManager.h"

#include "Gui/Utils/Widgets/ProgressBar.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/ImageSelectionDialog.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QStringList>
#include <QTimer>

using Cover::AlternativeLookup;
using Cover::Location;
using Gui::ProgressBar;

namespace
{
	QPixmap getPixmapFromListWidgetItem(QListWidgetItem* item)
	{
		QPixmap pm;

		if(item != nullptr)
		{
			const auto icon = item->icon();
			const auto sizes = icon.availableSizes();
			const auto itMaxSize = std::max_element(sizes.begin(), sizes.end(), [](const auto& s1, const auto& s2){
				return (s1.width() * s1.height()) < (s2.width() * s2.height());
			});

			if(!sizes.isEmpty())
			{
				pm = icon.pixmap(*itMaxSize);
			}
		}

		return pm;
	}

	void setButtonEnabled(QDialogButtonBox* buttonBox, QDialogButtonBox::StandardButton standardButton, bool enable)
	{
		auto* button = buttonBox->button(standardButton);
		if(button)
		{
			button->setEnabled(enable);
		}
	}
}

struct GUI_AlternativeCovers::Private
{
	AlternativeLookup* alternativeLookup;
	ProgressBar* loadingBar = nullptr;

	Private(const Cover::Location& coverLocation, bool silent, QObject* parent) :
		alternativeLookup{new AlternativeLookup(coverLocation, 20, silent, parent)}
	{}

	~Private()
	{
		if(alternativeLookup)
		{
			alternativeLookup->stop();
			alternativeLookup->reset();
		}
	}

};

GUI_AlternativeCovers::GUI_AlternativeCovers(const Cover::Location& cl, bool silent, QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<GUI_AlternativeCovers::Private>(cl, silent, this);

	connect(m->alternativeLookup,
	        &AlternativeLookup::sigCoverfetchersChanged,
	        this,
	        &GUI_AlternativeCovers::reloadCombobox);
}

GUI_AlternativeCovers::~GUI_AlternativeCovers()
{
	reset();

	delete ui;
	ui = nullptr;
}

void GUI_AlternativeCovers::initUi()
{
	if(!ui)
	{
		ui = new Ui::GUI_AlternativeCovers();
		ui->setupUi(this);

		ui->tvImages->setWrapping(true);
		ui->tvImages->setResizeMode(QListView::Adjust);
		ui->tvImages->setIconSize({150, 150});

		{ // create members
			m->loadingBar = new ProgressBar(ui->tvImages);
		}

		{ // add preference button
			auto* cpa = new Gui::CoverPreferenceAction(this);
			QPushButton* prefButton = cpa->createButton(this);
			prefButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
			ui->horizontalLayout->insertWidget(2, prefButton);
		}

		ui->cbAutostart->setChecked(GetSetting(Set::Cover_StartSearch));

		const auto isSilent = m->alternativeLookup->isSilent();
		ui->cbSaveToLibrary->setChecked(GetSetting(Set::Cover_SaveToLibrary) && !isSilent);
		ui->cbSaveToLibrary->setEnabled(!isSilent);

		auto* btnOk = ui->buttonBox->button(QDialogButtonBox::Ok);
		auto* btnApply = ui->buttonBox->button(QDialogButtonBox::Apply);
		auto* btnCancel = ui->buttonBox->button(QDialogButtonBox::Cancel);

		connect(btnOk, &QPushButton::clicked, this, &GUI_AlternativeCovers::okClicked);
		connect(btnApply, &QPushButton::clicked, this, &GUI_AlternativeCovers::applyClicked);
		connect(btnCancel, &QPushButton::clicked, this, &Dialog::close);

		connect(ui->btnSearch, &QPushButton::clicked, this, &GUI_AlternativeCovers::start);
		connect(ui->btnStopSearch, &QPushButton::clicked, this, &GUI_AlternativeCovers::stop);
		connect(ui->tvImages, &QListWidget::pressed, this, &GUI_AlternativeCovers::coverPressed);
		connect(ui->btnFile, &QPushButton::clicked, this, &GUI_AlternativeCovers::openFileDialog);
		connect(ui->cbAutostart, &QCheckBox::toggled, this, &GUI_AlternativeCovers::autostartToggled);
		connect(ui->leSearch, &QLineEdit::textChanged, this, &GUI_AlternativeCovers::searchTextEdited);

		connect(ui->rbAutosearch, &QRadioButton::toggled, this, &GUI_AlternativeCovers::rbAutosearchToggled);
		connect(ui->rbTextsearch, &QRadioButton::toggled, this, &GUI_AlternativeCovers::rbAutosearchToggled);

		connect(m->alternativeLookup,
		        &AlternativeLookup::sigCoverChanged,
		        this,
		        &GUI_AlternativeCovers::sigCoverChanged);
		connect(m->alternativeLookup, &AlternativeLookup::sigCoverFound, this, &GUI_AlternativeCovers::coverFound);
		connect(m->alternativeLookup,
		        &AlternativeLookup::sigFinished,
		        this,
		        &GUI_AlternativeCovers::coverLookupFinished);
		connect(m->alternativeLookup, &AlternativeLookup::sigStarted, this, &GUI_AlternativeCovers::coverLookupStarted);

		ListenSettingNoCall(Set::Cover_Server, GUI_AlternativeCovers::coverServersChanged);
		ListenSetting(Set::Cover_FetchFromWWW, GUI_AlternativeCovers::wwwActiveChanged);
	}

	const auto coverLocation = m->alternativeLookup->coverLocation();
	m->loadingBar->hide();
	ui->tabWidget->setCurrentIndex(0);
	ui->labStatus->clear();
	ui->rbAutosearch->setChecked(true);
	ui->leSearch->setText(coverLocation.searchTerm());

	initSaveToLibrary();
	reloadCombobox();
}

void GUI_AlternativeCovers::setCoverLocation(const Cover::Location& cl)
{
	m->alternativeLookup->setCoverLocation(cl);
}

void GUI_AlternativeCovers::start()
{
	this->reset();

	if(ui->rbTextsearch->isChecked())
	{
		const auto searchTerm = ui->leSearch->text();
		if(ui->comboSearchFetchers->currentIndex() > 0)
		{
			const auto identifier = ui->comboSearchFetchers->currentText();
			m->alternativeLookup->startTextSearch(searchTerm, identifier);
		}

		else
		{
			m->alternativeLookup->startTextSearch(searchTerm);
		}
	}

	else if(ui->rbAutosearch->isChecked())
	{
		if(ui->comboSearchFetchers->currentIndex() > 0)
		{
			const auto identifier = ui->comboSearchFetchers->currentText();
			m->alternativeLookup->start(identifier);
		}

		else
		{
			m->alternativeLookup->start();
		}
	}

	this->show();
}

void GUI_AlternativeCovers::stop()
{
	m->alternativeLookup->stop();
}

void GUI_AlternativeCovers::okClicked()
{
	applyClicked();
	close();
}

void GUI_AlternativeCovers::applyClicked()
{
	const auto pixmap = getPixmapFromListWidgetItem(ui->tvImages->currentItem());
	if(!pixmap.isNull())
	{
		m->alternativeLookup->save(pixmap, ui->cbSaveToLibrary->isChecked());
	}
}

void GUI_AlternativeCovers::coverLookupStarted()
{
	if(m->loadingBar)
	{
		QTimer::singleShot(200, this, &GUI_AlternativeCovers::readyForProgressbar);
	}

	ui->btnSearch->setVisible(false);
	ui->btnStopSearch->setVisible(true);
}

void GUI_AlternativeCovers::readyForProgressbar()
{
	if(m->alternativeLookup->isRunning())
	{
		m->loadingBar->show();
		m->loadingBar->refresh();
	}
}

void GUI_AlternativeCovers::coverLookupFinished([[maybe_unused]] bool success)
{
	m->loadingBar->hide();

	ui->btnSearch->setVisible(true);
	ui->btnStopSearch->setVisible(false);
}

void GUI_AlternativeCovers::coverFound(const QPixmap& pm)
{
	auto* listWidgetItem = new QListWidgetItem(ui->tvImages);
	listWidgetItem->setIcon(QIcon(pm));
	listWidgetItem->setText(QString("%1x%2").arg(pm.width()).arg(pm.height()));

	ui->tvImages->addItem(listWidgetItem);

	setButtonEnabled(ui->buttonBox, QDialogButtonBox::Ok, true);
	setButtonEnabled(ui->buttonBox, QDialogButtonBox::Apply, true);

	const auto text = tr("%n cover(s) found", "", ui->tvImages->count());
	ui->labStatus->setText(text);
}

void GUI_AlternativeCovers::coverServersChanged()
{
	reloadCombobox();
}

void GUI_AlternativeCovers::autostartToggled(bool b)
{
	SetSetting(Set::Cover_StartSearch, b);
}

void GUI_AlternativeCovers::rbAutosearchToggled(bool b)
{
	const auto isTextSearch = ui->rbTextsearch->isChecked();
	ui->leSearch->setEnabled(isTextSearch);
	ui->btnSearch->setEnabled(!isTextSearch);
	ui->comboSearchFetchers->setEnabled(!isTextSearch);

	if(isTextSearch)
	{
		searchTextEdited(ui->leSearch->text());
	}

	if(b)
	{
		reloadCombobox();
	}
}

void GUI_AlternativeCovers::wwwActiveChanged()
{
	const auto isActive = GetSetting(Set::Cover_FetchFromWWW);

	ui->labWebsearchDisabled->setVisible(!isActive);
	ui->btnSearch->setVisible(isActive);

	ui->comboSearchFetchers->setEnabled(isActive);
	ui->rbAutosearch->setEnabled(isActive);
	ui->rbTextsearch->setEnabled(isActive);
}

void GUI_AlternativeCovers::searchTextEdited(const QString& text)
{
	if(ui->rbTextsearch->isChecked())
	{
		ui->btnSearch->setEnabled(text.size() > 0);

		const auto isDirectUrl = Cover::Fetcher::Manager::isSearchstringWebsite(text);
		ui->comboSearchFetchers->setDisabled(isDirectUrl);
	}
}

void GUI_AlternativeCovers::coverPressed(const QModelIndex& idx)
{
	const auto pixmap = getPixmapFromListWidgetItem(ui->tvImages->currentItem());
	const auto isValid = (idx.isValid() && !(pixmap.isNull()));

	setButtonEnabled(ui->buttonBox, QDialogButtonBox::Ok, isValid);
	setButtonEnabled(ui->buttonBox, QDialogButtonBox::Apply, isValid);
}

void GUI_AlternativeCovers::reset()
{
	if(ui)
	{
		setButtonEnabled(ui->buttonBox, QDialogButtonBox::Ok, false);
		setButtonEnabled(ui->buttonBox, QDialogButtonBox::Apply, false);

		ui->btnSearch->setVisible(false);
		ui->btnStopSearch->setVisible(true);
		ui->labStatus->clear();

		ui->tvImages->clear();
		m->loadingBar->hide();
	}

	m->alternativeLookup->reset();
}

void GUI_AlternativeCovers::openFileDialog()
{
	const auto coverLocation = m->alternativeLookup->coverLocation();
	const auto dir = (!coverLocation.localPathDir().isEmpty())
	                 ? coverLocation.localPathDir()
	                 : QDir::homePath();

	auto* dialog = new Gui::ImageSelectionDialog(dir, this);
	if(dialog->exec())
	{
		const auto selectedFiles = dialog->selectedFiles();

		if(!selectedFiles.isEmpty())
		{
			reset();

			for(const auto& selectedFile : selectedFiles)
			{
				auto* widgetListItem = new QListWidgetItem(ui->tvImages);
				QPixmap pm(selectedFile);

				widgetListItem->setIcon(QIcon(pm));
				widgetListItem->setText(QString("%1x%2").arg(pm.width()).arg(pm.height()));
				ui->tvImages->addItem(widgetListItem);
			}
		}
	}

	dialog->deleteLater();
}

void GUI_AlternativeCovers::reloadCombobox()
{
	const auto fulltextSearch = ui->rbTextsearch->isChecked();
	const auto searchMode = (fulltextSearch)
	                        ? AlternativeLookup::SearchMode::Fulltext
	                        : AlternativeLookup::SearchMode::Default;

	ui->comboSearchFetchers->clear();
	ui->comboSearchFetchers->addItem(Lang::get(Lang::All));

	const auto coverfetchers = m->alternativeLookup->activeCoverfetchers(searchMode);
	for(const auto& coverfetcher : coverfetchers)
	{
		ui->comboSearchFetchers->addItem(coverfetcher);
	}
}

void GUI_AlternativeCovers::initSaveToLibrary()
{
	const auto cl = m->alternativeLookup->coverLocation();

	const auto text = tr("Also save cover to %1").arg(cl.localPathDir());
	const auto fontMetrics = this->fontMetrics();

	ui->cbSaveToLibrary->setText(
		fontMetrics.elidedText(text, Qt::ElideRight, this->width() - 50)
	);

	ui->cbSaveToLibrary->setToolTip(cl.localPathDir());
	ui->cbSaveToLibrary->setChecked(GetSetting(Set::Cover_SaveToLibrary));
	ui->cbSaveToLibrary->setVisible(!cl.localPathDir().isEmpty());
}

void GUI_AlternativeCovers::languageChanged()
{
	ui->retranslateUi(this);

	ui->labWebsearchDisabled->setText(tr("Cover web search is not enabled"));

	initSaveToLibrary();

	ui->btnSearch->setText(Lang::get(Lang::SearchVerb));
	ui->btnStopSearch->setText(Lang::get(Lang::Stop));

	ui->btnSearch->setVisible(!m->alternativeLookup->isRunning());
	ui->btnStopSearch->setVisible(m->alternativeLookup->isRunning());

	const auto coverLocation = m->alternativeLookup->coverLocation();
	const auto text = tr("Also save cover to %1").arg(coverLocation.localPathDir());

	ui->cbSaveToLibrary->setText(text);
}

void GUI_AlternativeCovers::showEvent(QShowEvent* e)
{
	const auto coverSize = GetSetting(Set::AlternativeCovers_Size);

	initUi();
	Gui::Dialog::showEvent(e);

	if(coverSize.isValid())
	{
		QTimer::singleShot(100, this, [coverSize, this]() {
			this->resize(coverSize);
		});
	}

	if(GetSetting(Set::Cover_StartSearch) && GetSetting(Set::Cover_FetchFromWWW))
	{
		start();
	}
}

void GUI_AlternativeCovers::resizeEvent(QResizeEvent* e)
{
	Gui::Dialog::resizeEvent(e);

	if(ui)
	{
		if(ui->cbSaveToLibrary)
		{
			auto* cbSaveToLibrary = ui->cbSaveToLibrary;
			const auto checked = cbSaveToLibrary->isChecked();
			initSaveToLibrary();
			cbSaveToLibrary->setChecked(checked);
		}

		if(isVisible())
		{
			const auto newSize = e->size();
			SetSetting(Set::AlternativeCovers_Size, newSize);
		}
	}

	if(m->loadingBar && m->loadingBar->isVisible())
	{
		m->loadingBar->hide();
		m->loadingBar->show();
		m->loadingBar->refresh();
	}
}
