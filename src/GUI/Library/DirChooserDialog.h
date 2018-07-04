#ifndef DIRCHOOSERDIALOG_H
#define DIRCHOOSERDIALOG_H

#include <QFileDialog>

class DirChooserDialog : public QFileDialog
{
	Q_OBJECT

public:
	explicit DirChooserDialog(QWidget* parent=nullptr);
	~DirChooserDialog();
};

#endif // DIRCHOOSERDIALOG_H
