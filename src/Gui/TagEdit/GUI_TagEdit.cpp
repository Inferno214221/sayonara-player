/* GUI_TagEdit.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "GUI_TagEdit.h"
#include "TagFromPath.h"
#include "TagLineEdit.h"

#include "Gui/TagEdit/ui_GUI_TagEdit.h"

#include "Components/Tagging/Expression.h"
#include "Components/Tagging/Editor.h"
#include "Components/Covers/CoverLocation.h"

#include "Gui/Utils/Delegates/ComboBoxDelegate.h"
#include "Gui/Utils/Widgets/Completer.h"
#include "Gui/Utils/Style.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Message/Message.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TaggingCover.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Language/Language.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include <QMap>
#include <QStringList>
#include <QPixmap>
#include <QFileInfo>


using namespace Tagging;

struct GUI_TagEdit::Private
{
	GUI_TagFromPath*	ui_tag_from_path=nullptr;
	Editor*				tag_edit=nullptr;

	QMap<int, QString>	cover_path_map;
	int					cur_idx;
};

GUI_TagEdit::GUI_TagEdit(QWidget* parent) :
	Widget(parent)
{
	ui = new Ui::GUI_TagEdit();
	ui->setupUi(this);

	m = Pimpl::make<Private>();
	m->tag_edit = new Tagging::Editor(this);
	m->ui_tag_from_path = new GUI_TagFromPath(ui->tab_from_path);
	ui->tab_from_path->layout()->addWidget(m->ui_tag_from_path);

	ui->tab_widget->setCurrentIndex(0);

	connect(ui->btn_next, &QPushButton::clicked, this, &GUI_TagEdit::next_button_clicked);
	connect(ui->btn_prev, &QPushButton::clicked, this, &GUI_TagEdit::prev_button_clicked);
	connect(ui->cb_replace, &QCheckBox::toggled, this, &GUI_TagEdit::cb_replace_toggled);

	connect(ui->cb_album_all, &QCheckBox::toggled, ui->le_album, &QWidget::setDisabled);
	connect(ui->cb_artist_all, &QCheckBox::toggled, ui->le_artist, &QWidget::setDisabled);
	connect(ui->cb_album_artist_all, &QCheckBox::toggled, ui->le_album_artist, &QWidget::setDisabled);
	connect(ui->cb_genre_all, &QCheckBox::toggled, ui->le_genre, &QWidget::setDisabled);
	connect(ui->cb_year_all, &QCheckBox::toggled, ui->sb_year, &QWidget::setDisabled);
	connect(ui->cb_discnumber_all, &QCheckBox::toggled, ui->sb_discnumber, &QWidget::setDisabled);
	connect(ui->cb_rating_all, &QCheckBox::toggled, ui->lab_rating, &QWidget::setDisabled);
	connect(ui->cb_comment_all, &QCheckBox::toggled, ui->te_comment, &QWidget::setDisabled);

	connect(ui->cb_cover_all, &QCheckBox::toggled, this, &GUI_TagEdit::cover_all_changed);

	connect(ui->btn_save, &QPushButton::clicked, this, &GUI_TagEdit::commit);
	connect(ui->btn_undo, &QPushButton::clicked, this, &GUI_TagEdit::undo_clicked);
	connect(ui->btn_undo_all, &QPushButton::clicked, this, &GUI_TagEdit::undo_all_clicked);
	connect(ui->btn_close, &QPushButton::clicked, this, &GUI_TagEdit::sig_cancelled);

	connect(m->tag_edit, &Editor::sig_progress, this, &GUI_TagEdit::progress_changed);
	connect(m->tag_edit, &Editor::sig_metadata_received, this, &GUI_TagEdit::metadata_changed);
	connect(m->tag_edit, &Editor::finished, this, &GUI_TagEdit::commit_finished);

	connect(ui->btn_replacement, &QPushButton::clicked, ui->btn_cover_replacement, &QPushButton::click);
	connect(ui->btn_load_entire_album, &QPushButton::clicked, this, &GUI_TagEdit::load_entire_album);

	connect(m->ui_tag_from_path, &GUI_TagFromPath::sig_apply, this, &GUI_TagEdit::apply_tag_from_path);
	connect(m->ui_tag_from_path, &GUI_TagFromPath::sig_apply_all, this, &GUI_TagEdit::apply_all_tag_from_path);

	metadata_changed(m->tag_edit->metadata());
}


GUI_TagEdit::~GUI_TagEdit() {}

static void set_all_text(QCheckBox* label, int n)
{
	QString text = Lang::get(Lang::All);
	text += QString(" (%1)").arg(n);

	label->setText(text);
}

void GUI_TagEdit::language_changed()
{
	ui->retranslateUi(this);

	ui->lab_track_title->setText(Lang::get(Lang::Title));
	ui->lab_album->setText(Lang::get(Lang::Album));
	ui->lab_artist->setText(Lang::get(Lang::Artist));
	ui->lab_year->setText(Lang::get(Lang::Year));
	ui->lab_genres->setText(Lang::get(Lang::Genres));
	ui->lab_rating_descr->setText(Lang::get(Lang::Rating));
	ui->lab_track_num->setText(Lang::get(Lang::TrackNo).toFirstUpper());
	ui->lab_comment->setText(Lang::get(Lang::Comment));
	ui->btn_load_entire_album->setText(tr("Load complete album"));

	set_all_text(ui->cb_album_all, m->tag_edit->count());
	set_all_text(ui->cb_artist_all, m->tag_edit->count());
	set_all_text(ui->cb_album_artist_all, m->tag_edit->count());
	set_all_text(ui->cb_genre_all, m->tag_edit->count());
	set_all_text(ui->cb_year_all, m->tag_edit->count());
	set_all_text(ui->cb_discnumber_all, m->tag_edit->count());
	set_all_text(ui->cb_rating_all, m->tag_edit->count());
	set_all_text(ui->cb_cover_all, m->tag_edit->count());
	set_all_text(ui->cb_comment_all, m->tag_edit->count());

	ui->btn_undo->setText(Lang::get(Lang::Undo));
	ui->btn_close->setText(Lang::get(Lang::Close));
	ui->btn_save->setText(Lang::get(Lang::Save));

	ui->btn_replacement->setText(Lang::get(Lang::SearchVerb));
	ui->lab_replacement->setText(Lang::get(Lang::Replace));
}

void GUI_TagEdit::commit_finished()
{
	ui->btn_save->setEnabled(true);
}

void GUI_TagEdit::progress_changed(int val)
{
	ui->pb_progress->setVisible(val >= 0);

	if(val >= 0){
		ui->pb_progress->setValue(val);
	}

	if(val < 0){
		metadata_changed(m->tag_edit->metadata() );
	}
}

int GUI_TagEdit::count() const
{
	return m->tag_edit->count();
}

void GUI_TagEdit::set_metadata(const MetaDataList& v_md)
{
	m->tag_edit->set_metadata(v_md);
}

void GUI_TagEdit::metadata_changed(const MetaDataList& md)
{
	Q_UNUSED(md)

	reset();
	language_changed();


	QStringList filepaths;
	const MetaDataList& v_md = m->tag_edit->metadata();
	for(const MetaData& md : v_md)
	{
		filepaths << md.filepath();
	}

	ui->btn_load_entire_album->setVisible(m->tag_edit->can_load_entire_album());

	m->cur_idx = 0;
	refresh_current_track();
}



void GUI_TagEdit::apply_tag_from_path()
{
	m->ui_tag_from_path->clear_invalid_filepaths();

	bool success = m->tag_edit->apply_regex(m->ui_tag_from_path->get_regex_string(), m->cur_idx);
	if(success){
		ui->tab_widget->setCurrentIndex(0);
	}

	refresh_current_track();
}

void GUI_TagEdit::apply_all_tag_from_path()
{
	m->ui_tag_from_path->clear_invalid_filepaths();

	int count = m->tag_edit->count();
	QString regex = m->ui_tag_from_path->get_regex_string();
	QStringList invalid_filepaths;

	for(int i=0; i<count; i++)
	{
		bool success = m->tag_edit->apply_regex(regex, i);
		if(!success)
		{
			QString invalid_filepath = m->tag_edit->metadata(i).filepath();
			invalid_filepaths << invalid_filepath;
			m->ui_tag_from_path->add_invalid_filepath(invalid_filepath);
		}
	}

	if(invalid_filepaths.count() > 0)
	{
		Message::Answer answer = Message::Answer::Yes;

		QStringList not_valid_str;
		not_valid_str << tr("Cannot apply tag for");
		not_valid_str << "";
		not_valid_str << tr("%1 tracks").arg(invalid_filepaths.count()) + ".";
		not_valid_str << tr("Ignore these tracks?");

		answer = Message::question_yn(not_valid_str.join("<br>"));
		if(answer != Message::Answer::Yes)
		{
			m->tag_edit->undo_all();
		}

		else
		{
			ui->tab_widget->setCurrentIndex(0);
		}
	}

	refresh_current_track();
}

bool GUI_TagEdit::check_idx(int idx) const
{
	return Util::between(idx, m->tag_edit->count());
}

void GUI_TagEdit::next_button_clicked()
{
	write_changes(m->cur_idx);

	m->cur_idx++;

	refresh_current_track();
}


void GUI_TagEdit::prev_button_clicked()
{
	write_changes(m->cur_idx);

	m->cur_idx--;

	refresh_current_track();
}

void GUI_TagEdit::refresh_current_track()
{
	int n_tracks = m->tag_edit->count();

	ui->btn_next->setEnabled(m->cur_idx >= 0 && m->cur_idx < n_tracks - 1);
	ui->btn_prev->setEnabled(m->cur_idx > 0 && m->cur_idx < n_tracks);

	if(!check_idx(m->cur_idx)) {
		return;
	}

	MetaData md = m->tag_edit->metadata(m->cur_idx);

	{ // set filepath label
		QString filepath_link = Util::create_link
		(
			md.filepath(),
			Style::is_dark(),
			true,
			Util::File::get_parent_directory(md.filepath())
		);

		ui->lab_filepath->setText(filepath_link);
		m->ui_tag_from_path->set_filepath(md.filepath());

		QFileInfo fi(md.filepath());
		ui->lab_read_only->setVisible(!fi.isWritable());
	}

	ui->le_title->setText(md.title());

	if(!ui->cb_album_all->isChecked()){
		ui->le_album->setText(md.album());
	}

	if(!ui->cb_artist_all->isChecked()){
		ui->le_artist->setText(md.artist());
	}

	if(!ui->cb_album_artist_all->isChecked()){
		ui->le_album_artist->setText(md.album_artist());
	}

	if(!ui->cb_genre_all->isChecked()){
		ui->le_genre->setText( md.genres_to_list().join(", ") );
	}

	if(!ui->cb_year_all->isChecked()){
		ui->sb_year->setValue(md.year);
	}

	if(!ui->cb_discnumber_all->isChecked()){
		ui->sb_discnumber->setValue(md.discnumber);
	}

	if(!ui->cb_rating_all->isChecked()){
		ui->lab_rating->set_rating(md.rating);
	}

	if(!ui->cb_comment_all->isChecked()){
		ui->te_comment->setPlainText(md.comment());
	}

	if(!ui->cb_cover_all->isChecked()){
		set_cover(md);

		bool has_replacement = m->tag_edit->has_cover_replacement(m->cur_idx);
		ui->cb_replace->setChecked(has_replacement);
	}

	bool is_cover_supported = m->tag_edit->is_cover_supported(m->cur_idx);
	ui->tab_cover->setEnabled(is_cover_supported);
	if(!is_cover_supported){
		ui->tab_widget->setCurrentIndex(0);
	}

	ui->sb_track_num->setValue(md.track_num);
	ui->lab_track_index->setText(
		Lang::get(Lang::Track).toFirstUpper().space() +
		QString::number(m->cur_idx+1 ) + "/" + QString::number( n_tracks )
	);
}

void GUI_TagEdit::reset()
{
	m->cur_idx = -1;

	m->ui_tag_from_path->reset();

	ui->cb_album_all->setChecked(false);
	ui->cb_artist_all->setChecked(false);
	ui->cb_album_artist_all->setChecked(false);
	ui->cb_genre_all->setChecked(false);
	ui->cb_discnumber_all->setChecked(false);
	ui->cb_rating_all->setChecked(false);
	ui->cb_year_all->setChecked(false);
	ui->cb_cover_all->setChecked(false);
	ui->cb_comment_all->setChecked(false);

	ui->lab_track_index ->setText(Lang::get(Lang::Track) + " 0/0");
	ui->btn_prev->setEnabled(false);
	ui->btn_next->setEnabled(false);

	ui->le_album->clear();
	ui->le_artist->clear();
	ui->le_album_artist->clear();
	ui->le_title->clear();
	ui->le_genre->clear();
	ui->te_comment->clear();
	ui->sb_year->setValue(0);
	ui->sb_discnumber->setValue(0);
	ui->lab_rating->set_rating(0);
	ui->sb_track_num->setValue(0);
	ui->le_album->setEnabled(true);
	ui->le_artist->setEnabled(true);
	ui->le_album_artist->setEnabled(true);
	ui->le_genre->setEnabled(true);
	ui->sb_year->setEnabled(true);
	ui->sb_discnumber->setEnabled(true);
	ui->lab_rating->setEnabled(true);

	ui->cb_replace->setChecked(false);

	ui->btn_cover_replacement->setEnabled(true);
	show_replacement_field(false);

	QIcon icon(Cover::Location::invalid_location().preferred_path());
	ui->btn_cover_replacement->setIcon( icon );

	ui->lab_filepath->clear();
	ui->pb_progress->setVisible(false);

	ui->btn_load_entire_album->setVisible(false);

	m->cover_path_map.clear();
	init_completer();
}

void GUI_TagEdit::init_completer()
{
	AlbumList albums;
	ArtistList artists;
	QStringList albumstr, artiststr;

	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->library_db(-1, 0);

	lib_db->getAllAlbums(albums, true);
	lib_db->getAllArtists(artists, true);

	for(const Album& album : albums){
		if(!album.name().isEmpty()){
			albumstr << album.name();
		}
	}

	for(const Artist& artist : artists){
		if(!artist.name().isEmpty()){
			artiststr << artist.name();
		}
	}

	if(ui->le_album->completer()){
		ui->le_album->completer()->deleteLater();
	}

	if(ui->le_artist->completer()){
		ui->le_artist->completer()->deleteLater();
	}

	if(ui->le_album_artist->completer()){
		ui->le_album_artist->completer()->deleteLater();
	}

	Gui::Completer* album_completer = new Gui::Completer(albumstr, ui->le_album);
	ui->le_album->setCompleter(album_completer);

	Gui::Completer* album_artist_completer = new Gui::Completer(artiststr, ui->le_album_artist);
	ui->le_album_artist->setCompleter(album_artist_completer);

	Gui::Completer* artist_completer = new Gui::Completer(artiststr, ui->le_artist);
	ui->le_artist->setCompleter(artist_completer);
}


void GUI_TagEdit::cover_all_changed(bool b)
{
	if(!b)
	{
		if(Util::between(m->cur_idx,m->tag_edit->count()) ){
			set_cover(m->tag_edit->metadata(m->cur_idx));
		}
	}

	ui->cb_replace->setEnabled(!b);
	ui->btn_cover_replacement->setEnabled(!b);
	ui->btn_replacement->setEnabled(!b);
}

void GUI_TagEdit::undo_clicked()
{
	m->tag_edit->undo(m->cur_idx);
	refresh_current_track();
}

void GUI_TagEdit::undo_all_clicked()
{
	m->tag_edit->undo_all();
	refresh_current_track();
}


void GUI_TagEdit::write_changes(int idx)
{
	if( !check_idx(idx) ) {
		return;
	}

	MetaData md =m->tag_edit->metadata(idx);

	md.set_title(ui->le_title->text());
	md.set_artist(ui->le_artist->text());
	md.set_album(ui->le_album->text());
	md.set_album_artist(ui->le_album_artist->text());
	md.set_genres(ui->le_genre->text().split(", "));
	md.discnumber = ui->sb_discnumber->value();
	md.year = ui->sb_year->value();
	md.track_num = ui->sb_track_num->value();
	md.rating = ui->lab_rating->get_rating();
	md.set_comment(ui->te_comment->toPlainText());

	m->tag_edit->update_track(idx, md);

	if(is_cover_replacement_active()){
		update_cover(idx, m->cover_path_map[idx]);
	}
}

void GUI_TagEdit::commit()
{
	if(!ui->btn_save->isEnabled()){
		return;
	}

	write_changes(m->cur_idx);

	for(int i=0; i<m->tag_edit->count(); i++)
	{
		if(i == m->cur_idx) continue;

		MetaData md =m->tag_edit->metadata(i);

		if( ui->cb_album_all->isChecked()){
			md.set_album(ui->le_album->text());
		}
		if( ui->cb_artist_all->isChecked()){
			md.set_artist(ui->le_artist->text());
		}
		if( ui->cb_album_artist_all->isChecked()){
			md.set_album_artist(ui->le_album_artist->text());
		}
		if( ui->cb_genre_all->isChecked())
		{
			QStringList genres = ui->le_genre->text().split(", ");
			md.set_genres(genres);
		}

		if( ui->cb_discnumber_all->isChecked() ){
			md.discnumber = ui->sb_discnumber->value();
		}

		if( ui->cb_rating_all->isChecked()){
			md.rating = ui->lab_rating->get_rating();
		}

		if( ui->cb_year_all->isChecked()){
			md.year = ui->sb_year->value();
		}

		if( ui->cb_cover_all->isChecked() ){
			update_cover(i, m->cover_path_map[m->cur_idx]);
		}

		if( ui->cb_comment_all->isChecked() ){
			md.set_comment(ui->te_comment->toPlainText());
		}

		m->tag_edit->update_track(i, md);
	}

	m->tag_edit->commit();
}

void GUI_TagEdit::show_close_button(bool show)
{
	ui->btn_close->setVisible(show);
}


void GUI_TagEdit::show_replacement_field(bool b)
{
	ui->lab_replacement->setVisible(b);
	ui->btn_cover_replacement->setVisible(b);
	ui->btn_replacement->setVisible(b);
	ui->cb_cover_all->setVisible(b);
	ui->cb_cover_all->setChecked(false);
}

bool GUI_TagEdit::is_cover_replacement_active() const
{
	return (ui->cb_replace->isChecked() &&
			ui->btn_cover_replacement->isVisible());
}

void GUI_TagEdit::set_cover(const MetaData& md)
{
	bool has_cover = Tagging::Covers::has_cover(md.filepath());

	if(!has_cover)
	{
		ui->btn_cover_original->setIcon(QIcon());
		ui->btn_cover_original->setText(Lang::get(Lang::None));
	}

	else
	{
		QSize sz = ui->btn_cover_original->size();

		QPixmap pm = Tagging::Covers::extract_cover(md.filepath())
			.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		QIcon icon;
		icon.addPixmap(pm);

		ui->btn_cover_original->setIcon(icon);
		ui->btn_cover_original->setIconSize(sz);
		ui->btn_cover_original->setText(QString());
	}

	Cover::Location cl = Cover::Location::cover_location(md);
	ui->btn_cover_replacement->set_cover_location(cl);

	ui->cb_cover_all->setEnabled(cl.valid());
	ui->btn_cover_replacement->setEnabled(cl.valid() && !ui->cb_cover_all->isChecked());

	if(cl.valid()){
		m->cover_path_map[m->cur_idx] = cl.cover_path();
	}
}

void GUI_TagEdit::update_cover(int idx, const QString& cover_path)
{
	QPixmap img(cover_path);
	m->tag_edit->update_cover(idx, img);
}


void GUI_TagEdit::cb_replace_toggled(bool b)
{
	show_replacement_field(b);
}

void GUI_TagEdit::load_entire_album()
{
	if(m->tag_edit->has_changes())
	{
		Message::Answer answer =
		Message::question(tr("All changes will be lost") + ".\n" + Lang::get(Lang::Continue).question(), "GUI_TagEdit", Message::QuestionType::YesNo);

		if(answer == Message::Answer::No){
			return;
		}
	}

	m->tag_edit->load_entire_album();
}

void GUI_TagEdit::showEvent(QShowEvent *e)
{
	Widget::showEvent(e);
	refresh_current_track();
}
