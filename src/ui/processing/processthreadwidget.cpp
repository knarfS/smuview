/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QWidget>

#include "processthreadwidget.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/processing/processor.hpp"
#include "src/processing/stepblock.hpp"
#include "src/ui/processing/dialogs/stepblockdialog.hpp"
#include "src/ui/processing/items/sequencesinitem.hpp"
#include "src/ui/processing/items/stepitem.hpp"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::unordered_set;

namespace sv {
namespace ui {
namespace processing {

ProcessThreadWidget::ProcessThreadWidget(shared_ptr<Session> session,
		QString name, shared_ptr<sv::processing::Processor> processor,
		QWidget *parent) :
	QMainWindow(parent),
	session_(session),
	name_(name),
	processor_(processor),
	action_add_block_(new QAction(this))
{
	setup_ui();
	setup_toolbar();
}

void ProcessThreadWidget::setup_ui()
{
	QVBoxLayout *main_layout = new QVBoxLayout();

	QFormLayout *form_layout = new QFormLayout();

	name_edit_ = new QLineEdit();
	name_edit_->setText(name_);
	form_layout->addRow(tr("Name"), name_edit_);

	run_at_startup_checkbox_ = new QCheckBox();
	form_layout->addRow(tr("Run at startup"), run_at_startup_checkbox_);

	main_layout->addLayout(form_layout);

	process_block_list_ = new QListWidget();
	process_block_list_->setIconSize(QSize(32, 32));
	main_layout->addWidget(process_block_list_);

	QWidget *main_widget = new QWidget();
	main_widget->setLayout(main_layout);
	this->setCentralWidget(main_widget);
}

void ProcessThreadWidget::setup_toolbar()
{
	action_add_block_->setText(tr("Add block"));
	action_add_block_->setIcon(
		QIcon::fromTheme("list-add",
		QIcon(":/icons/list-add.png")));
	connect(action_add_block_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_block_triggered()));

	toolbar_ = new QToolBar("Thread Toolbar");
	toolbar_->addAction(action_add_block_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void ProcessThreadWidget::on_action_add_block_triggered()
{

	dialogs::StepBlockDialog dlg(session_, nullptr);
	dlg.exec();

	/*
	for (auto signal : dlg.signals())
		add_time_curve(dynamic_pointer_cast<data::AnalogSignal>(signal));
	*/

	/*
	ui::processing::items::StepItem *item =
		new ui::processing::items::StepItem();
	process_block_list_->addItem(item);

	// DEMO: Get some device
	unordered_set<shared_ptr<devices::BaseDevice>> devices =
		session_->devices();
	for (auto device : devices) {
		if (auto hw_device = dynamic_pointer_cast<devices::HardwareDevice>(device)) {
			// DEMO: Get some configurabel
			for (auto configurable : hw_device->configurables()) {
				shared_ptr<sv::processing::StepBlock> block =
					make_shared<sv::processing::StepBlock>();
				block->set_configurable(configurable);
				block->set_config_key(sigrok::ConfigKey::AMPLITUDE);
				processor_->add_block_to_process(block);

				break;
			}
			break;
		}
	}
	*/
}

} // namespace processing
} // namespace ui
} // namespace sv
