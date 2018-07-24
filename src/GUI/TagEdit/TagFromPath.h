#ifndef TAGFROMPATH_H
#define TAGFROMPATH_H

#include "GUI/Utils/Widgets/Widget.h"
#include "GUI/Utils/GuiClass.h"
#include "Utils/Pimpl.h"

#include "Components/Tagging/Expression.h"

UI_FWD(GUI_TagFromPath)

class QPushButton;

namespace Tagging
{
	class Editor;
}
class MetaData;
class GUI_TagFromPath :
		public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(GUI_TagFromPath)
	PIMPL(GUI_TagFromPath)

signals:
	void sig_apply();
	void sig_apply_all();


public:
	GUI_TagFromPath(QWidget* parent=nullptr);
	~GUI_TagFromPath();

	void set_filepath(const QString& filepath);
	void add_invalid_filepath(const QString& filepath);
	void clear_invalid_filepaths();
	QString get_regex_string() const;

	void reset();

protected:
	void language_changed();

private:
	/**
	 * @brief sets red if not valid
	 * @param valid if tag is valid or not
	 */
	void set_tag_colors(bool valid);
	bool replace_selected_tag_text(Tagging::TagName t, bool b);
	void btn_checked(QPushButton* btn, bool b, Tagging::TagName tag_name);
	void show_error_frame(bool b);

private slots:

	/**
	 * @brief calls webpage with help
	 */
	void btn_tag_help_clicked();

	/**
	 * @brief tries to apply the tag
	 */
	void tag_text_changed(const QString&);

	void btn_title_checked(bool b);
	void btn_artist_checked(bool b);
	void btn_album_checked(bool b);
	void btn_track_nr_checked(bool b);
	void btn_disc_nr_checked(bool b);
	void btn_year_checked(bool b);
};

#endif // TAGFROMPATH_H
