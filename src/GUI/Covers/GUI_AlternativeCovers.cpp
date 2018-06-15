/* GUI_AlternativeCovers.cpp */

/* Copyright (C) 2011-2017 Lucio Carreras
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

#include "AlternativeCoverItemDelegate.h"
#include "AlternativeCoverItemModel.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookupAlternative.h"
#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/CoverFetcherInterface.h"
#include "Components/Covers/CoverUtils.h"

#include "Components/Library/LibraryManager.h"

#include "Utils/Utils.h"
#include "Utils/Message/Message.h"
#include "Utils/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"

#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QStringList>
#include <QModelIndex>


using Cover::AlternativeLookup;
using Cover::Location;
using Gui::ProgressBar;

struct GUI_AlternativeCovers::Private
{
	QStringList						filelist;

	AlternativeLookup*				cl_alternative=nullptr;
	AlternativeCoverItemModel*		model=nullptr;
	AlternativeCoverItemDelegate*	delegate=nullptr;

	ProgressBar*			loading_bar=nullptr;

	int						cur_idx;
	bool					is_searching;

	Private() :
		cur_idx(-1),
		is_searching(false)
	{}

	~Private()
	{
		if(model){
			delete model;
		}

		if(delegate){
			delete delegate;
		}

		if(cl_alternative) {
			cl_alternative->stop();
		}
	}
};


GUI_AlternativeCovers::GUI_AlternativeCovers(QWidget* parent) :
	Dialog(parent)
{
	ui = new Ui::GUI_AlternativeCovers();
	m = Pimpl::make<GUI_AlternativeCovers::Private>();

	m->model = new AlternativeCoverItemModel(this);
	m->delegate = new AlternativeCoverItemDelegate(this);

	int n_items = m->model->rowCount() * m->model->columnCount() + 5;
	m->cl_alternative = new AlternativeLookup(this, n_items);

	ui->setupUi(this);

	m->loading_bar = new ProgressBar(ui->tv_images);
	ui->tv_images->setModel(m->model);
	ui->tv_images->setItemDelegate(m->delegate);

	connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_AlternativeCovers::ok_clicked);
	connect(ui->btn_apply, &QPushButton::clicked, this, &GUI_AlternativeCovers::apply_clicked);
	connect(ui->btn_search, &QPushButton::clicked, this, &GUI_AlternativeCovers::search_clicked);
	connect(ui->tv_images, &QTableView::pressed, this, &GUI_AlternativeCovers::cover_pressed);
	connect(ui->btn_file, &QPushButton::clicked, this, &GUI_AlternativeCovers::open_file_dialog);
	connect(ui->btn_close, &QPushButton::clicked, this, &Dialog::close);
	connect(m->cl_alternative, &AlternativeLookup::sig_cover_found, this, &GUI_AlternativeCovers::cl_new_cover);
	connect(m->cl_alternative, &AlternativeLookup::sig_finished, this, &GUI_AlternativeCovers::cl_finished);

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
}


GUI_AlternativeCovers::~GUI_AlternativeCovers()
{
	Cover::Util::delete_temp_covers();

	delete ui;
}


void GUI_AlternativeCovers::start(const Location& cl)
{
	if(!cl.valid()){
		return;
	}

	m->cl_alternative->set_cover_location(cl);

	ui->tabWidget->setCurrentIndex(0);
	ui->le_search->setText( cl.search_term() );
	ui->rb_auto_search->setChecked(true);

	sp_log(Log::Develop, this) << "Search alternative cover";
	sp_log(Log::Develop, this) << cl.to_string();

	init_combobox();

	connect_and_start();
}


void GUI_AlternativeCovers::connect_and_start()
{
	reset_model();
	Cover::Util::delete_temp_covers();

	m->is_searching = true;

	ui->btn_ok->setEnabled(false);
	ui->btn_apply->setEnabled(false);
	ui->btn_search->setText( Lang::get(Lang::Stop) );

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

	m->loading_bar->show();

	show();
}

void GUI_AlternativeCovers::language_changed()
{
	ui->retranslateUi(this);

	ui->btn_ok->setText(Lang::get(Lang::OK));
	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	ui->btn_close->setText(Lang::get(Lang::Close));
	ui->btn_apply->setText(Lang::get(Lang::Apply));
}

void GUI_AlternativeCovers::ok_clicked()
{
	apply_clicked();
	close();
}

void GUI_AlternativeCovers::apply_clicked()
{
	if(m->cur_idx == -1) {
		return;
	}

	RowColumn rc = m->model->cvt_2_row_col(m->cur_idx);

	QModelIndex idx = m->model->index(rc.row, rc.col);

	if(!idx.isValid()) {
		return;
	}

	QString cover_path = m->model->data(idx, Qt::UserRole).toString();
	QFile file(cover_path);

	if(!file.exists()) {
		Message::warning(tr("This cover does not exist"));
		return;
	}

	QImage img(cover_path);
	if(img.isNull()){
		return;
	}

	Cover::Location cl = m->cl_alternative->cover_location();
	img.save(cl.cover_path());

	emit sig_cover_changed(cl);
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


void GUI_AlternativeCovers::cl_new_cover(const QString& cover_path)
{
	m->filelist << cover_path;

	int n_files = m->filelist.size();

	RowColumn rc_last =     m->model->cvt_2_row_col( n_files - 1 );
	RowColumn rc_cur_idx =  m->model->cvt_2_row_col( m->cur_idx );
	bool is_valid =         m->model->is_valid(rc_cur_idx.row, rc_cur_idx.col);

	m->model->set_cover(rc_last.row, rc_last.col, cover_path);

	ui->btn_ok->setEnabled(is_valid);
	ui->btn_apply->setEnabled(is_valid);
	ui->lab_status->setText( tr("%1 covers found").arg(n_files) ) ;
}


void GUI_AlternativeCovers::cl_finished(bool b)
{
	Q_UNUSED(b)

	m->is_searching = false;

	ui->btn_search->setText(Lang::get(Lang::SearchVerb));

	m->loading_bar->hide();
}


void GUI_AlternativeCovers::cover_pressed(const QModelIndex& idx)
{
	int row = idx.row();
	int col = idx.column();
	QSize sz = m->model->get_cover_size(idx);
	bool valid = m->model->is_valid(row, col);
	m->cur_idx = m->model->cvt_2_idx(row, col);

	ui->btn_ok->setEnabled(valid);
	ui->btn_apply->setEnabled(valid);

	QString size_str = QString("%1x%2").arg(sz.width()).arg(sz.height());
	ui->lab_img_size->setText( size_str );
}


void GUI_AlternativeCovers::reset_model()
{
	m->model->reset();
	ui->lab_status->clear();
}


void GUI_AlternativeCovers::open_file_dialog()
{
	QStringList filters;
		filters << "*.jpg";
		filters << "*.png";
		filters << "*.gif";

	QStringList lst = QFileDialog::getOpenFileNames(this,
								  tr("Open image files"),
								  QDir::homePath(),
								  filters.join(" "));
	if(lst.isEmpty())
	{
		return;
	}

	reset_model();

	int idx = 0;
	for(const QString& path : lst)
	{
		RowColumn rc = m->model->cvt_2_row_col( idx );
		m->model->set_cover(rc.row, rc.col, path);

		idx ++;
	}
}


void GUI_AlternativeCovers::init_combobox()
{
	bool is_text_mode = ui->rb_text_search->isChecked();

	Cover::Fetcher::Manager* cfm = Cover::Fetcher::Manager::instance();
	Cover::Location cl = m->cl_alternative->cover_location();

	QList<Cover::Fetcher::Base*> active_coverfetchers = cfm->active_coverfetchers();
	QStringList first_item_string;

	for(Cover::Fetcher::Base* cfi : active_coverfetchers)
	{
		if(!cfi->keyword().isEmpty()){
			first_item_string << cfi->keyword();
		}
	}

	ui->combo_search_fetchers->clear();
	ui->combo_search_fetchers->addItem(first_item_string.join(", "));

	QList<Cover::Fetcher::Base*> available_cover_fetchers = cfm->available_coverfetchers();
	for(const Cover::Fetcher::Base* cover_fetcher : available_cover_fetchers)
	{
		QString keyword = cover_fetcher->keyword();

		bool suitable = false;
		if(is_text_mode) {
			suitable = cover_fetcher->is_search_supported();
		}

		else {
			suitable = cl.all_search_urls().contains(keyword);
		}

		if(suitable){
			ui->combo_search_fetchers->addItem(cover_fetcher->keyword());
		}
	}
}

void GUI_AlternativeCovers::resizeEvent(QResizeEvent *e)
{
	Gui::Dialog::resizeEvent(e);
	m->loading_bar->hide();
	m->loading_bar->show();
}


void GUI_AlternativeCovers::closeEvent(QCloseEvent *e)
{
	if(m->cl_alternative) {
		m->cl_alternative->stop();
	}

	m->loading_bar->hide();

	Cover::Util::delete_temp_covers();

	Dialog::closeEvent(e);
}
