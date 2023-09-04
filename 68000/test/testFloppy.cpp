#include "stdafx.h"
#include "gtest/gtest.h"

#include "../Storage/DeviceFloppy.h"

constexpr size_t CLOCK_SPEED = 1000000;

Logger::SEVERITY TEST_FLOPPY_LOG_LEVEL = Logger::LOG_INFO;

class TestFloppy : public fdd::DeviceFloppy
{
public:
	TestFloppy() : fdd::DeviceFloppy(CLOCK_SPEED) 
	{ 
		EnableLog(TEST_FLOPPY_LOG_LEVEL);
		Init();
	}
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

	floppy.SetStepDelay(0);
	EXPECT_EQ(1, floppy.GetStepDelay()) << "Should clamp to 1 ms";

	floppy.SetStepDelay(15);
	EXPECT_EQ(15, floppy.GetStepDelay());

	floppy.SetStepDelay(1000);
	EXPECT_EQ(100, floppy.GetStepDelay()) << "Should clamp to 100 ms";
}
