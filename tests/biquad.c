
/*
 * Biquad filter regression tests
 *
 * Verify frequency response, coefficient validity,
 * and numerical stability of the biquad implementations.
 */

#include <math.h>
#include <stdio.h>

#define SAMPLES_PER_SEC 48000.0f

#include "../audio/util.h"
#include "../audio/biquad.h"

#define PI 3.14159265358979323846f
#define TEST_SAMPLES 48000
#define WARMUP_SAMPLES 4096

typedef void (*filter_setup_fn)(struct biquad *);

static int failures;

static void report(const char *name, int passed)
{
	printf("%s: %s\n", name, passed ? "PASS" : "FAIL");

	if (!passed)
		failures++;
}

static int finite_coefficients(const struct biquad_coeff *c)
{
	return isfinite(c->b0) &&
	       isfinite(c->b1) &&
	       isfinite(c->b2) &&
	       isfinite(c->a1) &&
	       isfinite(c->a2);
}

static int finite_processing(filter_setup_fn setup)
{
	struct biquad bq = { 0 };

	setup(&bq);

	if (!finite_coefficients(&bq.coeff))
		return 0;

	/* Test an impulse followed by silence. */
	for (int i = 0; i < TEST_SAMPLES; i++) {
		float input = (i == 0) ? 1.0f : 0.0f;
		float output = biquad_step(&bq, input);

		if (!isfinite(output))
			return 0;
	}

	return 1;
}

static float measure_rms(struct biquad *bq, float frequency)
{
	double sum = 0.0;
	unsigned int count = 0;

	for (int i = 0; i < TEST_SAMPLES; i++) {
		float phase = 2.0f * PI * frequency *
			      (float)i / SAMPLES_PER_SEC;
		float input = sinf(phase);
		float output = biquad_step(bq, input);

		if (i >= WARMUP_SAMPLES) {
			sum += (double)output * output;
			count++;
		}
	}

	return (float)sqrt(sum / count);
}

static float response_at(filter_setup_fn setup, float frequency)
{
	struct biquad bq = { 0 };

	setup(&bq);

	return measure_rms(&bq, frequency);
}

/* Filter configurations */

static void setup_lpf(struct biquad *bq)
{
	biquad_lpf(bq, 1000.0f, 0.707f);
}

static void setup_hpf(struct biquad *bq)
{
	biquad_hpf(bq, 1000.0f, 0.707f);
}

static void setup_notch(struct biquad *bq)
{
	biquad_notch_filter(bq, 2000.0f, 5.0f);
}

static void setup_bpf(struct biquad *bq)
{
	biquad_bpf(bq, 2000.0f, 5.0f);
}

static void setup_bpf_peak(struct biquad *bq)
{
	biquad_bpf_peak(bq, 2000.0f, 5.0f);
}

static void setup_allpass(struct biquad *bq)
{
	biquad_allpass_filter(bq, 1000.0f, 0.707f);
}

static void setup_peaking(struct biquad *bq)
{
	biquad_peaking(bq, 1000.0f, 0.707f, 2.0f);
}

static void setup_lowshelf(struct biquad *bq)
{
	biquad_lowshelf(bq, 1000.0f, 0.707f, 2.0f);
}

static void setup_highshelf(struct biquad *bq)
{
	biquad_highshelf(bq, 1000.0f, 0.707f, 2.0f);
}

static void test_filter_stability(void)
{
	report("Low-pass stability", finite_processing(setup_lpf));
	report("High-pass stability", finite_processing(setup_hpf));
	report("Notch stability", finite_processing(setup_notch));
	report("Band-pass stability", finite_processing(setup_bpf));
	report("Band-pass peak stability", finite_processing(setup_bpf_peak));
	report("All-pass stability", finite_processing(setup_allpass));
	report("Peaking stability", finite_processing(setup_peaking));
	report("Low-shelf stability", finite_processing(setup_lowshelf));
	report("High-shelf stability", finite_processing(setup_highshelf));
}

static void test_frequency_response(void)
{
	float low;
	float high;

	/* Low-pass should preserve low frequencies and attenuate high ones. */
	low = response_at(setup_lpf, 100.0f);
	high = response_at(setup_lpf, 10000.0f);

	report("Low-pass frequency response",
	       low > 0.60f && high < 0.20f);

	/* High-pass should attenuate low frequencies and preserve high ones. */
	low = response_at(setup_hpf, 100.0f);
	high = response_at(setup_hpf, 10000.0f);

	report("High-pass frequency response",
	       low < 0.20f && high > 0.60f);

	/* Notch should reject its center frequency. */
	low = response_at(setup_notch, 500.0f);
	high = response_at(setup_notch, 2000.0f);

	report("Notch center-frequency rejection",
	       low > 0.50f && high < 0.25f);

	/* Band-pass should preserve its center frequency. */
	low = response_at(setup_bpf, 500.0f);
	high = response_at(setup_bpf, 2000.0f);

	report("Band-pass center-frequency response",
	       low < 0.30f && high > 0.50f);
}

int main(void)
{
	printf("AudioNoise biquad regression tests\n");
	printf("Sample rate: %.0f Hz\n\n", SAMPLES_PER_SEC);

	test_filter_stability();

	printf("\nFrequency response tests\n");
	test_frequency_response();

	printf("\n%d test group(s) failed\n", failures);

	return failures ? 1 : 0;
}
