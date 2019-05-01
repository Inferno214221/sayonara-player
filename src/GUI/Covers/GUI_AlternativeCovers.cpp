/* GUI_AlternativeCovers.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
 *      Author: Lucio Carreras
 */

#include "GUI_AlternativeCovers.h"
#include "GUI/Covers/ui_GUI_AlternativeCovers.h"
#include "GUI/Utils/Widgets/ProgressBar.h"
#include "GUI/Utils/PreferenceAction.h"
#include "GUI/Utils/ImageSelectionDialog.h"

#include "AlternativeCoverItemDelegate.h"
#include "AlternativeCoverItemModel.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookupAlternative.h"
#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/CoverFetcherInterface.h"
#include "Components/Covers/CoverUtils.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Message/Message.h"
#include "Utils/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QStringList>

using Cover::AlternativeLookup;
using Cover::Location;
using Gui::ProgressBar;

struct GUI_AlternativeCovers::Private
{
	AlternativeLookup*				cl_alternative=nullptr;
	AlternativeCoverItemModel*		model=nullptr;
	AlternativeCoverItemDelegate*	delegate=nullptr;

	ProgressBar*			loading_bar=nullptr;

	bool					is_searching;

	Private() :
		is_searching(false)
	{}

	~Private()
	{
		if(cl_alternative) {
			cl_alternative->stop();
		}
	}
};


GUI_AlternativeCovers::GUI_AlternativeCovers(QWidget* parent) :
	Dialog(parent)
{
	ui = new Ui::GUI_AlternativeCovers();
	ui->setupUi(this);

	m = Pimpl::make<GUI_AlternativeCovers::Private>();

	m->cl_alternative = new AlternativeLookup(this, 20);
	m->loading_bar = new ProgressBar(ui->tv_images);

	m->model = new AlternativeCoverItemModel(this);
	ui->tv_images->setModel(m->model);

	m->delegate = new AlternativeCoverItemDelegate(this);
	ui->tv_images->setItemDelegate(m->delegate);

	// add preference button
	CoverPreferenceAction* cpa = new CoverPreferenceAction(this);
	QPushButton* pref_button = cpa->create_button(this);
	pref_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	ui->layout_server->addWidget(pref_button);
	ui->cb_autostart->setChecked(GetSetting(Set::Cover_StartSearch));

	connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_AlternativeCovers::ok_clicked);
	connect(ui->btn_apply, &QPushButton::clicked, this, &GUI_AlternativeCovers::apply_clicked);
	connect(ui->btn_search, &QPushButton::clicked, this, &GUI_AlternativeCovers::search_clicked);
	connect(ui->tv_images, &QTableView::pressed, this, &GUI_AlternativeCovers::cover_pressed);
	connect(ui->btn_file, &QPushButton::clicked, this, &GUI_AlternativeCovers::open_file_dialog);
	connect(ui->btn_close, &QPushButton::clicked, this, &Dialog::close);
	connect(ui->cb_autostart, &QCheckBox::toggled, this, &GUI_AlternativeCovers::autostart_toggled);

	connect(m->cl_alternative, &AlternativeLookup::sig_cover_found, this, &GUI_AlternativeCovers::cl_new_cover);
	connect(m->cl_alternative, &AlternativeLookup::sig_finished, this, &GUI_AlternativeCovers::cl_finished);
	connect(m->cl_alternative, &AlternativeLookup::sig_started, this, &GUI_AlternativeCovers::cl_started);

	connect(ui->rb_auto_search, &QRadioButton::toggled, this, [=](bool b)
	{
		if(b){
			init_combobox();
		}
	});

	connect(ui->rb_text_search, &QRadioButton::toggled, this, [=](bool b)
	{
		ui->le_search->setEnabled(b);

		if(b){
			init_combobox();
		}
	});

	ListenSettingNoCall(Set::Cover_Server, GUI_AlternativeCovers::servers_changed);
	ListenSetting(Set::Cover_FetchFromWWW, GUI_AlternativeCovers::www_active_changed);
}


GUI_AlternativeCovers::~GUI_AlternativeCovers()
{
	reset();

	delete ui; ui=nullptr;
}


void GUI_AlternativeCovers::start(const Location& cl)
{
	if(!cl.valid()){
		return;
	}

	m->cl_alternative->set_cover_location(cl);
	m->loading_bar->hide();

	ui->tabWidget->setCurrentIndex(0);
	ui->lab_status->setText("");
	ui->le_search->setText( cl.search_term() );
	ui->rb_auto_search->setChecked(true);

	sp_log(Log::Develop, this) << "Search alternative cover " << cl.to_string();

	init_combobox();

	if(!GetSetting(Set::Cover_StartSearch))
	{
		this->show();
	}

	else
	{
//		if(!GetSetting(Set::Cover_FetchFromWWW)){
//			return;
//		}

		connect_and_start();
	}
}


void GUI_AlternativeCovers::connect_and_start()
{
	this->reset();

	if(ui->rb_text_search->isChecked())
	{
		QString search_term = ui->le_search->text();
		if(ui->combo_search_fetchers->currentIndex() > 0)
		{
			QString cover_fetcher_identifier = ui->combo_search_fetchers->currentText();
			m->cl_alternative->start_text_search(search_term, cover_fetcher_identifier);
		}

		else {
			m->cl_alternative->start_text_search(search_term);
		}
	}

	else if(ui->rb_auto_search->isChecked())
	{
		if(ui->combo_search_fetchers->currentIndex() > 0)
		{
			QString cover_fetcher_identifier = ui->combo_search_fetchers->currentText();
			m->cl_alternative->start(cover_fetcher_identifier);
		}

		else {
			m->cl_alternative->start();
		}
	}

	this->show();
}

void GUI_AlternativeCovers::language_changed()
{
	ui->retranslateUi(this);

	ui->btn_ok->setText(Lang::get(Lang::OK));
	ui->btn_close->setText(Lang::get(Lang::Close));
	ui->btn_apply->setText(Lang::get(Lang::Apply));
	ui->lab_websearch_disabled->setText(tr("Cover web search is not enabled"));

	if(m->is_searching){
		ui->btn_search->setText(Lang::get(Lang::Stop));
	}

	else {
		ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	}
}

void GUI_AlternativeCovers::ok_clicked()
{
	apply_clicked();
	close();
}

void GUI_AlternativeCovers::apply_clicked()
{
	QModelIndex current_idx = ui->tv_images->currentIndex();

	QPixmap cover = m->model->data(current_idx, Qt::UserRole).value<QPixmap>();
	if(cover.isNull())
	{
		sp_log(Log::Warning, this) << "Cannot save invalid cover";
	}

	else
	{
		Cover::Location cl = m->cl_alternative->cover_location();
		QFileInfo fi(cl.cover_path());
		if(fi.isSymLink()){
			QFile::remove(cl.cover_path());
		}

		cover.save(cl.cover_path());

		DB::Covers* dbc = DB::Connector::instance()->cover_connector();
		dbc->set_cover(cl.hash(), cover);

		emit sig_cover_changed(cl);
	}
}

void GUI_AlternativeCovers::search_clicked()
{
	if( m->is_searching &&
		m->cl_alternative)
	{
		m->cl_alternative->stop();
		return;
	}

	connect_and_start();
}

void GUI_AlternativeCovers::cl_started()
{
	m->is_searching = true;
	m->loading_bar->show();

	ui->btn_search->setText(Lang::get(Lang::Stop));
}

void GUI_AlternativeCovers::cl_finished(bool success)
{
	Q_UNUSED(success)

	m->is_searching = false;
	m->loading_bar->hide();

	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
}

void GUI_AlternativeCovers::cl_new_cover(const QPixmap& pm)
{
	m->model->add_cover(pm);

	ui->btn_ok->setEnabled(true);
	ui->btn_apply->setEnabled(true);
	ui->lab_status->setText( tr("%1 covers found").arg(m->model->cover_count()) ) ;
}

void GUI_AlternativeCovers::servers_changed()
{
	init_combobox();
}

void GUI_AlternativeCovers::autostart_toggled(bool b)
{
	SetSetting(Set::Cover_StartSearch, b);
}

void GUI_AlternativeCovers::www_active_changed()
{
	bool is_active = GetSetting(Set::Cover_FetchFromWWW);

	ui->lab_websearch_disabled->setVisible(!is_active);
	ui->btn_search->setVisible(is_active);

	ui->combo_search_fetchers->setEnabled(is_active);
	ui->rb_auto_search->setEnabled(is_active);
	ui->rb_text_search->setEnabled(is_active);
}


void GUI_AlternativeCovers::cover_pressed(const QModelIndex& idx)
{
	bool valid = m->model->is_valid(idx);

	ui->btn_ok->setEnabled(valid);
	ui->btn_apply->setEnabled(valid);

	QSize sz = m->model->cover_size(idx);
	QString size_str = QString("%1x%2").arg(sz.width()).arg(sz.height());
	ui->lab_img_size->setText( size_str );
}


void GUI_AlternativeCovers::reset()
{
	ui->btn_ok->setEnabled(false);
	ui->btn_apply->setEnabled(false);
	ui->btn_search->setText(Lang::get(Lang::Stop));
	ui->lab_status->clear();

	m->is_searching = false;
	m->model->reset();
	m->loading_bar->hide();

	if(m->cl_alternative)
	{
		m->cl_alternative->stop();
	}

	Cover::Utils::delete_temp_covers();
}


void GUI_AlternativeCovers::open_file_dialog()
{
	QString dir = QDir::homePath();
	Cover::Location cl = m->cl_alternative->cover_location();
	if(!cl.local_path_hint().isEmpty())
	{
		QString filename;
		::Util::File::split_filename(cl.local_path_hint(), dir, filename);
	}

	ImageSelectionDialog* dialog = new ImageSelectionDialog(dir, this);

	if(!dialog->exec()){
		return;
	};

	QStringList lst = dialog->selectedFiles();
	dialog->deleteLater();
	if(lst.isEmpty())
	{
		return;
	}

	reset();

	int idx = 0;
	for(const QString& path : lst)
	{
		m->model->add_cover(path);

		idx ++;
	}
}


void GUI_AlternativeCovers::init_combobox()
{
	bool is_text_mode = ui->rb_text_search->isChecked();

	Cover::Fetcher::Manager* cfm = Cover::Fetcher::Manager::instance();
	Cover::Location cl = m->cl_alternative->cover_location();

	ui->combo_search_fetchers->clear();

	QList<Cover::Fetcher::Base*> cover_fetchers = cfm->coverfetchers();
	for(const Cover::Fetcher::Base* cover_fetcher : cover_fetchers)
	{
		QString keyword = cover_fetcher->keyword();
		if(keyword.isEmpty()){
			continue;
		}

		bool suitable = false;
		if(is_text_mode)
		{
			suitable = cover_fetcher->is_search_supported();
		}

		else
		{
			QStringList search_urls = cl.search_urls();
			suitable = ::Util::contains(search_urls, [=](const QString& url){
				QString id = cfm->identifier_by_url(url);
				return (id == keyword);
			});
		}

		if(suitable){
			ui->combo_search_fetchers->addItem(cover_fetcher->keyword());
		}
	}
}

void GUI_AlternativeCovers::resizeEvent(QResizeEvent *e)
{
	Gui::Dialog::resizeEvent(e);

	if(m->loading_bar->isVisible())
	{
		m->loading_bar->hide();
		m->loading_bar->show();
	}
}


void GUI_AlternativeCovers::closeEvent(QCloseEvent *e)
{
	if(m->cl_alternative) {
		m->cl_alternative->stop();
	}

	m->loading_bar->hide();

	Cover::Utils::delete_temp_covers();

	Dialog::closeEvent(e);
}
