/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <boost/test/tools/old/interface.hpp>
#include <memory>
#include <vector>
#include <boost/test/unit_test.hpp>

#include <QDateTime>

#include "src/utils/exportutil.hpp"
#include "test/testutils/boostutil.hpp"
#include "test/testutils/signalutil.hpp"

using std::shared_ptr;
using std::vector;

BOOST_AUTO_TEST_SUITE(ExportUtilTest)

/**
 * Local helper for timezone independent date and time formating, same as
 * sv::util::format_time_date
 * TODO: I don't like, maybe add `Qt::TimeSpec` or `QTimeZone` parameter to
 * `format_time_date()`, with the local time as default. This also makes
 * the function testable
 */
inline std::string local_ts(double t) {
	return QDateTime::fromMSecsSinceEpoch(qRound64(t * 1000))
		.toString("yyyy.MM.dd hh:mm:ss.zzz").toStdString();
}

BOOST_AUTO_TEST_CASE(align_timestamps_empty_signals_test)
{
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.);

	BOOST_CHECK_EQUAL(result.headers.size(), 1);
	BOOST_CHECK_EQUAL(result.headers, (vector<string> {"Time"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 0);

	std::ostringstream out;
	sv::utils::exportutil::export_csv_aligned(out, result, ';');

	BOOST_CHECK_EQUAL(out.str(), "Time\n");
}

BOOST_AUTO_TEST_CASE(align_timestamps_signal_empty_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1000., 1.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2").build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, true, 0.);

	BOOST_CHECK_EQUAL(result.headers.size(), 3);
	BOOST_CHECK_EQUAL(result.headers, (vector<string> {"Time [s]", "S1", "S2"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 1);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 0.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {1., {}}));

	std::ostringstream out;
	sv::utils::exportutil::export_csv_aligned(out, result, ';');

	BOOST_CHECK_EQUAL(out.str(),
		"Time [s];S1;S2\n"
		"0.0000;1;\n");
}

BOOST_AUTO_TEST_CASE(align_timestamps_signal_all_empty_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1").build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2").build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.);

	BOOST_CHECK_EQUAL(result.headers.size(), 3);
	BOOST_CHECK_EQUAL(result.headers, (vector<string> {"Time", "S1", "S2"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 0);

	std::ostringstream out;
	sv::utils::exportutil::export_csv_aligned(out, result, ';');

	BOOST_CHECK_EQUAL(out.str(), "Time;S1;S2\n");
}

BOOST_AUTO_TEST_CASE(align_timestamps_absolute_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1010., 1.).push(1011., 2.).push(1012., 3.).push(1013., 4.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1008., 10.).push(1009., 20.).push(1010., 30.).push(1011., 40.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.);

	BOOST_CHECK_EQUAL(result.headers.size(), 3);
	BOOST_CHECK_EQUAL(result.headers, (vector<string> {"Time", "S1", "S2"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 6);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1008.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {{}, 10.}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 1009.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {{}, 20.}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 1010.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (vector<optional<double>> {1., 30.}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 1011.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (vector<optional<double>> {2., 40.}));
	BOOST_CHECK_EQUAL(result.rows.at(4).time, 1012.);
	BOOST_CHECK_EQUAL(result.rows.at(4).data, (vector<optional<double>> {3., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(5).time, 1013.);
	BOOST_CHECK_EQUAL(result.rows.at(5).data, (vector<optional<double>> {4., {}}));

	std::ostringstream out;
	sv::utils::exportutil::export_csv_aligned(out, result, ';');

	BOOST_CHECK_EQUAL(out.str(),
		"Time;S1;S2\n"
		+ local_ts(1008.) + ";;10\n"
		+ local_ts(1009.) + ";;20\n"
		+ local_ts(1010.) + ";1;30\n"
		+ local_ts(1011.) + ";2;40\n"
		+ local_ts(1012.) + ";3;\n"
		+ local_ts(1013.) + ";4;\n");
}

BOOST_AUTO_TEST_CASE(align_timestamps_relative_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1010., 1.).push(1011., 2.).push(1012., 3.).push(1013., 4.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1008., 10.).push(1009., 20.).push(1010., 30.).push(1011., 40.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, true, 0.);

	BOOST_CHECK_EQUAL(result.headers.size(), 3);
	BOOST_CHECK_EQUAL(result.headers, (vector<string> {"Time [s]", "S1", "S2"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 6);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 8.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {{}, 10.}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 9.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {{}, 20.}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 10.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (vector<optional<double>> {1., 30.}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 11.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (vector<optional<double>> {2., 40.}));
	BOOST_CHECK_EQUAL(result.rows.at(4).time, 12.);
	BOOST_CHECK_EQUAL(result.rows.at(4).data, (vector<optional<double>> {3., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(5).time, 13.);
	BOOST_CHECK_EQUAL(result.rows.at(5).data, (vector<optional<double>> {4., {}}));

	std::ostringstream out;
	sv::utils::exportutil::export_csv_aligned(out, result, ';');

	BOOST_CHECK_EQUAL(out.str(),
		"Time [s];S1;S2\n"
		"8.0000;;10\n"
		"9.0000;;20\n"
		"10.0000;1;30\n"
		"11.0000;2;40\n"
		"12.0000;3;\n"
		"13.0000;4;\n");
}

BOOST_AUTO_TEST_CASE(align_timestamps_tolerance_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1010.5, 1.).push(1011.5, 2.).push(1012.5, 3.).push(1013.5, 4.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1008., 10.).push(1009., 20.).push(1010., 30.).push(1011., 40.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.5);

	BOOST_CHECK_EQUAL(result.headers.size(), 3);
	BOOST_CHECK_EQUAL(result.headers, (vector<string> {"Time", "S1", "S2"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 6);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1008.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {{}, 10.}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 1009.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {{}, 20.}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 1010.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (vector<optional<double>> {1., 30.}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 1011.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (vector<optional<double>> {2., 40.}));
	BOOST_CHECK_EQUAL(result.rows.at(4).time, 1012.5);
	BOOST_CHECK_EQUAL(result.rows.at(4).data, (vector<optional<double>> {3., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(5).time, 1013.5);
	BOOST_CHECK_EQUAL(result.rows.at(5).data, (vector<optional<double>> {4., {}}));
}

BOOST_AUTO_TEST_CASE(align_timestamps_tolerance_three_signals_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1000., 1.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1000.25, 2.).build();
	auto s3 = sv::test::testutils::TimeSignalBuilder().name("S3")
		.push(1000.5, 3.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2, s3};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.5);

	BOOST_CHECK_EQUAL(result.headers.size(), 4);
	BOOST_CHECK_EQUAL(result.headers, (vector<string> {"Time", "S1", "S2", "S3"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 1);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {1., 2., 3.}));
}

BOOST_AUTO_TEST_CASE(align_timestamps_window_is_anchored_not_chained_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1000.0, 1.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1000.4, 2.).build();
	auto s3 = sv::test::testutils::TimeSignalBuilder().name("S3")
		.push(1000.8, 3.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2, s3};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.5);

	BOOST_REQUIRE_EQUAL(result.rows.size(), 2u);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {1., 2., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 1000.8);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {{}, {}, 3.}));
}

BOOST_AUTO_TEST_CASE(align_timestamps_tolerance_to_small_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1010.4, 1.).push(1010.5, 2.).push(1012.5, 3.).push(1013.5, 4.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1008., 10.).push(1009., 20.).push(1010., 30.).push(1011., 40.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.5);

	BOOST_CHECK_EQUAL(result.headers.size(), 3);
	BOOST_CHECK_EQUAL(result.headers, (vector<string> {"Time", "S1", "S2"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 6);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1008.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {{}, 10.}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 1009.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {{}, 20.}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 1010.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (vector<optional<double>> {1., 30.}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 1010.5);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (vector<optional<double>> {2., 40.}));
	BOOST_CHECK_EQUAL(result.rows.at(4).time, 1012.5);
	BOOST_CHECK_EQUAL(result.rows.at(4).data, (vector<optional<double>> {3., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(5).time, 1013.5);
	BOOST_CHECK_EQUAL(result.rows.at(5).data, (vector<optional<double>> {4., {}}));
}

BOOST_AUTO_TEST_CASE(align_timestamps_huge_tolerance_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1010., 1.).push(1010.5, 2.).push(1012.5, 3.).push(1013.5, 4.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1008., 10.).push(1009., 20.).push(1010., 30.).push(1011., 40.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 50.);

	BOOST_CHECK_EQUAL(result.headers.size(), 3);
	BOOST_CHECK_EQUAL(result.headers, (vector<string> {"Time", "S1", "S2"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 5);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1008.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {{}, 10.}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 1009.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {1., 20.}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 1010.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (vector<optional<double>> {2., 30.}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 1011.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (vector<optional<double>> {3., 40.}));
	BOOST_CHECK_EQUAL(result.rows.at(4).time, 1013.5);
	BOOST_CHECK_EQUAL(result.rows.at(4).data, (vector<optional<double>> {4., {}}));
}

BOOST_AUTO_TEST_CASE(align_timestamps_just_beyond_tolerance_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1000., 1.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1000.501, 2.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.5);

	BOOST_CHECK_EQUAL(result.rows.size(), 2u);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {1., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 1000.501);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {{}, 2.}));
}

BOOST_AUTO_TEST_CASE(align_timestamps_same_timestamp_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1000., 1.).push(1000., 2.).push(1000., 3.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1000., 10.).push(1000., 20.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.);

	BOOST_CHECK_EQUAL(result.headers.size(), 3);
	BOOST_CHECK_EQUAL(result.headers, (vector<string> {"Time", "S1", "S2"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 4);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {1., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {2., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (vector<optional<double>> {3., 10.}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (vector<optional<double>> {{}, 20.}));
}

BOOST_AUTO_TEST_CASE(align_timestamps_same_timestamp_with_tolerance_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1000., 1.).push(1000., 2.).push(1000., 3.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1000., 10.).push(1000., 20.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 5.);

	BOOST_CHECK_EQUAL(result.headers.size(), 3);
	BOOST_CHECK_EQUAL(result.headers, (vector<string> {"Time", "S1", "S2"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 4);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {1., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {2., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (vector<optional<double>> {3., 10.}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (vector<optional<double>> {{}, 20.}));
}

BOOST_AUTO_TEST_CASE(align_timstamps_non_monotonic_timestamps_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1010., 1.).push(1000., 2.).push(1011., 3.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1005., 10.).push(1004., 20.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.);

	BOOST_CHECK_EQUAL(result.rows.size(), 5u);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1005.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {{}, 10.}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 1004.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {{}, 20.}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 1010.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (vector<optional<double>> {1., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (vector<optional<double>> {2., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(4).time, 1011.);
	BOOST_CHECK_EQUAL(result.rows.at(4).data, (vector<optional<double>> {3., {}}));

	result = sv::utils::exportutil::align_timestamps(signals, false, 2.);

	BOOST_CHECK_EQUAL(result.rows.size(), 5u);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1005.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {{}, 10.}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 1004.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {{}, 20.}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 1010.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (vector<optional<double>> {1., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (vector<optional<double>> {2., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(4).time, 1011.);
	BOOST_CHECK_EQUAL(result.rows.at(4).data, (vector<optional<double>> {3., {}}));

	result = sv::utils::exportutil::align_timestamps(signals, false, 100.);

	BOOST_CHECK_EQUAL(result.rows.size(), 4u);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 1005.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (vector<optional<double>> {{}, 10.}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 1004.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (vector<optional<double>> {1., 20.}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 1000.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (vector<optional<double>> {2., {}}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 1011.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (vector<optional<double>> {3., {}}));
}

BOOST_AUTO_TEST_CASE(align_timestamps_special_values_test)
{
	const double k_inf = std::numeric_limits<double>::infinity();
	const double k_nan = std::numeric_limits<double>::quiet_NaN();

	// DMM overflow (inf) and e.g. a divide channel (inf, NaN) produce these.
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1000., k_inf).push(1001., -k_inf).push(1002., k_nan).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1000., k_nan).push(1001., k_inf).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, true, 0.);

	std::ostringstream out;
	sv::utils::exportutil::export_csv_aligned(out, result, ';');

	BOOST_CHECK_EQUAL(out.str(),
		"Time [s];S1;S2\n"
		"0.0000;inf;nan\n"
		"1.0000;-inf;inf\n"
		"2.0000;nan;\n");
}

BOOST_AUTO_TEST_CASE(export_csv_parallel_empty_signals_test)
{
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{};

	std::ostringstream out;
	sv::utils::exportutil::export_csv_parallel(out, signals, ';', false);

	BOOST_CHECK_EQUAL(out.str(), "");
}

BOOST_AUTO_TEST_CASE(export_csv_parallel_signal_empty_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1000., 1.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2").build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	std::ostringstream out;
	sv::utils::exportutil::export_csv_parallel(out, signals, ';', true);

	BOOST_CHECK_EQUAL(out.str(),
		"Time [s]: S1;S1;Time [s]: S2;S2\n"
		"0.0000;1;;\n");
}

BOOST_AUTO_TEST_CASE(export_csv_parallel_signal_all_empty_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1").build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2").build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	std::ostringstream out;
	sv::utils::exportutil::export_csv_parallel(out, signals, ';', false);

	BOOST_CHECK_EQUAL(out.str(), "Time: S1;S1;Time: S2;S2\n");
}

BOOST_AUTO_TEST_CASE(export_csv_parallel_absolute_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1010., 1.).push(1011., 2.).push(1012., 3.).push(1013., 4.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1008., 10.).push(1009., 20.).push(1010., 30.).push(1011., 40.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	std::ostringstream out;
	sv::utils::exportutil::export_csv_parallel(out, signals, ';', false);

	BOOST_CHECK_EQUAL(out.str(),
		"Time: S1;S1;Time: S2;S2\n"
		+ local_ts(1010.) + ";1;" + local_ts(1008.) + ";10\n"
		+ local_ts(1011.) + ";2;" + local_ts(1009.) + ";20\n"
		+ local_ts(1012.) + ";3;" + local_ts(1010.) + ";30\n"
		+ local_ts(1013.) + ";4;" + local_ts(1011.) + ";40\n");
}

BOOST_AUTO_TEST_CASE(export_csv_parallel_relative_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().name("S1")
		.push(1010., 1.).push(1011., 2.).push(1012., 3.).push(1013., 4.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().name("S2")
		.push(1008., 10.).push(1009., 20.).push(1010., 30.).push(1011., 40.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	std::ostringstream out;
	sv::utils::exportutil::export_csv_parallel(out, signals, ';', true);

	BOOST_CHECK_EQUAL(out.str(),
		"Time [s]: S1;S1;Time [s]: S2;S2\n"
		"10.0000;1;8.0000;10\n"
		"11.0000;2;9.0000;20\n"
		"12.0000;3;10.0000;30\n"
		"13.0000;4;11.0000;40\n");
}

BOOST_AUTO_TEST_SUITE_END()
