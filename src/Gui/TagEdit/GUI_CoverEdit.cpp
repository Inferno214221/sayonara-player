#include "GUI_CoverEdit.h"
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

struct GUI_CoverEdit::Private
{
	Editor*				tag_edit=nullptr;
	QMap<int, QString>	cover_path_map;

	int cur_idx;

	Private(Editor* editor) :
		tag_edit(editor),
		cur_idx(0)
	{}
};

GUI_CoverEdit::GUI_CoverEdit(Tagging::Editor* editor, QWidget* parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>(editor);
	ui = new Ui::GUI_CoverEdit();
	ui->setupUi(this);

	connect(m->tag_edit, &Editor::sig_metadata_received, this, &GUI_CoverEdit::set_metadata);
	connect(ui->cb_cover_all, &QCheckBox::toggled, this, &GUI_CoverEdit::cover_all_toggled);
	connect(ui->btn_search, &QPushButton::clicked, ui->btn_cover_replacement, &QPushButton::click);
	connect(ui->btn_replace, &QPushButton::toggled, this, &GUI_CoverEdit::replace_toggled);
}

GUI_CoverEdit::~GUI_CoverEdit() = default;

void GUI_CoverEdit::reset()
{
	ui->btn_replace->setChecked(false);
	ui->cb_cover_all->setChecked(false);
	ui->btn_cover_replacement->setEnabled(true);
	show_replacement_field(false);

	ui->btn_cover_replacement->set_cover_location(Cover::Location());

	m->cover_path_map.clear();

}

void GUI_CoverEdit::set_metadata(const MetaDataList& v_md)
{
	Q_UNUSED(v_md)
	refresh_current_track();
}

void GUI_CoverEdit::set_current_index(int index)
{
	m->cur_idx = index;
}

QPixmap GUI_CoverEdit::selected_cover(int index) const
{
	if(!is_cover_replacement_active()) {
		return QPixmap();
	}

	QString cover_path;
	if(ui->cb_cover_all->isChecked()) {
		cover_path = m->cover_path_map[m->cur_idx];
	}

	else {
		cover_path = m->cover_path_map[index];
	}

	if(cover_path.isEmpty()) {
		return QPixmap();
	}

	return QPixmap(cover_path);
}

void GUI_CoverEdit::refresh_current_track()
{
	MetaData md = m->tag_edit->metadata(m->cur_idx);
	set_cover(md);

	if(!ui->cb_cover_all->isChecked())
	{
		bool has_replacement = m->tag_edit->has_cover_replacement(m->cur_idx);
		ui->btn_cover_replacement->setChecked(has_replacement);
	}
}

void GUI_CoverEdit::show_replacement_field(bool b)
{
	ui->lab_replacement->setVisible(b);
	ui->btn_cover_replacement->setVisible(b);
	ui->btn_search->setVisible(b);
	ui->cb_cover_all->setVisible(b);
	ui->cb_cover_all->setChecked(false);
}


void GUI_CoverEdit::set_cover(const MetaData& md)
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

	if(cl.valid()) {
		m->cover_path_map[m->cur_idx] = cl.cover_path();
	}
}

void GUI_CoverEdit::replace_toggled(bool b)
{
	show_replacement_field(b);
}

void GUI_CoverEdit::cover_all_toggled(bool b)
{
	if(!b)
	{
		if(Util::between(m->cur_idx, m->tag_edit->count()) ) {
			set_cover(m->tag_edit->metadata(m->cur_idx));
		}
	}

	ui->btn_replace->setEnabled(!b);
	ui->btn_cover_replacement->setEnabled(!b);
	ui->btn_search->setEnabled(!b);
}

bool GUI_CoverEdit::is_cover_replacement_active() const
{
	return (ui->btn_replace->isChecked() &&
			ui->btn_cover_replacement->isVisible());
}

void GUI_CoverEdit::language_changed()
{
	QString text = QString("%1 (%2)")
		.arg(Lang::get(Lang::All))
		.arg(m->tag_edit->count());

	ui->cb_cover_all->setText(text);
	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	ui->lab_replacement->setText(Lang::get(Lang::Replace));
}
