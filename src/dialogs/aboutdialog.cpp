/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Soeren Apel <soeren@apelpie.net>
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <boost/version.hpp>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QTextDocument>

#include "aboutdialog.hpp"
#include "config.h"

namespace sv {
namespace dialogs {

AboutDialog::AboutDialog(DeviceManager &device_manager, QWidget *parent) :
	QDialog(parent),
	device_manager_(device_manager)
{
	const int icon_size = 64;

	resize(600, 400);

	page_list = new QListWidget;
	page_list->setViewMode(QListView::IconMode);
	page_list->setIconSize(QSize(icon_size, icon_size));
	page_list->setMovement(QListView::Static);
	page_list->setMaximumWidth(icon_size + (icon_size / 2) + 2);
	page_list->setSpacing(12);

	pages = new QStackedWidget;
	create_pages();
	page_list->setCurrentIndex(page_list->model()->index(0, 0));

	QHBoxLayout *tab_layout = new QHBoxLayout;
	tab_layout->addWidget(page_list);
	tab_layout->addWidget(pages, Qt::AlignLeft);

	QDialogButtonBox *button_box = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* root_layout = new QVBoxLayout(this);
	root_layout->addLayout(tab_layout);
	root_layout->addWidget(button_box);

	connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));
	connect(page_list, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
		this, SLOT(on_page_changed(QListWidgetItem*, QListWidgetItem*)));
}

void AboutDialog::create_pages()
{
	// About page
	pages->addWidget(get_about_page(pages));

	QListWidgetItem *aboutButton = new QListWidgetItem(page_list);
	aboutButton->setIcon(QIcon(":/icons/information.svg"));
	aboutButton->setText(tr("About"));
	aboutButton->setTextAlignment(Qt::AlignHCenter);
	aboutButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	// View page
	/*
	pages->addWidget(get_device_page(pages));

	QListWidgetItem *viewButton = new QListWidgetItem(page_list);
	viewButton->setIcon(QIcon(":/icons/settings-views.svg"));
	viewButton->setText(tr("Views"));
	viewButton->setTextAlignment(Qt::AlignHCenter);
	viewButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	*/
}

QWidget *AboutDialog::get_about_page(QWidget *parent) const
{
	QLabel *icon = new QLabel();
	icon->setPixmap(QPixmap(QString::fromUtf8(":/icons/pulseview.svg")));

	/* Setup the version field */
	QLabel *version_info = new QLabel();
	version_info->setText(tr("%1 %2<br />%3<br /><a href=\"http://%4\">%4</a>")
		.arg(QApplication::applicationName(),
		QApplication::applicationVersion(),
		tr("GNU GPL, version 3 or later"),
		QApplication::organizationDomain()));
	version_info->setOpenExternalLinks(true);

	shared_ptr<sigrok::Context> context = device_manager_.context();

	QString s;

	s.append("<style type=\"text/css\"> tr .id { white-space: pre; padding-right: 5px; } </style>");

	s.append("<table>");

	/* Library info */
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("Libraries and features:") + "</b></td></tr>");

	s.append(QString("<tr><td><i>%1</i></td><td>%2</td></tr>")
		.arg(QString("Qt"), qVersion()));
	s.append(QString("<tr><td><i>%1</i></td><td>%2</td></tr>")
		.arg(QString("glibmm"), SV_GLIBMM_VERSION));
	s.append(QString("<tr><td><i>%1</i></td><td>%2</td></tr>")
		.arg(QString("Boost"), BOOST_LIB_VERSION));

	s.append(QString("<tr><td><i>%1</i></td><td>%2/%3 (rt: %4/%5)</td></tr>")
		.arg(QString("libsigrok"), SR_PACKAGE_VERSION_STRING,
		SR_LIB_VERSION_STRING, sr_package_version_string_get(),
		sr_lib_version_string_get()));

	GSList *l_orig = sr_buildinfo_libs_get();
	for (GSList *l = l_orig; l; l = l->next) {
		GSList *m = (GSList *)l->data;
		const char *lib = (const char *)m->data;
		const char *version = (const char *)m->next->data;
		s.append(QString("<tr><td><i>- %1</i></td><td>%2</td></tr>")
			.arg(QString(lib), QString(version)));
		g_slist_free_full(m, g_free);
	}
	g_slist_free(l_orig);

	char *host = sr_buildinfo_host_get();
	s.append(QString("<tr><td><i>- Host</i></td><td>%1</td></tr>")
		.arg(QString(host)));
	g_free(host);

	char *scpi_backends = sr_buildinfo_scpi_backends_get();
	s.append(QString("<tr><td><i>- SCPI backends</i></td><td>%1</td></tr>")
		.arg(QString(scpi_backends)));
	g_free(scpi_backends);

	/* Set up the supported field */
	s.append("<tr><td colspan=\"2\"></td></tr>");
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("Supported hardware drivers:") + "</b></td></tr>");
	for (auto entry : context->drivers()) {
		s.append(QString("<tr><td class=\"id\"><i>%1</i></td><td>%2</td></tr>")
			.arg(QString::fromUtf8(entry.first.c_str()),
				QString::fromUtf8(entry.second->long_name().c_str())));
	}

	// No need for input formats
	/*
	s.append("<tr><td colspan=\"2\"></td></tr>");
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("Supported input formats:") + "</b></td></tr>");
	for (auto entry : context->input_formats()) {
		s.append(QString("<tr><td class=\"id\"><i>%1</i></td><td>%2</td></tr>")
			.arg(QString::fromUtf8(entry.first.c_str()),
				QString::fromUtf8(entry.second->description().c_str())));
	}
	*/

	// No need for output formats
	/*
	s.append("<tr><td colspan=\"2\"></td></tr>");
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("Supported output formats:") + "</b></td></tr>");
	for (auto entry : context->output_formats()) {
		s.append(QString("<tr><td class=\"id\"><i>%1</i></td><td>%2</td></tr>")
			.arg(QString::fromUtf8(entry.first.c_str()),
				QString::fromUtf8(entry.second->description().c_str())));
	}
	*/

	s.append("</table>");

	QTextDocument *supported_doc = new QTextDocument();
	supported_doc->setHtml(s);

	QTextBrowser *support_list = new QTextBrowser();
	support_list->setDocument(supported_doc);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(icon, 0, 0, 1, 1);
	layout->addWidget(version_info, 0, 1, 1, 1);
	layout->addWidget(support_list, 1, 1, 1, 1);

	QWidget *page = new QWidget(parent);
	page->setLayout(layout);

	return page;
}

QWidget *AboutDialog::get_device_page(QWidget *parent) const
{
	(void)parent;
	return nullptr;
}

void AboutDialog::on_page_changed(
	QListWidgetItem *current, QListWidgetItem *previous)
{
	if (!current)
		current = previous;

	pages->setCurrentIndex(page_list->row(current));
}

} // namespace dialogs
} // namespace sv
