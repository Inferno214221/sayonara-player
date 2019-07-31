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
#include "Gui/Covers/ui_GUI_AlternativeCovers.h"
#include "AlternativeCoverItemDelegate.h"
#include "AlternativeCoverItemModel.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookupAlternative.h"

#include "Gui/Utils/Widgets/ProgressBar.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/ImageSelectionDialog.h"

#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QStringList>

using Cover::AlternativeLookup;
using Cover::Location;
using Gui::ProgressBar;

struct GUI_AlternativeCovers::Private
{
	AlternativeLookup*				cl_alternative=nullptr;
	AlternativeCoverItemModel*		model=nullptr;
	AlternativeCoverItemDelegate*	delegate=nullptr;

	ProgressBar*					loading_bar=nullptr;

	Private(const Cover::Location& cl, bool silent, QObject* parent)
	{
		cl_alternative = new AlternativeLookup(cl, 20, silent, parent);
	}

	~Private()
	{
		if(cl_alternative)
		{
			cl_alternative->stop();
			cl_alternative->reset();
		}
	}
};

GUI_AlternativeCovers::GUI_AlternativeCovers(const Cover::Location& cl, bool silent, QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<GUI_AlternativeCovers::Private>(cl, silent, this);

	connect(m->cl_alternative, &AlternativeLookup::sig_coverfetchers_changed, this, &GUI_AlternativeCovers::reload_combobox);
}


GUI_AlternativeCovers::~GUI_AlternativeCovers()
{
	reset();

	delete ui; ui=nullptr;
}


void GUI_AlternativeCovers::init_ui()
{
	if(!ui)
	{
		ui = new Ui::GUI_AlternativeCovers();
		ui->setupUi(this);

		{ // create members
			m->model = new AlternativeCoverItemModel(this);
			m->delegate = new AlternativeCoverItemDelegate(this);
			m->loading_bar = new ProgressBar(ui->tv_images);

			ui->tv_images->setModel(m->model);
			ui->tv_images->setItemDelegate(m->delegate);
		}

		{ // add preference button
			CoverPreferenceAction* cpa = new CoverPreferenceAction(this);
			QPushButton* pref_button = cpa->create_button(this);
			pref_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
			ui->layout_server->addWidget(pref_button);
		}

		ui->cb_autostart->setChecked(GetSetting(Set::Cover_StartSearch));

		bool is_silent = m->cl_alternative->is_silent();
		ui->cb_save_to_library->setChecked(GetSetting(Set::Cover_SaveToLibrary) && !is_silent);
		ui->cb_save_to_library->setEnabled(!is_silent);

		connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_AlternativeCovers::ok_clicked);
		connect(ui->btn_apply, &QPushButton::clicked, this, &GUI_AlternativeCovers::apply_clicked);
		connect(ui->btn_search, &QPushButton::clicked, this, &GUI_AlternativeCovers::start);
		connect(ui->btn_stop_search, &QPushButton::clicked, this, &GUI_AlternativeCovers::stop);
		connect(ui->tv_images, &QTableView::pressed, this, &GUI_AlternativeCovers::cover_pressed);
		connect(ui->btn_file, &QPushButton::clicked, this, &GUI_AlternativeCovers::open_file_dialog);
		connect(ui->btn_close, &QPushButton::clicked, this, &Dialog::close);
		connect(ui->cb_autostart, &QCheckBox::toggled, this, &GUI_AlternativeCovers::autostart_toggled);

		connect(ui->rb_auto_search, &QRadioButton::toggled, this, &GUI_AlternativeCovers::rb_autosearch_toggled);
		connect(ui->rb_text_search, &QRadioButton::toggled, this, &GUI_AlternativeCovers::rb_textsearch_toggled);

		connect(m->cl_alternative, &AlternativeLookup::sig_cover_changed, this, &GUI_AlternativeCovers::sig_cover_changed);
		connect(m->cl_alternative, &AlternativeLookup::sig_cover_found, this, &GUI_AlternativeCovers::cover_found);
		connect(m->cl_alternative, &AlternativeLookup::sig_finished, this, &GUI_AlternativeCovers::lookup_finished);
		connect(m->cl_alternative, &AlternativeLookup::sig_started, this, &GUI_AlternativeCovers::lookup_started);

		ListenSettingNoCall(Set::Cover_Server, GUI_AlternativeCovers::servers_changed);
		ListenSetting(Set::Cover_FetchFromWWW, GUI_AlternativeCovers::www_active_changed);
	}

	Cover::Location cl = m->cl_alternative->cover_location();
	m->loading_bar->hide();
	ui->tabWidget->setCurrentIndex(0);
	ui->lab_status->setText("");
	ui->rb_auto_search->setChecked(true);
	ui->le_search->setText( cl.search_term() );

	init_save_to_library();
	reload_combobox();
}


void GUI_AlternativeCovers::set_cover_location(const Cover::Location& cl)
{
	m->cl_alternative->set_cover_location(cl);
}


void GUI_AlternativeCovers::start()
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

void GUI_AlternativeCovers::stop()
{
	m->cl_alternative->stop();
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
	m->cl_alternative->save(cover, ui->cb_save_to_library->isChecked());
}

void GUI_AlternativeCovers::search_clicked()
{
	if( m->cl_alternative->is_running() ) {
		m->cl_alternative->stop();
		return;
	}

	start();
}

void GUI_AlternativeCovers::lookup_started()
{
	m->loading_bar->show();

	ui->btn_search->setVisible(false);
	ui->btn_stop_search->setVisible(true);
}

void GUI_AlternativeCovers::lookup_finished(bool success)
{
	Q_UNUSED(success)

	m->loading_bar->hide();

	ui->btn_search->setVisible(true);
	ui->btn_stop_search->setVisible(false);
}

void GUI_AlternativeCovers::cover_found(const QPixmap& pm)
{
	m->model->add_cover(pm);

	ui->btn_ok->setEnabled(true);
	ui->btn_apply->setEnabled(true);

	QString text = tr("%n cover(s) found", "", m->model->cover_count());
	ui->lab_status->setText(text) ;
}

void GUI_AlternativeCovers::servers_changed()
{
	reload_combobox();
}

void GUI_AlternativeCovers::autostart_toggled(bool b)
{
	SetSetting(Set::Cover_StartSearch, b);
}

void GUI_AlternativeCovers::rb_autosearch_toggled(bool b)
{
	if(b){
		reload_combobox();
	}
}

void GUI_AlternativeCovers::rb_textsearch_toggled(bool b)
{
	ui->le_search->setEnabled(b);

	if(b){
		reload_combobox();
	}
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
	if(ui)
	{
		ui->btn_ok->setEnabled(false);
		ui->btn_apply->setEnabled(false);
		ui->btn_search->setVisible(false);
		ui->btn_stop_search->setVisible(true);
		ui->lab_status->clear();

		m->model->reset();
		m->loading_bar->hide();
	}

	m->cl_alternative->reset();
}


void GUI_AlternativeCovers::open_file_dialog()
{
	QString dir = QDir::homePath();

	Cover::Location cl = m->cl_alternative->cover_location();
	if(!cl.local_path_dir().isEmpty()) {
		dir = cl.local_path_dir();
	}

	ImageSelectionDialog* dialog = new ImageSelectionDialog(dir, this);
	if(dialog->exec())
	{
		QStringList selected_files = dialog->selectedFiles();

		if(selected_files.count() > 0)
		{
			reset();

			for(const QString& path : selected_files)
			{
				m->model->add_cover(path);
			}
		}
	}

	dialog->deleteLater();
}


void GUI_AlternativeCovers::reload_combobox()
{
	bool fulltext_search = ui->rb_text_search->isChecked();

	AlternativeLookup::SearchMode search_mode = AlternativeLookup::SearchMode::Default;
	if(fulltext_search) {
		search_mode = AlternativeLookup::SearchMode::Fulltext;
	}

	ui->combo_search_fetchers->clear();
	ui->combo_search_fetchers->addItem(Lang::get(Lang::All));

	const QStringList coverfetchers = m->cl_alternative->active_coverfetchers(search_mode);
	for(const QString& coverfetcher : coverfetchers)
	{
		ui->combo_search_fetchers->addItem(coverfetcher);
	}
}

void GUI_AlternativeCovers::init_save_to_library()
{
	Cover::Location cl = m->cl_alternative->cover_location();

	QString text = tr("Also save cover to %1").arg(cl.local_path_dir());
	QFontMetrics fm = this->fontMetrics();

	ui->cb_save_to_library->setText(
		fm.elidedText(text, Qt::ElideRight, this->width() - 50)
	);

	ui->cb_save_to_library->setToolTip(cl.local_path_dir());
	ui->cb_save_to_library->setChecked(GetSetting(Set::Cover_SaveToLibrary));
	ui->cb_save_to_library->setVisible(!cl.local_path_dir().isEmpty());
}


void GUI_AlternativeCovers::language_changed()
{
	ui->retranslateUi(this);

	ui->btn_ok->setText(Lang::get(Lang::OK));
	ui->btn_close->setText(Lang::get(Lang::Close));
	ui->btn_apply->setText(Lang::get(Lang::Apply));
	ui->lab_websearch_disabled->setText(tr("Cover web search is not enabled"));

	init_save_to_library();

	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	ui->btn_stop_search->setText(Lang::get(Lang::Stop));

	ui->btn_search->setVisible(!m->cl_alternative->is_running());
	ui->btn_stop_search->setVisible(m->cl_alternative->is_running());

	Cover::Location cl = m->cl_alternative->cover_location();
	QString text = tr("Also save cover to %1").arg(cl.local_path_dir());
	ui->cb_save_to_library->setText(text);
}

void GUI_AlternativeCovers::showEvent(QShowEvent* e)
{
	init_ui();

	Gui::Dialog::showEvent(e);
	if(GetSetting(Set::Cover_StartSearch) && GetSetting(Set::Cover_FetchFromWWW))
	{
		start();
	}
}


void GUI_AlternativeCovers::resizeEvent(QResizeEvent *e)
{
	Gui::Dialog::resizeEvent(e);

	if(ui && ui->cb_save_to_library)
	{
		QCheckBox* cb = ui->cb_save_to_library;
		bool checked = cb->isChecked();
		init_save_to_library();
		cb->setChecked(checked);
	}

	if(m->loading_bar && m->loading_bar->isVisible())
	{
		m->loading_bar->hide();
		m->loading_bar->show();
	}
}
