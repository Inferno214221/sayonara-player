#ifndef CONTAINERINTERFACE_H
#define CONTAINERINTERFACE_H

class QWidget;
class QFrame;
class QPixmap;
class QMenu;
class QAction;

#include <QObject>

namespace Library
{
	/**
	 * @brief This container is only used internally to avoid Ui dependencies
	 * in the Components. If implementing your own library plugin, use
	 * Gui/Library/LibraryContainer.h instead
	 */
	class AbstractContainer
	{
		public:

			virtual ~AbstractContainer()=default;
			/**
			 * @brief Should return an untranslated name used for identifying this widget
			 * @return name
			 */
			virtual QString				name() const=0;

			virtual void				rename(const QString& newName)=0;

			/**
			 * @brief Should return the translated name displayed in the library view combobox
			 * @return display name
			 */
			virtual QString				displayName() const=0;

			/**
			 * @brief Should return the UI for the library view
			 * @return pointer to the ui
			 */
			virtual QWidget*			widget() const=0;

			/**
			 * @brief this is a frame at the top left of the container
			 * where the combo box will be located
			 * @return
			 */
			virtual QFrame*				header() const=0;

			/**
			 * @brief return actions menu (may be nullptr). The title does not have to be set
			 * @return the translated menu relevant for the corresponding library
			 */
			virtual QMenu*				menu()=0;

			/**
			 * @brief Every library should show a icon in the
			 * combo box
			 * @return
			 */
			virtual QPixmap				icon() const=0;

			/**
			 * @brief init
			 */
			virtual void				init()=0;

			/**
			 * @brief a local library is a library which writes to the
			 * library field of the database. This should be false for
			 * every new plugin
			 * @return
			 */
			virtual bool				isLocal() const=0;
	};
}

using LibraryContainerInterface=Library::AbstractContainer;
Q_DECLARE_INTERFACE(LibraryContainerInterface, "com.sayonara-player.library")

#endif // CONTAINERINTERFACE_H
