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
class GUI_TagEdit;

/**
 * @brief The GUI_CoverEdit class
 * @ingroup Tagging
 */
class GUI_CoverEdit :
	public Gui::Widget
{
	Q_OBJECT
	PIMPL(GUI_CoverEdit)
	UI_CLASS(GUI_CoverEdit)

	public:
		/**
		 * @brief GUI_CoverEdit
		 * @param editor The same tag editor as used in GUI_TagEdit
		 * @param parent
		 */
		explicit GUI_CoverEdit(GUI_TagEdit* parent);
		~GUI_CoverEdit();

		/**
		 * @brief Shows the current cover (if there) and offers to replace it
		 * Every other widget is hidden
		 */
		void reset();

		/**
		 * @brief refetches the track from the tag editor
		 * and sets the cover to the left button
		 */
		void refresh_current_track();

		/**
		 * @brief sets the current index for a track which is currently processed
		 * @param index
		 */
		void set_current_index(int index);

		/**
		 * @brief returns the new cover for a current track.
		 * @param index
		 * @return empty pixmap if index is invalid, or no new cover is desired a track
		 */
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
