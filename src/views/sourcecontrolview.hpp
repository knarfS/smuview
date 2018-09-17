/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef VIEWS_SOURCECONTROLVIEW_HPP
#define VIEWS_SOURCECONTROLVIEW_HPP

#include <memory>

#include "src/devices/deviceutil.hpp"
#include "src/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace devices {
class Configurable;
}

namespace ui {
namespace datatypes {
class BoolButton;
class BoolLed;
class DoubleControl;
class ThresholdControl;
}
}

namespace views {

class SourceControlView : public BaseView
{
	Q_OBJECT

public:
	SourceControlView(const Session& session,
		std::shared_ptr<devices::Configurable> configurable,
		QWidget* parent = nullptr);

	QString title() const;

private:
	shared_ptr<devices::Configurable> configurable_;

	ui::datatypes::BoolLed *cc_led_;
	ui::datatypes::BoolLed *cv_led_;
	ui::datatypes::BoolLed *ovp_led_;
	ui::datatypes::BoolLed *ocp_led_;
	ui::datatypes::BoolLed *otp_led_;
	ui::datatypes::BoolLed *uvc_led_;
	ui::datatypes::BoolButton *enable_button_;
	ui::datatypes::DoubleControl *voltage_control_;
	ui::datatypes::DoubleControl *current_control_;
	ui::datatypes::ThresholdControl *ovp_control_;
	ui::datatypes::ThresholdControl *ocp_control_;
	ui::datatypes::ThresholdControl *uvc_control_;

	void setup_ui();

};

} // namespace views
} // namespace sv

#endif // VIEWS_SOURCECONTROLVIEW_HPP
