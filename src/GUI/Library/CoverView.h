#ifndef COVERVIEW_H
#define COVERVIEW_H

#include "GUI/Library/ItemView.h"
#include "GUI/Library/Utils/ActionPair.h"
#include "Utils/Library/Sortorder.h"

class LocalLibrary;
struct ActionPair;
class QAction;

namespace Library
{
	class CoverView :
			public ItemView
	{
		Q_OBJECT
		PIMPL(CoverView)

	public:
		explicit CoverView(QWidget* parent=nullptr);
		virtual ~CoverView();

		void init(LocalLibrary* library);
		AbstractLibrary* library() const override;

		// QAbstractItemView
		QStyleOptionViewItem viewOptions() const override;

		//SayonaraSelectionView
		int index_by_model_index(const QModelIndex& idx) const override;
		ModelIndexRange model_indexrange_by_index(int idx) const override;

		void change_zoom(int zoom=-1);
		void change_sortorder(SortOrder so);

		static QList<ActionPair> sorting_actions();
		static QStringList zoom_actions();

	protected:
		void init_context_menu() override;

		void language_changed() override;
		void wheelEvent(QWheelEvent* e) override;
		void resizeEvent(QResizeEvent* e) override;
		void hideEvent(QHideEvent* e) override;

	private slots:
		void timer_timed_out();
		void reload();

	private:
		void timer_start();

		// Library::ItemView
		void play_clicked() override;
		void play_new_tab_clicked() override;
		void play_next_clicked() override;
		void append_clicked() override;
		void selection_changed(const IndexSet& indexes) override;
		void refresh_clicked() override;
		void run_merge_operation(const MergeData& mergedata) override;
	};
}

#endif // COVERVIEW_H
