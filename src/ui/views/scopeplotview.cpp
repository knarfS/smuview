/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>
#include <string>

#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QImageWriter>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <qwt_plot_renderer.h>

#include "scopeplotview.hpp"
#include "src/session.hpp"
#include "src/channels/scopechannel.hpp"
#include "src/data/analogscopesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/properties/doubleproperty.hpp"
#include "src/data/properties/int32property.hpp"
#include "src/data/properties/rationalproperty.hpp"
#include "src/data/properties/stringproperty.hpp"
#include "src/data/properties/uint64property.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/oscilloscopedevice.hpp"
#include "src/ui/dialogs/addplotcurvedialog.hpp"
#include "src/ui/dialogs/plotconfigdialog.hpp"
#include "src/ui/dialogs/plotdiffmarkerdialog.hpp"
#include "src/ui/dialogs/selectsignaldialog.hpp"
#include "src/ui/widgets/plot/scopecurve.hpp"
#include "src/ui/widgets/plot/scopeplot.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"
#include "src/ui/widgets/plot/scopecurvedata.hpp"

using std::dynamic_pointer_cast;
using std::static_pointer_cast;
using std::string;
using sv::devices::ConfigKey;

Q_DECLARE_METATYPE(sv::ui::widgets::plot::BaseCurveData *)

namespace sv {
namespace ui {
namespace views {

ScopePlotView::ScopePlotView(Session &session, QUuid uuid, QWidget *parent) :
	BaseView(session, uuid, parent),
	action_add_marker_(new QAction(this)),
	action_add_diff_marker_(new QAction(this)),
	action_zoom_best_fit_(new QAction(this)),
	action_add_curve_(new QAction(this)),
	action_save_(new QAction(this)),
	action_config_plot_(new QAction(this))
{
	setup_ui();
	setup_toolbar();
}

QString ScopePlotView::title() const
{
	return tr("Channel(s)").append(" ").append(channel_names_);
}

void ScopePlotView::set_scope_device(
	shared_ptr<sv::devices::OscilloscopeDevice> device)
{
	device_ = device;

	auto dev_configurable = device_->configurable_map()[""];
	if (dev_configurable == nullptr)
		return;

	// TODO: Do we need class varaiables for the properties?

	if (dev_configurable->has_get_config(ConfigKey::Samplerate)) {
		auto samplerate_prop =
			static_pointer_cast<data::properties::UInt64Property>(
				dev_configurable->get_property(ConfigKey::Samplerate));
		plot_->update_samplerate(samplerate_prop->value());
		connect(samplerate_prop.get(),
			&data::properties::BaseProperty::value_changed,
			plot_, &widgets::plot::ScopePlot::update_samplerate);
	}

	if (dev_configurable->has_get_config(ConfigKey::NumHDiv)) {
		auto num_hdiv_prop =
			static_pointer_cast<data::properties::Int32Property>(
				dev_configurable->get_property(ConfigKey::NumHDiv));
		plot_->update_num_hdiv(num_hdiv_prop->value());
		connect(num_hdiv_prop.get(),
			&data::properties::BaseProperty::value_changed,
			plot_, &widgets::plot::ScopePlot::update_num_hdiv);
	}

	if (dev_configurable->has_get_config(ConfigKey::TimeBase)) {
		auto timebase_prop =
			static_pointer_cast<data::properties::RationalProperty>(
				dev_configurable->get_property(ConfigKey::TimeBase));
		plot_->update_timebase(timebase_prop->value());
		connect(timebase_prop.get(),
			&data::properties::BaseProperty::value_changed,
			plot_, &widgets::plot::ScopePlot::update_timebase);
	}

	if (dev_configurable->has_get_config(ConfigKey::TriggerSource)) {
		auto trigger_source_prop =
			static_pointer_cast<data::properties::StringProperty>(
				dev_configurable->get_property(ConfigKey::TriggerSource));
		plot_->update_trigger_source(trigger_source_prop->value());
		connect(trigger_source_prop.get(),
			&data::properties::BaseProperty::value_changed,
			plot_, &widgets::plot::ScopePlot::update_trigger_source);
	}

	if (dev_configurable->has_get_config(ConfigKey::TriggerLevel)) {
		auto trigger_level_prop =
			static_pointer_cast<data::properties::DoubleProperty>(
				dev_configurable->get_property(ConfigKey::TriggerLevel));
		plot_->update_trigger_level(trigger_level_prop->value());
		connect(trigger_level_prop.get(),
			&data::properties::BaseProperty::value_changed,
			plot_, &widgets::plot::ScopePlot::update_trigger_level);
	}

	if (dev_configurable->has_get_config(ConfigKey::HorizTriggerPos)) {
		auto horiz_trigger_pos =
			static_pointer_cast<data::properties::DoubleProperty>(
				dev_configurable->get_property(ConfigKey::HorizTriggerPos));
		plot_->update_horiz_trigger_pos(horiz_trigger_pos->value());
		connect(horiz_trigger_pos.get(),
			&data::properties::BaseProperty::value_changed,
			plot_, &widgets::plot::ScopePlot::update_horiz_trigger_pos);
	}
}

string ScopePlotView::add_channel(
	shared_ptr<channels::ScopeChannel> channel, QwtPlot::Axis y_axis_id)
{
	assert(channel);

	(void)y_axis_id; // TODO: How to handle teh axis id for the add_signal slot?

	connect(channel.get(), &channels::ScopeChannel::signal_added,
		this, &ScopePlotView::add_signal);

	// TODO: When adding channles at startup, there is probaly yet no signal.
	//       Lock, so the same signal didn't get add two times.
	/*
	// TODO: Use (Analog)BaseSignal
	auto as_signal = static_pointer_cast<data::AnalogScopeSignal>(
		channel->actual_signal());
	if (as_signal == nullptr)
		return "";

	auto curve = new widgets::plot::ScopeCurve(
		as_signal, y_axis_id, QwtPlot::xBottom);
	plot_->add_curve(curve);

	if (!channel_names_.isEmpty())
		channel_names_.append(", ");
	channel_names_.append(channel->display_name());
	Q_EMIT title_changed();

	return curve->id();
	*/
	return "";
}

void ScopePlotView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	plot_ = new widgets::plot::ScopePlot(session_);

	layout->addWidget(plot_);

	this->central_widget_->setLayout(layout);
}

void ScopePlotView::setup_toolbar()
{
	add_marker_menu_ = new QMenu();
	update_add_marker_menu();

	add_marker_button_ = new QToolButton();
	add_marker_button_->setText(tr("Add marker"));
	add_marker_button_->setIcon(
		QIcon::fromTheme("snap-orthogonal",
		QIcon(":/icons/snap-orthogonal.png")));
	add_marker_button_->setMenu(add_marker_menu_);
	add_marker_button_->setPopupMode(QToolButton::MenuButtonPopup);

	action_add_diff_marker_->setText(tr("Add diff-marker"));
	action_add_diff_marker_->setIcon(
		QIcon::fromTheme("snap-guideline",
		QIcon(":/icons/snap-guideline.png")));
	connect(action_add_diff_marker_, &QAction::triggered,
		this, &ScopePlotView::on_action_add_diff_marker_triggered);

	action_zoom_best_fit_->setText(tr("Best fit"));
	action_zoom_best_fit_->setIcon(
		QIcon::fromTheme("zoom-fit-best",
		QIcon(":/icons/zoom-fit-best.png")));
	connect(action_zoom_best_fit_, &QAction::triggered,
		this, &ScopePlotView::on_action_zoom_best_fit_triggered);

	action_add_curve_->setText(tr("Add Curve"));
	action_add_curve_->setIcon(
		QIcon::fromTheme("office-chart-line",
		QIcon(":/icons/office-chart-line.png")));
	connect(action_add_curve_, &QAction::triggered,
		this, &ScopePlotView::on_action_add_curve_triggered);

	action_config_plot_->setText(tr("Configure Plot"));
	action_config_plot_->setIcon(
		QIcon::fromTheme("configure",
		QIcon(":/icons/configure.png")));
	connect(action_config_plot_, &QAction::triggered,
		this, &ScopePlotView::on_action_config_plot_triggered);

	toolbar_ = new QToolBar("Plot Toolbar");
	toolbar_->addWidget(add_marker_button_);
	toolbar_->addAction(action_add_diff_marker_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_zoom_best_fit_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_add_curve_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_config_plot_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void ScopePlotView::update_add_marker_menu()
{
	/*
	// First remove all existing actions
	for (QAction *action : add_marker_menu_->actions()) {
		disconnect(action, SIGNAL(triggered(bool)),
			this, SLOT(on_action_add_marker_triggered()));
		add_marker_menu_->removeAction(action);
		delete action;
	}

	// One add marker action for each curve
	for (const auto &curve : curves_) {
		QAction *action = new QAction(this);
		action->setText(curve->name());
		action->setData(QVariant::fromValue(curve));
		connect(action, SIGNAL(triggered(bool)),
			this, SLOT(on_action_add_marker_triggered()));
		add_marker_menu_->addAction(action);
	}
	*/
}

void ScopePlotView::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	(void)settings;
	(void)origin_device;
	/*
	BasePlotView::save_settings(settings, origin_device);

	// TODO: Can the channel be saved inside the plot widget?
	bool save_curves = true;
	if (channel_) {
		SettingsManager::save_channel(channel_, settings, origin_device);
		save_curves = false;
	}
	plot_->save_settings(settings, save_curves, origin_device);
	*/
}

void ScopePlotView::restore_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	(void)settings;
	(void)origin_device;
	/*
	BasePlotView::restore_settings(settings, origin_device);

	// TODO: Can the channel be restored inside the plot widget?
	bool restore_curves = true;
	auto channel = SettingsManager::restore_channel(
		session_, settings, origin_device);
	if (channel) {
		set_channel(channel);
		restore_curves = false;
	}
	plot_->restore_settings(settings, restore_curves, origin_device);
	*/
}

void ScopePlotView::add_signal(std::shared_ptr<sv::data::BaseSignal> signal)
{
	// TODO: Use (Analog)BaseSignal
	auto as_signal = static_pointer_cast<data::AnalogScopeSignal>(signal);

	// Check if the new channel is already added to this plot
	/* TODO
	for (const auto &curve : plot_->curve_map()) {
		auto curve_data = qobject_cast<widgets::plot::ScopeCurveData *>(
			curve.second->curve_data());
		if (!curve_data)
			continue;
		if (curve_data->signal() == as_signal)
			return;
	}
	*/

	// TODO: axes
	auto curve = new widgets::plot::ScopeCurve(
		as_signal, QwtPlot::xBottom, QwtPlot::yLeft);
	plot_->add_curve(curve);

	/* TODO
	string id = plot_->add_curve(curve);
	if (!id.empty()) {
		update_add_marker_menu();
		Q_EMIT title_changed();
	}
	else {
		QMessageBox::warning(this,
			tr("Cannot add signal"), tr("Cannot add scope signal to plot!"),
			QMessageBox::Ok);
	}
	*/
}

void ScopePlotView::on_action_add_curve_triggered()
{
	ui::dialogs::AddPlotCurveDialog dlg(session_, plot_, device_);
	if (!dlg.exec())
		return;

	// TODO: Use (Analog)BaseSignal
	auto as_signal = static_pointer_cast<data::AnalogScopeSignal>(dlg.signal());
	if (as_signal == nullptr)
		return;

	auto curve = new widgets::plot::ScopeCurve(
		as_signal, dlg.x_axis_id(), dlg.y_axis_id());
	plot_->add_curve(curve);

	Q_EMIT title_changed();
}

void ScopePlotView::on_action_add_marker_triggered()
{
	/*
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
		plot_->add_marker(action->data().value<widgets::plot::Curve *>());

	if (plot_->marker_curve_map().size() >= 2)
		action_add_diff_marker_->setDisabled(false);
	else
		action_add_diff_marker_->setDisabled(true);
	*/
}

void ScopePlotView::on_action_add_diff_marker_triggered()
{
	/*
	ui::dialogs::PlotDiffMarkerDialog dlg(plot_);
	dlg.exec();
	*/
}

void ScopePlotView::on_action_zoom_best_fit_triggered()
{
	plot_->set_all_axis_locked(false);
}

void ScopePlotView::on_action_save_triggered()
{
	QString filter("SVG Image (*.svg);;PDF File (*.pdf)");
	const auto supported_formats = QImageWriter::supportedImageFormats();
	for (const auto &supported : supported_formats) {
		filter += ";;" + supported.toUpper() + " Image (*." + supported + ")";
	}
	QString *selected_filter = new QString("SVG Image (*.svg)");
	QString file_name = QFileDialog::getSaveFileName(this,
		tr("Save Plot"), QDir::homePath(), filter, selected_filter);
	delete selected_filter;
	if (file_name.length() <= 0)
		return;

	// TODO
	QSizeF size(300, 300);
	int resolution = 90;
	QwtPlotRenderer renderer;
	renderer.renderDocument(plot_, file_name, size, resolution);
}

void ScopePlotView::on_action_config_plot_triggered()
{
	/*
	ui::dialogs::PlotConfigDialog dlg(plot_, plot_type_);
	dlg.exec();
	*/
}

} // namespace views
} // namespace ui
} // namespace sv
