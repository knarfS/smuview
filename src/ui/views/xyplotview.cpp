/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>
#include <memory>
#include <string>

#include <QMessageBox>
#include <QSettings>
#include <QUuid>

#include "xyplotview.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/dialogs/selectxysignalsdialog.hpp"
#include "src/ui/views/baseplotview.hpp"
#include "src/ui/views/viewhelper.hpp"
#include <src/ui/widgets/plot/curve.hpp>
#include "src/ui/widgets/plot/plot.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"
#include "src/ui/widgets/plot/xycurvedata.hpp"

using std::dynamic_pointer_cast;
using std::string;

Q_DECLARE_METATYPE(sv::ui::widgets::plot::BaseCurveData *)

namespace sv {
namespace ui {
namespace views {

XYPlotView::XYPlotView(Session &session, QUuid uuid, QWidget *parent) :
	BasePlotView(session, uuid, parent)
{
	id_ = "xyplot:" + uuid_.toString(QUuid::WithoutBraces).toStdString();
	plot_type_ = PlotType::XYPlot;
}


QString XYPlotView::title() const
{
	QString title = tr("Signal");
	QString sep(" ");
	for (const auto &curve : plot_->curve_map()) {
		title = title.append(sep).append(curve.second->name());
		sep = ", ";
	}
	return title;
}

string XYPlotView::add_signals(shared_ptr<sv::data::AnalogTimeSignal> x_signal,
	shared_ptr<sv::data::AnalogTimeSignal> y_signal)
{
	auto curve = new widgets::plot::XYCurveData(x_signal, y_signal);
	string id = plot_->add_curve(curve);
	if (!id.empty()) {
		update_add_marker_menu();
	}
	else {
		QMessageBox::warning(this,
			tr("Cannot add signal"), tr("Cannot add xy signal to plot!"),
			QMessageBox::Ok);
	}
	return id;
}

void XYPlotView::save_settings(QSettings &settings) const
{
	BasePlotView::save_settings(settings);
	plot_->save_settings(settings, true);
}

void XYPlotView::restore_settings(QSettings &settings)
{
	BasePlotView::restore_settings(settings);
	plot_->restore_settings(settings, true);
	update_add_marker_menu();
}

void XYPlotView::on_action_add_curve_triggered()
{
	ui::dialogs::SelectXYSignalsDialog dlg(session(), nullptr);
	if (!dlg.exec())
		return;

	add_signals(
		dynamic_pointer_cast<sv::data::AnalogTimeSignal>(dlg.x_signal()),
		dynamic_pointer_cast<sv::data::AnalogTimeSignal>(dlg.y_signal()));
}

} // namespace views
} // namespace ui
} // namespace sv
