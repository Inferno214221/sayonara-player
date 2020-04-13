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

static QPixmap getPixmapFromListWidgetItem(QListWidgetItem* item)
{
	QPixmap pm;

	if(item != nullptr)
	{
		QIcon icon = item->icon();
		QList<QSize> sizes = icon.availableSizes();

		Util::Algorithm::sort(sizes, [](auto s1, auto s2){
			return (s1.width() * s1.height()) < (s2.width() * s2.height());
		});

		if(!sizes.isEmpty())
		{
			QSize sz = sizes.first();
			pm = icon.pixmap(sz);
		}
	}

	return pm;
}


struct GUI_AlternativeCovers::Private
{
	AlternativeLookup* alternativeLookup=nullptr;
	ProgressBar* loadingBar=nullptr;

	Private(const Cover::Location& cl, bool silent, QObject* parent)
	{
		alternativeLookup = new AlternativeLookup(cl, 20, silent, parent);
	}

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

	connect(m->alternativeLookup, &AlternativeLookup::sigCoverfetchersChanged, this, &GUI_AlternativeCovers::reloadCombobox);
}


GUI_AlternativeCovers::~GUI_AlternativeCovers()
{
	reset();

	delete ui; ui=nullptr;
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
			ui->horizontalLayout->insertWidget(0, prefButton);
		}

		ui->cbAutostart->setChecked(GetSetting(Set::Cover_StartSearch));

		bool is_silent = m->alternativeLookup->isSilent();
		ui->cbSaveToLibrary->setChecked(GetSetting(Set::Cover_SaveToLibrary) && !is_silent);
		ui->cbSaveToLibrary->setEnabled(!is_silent);

		connect(ui->btnOk, &QPushButton::clicked, this, &GUI_AlternativeCovers::okClicked);
		connect(ui->btnApply, &QPushButton::clicked, this, &GUI_AlternativeCovers::applyClicked);
		connect(ui->btnSearch, &QPushButton::clicked, this, &GUI_AlternativeCovers::start);
		connect(ui->btnStopSearch, &QPushButton::clicked, this, &GUI_AlternativeCovers::stop);
		connect(ui->tvImages, &QListWidget::pressed, this, &GUI_AlternativeCovers::coverPressed);
		connect(ui->btnFile, &QPushButton::clicked, this, &GUI_AlternativeCovers::openFileDialog);
		connect(ui->btnClose, &QPushButton::clicked, this, &Dialog::close);
		connect(ui->cbAutostart, &QCheckBox::toggled, this, &GUI_AlternativeCovers::autostartToggled);
		connect(ui->leSearch, &QLineEdit::textChanged, this, &GUI_AlternativeCovers::searchTextEdited);

		connect(ui->rbAutosearch, &QRadioButton::toggled, this, &GUI_AlternativeCovers::rbAutosearchToggled);
		connect(ui->rbTextsearch, &QRadioButton::toggled, this, &GUI_AlternativeCovers::rbAutosearchToggled);

		connect(m->alternativeLookup, &AlternativeLookup::sigCoverChanged, this, &GUI_AlternativeCovers::sigCoverChanged);
		connect(m->alternativeLookup, &AlternativeLookup::sigCoverFound, this, &GUI_AlternativeCovers::coverFound);
		connect(m->alternativeLookup, &AlternativeLookup::sigFinished, this, &GUI_AlternativeCovers::coverLookupFinished);
		connect(m->alternativeLookup, &AlternativeLookup::sigStarted, this, &GUI_AlternativeCovers::coverLookupStarted);

		ListenSettingNoCall(Set::Cover_Server, GUI_AlternativeCovers::coverServersChanged);
		ListenSetting(Set::Cover_FetchFromWWW, GUI_AlternativeCovers::wwwActiveChanged);
	}

	Cover::Location cl = m->alternativeLookup->coverLocation();
	m->loadingBar->hide();
	ui->tabWidget->setCurrentIndex(0);
	ui->labStatus->setText("");
	ui->rbAutosearch->setChecked(true);
	ui->leSearch->setText(cl.searchTerm());

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
		QString searchTerm = ui->leSearch->text();
		if(ui->comboSearchFetchers->currentIndex() > 0)
		{
			QString identifier = ui->comboSearchFetchers->currentText();
			m->alternativeLookup->startTextSearch(searchTerm, identifier);
		}

		else {
			m->alternativeLookup->startTextSearch(searchTerm);
		}
	}

	else if(ui->rbAutosearch->isChecked())
	{
		if(ui->comboSearchFetchers->currentIndex() > 0)
		{
			QString identifier = ui->comboSearchFetchers->currentText();
			m->alternativeLookup->start(identifier);
		}

		else {
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
	QPixmap cover = getPixmapFromListWidgetItem(ui->tvImages->currentItem());
	if(cover.isNull()) {
		return;
	}

	m->alternativeLookup->save(cover, ui->cbSaveToLibrary->isChecked());
}

void GUI_AlternativeCovers::searchClicked()
{
	if( m->alternativeLookup->isRunning() ) {
		m->alternativeLookup->stop();
		return;
	}

	start();
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

void GUI_AlternativeCovers::coverLookupFinished(bool success)
{
	Q_UNUSED(success)

	m->loadingBar->hide();

	ui->btnSearch->setVisible(true);
	ui->btnStopSearch->setVisible(false);
}

void GUI_AlternativeCovers::coverFound(const QPixmap& pm)
{
	QListWidgetItem* item = new QListWidgetItem(ui->tvImages);
	item->setIcon(QIcon(pm));
	item->setText(QString("%1x%2").arg(pm.width()).arg(pm.height()));

	ui->tvImages->addItem(item);

	ui->btnOk->setEnabled(true);
	ui->btnApply->setEnabled(true);

	QString text = tr("%n cover(s) found", "", ui->tvImages->count());
	ui->labStatus->setText(text) ;
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
	bool isTextSearch = ui->rbTextsearch->isChecked();

	ui->leSearch->setEnabled(isTextSearch);
	ui->btnSearch->setEnabled(!isTextSearch);
	ui->comboSearchFetchers->setEnabled(!isTextSearch);

	if(isTextSearch){
		searchTextEdited(ui->leSearch->text());
	}

	if(b) {
		reloadCombobox();
	}
}

void GUI_AlternativeCovers::wwwActiveChanged()
{
	bool is_active = GetSetting(Set::Cover_FetchFromWWW);

	ui->labWebsearchDisabled->setVisible(!is_active);
	ui->btnSearch->setVisible(is_active);

	ui->comboSearchFetchers->setEnabled(is_active);
	ui->rbAutosearch->setEnabled(is_active);
	ui->rbTextsearch->setEnabled(is_active);
}

void GUI_AlternativeCovers::searchTextEdited(const QString& text)
{
	if(!ui->rbTextsearch->isChecked()){
		return;
	}

	ui->btnSearch->setEnabled(text.size() > 0);

	bool isDirectUrl = Cover::Fetcher::Manager::isSearchstringWebsite(text);
	ui->comboSearchFetchers->setDisabled(isDirectUrl);
}

void GUI_AlternativeCovers::coverPressed(const QModelIndex& idx)
{
	QPixmap pm = getPixmapFromListWidgetItem(ui->tvImages->currentItem());
	bool valid = (idx.isValid() && !(pm.isNull()));

	ui->btnOk->setEnabled(valid);
	ui->btnApply->setEnabled(valid);
}

void GUI_AlternativeCovers::reset()
{
	if(ui)
	{
		ui->btnOk->setEnabled(false);
		ui->btnApply->setEnabled(false);
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
	QString dir = QDir::homePath();

	Cover::Location cl = m->alternativeLookup->coverLocation();
	if(!cl.localPathDir().isEmpty()) {
		dir = cl.localPathDir();
	}

	auto* dialog = new Gui::ImageSelectionDialog(dir, this);
	if(dialog->exec())
	{
		QStringList selectedFiles = dialog->selectedFiles();

		if(selectedFiles.count() > 0)
		{
			reset();

			for(const QString& path : selectedFiles)
			{

				QListWidgetItem* item = new QListWidgetItem(ui->tvImages);
				QPixmap pm(path);

				item->setIcon(QIcon(pm));
				item->setText(QString("%1x%2").arg(pm.width()).arg(pm.height()));
				ui->tvImages->addItem(item);
			}
		}
	}

	dialog->deleteLater();
}


void GUI_AlternativeCovers::reloadCombobox()
{
	bool fulltextSearch = ui->rbTextsearch->isChecked();

	AlternativeLookup::SearchMode search_mode = AlternativeLookup::SearchMode::Default;
	if(fulltextSearch) {
		search_mode = AlternativeLookup::SearchMode::Fulltext;
	}

	ui->comboSearchFetchers->clear();
	ui->comboSearchFetchers->addItem(Lang::get(Lang::All));

	const QStringList coverfetchers = m->alternativeLookup->activeCoverfetchers(search_mode);
	for(const QString& coverfetcher : coverfetchers)
	{
		ui->comboSearchFetchers->addItem(coverfetcher);
	}
}

void GUI_AlternativeCovers::initSaveToLibrary()
{
	Cover::Location cl = m->alternativeLookup->coverLocation();

	QString text = tr("Also save cover to %1").arg(cl.localPathDir());
	QFontMetrics fm = this->fontMetrics();

	ui->cbSaveToLibrary->setText(
		fm.elidedText(text, Qt::ElideRight, this->width() - 50)
	);

	ui->cbSaveToLibrary->setToolTip(cl.localPathDir());
	ui->cbSaveToLibrary->setChecked(GetSetting(Set::Cover_SaveToLibrary));
	ui->cbSaveToLibrary->setVisible(!cl.localPathDir().isEmpty());
}


void GUI_AlternativeCovers::languageChanged()
{
	ui->retranslateUi(this);

	ui->btnOk->setText(Lang::get(Lang::OK));
	ui->btnClose->setText(Lang::get(Lang::Close));
	ui->btnApply->setText(Lang::get(Lang::Apply));
	ui->labWebsearchDisabled->setText(tr("Cover web search is not enabled"));

	initSaveToLibrary();

	ui->btnSearch->setText(Lang::get(Lang::SearchVerb));
	ui->btnStopSearch->setText(Lang::get(Lang::Stop));

	ui->btnSearch->setVisible(!m->alternativeLookup->isRunning());
	ui->btnStopSearch->setVisible(m->alternativeLookup->isRunning());

	Cover::Location cl = m->alternativeLookup->coverLocation();
	QString text = tr("Also save cover to %1").arg(cl.localPathDir());
	ui->cbSaveToLibrary->setText(text);
}

void GUI_AlternativeCovers::showEvent(QShowEvent* e)
{
	QSize sz = GetSetting(Set::AlternativeCovers_Size);

	initUi();
	Gui::Dialog::showEvent(e);

	if(sz.isValid())
	{
		QTimer::singleShot(100, this, [sz, this]() {
			this->resize(sz);
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
			QCheckBox* cb = ui->cbSaveToLibrary;
			bool checked = cb->isChecked();
			initSaveToLibrary();
			cb->setChecked(checked);
		}

		if(isVisible())
		{
			QSize sz = e->size();
			SetSetting(Set::AlternativeCovers_Size, sz);
		}
	}

	if(m->loadingBar && m->loadingBar->isVisible())
	{
		m->loadingBar->hide();
		m->loadingBar->show();
		m->loadingBar->refresh();
	}
}
