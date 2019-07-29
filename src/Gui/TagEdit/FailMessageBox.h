#ifndef FAILMESSAGEBOX_H
#define FAILMESSAGEBOX_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Components/Tagging/Editor.h"

#include <QMap>
#include <QString>

UI_FWD(GUI_FailMessageBox)

class FailMessageBox : public Gui::Dialog
{
	Q_OBJECT
	UI_CLASS(GUI_FailMessageBox)

public:
	FailMessageBox(QWidget* parent=nullptr);
	~FailMessageBox()=default;

	void set_failed_files(const QMap<QString, Tagging::Editor::FailReason>& failed_files);

private slots:
	void details_toggled(bool b);

protected:
	void language_changed();
	void showEvent(QShowEvent* e) override;
};

#endif // FAILMESSAGEBOX_H
