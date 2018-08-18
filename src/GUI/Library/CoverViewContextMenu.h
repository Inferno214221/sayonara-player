#ifndef COVERVIEWCONTEXTMENU_H
#define COVERVIEWCONTEXTMENU_H

#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"
#include "Utils/Library/Sortorder.h"
#include <QStringList>

class MetaData;
struct ActionPair;

class CoverViewContextMenu :
		public LibraryContextMenu
{
	Q_OBJECT
	PIMPL(CoverViewContextMenu)

signals:
	void sig_zoom_changed(int zoom);
	void sig_sorting_changed(Library::SortOrder sortorder);

public:
	enum Entry
	{
		EntryShowUtils=(LibraryContextMenu::EntryLast << 1),
		EntrySorting=(EntryShowUtils << 1),
		EntryZoom=(EntrySorting << 1),
		EntryShowArtist=(EntryZoom << 1)
	};

	using Entries=LibraryContextMenu::Entries;

	explicit CoverViewContextMenu(QWidget* parent);
	~CoverViewContextMenu();

	CoverViewContextMenu::Entries get_entries() const override;
	void show_actions(CoverViewContextMenu::Entries entries) override;

protected:
	void showEvent(QShowEvent* e) override;

private:
	void language_changed() override;
	void skin_changed() override;

	void init();
	void init_sorting_actions();
	void init_zoom_actions();

	void set_zoom(int zoom);
	void set_sorting(Library::SortOrder so);

private slots:
	void action_zoom_triggered(bool b);
	void action_sorting_triggered(bool b);


};
#endif // COVERVIEWCONTEXTMENU_H
