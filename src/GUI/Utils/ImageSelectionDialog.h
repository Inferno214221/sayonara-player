#ifndef IMAGESELECTIONDIALOG_H
#define IMAGESELECTIONDIALOG_H

#include "GUI/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"

#include <QFileDialog>

class ImageSelectionDialog :
        public Gui::WidgetTemplate<QFileDialog>
{
	Q_OBJECT
	PIMPL(ImageSelectionDialog)

public:
	ImageSelectionDialog(const QString& dir, QWidget* parent=nullptr);
	~ImageSelectionDialog();

private slots:
	void file_selected(const QString& file);
};

#endif // IMAGESELECTIONDIALOG_H
