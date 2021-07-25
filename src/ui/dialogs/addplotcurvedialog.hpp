/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DIALOGS_ADDPLOTCURVEDIALOG_HPP
#define UI_DIALOGS_ADDPLOTCURVEDIALOG_HPP

#include <memory>

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>

#include <qwt_plot.h>

#include "src/session.hpp"

using std::shared_ptr;

namespace sv {

namespace data {
class BaseSignal;
}
namespace devices {
class BaseDevice;
}

namespace ui {

namespace devices {
class SelectSignalWidget;
}
namespace widgets {
namespace plot {
class ScopePlot;
}
}

namespace dialogs {

class AddPlotCurveDialog : public QDialog
{
	Q_OBJECT

public:
	AddPlotCurveDialog(const Session &session,
		const ui::widgets::plot::ScopePlot *plot,
		const shared_ptr<sv::devices::BaseDevice> selected_device,
		QWidget *parent = nullptr);

	shared_ptr<sv::data::BaseSignal> signal() const;
	QwtPlot::Axis x_axis_id() const;
	QwtPlot::Axis y_axis_id() const;

private:
	void setup_ui();
	void fill_x_axis_box();
	void fill_y_axis_box();

	const Session &session_;
	const ui::widgets::plot::ScopePlot *plot_;
	const shared_ptr<sv::devices::BaseDevice> selected_device_;

	ui::devices::SelectSignalWidget *signal_widget_;
	QComboBox *x_axis_box_;
	QComboBox *y_axis_box_;
	QDialogButtonBox *button_box_;

private Q_SLOT:
	void on_signal_changed();

};

} // namespace dialogs
} // namespace ui
} // namespace sv

#endif // UI_DIALOGS_ADDPLOTCURVEDIALOG_HPP
