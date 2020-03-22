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

		ui->tv_images->setWrapping(true);
		ui->tv_images->setResizeMode(QListView::Fixed);
		ui->tv_images->setIconSize({150, 150});


		{ // create members
			m->loadingBar = new ProgressBar(ui->tv_images);
		}

		{ // add preference button
			auto* cpa = new Gui::CoverPreferenceAction(this);
			QPushButton* prefButton = cpa->createButton(this);
			prefButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
			ui->horizontalLayout->insertWidget(0, prefButton);
		}

		ui->cb_autostart->setChecked(GetSetting(Set::Cover_StartSearch));

		bool is_silent = m->alternativeLookup->isSilent();
		ui->cb_save_to_library->setChecked(GetSetting(Set::Cover_SaveToLibrary) && !is_silent);
		ui->cb_save_to_library->setEnabled(!is_silent);

		connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_AlternativeCovers::okClicked);
		connect(ui->btn_apply, &QPushButton::clicked, this, &GUI_AlternativeCovers::applyClicked);
		connect(ui->btn_search, &QPushButton::clicked, this, &GUI_AlternativeCovers::start);
		connect(ui->btn_stop_search, &QPushButton::clicked, this, &GUI_AlternativeCovers::stop);
		connect(ui->tv_images, &QListWidget::pressed, this, &GUI_AlternativeCovers::coverPressed);
		connect(ui->btn_file, &QPushButton::clicked, this, &GUI_AlternativeCovers::openFileDialog);
		connect(ui->btn_close, &QPushButton::clicked, this, &Dialog::close);
		connect(ui->cb_autostart, &QCheckBox::toggled, this, &GUI_AlternativeCovers::autostartToggled);
		connect(ui->le_search, &QLineEdit::textChanged, this, &GUI_AlternativeCovers::searchTextEdited);

		connect(ui->rb_auto_search, &QRadioButton::toggled, this, &GUI_AlternativeCovers::rbAutosearchToggled);
		connect(ui->rb_text_search, &QRadioButton::toggled, this, &GUI_AlternativeCovers::rbAutosearchToggled);

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
	ui->lab_status->setText("");
	ui->rb_auto_search->setChecked(true);
	ui->le_search->setText(cl.searchTerm());

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

	if(ui->rb_text_search->isChecked())
	{
		QString searchTerm = ui->le_search->text();
		if(ui->combo_search_fetchers->currentIndex() > 0)
		{
			QString identifier = ui->combo_search_fetchers->currentText();
			m->alternativeLookup->startTextSearch(searchTerm, identifier);
		}

		else {
			m->alternativeLookup->startTextSearch(searchTerm);
		}
	}

	else if(ui->rb_auto_search->isChecked())
	{
		if(ui->combo_search_fetchers->currentIndex() > 0)
		{
			QString identifier = ui->combo_search_fetchers->currentText();
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
	QPixmap cover = getPixmapFromListWidgetItem(ui->tv_images->currentItem());
	if(cover.isNull()) {
		return;
	}

	m->alternativeLookup->save(cover, ui->cb_save_to_library->isChecked());
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

	ui->btn_search->setVisible(false);
	ui->btn_stop_search->setVisible(true);
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

	ui->btn_search->setVisible(true);
	ui->btn_stop_search->setVisible(false);
}

void GUI_AlternativeCovers::coverFound(const QPixmap& pm)
{
	QListWidgetItem* item = new QListWidgetItem(ui->tv_images);
	item->setIcon(QIcon(pm));
	item->setText(QString("%1x%2").arg(pm.width()).arg(pm.height()));

	ui->tv_images->addItem(item);

	ui->btn_ok->setEnabled(true);
	ui->btn_apply->setEnabled(true);

	QString text = tr("%n cover(s) found", "", ui->tv_images->count());
	ui->lab_status->setText(text) ;
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
	bool isTextSearch = ui->rb_text_search->isChecked();

	ui->le_search->setEnabled(isTextSearch);
	ui->btn_search->setEnabled(!isTextSearch);
	ui->combo_search_fetchers->setEnabled(!isTextSearch);

	if(isTextSearch){
		searchTextEdited(ui->le_search->text());
	}

	if(b) {
		reloadCombobox();
	}
}

void GUI_AlternativeCovers::wwwActiveChanged()
{
	bool is_active = GetSetting(Set::Cover_FetchFromWWW);

	ui->lab_websearch_disabled->setVisible(!is_active);
	ui->btn_search->setVisible(is_active);

	ui->combo_search_fetchers->setEnabled(is_active);
	ui->rb_auto_search->setEnabled(is_active);
	ui->rb_text_search->setEnabled(is_active);
}

void GUI_AlternativeCovers::searchTextEdited(const QString& text)
{
	if(!ui->rb_text_search->isChecked()){
		return;
	}

	ui->btn_search->setEnabled(text.size() > 0);

	bool isDirectUrl = Cover::Fetcher::Manager::isSearchstringWebsite(text);
	ui->combo_search_fetchers->setDisabled(isDirectUrl);
}

void GUI_AlternativeCovers::coverPressed(const QModelIndex& idx)
{
	QPixmap pm = getPixmapFromListWidgetItem(ui->tv_images->currentItem());
	bool valid = (idx.isValid() && !(pm.isNull()));

	ui->btn_ok->setEnabled(valid);
	ui->btn_apply->setEnabled(valid);
}

void GUI_AlternativeCovers::reset()
{
	if(ui)
	{
		ui->btn_ok->setEnabled(false);
		ui->btn_apply->setEnabled(false);
		ui->btn_search->setVisible(false);
		ui->btn_stop_search->setVisible(true);
		ui->lab_status->clear();

		ui->tv_images->clear();
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

				QListWidgetItem* item = new QListWidgetItem(ui->tv_images);
				QPixmap pm(path);

				item->setIcon(QIcon(pm));
				item->setText(QString("%1x%2").arg(pm.width()).arg(pm.height()));
				ui->tv_images->addItem(item);
			}
		}
	}

	dialog->deleteLater();
}


void GUI_AlternativeCovers::reloadCombobox()
{
	bool fulltextSearch = ui->rb_text_search->isChecked();

	AlternativeLookup::SearchMode search_mode = AlternativeLookup::SearchMode::Default;
	if(fulltextSearch) {
		search_mode = AlternativeLookup::SearchMode::Fulltext;
	}

	ui->combo_search_fetchers->clear();
	ui->combo_search_fetchers->addItem(Lang::get(Lang::All));

	const QStringList coverfetchers = m->alternativeLookup->activeCoverfetchers(search_mode);
	for(const QString& coverfetcher : coverfetchers)
	{
		ui->combo_search_fetchers->addItem(coverfetcher);
	}
}

void GUI_AlternativeCovers::initSaveToLibrary()
{
	Cover::Location cl = m->alternativeLookup->coverLocation();

	QString text = tr("Also save cover to %1").arg(cl.localPathDir());
	QFontMetrics fm = this->fontMetrics();

	ui->cb_save_to_library->setText(
		fm.elidedText(text, Qt::ElideRight, this->width() - 50)
	);

	ui->cb_save_to_library->setToolTip(cl.localPathDir());
	ui->cb_save_to_library->setChecked(GetSetting(Set::Cover_SaveToLibrary));
	ui->cb_save_to_library->setVisible(!cl.localPathDir().isEmpty());
}


void GUI_AlternativeCovers::languageChanged()
{
	ui->retranslateUi(this);

	ui->btn_ok->setText(Lang::get(Lang::OK));
	ui->btn_close->setText(Lang::get(Lang::Close));
	ui->btn_apply->setText(Lang::get(Lang::Apply));
	ui->lab_websearch_disabled->setText(tr("Cover web search is not enabled"));

	initSaveToLibrary();

	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	ui->btn_stop_search->setText(Lang::get(Lang::Stop));

	ui->btn_search->setVisible(!m->alternativeLookup->isRunning());
	ui->btn_stop_search->setVisible(m->alternativeLookup->isRunning());

	Cover::Location cl = m->alternativeLookup->coverLocation();
	QString text = tr("Also save cover to %1").arg(cl.localPathDir());
	ui->cb_save_to_library->setText(text);
}

void GUI_AlternativeCovers::showEvent(QShowEvent* e)
{
	initUi();

	Gui::Dialog::showEvent(e);
	if(GetSetting(Set::Cover_StartSearch) && GetSetting(Set::Cover_FetchFromWWW))
	{
		start();
	}
}


void GUI_AlternativeCovers::resizeEvent(QResizeEvent* e)
{
	Gui::Dialog::resizeEvent(e);

	if(ui && ui->cb_save_to_library)
	{
		QCheckBox* cb = ui->cb_save_to_library;
		bool checked = cb->isChecked();
		initSaveToLibrary();
		cb->setChecked(checked);
	}

	if(m->loadingBar && m->loadingBar->isVisible())
	{
		m->loadingBar->hide();
		m->loadingBar->show();
		m->loadingBar->refresh();
	}
}
