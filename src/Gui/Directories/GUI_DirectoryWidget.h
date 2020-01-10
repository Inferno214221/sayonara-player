/* GUI_DirectoryWidget.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef GUI_DIRECTORYWIDGET_H
#define GUI_DIRECTORYWIDGET_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Gui/InfoDialog/InfoDialogContainer.h"
#include "Utils/Pimpl.h"

class QPoint;
class QFrame;
class QComboBox;

namespace Library
{
	class Info;
}

UI_FWD(GUI_DirectoryWidget)

/**
 * @brief The GUI_DirectoryWidget class
 * @ingroup GuiDirectories
 */
class GUI_DirectoryWidget :
		public Gui::Widget,
		public InfoDialogContainer
{
	Q_OBJECT
	PIMPL(GUI_DirectoryWidget)
	UI_CLASS(GUI_DirectoryWidget)

	public:
		explicit GUI_DirectoryWidget(QWidget* parent=nullptr);
		~GUI_DirectoryWidget() override;

		QFrame* header_frame() const;

	private:
		void init_shortcuts();
		void init_library_combobox();

	private slots:
		void search_button_clicked();
		void search_text_edited(const QString& text);

		void dir_enter_pressed();
		void dir_opened(QModelIndex idx);
		void dir_pressed(QModelIndex idx);
		void dir_clicked(QModelIndex idx);
		void dir_append_clicked();
		void dir_play_clicked();
		void dir_play_next_clicked();
		void dir_play_new_tab_clicked();
		void dir_delete_clicked();
		void dir_copy_requested(const QStringList& files, const QString& target);
		void dir_move_requested(const QStringList& files, const QString& target);
		void dir_rename_requested(const QString& old_name, const QString& new_name);

		void file_dbl_clicked(QModelIndex idx);
		void file_enter_pressed();
		void file_pressed(QModelIndex idx);
		void file_append_clicked();
		void file_play_clicked();
		void file_play_next_clicked();
		void file_play_new_tab_clicked();
		void file_delete_clicked();
		void file_rename_requested(const QString& old_name, const QString& new_name);
		void file_rename_by_expression_requested(const QString& old_name, const QString& expression);

		void file_operation_started();
		void file_operation_finished();

		void import_requested(LibraryId library_id, const QStringList& paths, const QString& target_dir);
		void import_dialog_requested(const QString& target_dir);

		void splitter_moved(int pos, int index);

		void set_library_path_clicked();
		void check_libraries();

		void current_library_changed(int index);



	protected:
		MD::Interpretation metadata_interpretation() const override;
		MetaDataList info_dialog_data() const override;
		bool has_metadata() const override;
		QStringList pathlist() const override;

		void language_changed() override;
		void skin_changed() override;
};

#endif // GUI_DIRECTORYWIDGET_H
