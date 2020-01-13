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

#include <cmath>
#include <memory>
#include <vector>

#include <QChar>
#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QVariant>
#include <QVBoxLayout>

#include "generatewaveformdialog.hpp"
#include "src/data/properties/doubleproperty.hpp"
#include "src/data/datautil.hpp"

using std::shared_ptr;
using std::vector;

Q_DECLARE_METATYPE(sv::ui::dialogs::WaveformType)

namespace sv {
namespace ui {
namespace dialogs {

GenerateWaveformDialog::GenerateWaveformDialog(
		double min_value, double max_value, double step, int decimals,
		QString unit, QWidget *parent) :
	QDialog(parent),
	min_value_(min_value),
	max_value_(max_value),
	step_(step),
	decimals_(decimals),
	unit_(unit)
{
	setup_ui();
}

GenerateWaveformDialog::GenerateWaveformDialog(
		shared_ptr<sv::data::properties::DoubleProperty> property,
		QWidget *parent) :
	QDialog(parent)
{
	min_value_ = property->min();
	max_value_ = property->max();
	step_ = property->step();
	decimals_ = property->decimal_places();
	sv::data::Unit unit = property->unit();
	if (unit != sv::data::Unit::Unitless && unit != sv::data::Unit::Unknown)
		unit_ = " " + sv::data::datautil::format_unit(unit);
	else
		unit_ = "";

	setup_ui();
}

void GenerateWaveformDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Generate Waveform"));
	this->setMinimumWidth(500);

	QFormLayout *layout = new QFormLayout;

	waveform_box_ = new QComboBox();
	waveform_box_->addItem(tr("Sine"),
		QVariant::fromValue(WaveformType::Sine));
	waveform_box_->addItem(tr("Square"),
		QVariant::fromValue(WaveformType::Square));
	waveform_box_->addItem(tr("Triangle"),
		QVariant::fromValue(WaveformType::Triangle));
	waveform_box_->addItem(tr("Sawtooth"),
		QVariant::fromValue(WaveformType::Sawtooth));
	waveform_box_->addItem(tr("Sawtooth inverted"),
		QVariant::fromValue(WaveformType::SawtoothInv));
	connect(waveform_box_, SIGNAL(currentIndexChanged(int)),
			this, SLOT(on_waveform_changed()));
	layout->addRow(tr("Waveform"), waveform_box_);

	QGroupBox *amp_group = new QGroupBox(tr("Min/Max - Amplitude"));
	QHBoxLayout *amp_layout = new QHBoxLayout;
	QFormLayout *ampmm_layout = new QFormLayout;
	QFormLayout *ampf_layout = new QFormLayout;

	min_value_box_ = new QDoubleSpinBox();
	min_value_box_->setMinimum(min_value_);
	min_value_box_->setMaximum(max_value_);
	min_value_box_->setSingleStep(step_);
	min_value_box_->setDecimals(decimals_);
	min_value_box_->setSuffix(unit_);
	connect(min_value_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_min_max_changed()));
	ampmm_layout->addRow(tr("Min. value"), min_value_box_);

	max_value_box_ = new QDoubleSpinBox();
	max_value_box_->setMinimum(min_value_);
	max_value_box_->setMaximum(max_value_);
	max_value_box_->setSingleStep(step_);
	max_value_box_->setDecimals(decimals_);
	max_value_box_->setSuffix(unit_);
	connect(max_value_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_min_max_changed()));
	ampmm_layout->addRow(tr("Max. value"), max_value_box_);
	amp_layout->addLayout(ampmm_layout);

	amplitude_box_ = new QDoubleSpinBox();
	amplitude_box_->setMinimum(min_value_);
	amplitude_box_->setMaximum(max_value_);
	amplitude_box_->setSingleStep(step_);
	amplitude_box_->setDecimals(decimals_);
	amplitude_box_->setSuffix(unit_);
	connect(amplitude_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_amp_offs_changed()));
	ampf_layout->addRow(tr("Amplitude"), amplitude_box_);

	offset_box_ = new QDoubleSpinBox();
	offset_box_->setMinimum(min_value_);
	offset_box_->setMaximum(max_value_);
	offset_box_->setSingleStep(step_);
	offset_box_->setDecimals(decimals_);
	offset_box_->setSuffix(unit_);
	connect(offset_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_amp_offs_changed()));
	ampf_layout->addRow(tr("Offset"), offset_box_);
	amp_layout->addLayout(ampf_layout);
	amp_group->setLayout(amp_layout);
	layout->addRow(amp_group);

	QGroupBox *freq_group = new QGroupBox(tr("Periode - Frequency"));
	QHBoxLayout *freq_layout = new QHBoxLayout;
	QFormLayout *freqp_layout = new QFormLayout;
	QFormLayout *freqf_layout = new QFormLayout;

	periode_box_ = new QDoubleSpinBox();
	periode_box_->setMinimum(0);
	periode_box_->setMaximum(500000);
	periode_box_->setSingleStep(0.1);
	periode_box_->setDecimals(3);
	periode_box_->setSuffix(" s");
	connect(periode_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_periode_changed()));
	freqp_layout->addRow(tr("Periode"), periode_box_);
	freq_layout->addLayout(freqp_layout);

	frequency_box_ = new QDoubleSpinBox();
	frequency_box_->setMinimum(0);
	frequency_box_->setMaximum(5000);
	frequency_box_->setSingleStep(0.001);
	frequency_box_->setDecimals(4);
	frequency_box_->setSuffix(" Hz");
	connect(frequency_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_frequency_changed()));
	freqf_layout->addRow(tr("Frequency"), frequency_box_);
	freq_layout->addLayout(freqf_layout);
	freq_group->setLayout(freq_layout);
	layout->addRow(freq_group);

	QGroupBox *samples_group = new QGroupBox(tr("Samples"));
	QHBoxLayout *samples_layout = new QHBoxLayout;
	QFormLayout *samplesi_layout = new QFormLayout;
	QFormLayout *samplesc_layout = new QFormLayout;

	interval_box_ = new QDoubleSpinBox();
	interval_box_->setMinimum(0);
	interval_box_->setMaximum(100000);
	interval_box_->setSingleStep(0.1);
	interval_box_->setDecimals(3);
	interval_box_->setSuffix(" s");
	connect(interval_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_interval_changed()));
	samplesi_layout->addRow(tr("Time between samples"), interval_box_);
	samples_layout->addLayout(samplesi_layout);

	sample_count_box_ = new QSpinBox();
	sample_count_box_->setMinimum(0);
	sample_count_box_->setMaximum(1000000);
	connect(sample_count_box_, SIGNAL(valueChanged(int)),
		this, SLOT(on_sample_cnt_changed()));
	samplesc_layout->addRow(tr("Number of samples"), sample_count_box_);
	samples_layout->addLayout(samplesc_layout);
	samples_group->setLayout(samples_layout);
	layout->addRow(samples_group);

	QGroupBox *phi_group = new QGroupBox(tr("Phase offset"));
	QHBoxLayout *phi_layout = new QHBoxLayout;
	QFormLayout *phid_layout = new QFormLayout;
	QFormLayout *phir_layout = new QFormLayout;

	phi_deg_box_ = new QDoubleSpinBox();
	phi_deg_box_->setMinimum(-360);
	phi_deg_box_->setMaximum(360);
	phi_deg_box_->setSingleStep(0.1);
	phi_deg_box_->setDecimals(1);
	phi_deg_box_->setSuffix(QString(" %1").arg(QChar(0x00B0)));
	connect(phi_deg_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_phi_deg_changed()));
	phid_layout->addRow(tr("%1 (deg)").arg(QChar(0x03C6)), phi_deg_box_);
	phi_layout->addLayout(phid_layout);

	phi_rad_box_ = new QDoubleSpinBox();
	phi_rad_box_->setMinimum(-2 * pi);
	phi_rad_box_->setMaximum(2 * pi);
	phi_rad_box_->setSingleStep(0.01);
	phi_rad_box_->setDecimals(3);
	connect(phi_rad_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_phi_rad_changed()));
	phir_layout->addRow(tr("%1 (rad)").arg(QChar(0x03C6)), phi_rad_box_);
	phi_layout->addLayout(phir_layout);
	phi_group->setLayout(phi_layout);
	layout->addRow(phi_group);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	layout->addRow(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	// Set values
	min_value_box_->setValue(min_value_);
	max_value_box_->setValue(max_value_);
	periode_box_->setValue(60);
	interval_box_->setValue(0.1);
	phi_deg_box_->setValue(270);

	this->setLayout(layout);
}

vector<double> GenerateWaveformDialog::sequence_values() const
{
	return sequence_values_;
}

vector<double> GenerateWaveformDialog::sequence_delays() const
{
	return sequence_delays_;
}

void GenerateWaveformDialog::accept()
{
	double amplitude = amplitude_box_->value();
	double offset = offset_box_->value();
	double interval = interval_box_->value();
	double periode;
	double frequency;
	// Get the most precise values for periode and frequency, because
	// the values from the spin boxes are truncated!
	if (frequency_box_->value() > 1) {
		frequency = frequency_box_->value();
		periode = 1 / frequency;
	}
	else {
		periode = periode_box_->value();
		frequency = 1 / periode;
	}
	double phi = phi_rad_box_->value();
	double omega = 2 * pi * frequency;

	WaveformType w_type = waveform_box_->currentData().value<WaveformType>();
	for (double t=0; t<periode; t+=interval) {
		double x = omega * t + phi;
		double value;
		if (w_type == WaveformType::Sine)
			value = std::sin(x);
		else if (w_type == WaveformType::Square)
			value = std::sin(x) < 0 ? -1 : 1;
		else if (w_type == WaveformType::Triangle)
			value = (std::asin(std::sin(x))) / (pi/2);
		else if (w_type == WaveformType::Sawtooth)
			// y = −arctan(cotan(x))
			value = -1 * std::atan(1 / std::tan(x)) / (pi/2);
		else if (w_type == WaveformType::SawtoothInv)
			value = std::atan(1 / std::tan(x)) / (pi/2);
		else
			value = 0;

		value = (amplitude * value) + offset;
		sequence_values_.push_back(value);
		sequence_delays_.push_back(interval);
	}

	QDialog::accept();
}

void GenerateWaveformDialog::on_waveform_changed()
{
	WaveformType w_type = waveform_box_->currentData().value<WaveformType>();
	if (w_type == WaveformType::Sine || w_type == WaveformType::Triangle)
		phi_deg_box_->setValue(270);
	else
		phi_deg_box_->setValue(0);
}

void GenerateWaveformDialog::on_min_max_changed()
{
	disconnect(amplitude_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_amp_offs_changed()));
	disconnect(offset_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_amp_offs_changed()));

	amplitude_box_->setValue((max_value_box_->value() - min_value_box_->value()) / 2);
	offset_box_->setValue(amplitude_box_->value() +  min_value_box_->value());

	connect(amplitude_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_amp_offs_changed()));
	connect(offset_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_amp_offs_changed()));
}

void GenerateWaveformDialog::on_amp_offs_changed()
{
	disconnect(min_value_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_min_max_changed()));
	disconnect(max_value_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_min_max_changed()));

	min_value_box_->setValue(offset_box_->value() - amplitude_box_->value());
	max_value_box_->setValue(offset_box_->value() + amplitude_box_->value());

	connect(min_value_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_min_max_changed()));
	connect(max_value_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_min_max_changed()));
}

void GenerateWaveformDialog::on_periode_changed()
{
	disconnect(frequency_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_frequency_changed()));
	disconnect(sample_count_box_, SIGNAL(valueChanged(int)),
		this, SLOT(on_sample_cnt_changed()));

	frequency_box_->setValue(1 / periode_box_->value());
	sample_count_box_->setValue(periode_box_->value() / interval_box_->value());

	connect(frequency_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_frequency_changed()));
	connect(sample_count_box_, SIGNAL(valueChanged(int)),
		this, SLOT(on_sample_cnt_changed()));
}

void GenerateWaveformDialog::on_frequency_changed()
{
	disconnect(periode_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_periode_changed()));
	disconnect(sample_count_box_, SIGNAL(valueChanged(int)),
		this, SLOT(on_sample_cnt_changed()));

	periode_box_->setValue(1 / frequency_box_->value());
	sample_count_box_->setValue(periode_box_->value() / interval_box_->value());

	connect(periode_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_periode_changed()));
	connect(sample_count_box_, SIGNAL(valueChanged(int)),
		this, SLOT(on_sample_cnt_changed()));
}

void GenerateWaveformDialog::on_interval_changed()
{
	disconnect(sample_count_box_, SIGNAL(valueChanged(int)),
		this, SLOT(on_sample_cnt_changed()));

	sample_count_box_->setValue(periode_box_->value() / interval_box_->value());

	connect(sample_count_box_, SIGNAL(valueChanged(int)),
		this, SLOT(on_sample_cnt_changed()));
}

void GenerateWaveformDialog::on_sample_cnt_changed()
{
	disconnect(interval_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_interval_changed()));

	interval_box_->setValue(periode_box_->value() / sample_count_box_->value());

	connect(interval_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_interval_changed()));
}

void GenerateWaveformDialog::on_phi_deg_changed()
{
	disconnect(phi_rad_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_phi_rad_changed()));

	phi_rad_box_->setValue(phi_deg_box_->value() * (pi/180));

	connect(phi_rad_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_phi_rad_changed()));
}

void GenerateWaveformDialog::on_phi_rad_changed()
{
	disconnect(phi_deg_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_phi_deg_changed()));

	phi_deg_box_->setValue(phi_rad_box_->value() * (180/pi));

	connect(phi_deg_box_, SIGNAL(valueChanged(double)),
		this, SLOT(on_phi_deg_changed()));
}

} // namespace dialogs
} // namespace ui
} // namespace sv
