#include "stdafx.h"
#include "gtest/gtest.h"
#include "../Storage/DeviceFloppy.h"

constexpr size_t CLOCK_SPEED = 1000000;
constexpr size_t TIME_10MS = CLOCK_SPEED / 100;
constexpr size_t TIME_100MS = CLOCK_SPEED / 10;
constexpr size_t TIME_1S = CLOCK_SPEED / 1;

constexpr uint32_t DelayToTicks(uint32_t millis) { return millis * CLOCK_SPEED / 1000; }

Logger::SEVERITY TEST_FLOPPY_LOG_LEVEL = Logger::LOG_INFO;

using fdd::StepDirection;

class TestFloppy : public fdd::DeviceFloppy
{
public:
	TestFloppy(bool forceActive = false) : 
		fdd::DeviceFloppy(CLOCK_SPEED),
		m_forceActive(forceActive)
	{ 
		EnableLog(TEST_FLOPPY_LOG_LEVEL);
		Init();
	}

	virtual bool IsActive() const override { return m_forceActive ? true : DeviceFloppy::IsActive(); }

private:
	bool m_forceActive = false;
};

TEST(TestFloppy, TestInit) 
{
	TestFloppy floppy;	

	EXPECT_FALSE(floppy.IsActive());
	EXPECT_FALSE(floppy.IsDiskChanged());
	EXPECT_FALSE(floppy.IsDiskLoaded());
	EXPECT_FALSE(floppy.IsMotorEnabled());
	EXPECT_FALSE(floppy.IsSeeking());
	EXPECT_FALSE(floppy.IsTrack0());
	EXPECT_EQ(0, floppy.GetCurrentHead());
	EXPECT_EQ(0, floppy.GetCurrentSector());
	EXPECT_EQ(0, floppy.GetCurrentTrack());
	EXPECT_FALSE(floppy.GetMotorPulse());
	EXPECT_NEAR(300, floppy.GetMotorSpeed(), 200); // Check for sensible default value
	EXPECT_NEAR(20, floppy.GetStepDelay(), 10); // Check for sensible default value
}

TEST(TestFloppy, TestMotor)
{
	TestFloppy floppy;

	EXPECT_FALSE(floppy.IsMotorEnabled());
	floppy.EnableMotor(true);
	EXPECT_TRUE(floppy.IsMotorEnabled());
	floppy.EnableMotor(false);
	EXPECT_FALSE(floppy.IsMotorEnabled());

	floppy.EnableMotor(true);
	floppy.Reset();
	EXPECT_FALSE(floppy.IsMotorEnabled());
}

void TestRPM(bool motor, int rpm, int expectedPulses, std::string msg)
{
	SCOPED_TRACE("TestRPM");

	TestFloppy floppy;

	floppy.SetMotorSpeed(rpm);
	floppy.EnableMotor(motor);
	int tolerance = (int)round((float)rpm / 500.); // ~.2% error

	int posTransitions = 0;
	int negTransitions = 0;
	bool lastPulse = false;
	for (int i = 0; i < CLOCK_SPEED; ++i)
	{
		floppy.Tick();

		bool pulse = floppy.GetMotorPulse();

		if (!lastPulse && pulse)
		{
			++posTransitions;
		}
		else if (lastPulse && !pulse)
		{
			++negTransitions;
		}

		lastPulse = pulse;
	}

	EXPECT_NEAR(expectedPulses, posTransitions, tolerance) << msg;
	EXPECT_NEAR(expectedPulses, negTransitions, tolerance) << msg;
}

TEST(TestFloppy, TestMotorRPM)
{
	TestRPM(false, 360, 0, "Motor off");
	TestRPM(true, 0, 60, "@0 RPM"); // Should clamp to 60rpm
	TestRPM(true, 59, 60, "@59 RPM"); // Should clamp to 60rpm
	TestRPM(true, 60, 60, "@60 RPM");
	TestRPM(true, 360, 360, "@360 RPM");
	TestRPM(true, 720, 720, "@720 RPM");
	TestRPM(true, 10000, 1000, "@10000 RPM"); // Should clamp to 1000rpm
}

TEST(TestFloppy, TestHeadCount)
{
	TestFloppy floppy;

	EXPECT_EQ(1, floppy.GetHeadCount()) << "Default";

	floppy.SetHeadCount(0);
	EXPECT_EQ(1, floppy.GetHeadCount()) << "Should clamp to 1";
	floppy.SetHeadCount(1);
	EXPECT_EQ(1, floppy.GetHeadCount());
	floppy.SetHeadCount(2);
	EXPECT_EQ(2, floppy.GetHeadCount());
	floppy.SetHeadCount(3);
	EXPECT_EQ(2, floppy.GetHeadCount()) << "Should clamp to 2";
}

TEST(TestFloppy, TestHeadSelect)
{
	TestFloppy floppy;

	EXPECT_EQ(0, floppy.GetCurrentHead()) << "Default";

	floppy.SetHeadCount(1);
	floppy.SelectHead(0);
	EXPECT_EQ(0, floppy.GetCurrentHead());
	floppy.SelectHead(1); // Invalid head 
	EXPECT_EQ(0, floppy.GetCurrentHead()) << "Should reset to 0";

	floppy.SetHeadCount(2);
	floppy.SelectHead(0);
	EXPECT_EQ(0, floppy.GetCurrentHead());
	floppy.SelectHead(1);
	EXPECT_EQ(1, floppy.GetCurrentHead());
}

TEST(TestFloppy, TestStepDelay)
{
	TestFloppy floppy;

	EXPECT_EQ(10, floppy.GetStepDelay()) << "Default";

	floppy.SetStepDelay(0);
	EXPECT_EQ(1, floppy.GetStepDelay()) << "Should clamp to 1 ms";

	floppy.SetStepDelay(15);
	EXPECT_EQ(15, floppy.GetStepDelay());

	floppy.SetStepDelay(1000);
	EXPECT_EQ(100, floppy.GetStepDelay()) << "Should clamp to 100 ms";
}

TEST(TestFloppy, TestTrackCount)
{
	TestFloppy floppy;

	EXPECT_EQ(40, floppy.GetTrackCount()) << "Default";

	floppy.SetTrackCount(0);
	EXPECT_EQ(10, floppy.GetTrackCount()) << "Should clamp to 10 tracks";

	floppy.SetTrackCount(40);
	EXPECT_EQ(40, floppy.GetTrackCount());

	floppy.SetTrackCount(1000);
	EXPECT_EQ(100, floppy.GetTrackCount()) << "Should clamp to 100 tracks";
}

TEST(TestFloppy, TestStepDirection)
{
	TestFloppy floppy;
	EXPECT_EQ(StepDirection::INNER, floppy.GetStepDirection());

	floppy.SetStepDirection(StepDirection::OUTER);
	EXPECT_EQ(StepDirection::OUTER, floppy.GetStepDirection());

	floppy.SetStepDirection(StepDirection::INNER);
	EXPECT_EQ(StepDirection::INNER, floppy.GetStepDirection());
}

TEST(TestFloppy, TestCalibration)
{
	TestFloppy floppy;

	EXPECT_FALSE(floppy.IsCalibrating());
	
	// TODO
}

int Step(TestFloppy& floppy, StepDirection dir, int stepDelay = 10)
{
	SCOPED_TRACE("Step");

	floppy.SetStepDelay(stepDelay);
	floppy.SetStepDirection(dir);

	floppy.Step();
	EXPECT_TRUE(floppy.IsSeeking());

	int elapsed;
	for (elapsed = 0; elapsed < TIME_100MS; ++elapsed)
	{
		floppy.Tick();
		if (!floppy.IsSeeking())
		{
			break;
		}
	}
	EXPECT_FALSE(floppy.IsSeeking());
	EXPECT_NEAR(DelayToTicks(stepDelay), elapsed, DelayToTicks(1));
	return floppy.GetCurrentTrack();
}

TEST(TestFloppy, TestStep1)
{
	TestFloppy floppy(true); // Force "Active"
	floppy.EnableMotor(true);

	EXPECT_FALSE(floppy.IsSeeking());
	EXPECT_TRUE(floppy.IsTrack0());
	EXPECT_EQ(0, floppy.GetCurrentTrack());
	floppy.SetStepDelay(10); // 10 ms
	floppy.SetTrackCount(40);

	// Step 40 tracks (towards INNER tracks)
	for (int track = 0; track < 40; ++track)
	{
		static char buf[32];
		sprintf(buf, "Track %d->%d", track - 1, track);
		SCOPED_TRACE(buf);

		int newTrack = Step(floppy, StepDirection::INNER);
		EXPECT_EQ(track + 1, newTrack) << "New Track";
	}

	// Step out again
	for (int i = 0; i < 2; ++i)
	{
		int newTrack = Step(floppy, StepDirection::INNER);
		EXPECT_EQ(40, newTrack) << "Step out";
	}

	// Step in
	int newTrack = Step(floppy, StepDirection::OUTER);
	EXPECT_EQ(39, newTrack) << "Step in";
}

TEST(TestFloppy, TestStep2)
{
	TestFloppy floppy(true); // Force "Active"
	floppy.EnableMotor(true);

	EXPECT_FALSE(floppy.IsSeeking());
	EXPECT_TRUE(floppy.IsTrack0());
	EXPECT_EQ(0, floppy.GetCurrentTrack());
	floppy.SetTrackCount(20);

	// Step one track in
	int newTrack = Step(floppy, StepDirection::INNER, 30);
	EXPECT_EQ(1, newTrack) << "Track 1";
	EXPECT_FALSE(floppy.IsTrack0());

	// Step one track out
	newTrack = Step(floppy, StepDirection::OUTER, 30);
	EXPECT_EQ(0, newTrack) << "Track 0";
	EXPECT_TRUE(floppy.IsTrack0());

	// Step one track out again
	newTrack = Step(floppy, StepDirection::OUTER, 30);
	EXPECT_EQ(0, newTrack) << "Track 0";
	EXPECT_TRUE(floppy.IsTrack0());
}