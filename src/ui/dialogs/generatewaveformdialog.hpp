/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DIALOGS_GENERATEWAVEFORMDIALOG_HPP
#define UI_DIALOGS_GENERATEWAVEFORMDIALOG_HPP

#include <cmath>
#include <memory>
#include <vector>

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QSpinBox>

using std::shared_ptr;
using std::vector;

namespace sv {

namespace data {
namespace properties {
class DoubleProperty;
}
}

namespace ui {
namespace dialogs {

enum class WaveformType {
	Sine,
	Square,
	Triangle,
	Sawtooth,
	SawtoothInv,
};

class GenerateWaveformDialog : public QDialog
{
	Q_OBJECT

public:
	GenerateWaveformDialog(double min_value, double max_value, double step,
		int decimals = 3, const QString &unit = "", QWidget *parent = nullptr);
	explicit GenerateWaveformDialog(
		shared_ptr<sv::data::properties::DoubleProperty> property,
		QWidget *parent = nullptr);

	vector<double> sequence_values() const;
	vector<double> sequence_delays() const;

private:
	void setup_ui();

	const double pi = std::acos(-1);

	double min_value_;
	double max_value_;
	double step_;
	int decimals_;
	QString unit_;
	vector<double> sequence_values_;
	vector<double> sequence_delays_;
	QComboBox *waveform_box_;
	QDoubleSpinBox *min_value_box_;
	QDoubleSpinBox *max_value_box_;
	QDoubleSpinBox *amplitude_box_;
	QDoubleSpinBox *offset_box_;
	QDoubleSpinBox *periode_box_;
	QDoubleSpinBox *frequency_box_;
	QDoubleSpinBox *interval_box_;
	QSpinBox *sample_count_box_;
	QDoubleSpinBox *phi_deg_box_;
	QDoubleSpinBox *phi_rad_box_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void accept() override;

private Q_SLOTS:
	void on_waveform_changed();
	void on_min_max_changed();
	void on_amp_offs_changed();
	void on_periode_changed();
	void on_frequency_changed();
	void on_interval_changed();
	void on_sample_cnt_changed();
	void on_phi_deg_changed();
	void on_phi_rad_changed();

};

} // namespace dialogs
} // namespace ui
} // namespace sv

#endif // UI_DIALOGS_GENERATEWAVEFORMDIALOG_HPP
