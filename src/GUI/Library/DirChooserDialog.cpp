#include "DirChooserDialog.h"
#include <QDir>
#include <QList>
#include <QUrl>
#include <QStandardPaths>
#include <QStringList>
#include <QListView>
#include <QTreeView>
#include "Utils/Language.h"

DirChooserDialog::DirChooserDialog(QWidget* parent) :
	QFileDialog(parent)
{
	this->setDirectory(QDir::homePath());
	this->setWindowTitle(Lang::get(Lang::ImportDir));
	this->setFileMode(QFileDialog::DirectoryOnly);
	this->setOption(QFileDialog::DontUseNativeDialog, true);

	QList<QUrl> sidebar_urls = this->sidebarUrls();

	const QList<QStandardPaths::StandardLocation> locations {
		QStandardPaths::HomeLocation,
		QStandardPaths::DesktopLocation,
		QStandardPaths::DownloadLocation,
		QStandardPaths::MusicLocation,
		QStandardPaths::TempLocation
	};

	for(const QStandardPaths::StandardLocation& location : locations)
	{
		QStringList std_locations = QStandardPaths::standardLocations(location);
		for(const QString& std_location : std_locations)
		{
			QUrl url = QUrl::fromLocalFile(std_location);
			if(sidebar_urls.contains(url)){
				continue;
			}

			sidebar_urls << url;
		}
	}

	this->setSidebarUrls(sidebar_urls);

	QListView* list_view = this->findChild<QListView*>("listView");
	if(list_view != nullptr)
	{
		list_view->setSelectionMode(QAbstractItemView::MultiSelection);
		QTreeView* tree_view = this->findChild<QTreeView*>();
		if(tree_view){
			tree_view->setSelectionMode(QAbstractItemView::MultiSelection);
		}
	}
}

DirChooserDialog::~DirChooserDialog() {}
