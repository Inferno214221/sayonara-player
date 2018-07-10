#ifndef COVERVIEW_H
#define COVERVIEW_H

#include "GUI/Library/ItemView.h"
#include "GUI/Library/Utils/ActionPair.h"
#include "Utils/Library/Sortorder.h"

class LocalLibrary;
class ActionPair;
class QAction;

class CoverView :
		public Library::ItemView
{
	Q_OBJECT
	PIMPL(CoverView)

signals:
	void sig_sortorder_changed(Library::SortOrder so);
	void sig_zoom_changed(int);

public:
	explicit CoverView(QWidget* parent=nullptr);
	virtual ~CoverView();

	void init(LocalLibrary* library);

	QList<ActionPair> sorting_options() const;
	QStringList zoom_actions() const;

	void change_zoom(int zoom=-1);
	void change_sortorder(Library::SortOrder so);

	void init_sorting_actions();
	void init_zoom_actions();

protected:
	//SayonaraSelectionView
	int index_by_model_index(const QModelIndex& idx) const override;
	QModelIndex model_index_by_index(int idx) const override;

	void init_context_menu() override;

	void double_clicked(const QModelIndex& index);
	void middle_clicked() override;
	void play_next_clicked() override;
	void append_clicked() override;
	void selection_changed(const IndexSet& indexes) override;

	void language_changed() override;
	QStyleOptionViewItem viewOptions() const override;

	void wheelEvent(QWheelEvent* e) override;
	void resizeEvent(QResizeEvent *e) override;

private slots:
	void show_utils_triggered(bool b);
	void albums_ready();
	void action_sortorder_triggered();
	void action_zoom_triggered();
	void cover_changed();
	void timed_out();

private:
	void refresh();
};

#endif // COVERVIEW_H
