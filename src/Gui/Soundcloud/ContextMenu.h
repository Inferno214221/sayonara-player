#ifndef SOUNDCLOUDCONTEXTMENU_H
#define SOUNDCLOUDCONTEXTMENU_H

#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"

namespace SC
{
	class ContextMenu : public Library::ContextMenu
	{
		Q_OBJECT
		PIMPL(ContextMenu)

		signals:
			void sigAddArtistTriggered();

		public:
			enum SCEntry
			{
				SCEntryAddArtist=Library::ContextMenu::EntryLast
			};

			using Entries=uint64_t;

			explicit ContextMenu(QWidget* parent=nullptr);
			~ContextMenu() override;

			// WidgetTemplateParent interface
		protected:
			void languageChanged() override;

			// ContextMenu interface
		public:
			ContextMenu::Entries entries() const override;
			void showActions(ContextMenu::Entries entries) override;
			void showAction(ContextMenu::Entry entry, bool visible) override;
	};

}

#endif // SOUNDCLOUDCONTEXTMENU_H
