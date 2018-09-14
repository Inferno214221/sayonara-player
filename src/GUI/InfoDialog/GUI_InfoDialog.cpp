/* GUI_InfoDialog.cpp

 * Copyright (C) 2011-2017 Lucio Carreras
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Lucio Carreras,
 * Jul 19, 2012
 *
 */

#include "GUI_InfoDialog.h"
#include "GUI/InfoDialog/ui_GUI_InfoDialog.h"

#include "InfoDialogContainer.h"

#include "GUI/TagEdit/GUI_TagEdit.h"
#include "GUI/InfoDialog/GUI_Lyrics.h"
#include "GUI/Utils/Icons.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/MetaDataInfo/MetaDataInfo.h"
#include "Components/MetaDataInfo/AlbumInfo.h"
#include "Components/MetaDataInfo/ArtistInfo.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Language.h"
#include "Utils/globals.h"

#include <QTabBar>
#include <QTableWidget>
#include <QTableWidgetItem>

struct GUI_InfoDialog::Private
{
	InfoDialogContainer*	info_dialog_container=nullptr;
	GUI_TagEdit*			ui_tag_edit=nullptr;
	GUI_Lyrics*				ui_lyrics=nullptr;
	Cover::Location			cl;
	MetaDataList			v_md;
	MD::Interpretation		md_interpretation;
};


GUI_InfoDialog::GUI_InfoDialog(InfoDialogContainer* container, QWidget* parent) :
	Dialog(parent)
{
	ui = nullptr;
	m = Pimpl::make<Private>();

	m->info_dialog_container = container;
	m->md_interpretation = MD::Interpretation::None;
}

GUI_InfoDialog::~GUI_InfoDialog() {}

void GUI_InfoDialog::language_changed()
{
	if(!ui){
		return;
	}

	ui->retranslateUi(this);

	prepare_info(m->md_interpretation);
	ui->tab_widget->setTabText(0, Lang::get(Lang::Info));
	ui->tab_widget->setTabText(1, Lang::get(Lang::Lyrics));
	ui->tab_widget->setTabText(2, Lang::get(Lang::Edit));
	ui->btn_close1->setText(Lang::get(Lang::Close));
}

void GUI_InfoDialog::skin_changed()
{
	if(!ui){
		return;
	}

	QTabBar* tab_bar = ui->tab_widget->tabBar();
	if(tab_bar)
	{
		using namespace Gui;
		tab_bar->setTabIcon(0, Icons::icon(Icons::Info));
		tab_bar->setTabIcon(1, Icons::icon(Icons::Lyrics));
		tab_bar->setTabIcon(2, Icons::icon(Icons::Edit));
	}
}


static void prepare_info_table(QTableWidget* table, const QList<StringPair>& data)
{
	table->clear();
	table->setRowCount(data.count());
	table->setColumnCount(2);
	table->setAlternatingRowColors(true);
	table->setShowGrid(false);
	table->setEditTriggers(QTableView::NoEditTriggers);
	table->setSelectionBehavior(QTableView::SelectRows);

	int row	= 0;
	for(const StringPair& p : data)
	{
		QTableWidgetItem* i1 = new QTableWidgetItem(p.first);
		QFont f(i1->font());
		f.setBold(true);
		i1->setFont(f);
		QTableWidgetItem* i2 = new QTableWidgetItem(p.second);

		table->setItem(row, 0, i1);
		table->setItem(row, 1, i2);

		row++;
	}

	table->resizeColumnToContents(0);
}

static void prepare_paths(QListWidget* path_list_widget, const QStringList& paths)
{
	path_list_widget->clear();
	path_list_widget->setAlternatingRowColors(true);
	path_list_widget->setEditTriggers(QListView::NoEditTriggers);

	for(const QString& path : paths)
	{
		QLabel* label = new QLabel(path_list_widget);
		label->setText(path);
		label->setOpenExternalLinks(true);
		label->setTextFormat(Qt::RichText);

		QListWidgetItem* item = new QListWidgetItem(path_list_widget);
		path_list_widget->setItemWidget(item, label);
		path_list_widget->addItem(item);
	}
}


void GUI_InfoDialog::prepare_info(MD::Interpretation md_interpretation)
{
	if(!ui){
		return;
	}

	MetaDataInfo* info;

	switch (md_interpretation)
	{
		case MD::Interpretation::Artists:
			info = new ArtistInfo(m->v_md);
			break;
		case MD::Interpretation::Albums:
			info = new AlbumInfo(m->v_md);
			break;

		case MD::Interpretation::Tracks:
			info = new MetaDataInfo(m->v_md);
			break;

		default:
			return;
	}

	ui->lab_title->setText(info->header());
	ui->lab_subheader->setText(info->subheader());

	const QList<StringPair> info_map = info->infostring_map();
	prepare_info_table(ui->table_info, info_map);

	QStringList paths = info->paths();
	prepare_paths(ui->list_paths, paths);

	m->cl = info->cover_location();
	prepare_cover(m->cl);

	delete info; info = nullptr;
}


void GUI_InfoDialog::set_metadata(const MetaDataList& v_md, MD::Interpretation md_interpretation)
{
	m->md_interpretation = md_interpretation;
	m->v_md = v_md;
}

bool GUI_InfoDialog::has_metadata() const
{
	return (m->v_md.size() > 0);
}


void GUI_InfoDialog::show(GUI_InfoDialog::Tab tab)
{
	if(!ui){
		init();
	}

	if(m->v_md.isEmpty()){
		return;
	}

	QTabWidget* tab_widget = ui->tab_widget;

	bool lyric_enabled = (m->v_md.size() == 1);
	bool tag_edit_enabled = Util::contains(m->v_md, [](const MetaData& md){
		return (!Util::File::is_www(md.filepath()));
	});

	tab_widget->setTabEnabled((int) GUI_InfoDialog::Tab::Edit, tag_edit_enabled);
	tab_widget->setTabEnabled((int) GUI_InfoDialog::Tab::Lyrics, lyric_enabled);

	if( !tab_widget->isTabEnabled((int) tab) )
	{
		tab = GUI_InfoDialog::Tab::Info;
	}

	if(tab_widget->currentIndex() == ((int) tab))
	{
		tab_index_changed(tab);
	}

	else
	{
		tab_widget->setCurrentIndex((int) tab);
	}


	Dialog::show();
}

void GUI_InfoDialog::prepare_cover(const Cover::Location& cl)
{
	ui->btn_image->set_cover_location(cl);
}


void GUI_InfoDialog::init()
{
	if(ui){
		return;
	}

	ui = new Ui::InfoDialog();
	ui->setupUi(this);

	QTabWidget* tab_widget = ui->tab_widget;
	tab_widget->setFocusPolicy(Qt::NoFocus);

	connect(tab_widget, &QTabWidget::currentChanged, this, &GUI_InfoDialog::tab_index_changed_int);

	ui->btn_image->setStyleSheet("QPushButton:hover {background-color: transparent;}");
}


void GUI_InfoDialog::init_tag_edit()
{
	if(!m->ui_tag_edit)
	{
		QLayout* tab3_layout = ui->tab_3->layout();
		m->ui_tag_edit = new GUI_TagEdit(ui->tab_3);
		tab3_layout->addWidget(m->ui_tag_edit);

		connect(m->ui_tag_edit, &GUI_TagEdit::sig_cancelled, this, &GUI_InfoDialog::close);
	}
}

void GUI_InfoDialog::init_lyrics()
{
	if(!m->ui_lyrics)
	{
		QLayout* tab2_layout = ui->tab_2->layout();
		m->ui_lyrics = new GUI_Lyrics(ui->tab_2);
		tab2_layout->addWidget(m->ui_lyrics);

		connect(m->ui_lyrics, &GUI_Lyrics::sig_closed, this, &GUI_InfoDialog::close);
	}
}


void GUI_InfoDialog::tab_index_changed_int(int idx)
{
	idx = std::min( (int) GUI_InfoDialog::Tab::Edit, idx);
	idx = std::max( (int) GUI_InfoDialog::Tab::Info, idx);

	tab_index_changed( (GUI_InfoDialog::Tab) idx );
}


void GUI_InfoDialog::tab_index_changed(GUI_InfoDialog::Tab idx)
{
	if(!ui){
		return;
	}

	switch(idx)
	{
		case GUI_InfoDialog::Tab::Edit:
			show_tag_edit_tab();
			break;

		case GUI_InfoDialog::Tab::Lyrics:
			show_lyrics_tab();
			break;

		default:
			show_info_tab();
			break;
	}
}

void GUI_InfoDialog::show_info_tab()
{
	prepare_info(m->md_interpretation);

	ui->tab_widget->setCurrentWidget(ui->ui_info_widget);
	ui->ui_info_widget->show();
	prepare_cover(m->cl);
}

void GUI_InfoDialog::show_lyrics_tab()
{
	init_lyrics();

	m->ui_lyrics->set_metadata(m->v_md.first());
	ui->tab_widget->setCurrentWidget(m->ui_lyrics);
	m->ui_lyrics->set_metadata(m->v_md.first());
	m->ui_lyrics->show();
}

void GUI_InfoDialog::show_tag_edit_tab()
{
	MetaDataList local_md;
	for(const MetaData& md : m->v_md)
	{
		if(!Util::File::is_www(md.filepath())){
			local_md << md;
		}
	}

	if(local_md.isEmpty())
	{
		ui->tab_widget->setCurrentIndex(0);
		return;
	}

	init_tag_edit();

	m->ui_tag_edit->set_metadata(local_md);
	ui->tab_widget->setCurrentWidget(m->ui_tag_edit);
	if(m->ui_tag_edit->count() == 0)
	{
		MetaDataList local_md;
		for(const MetaData& md : m->v_md)
		{
			if(!Util::File::is_www(md.filepath()))
			{
				local_md << md;
			}
		}

		if(local_md.size() > 0) {
			m->ui_tag_edit->set_metadata(local_md);
		}
	}

	m->ui_tag_edit->show();
}


void GUI_InfoDialog::closeEvent(QCloseEvent* e)
{
	Dialog::closeEvent(e);

	m->v_md.clear();
	m->info_dialog_container->info_dialog_closed();
}

void GUI_InfoDialog::showEvent(QShowEvent *e)
{
	if(!ui){
		init();
	}

	Dialog::showEvent(e);
}


