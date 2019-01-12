/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>

#include <QDebug>
#include <QHBoxLayout>
#include <QPoint>
#include <QToolBar>

#include <ConnectionStyle>
#include <DataModelRegistry>
#include <FlowScene>
#include <FlowView>
#include <Node>

#include "flowview.hpp"
#include "src/session.hpp"
#include "src/flow/analogsourcedatamodel.hpp"
#include "src/flow/doubleconfigsinkdatamodel.hpp"
#include "src/flow/rampsourcedatamodel.hpp"
#include "src/flow/startdatamodel.hpp"
#include "src/flow/nodes/analogsignalsinkdatamodel.hpp"
#include "src/ui/widgets/popup.hpp"
#include "src/ui/widgets/flow/nodehelper.hpp"

using std::make_shared;
using std::shared_ptr;
using QtNodes::Node;

namespace sv {
namespace ui {
namespace views {

FlowView::FlowView(const Session &session, QWidget *parent) :
		BaseView(session, parent),
	action_start_(new QAction(this)),
	action_pause_(new QAction(this)),
	action_stop_(new QAction(this)),
	action_load_(new QAction(this)),
	action_save_(new QAction(this))
{
	setup_ui();
	setup_toolbar();
}


static void set_style()
{
	QtNodes::ConnectionStyle::setConnectionStyle(
	R"(
	{
		"ConnectionStyle": {
			"ConstructionColor": "gray",
			"NormalColor": "black",
			"SelectedColor": "gray",
			"SelectedHaloColor": "deepskyblue",
			"HoveredColor": "deepskyblue",

			"LineWidth": 3.0,
			"ConstructionLineWidth": 2.0,
			"PointDiameter": 10.0,

			"UseDataDefinedColors": true
		}
	}
	)");
}

static shared_ptr<QtNodes::DataModelRegistry> register_data_models()
{
	auto registry = make_shared<QtNodes::DataModelRegistry>();

	// TODO: tr("")
	registry->registerModel<sv::flow::StartDataModel>("Logic");

	registry->registerModel<sv::flow::AnalogSourceDataModel>("Sources");
	registry->registerModel<sv::flow::RampSourceDataModel>("Sources");

	registry->registerModel<sv::flow::nodes::AnalogSignalSinkDataModel>("Sinks");
	registry->registerModel<sv::flow::DoubleConfigSinkDataModel>("Sinks");

	/*
	registry->registerTypeConverter(
		std::make_pair(DecimalData().type(),
		IntegerData().type()),
		TypeConverter { DecimalToIntegerConverter() });
	*/

	return registry;
}

QString FlowView::title() const
{
	return tr("Control Flow");
}

void FlowView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	set_style();

	flow_scene_ = new QtNodes::FlowScene(register_data_models());
	connect(flow_scene_, SIGNAL(nodeDoubleClicked(Node &, QPoint)),
		this, SLOT(show_node_popup(Node &, QPoint)));

	flow_view_widget_ = new QtNodes::FlowView(flow_scene_);
	layout->addWidget(flow_view_widget_);

	this->central_widget_->setLayout(layout);
}

void FlowView::setup_toolbar()
{
	action_start_->setText(tr("Start control flow"));
	action_start_->setIcon(
		QIcon::fromTheme("media-playback-start",
		QIcon(":/icons/media-playback-start.png")));
	action_start_->setCheckable(true);
	action_start_->setChecked(false);
	connect(action_start_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_start_triggered()));

	action_pause_->setText(tr("Pause control flow"));
	action_pause_->setIcon(
		QIcon::fromTheme("media-playback-pause",
		QIcon(":/icons/media-playback-pause.png")));
	action_pause_->setCheckable(true);
	action_pause_->setChecked(false);
	connect(action_pause_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_pause_triggered()));

	action_stop_->setText(tr("Stop control flow"));
	action_stop_->setIcon(
		QIcon::fromTheme("media-playback-stop",
		QIcon(":/icons/media-playback-stop.png")));
	action_stop_->setCheckable(true);
	action_stop_->setChecked(true);
	connect(action_stop_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_stop_triggered()));

	action_load_->setText(tr("Load control flow"));
	action_load_->setIcon(
		QIcon::fromTheme("document-open",
		QIcon(":/icons/document-open.png")));
	connect(action_load_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_load_triggered()));

	action_save_->setText(tr("Save control flow"));
	action_save_->setIcon(
		QIcon::fromTheme("document-save",
		QIcon(":/icons/document-save.png")));
	connect(action_save_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_save_triggered()));

	toolbar_ = new QToolBar("Control Flow Processing Toolbar");
	toolbar_->addAction(action_start_);
	toolbar_->addAction(action_pause_);
	toolbar_->addAction(action_stop_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_load_);
	toolbar_->addAction(action_save_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void FlowView::on_action_start_triggered()
{
	action_pause_->setChecked(false);
	action_stop_->setChecked(false);

	action_start_->setChecked(true);
}

void FlowView::on_action_pause_triggered()
{
	action_start_->setChecked(false);
	action_stop_->setChecked(false);

	action_pause_->setChecked(true);
}

void FlowView::on_action_stop_triggered()
{
	action_start_->setChecked(false);
	action_pause_->setChecked(false);

	action_stop_->setChecked(true);
}

void FlowView::on_action_save_triggered()
{
	flow_scene_->save();
}

void FlowView::on_action_load_triggered()
{
	flow_scene_->load();
}

void FlowView::show_node_popup(Node &node, QPoint pos)
{
	widgets::Popup *popup = widgets::flow::nodehelper::get_popup_for_node(
		session_, node, flow_view_widget_);

	if (popup) {
		popup->set_position(pos, widgets::PopupPosition::Right);
		popup->show();
	}
}

} // namespace views
} // namespace ui
} // namespace sv
