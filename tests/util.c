
/*
 * Utility function regression tests
 *
 * Verify numerical helpers, conversions, sample-array operations,
 * logarithmic functions, and audio utility calculations.
 */

#include <math.h>
#include <stdio.h>
#include <limits.h>

#define SAMPLES_PER_SEC (48000.0)

#include "../audio/util.h"

#define TEST_EPSILON 0.01f

static int failures;
static int tests;

static void check(const char *name, int passed)
{
	tests++;

	if (passed) {
		printf("PASS: %s\n", name);
	} else {
		printf("FAIL: %s\n", name);
		failures++;
	}
}

static int near(float actual, float expected, float tolerance)
{
	return isfinite(actual) &&
	       fabsf(actual - expected) <= tolerance;
}

static void test_limit_value(void)
{
	check("limit_value zero",
	      near(limit_value(0.0f), 0.0f, TEST_EPSILON));

	check("limit_value positive",
	      limit_value(10.0f) > 0.0f &&
	      limit_value(10.0f) < 1.0f);

	check("limit_value negative",
	      limit_value(-10.0f) < 0.0f &&
	      limit_value(-10.0f) > -1.0f);
}

static void test_fraction_conversion(void)
{
	check("u32_to_fraction zero",
	      near(u32_to_fraction(0), 0.0f, TEST_EPSILON));

	check("u32_to_fraction midpoint",
	      near(u32_to_fraction(0x80000000u), 0.5f,
		   TEST_EPSILON));

	check("fraction_to_u32 midpoint",
	      fraction_to_u32(0.5f) == 0x80000000u);
}

static void test_pot_conversion(void)
{
	check("POT_TO_FLOAT minimum",
	      near(POT_TO_FLOAT(-100), 0.0f, TEST_EPSILON));

	check("POT_TO_FLOAT midpoint",
	      near(POT_TO_FLOAT(0), 0.5f, TEST_EPSILON));

	check("POT_TO_FLOAT maximum",
	      near(POT_TO_FLOAT(100), 1.0f, TEST_EPSILON));

	check("linear midpoint",
	      near(linear(0.5f, 0.0f, 10.0f),
		   5.0f, TEST_EPSILON));

	check("cubic midpoint",
	      near(cubic(0.5f, 0.0f, 8.0f),
		   1.0f, TEST_EPSILON));
}

static void test_sample_array(void)
{
	float samples[4] = { 0 };
	unsigned index = 0;

	sample_array_write(1.0f, &index, samples);
	sample_array_write(2.0f, &index, samples);
	sample_array_write(3.0f, &index, samples);
	sample_array_write(4.0f, &index, samples);

	check("sample array write index",
	      index == 4);

	check("sample array read latest",
	      near(sample_array_read(0.0f, &index, samples),
		   4.0f, TEST_EPSILON));

	check("sample array read previous",
	      near(sample_array_read(1.0f, &index, samples),
		   3.0f, TEST_EPSILON));

	check("sample array fractional interpolation",
	      near(sample_array_read(0.5f, &index, samples),
		   3.5f, TEST_EPSILON));
}

static void test_logarithmic_functions(void)
{
	check("log2f of one",
	      near(log2f(1.0f), 0.0f, TEST_EPSILON));

	check("log2f of two",
	      near(log2f(2.0f), 1.0f, TEST_EPSILON));

	check("log2f of four",
	      near(log2f(4.0f), 2.0f, TEST_EPSILON));

	check("pow2 of zero",
	      near(pow2(0.0f), 1.0f, TEST_EPSILON));

	check("pow2 of one",
	      near(pow2(1.0f), 2.0f, TEST_EPSILON));

	check("pow2 of two",
	      near(pow2(2.0f), 4.0f, TEST_EPSILON));
}

static void test_fast_sincos(void)
{
	struct sincos result;

	result = fastsincos(0.0f);
	check("fastsincos zero phase",
	      near(result.sin, 0.0f, TEST_EPSILON) &&
	      near(result.cos, 1.0f, TEST_EPSILON));

	result = fastsincos(0.25f);
	check("fastsincos quarter phase",
	      near(result.sin, 1.0f, TEST_EPSILON) &&
	      near(result.cos, 0.0f, TEST_EPSILON));

	result = fastsincos(0.5f);
	check("fastsincos half phase",
	      near(result.sin, 0.0f, TEST_EPSILON) &&
	      near(result.cos, -1.0f, TEST_EPSILON));

	result = fastsincos(0.75f);
	check("fastsincos three-quarter phase",
	      near(result.sin, -1.0f, TEST_EPSILON) &&
	      near(result.cos, 0.0f, TEST_EPSILON));
}

static void test_audio_utilities(void)
{
	float half_time = time_constant(100.0f);
	float unity_level = db_to_level(0.0f);
	float positive_level = db_to_level(6.0f);
	float negative_level = db_to_level(-6.0f);

	check("time_constant is finite and positive",
	      isfinite(half_time) && half_time > 0.0f);

	check("time_constant is below one",
	      half_time < 1.0f);

	check("zero dB equals unity level",
	      near(unity_level, 1.0f, TEST_EPSILON));

	check("positive dB increases level",
	      positive_level > 1.0f);

	check("negative dB decreases level",
	      negative_level > 0.0f && negative_level < 1.0f);
}

int main(void)
{
	test_limit_value();
	test_fraction_conversion();
	test_pot_conversion();
	test_sample_array();
	test_logarithmic_functions();
	test_fast_sincos();
	test_audio_utilities();

	printf("\n%d tests, %d failures\n", tests, failures);

	return failures ? 1 : 0;
}
