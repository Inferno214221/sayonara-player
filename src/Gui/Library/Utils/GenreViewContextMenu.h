#ifndef GENREVIEWCONTEXTMENU_H
#define GENREVIEWCONTEXTMENU_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/ContextMenu/ContextMenu.h"

namespace Library
{
	/**
	 * @brief Context Menu for the tree view.
	 * Notification of the tree view action is done
	 * by using a bool setting listener to Set::Lib_GenreTree
	 * So there's not signal for it
	 */
	class GenreViewContextMenu :
		public ContextMenu
	{
		Q_OBJECT
		PIMPL(GenreViewContextMenu)

		public:
			GenreViewContextMenu(QWidget* parent=nullptr);
			~GenreViewContextMenu();

		private slots:
			void toggle_tree_triggered();

		protected:
			void language_changed() override;
	};
}

#endif // GENREVIEWCONTEXTMENU_H
