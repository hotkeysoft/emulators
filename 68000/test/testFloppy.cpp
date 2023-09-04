#include "stdafx.h"
#include "gtest/gtest.h"

#include "../Storage/DeviceFloppy.h"

constexpr size_t CLOCK_SPEED = 1000000;

class TestFloppy : public fdd::DeviceFloppy
{
public:
	TestFloppy() : fdd::DeviceFloppy(CLOCK_SPEED) {}
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
	EXPECT_EQ(0, floppy.GetActiveHead());
	EXPECT_EQ(0, floppy.GetCurrentSector());
	EXPECT_EQ(0, floppy.GetCurrentTrack());
	EXPECT_FALSE(floppy.GetMotorPulse());
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
	TestFloppy floppy;

	floppy.SetMotorRPM(rpm);
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
