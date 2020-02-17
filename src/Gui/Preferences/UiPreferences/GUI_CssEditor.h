#ifndef GUI_CSSEDITOR_H
#define GUI_CSSEDITOR_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_CssEditor)

class GUI_CssEditor : public
	Gui::Dialog
{
		Q_OBJECT
		PIMPL(GUI_CssEditor)
		UI_CLASS(GUI_CssEditor)

	public:
		explicit GUI_CssEditor(QWidget* parent = nullptr);
		~GUI_CssEditor();

	private slots:
		void saveClicked();
		void applyClicked();
		void undoClicked();
		void darkModeToggled(bool b);

	protected:
		void showEvent(QShowEvent* e) override;
		void languageChanged() override;
};

#endif // GUI_CSSEDITOR_H
