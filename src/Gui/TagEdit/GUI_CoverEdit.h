#ifndef GUI_COVEREDIT_H
#define GUI_COVEREDIT_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_CoverEdit)

namespace Tagging
{
	class Editor;
}

class MetaData;
class MetaDataList;
class QPixmap;

class GUI_CoverEdit :
	public Gui::Widget
{
	Q_OBJECT
	PIMPL(GUI_CoverEdit)
	UI_CLASS(GUI_CoverEdit)

	public:
		explicit GUI_CoverEdit(Tagging::Editor* editor, QWidget* parent=nullptr);
		~GUI_CoverEdit();

		void reset();
		void refresh_current_track();
		void set_current_index(int index);
		QPixmap selected_cover(int index) const;

	private:
		void set_cover(const MetaData& md);
		void show_replacement_field(bool b);
		bool is_cover_replacement_active() const;

	protected:
		void language_changed() override;

	private slots:
		/**
		 * @brief When button has finished setting up its button
		 */
		void cover_changed();
		void replace_toggled(bool b);
		void cover_all_toggled(bool b);
		void set_metadata(const MetaDataList& v_md);
};

#endif // GUI_COVEREDIT_H
