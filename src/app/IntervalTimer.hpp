#ifndef BLANK_APP_INTERVALTIMER_HPP
#define BLANK_APP_INTERVALTIMER_HPP


namespace blank {

/// Timer that hits every n milliseconds. Resolution is that of the
/// delta values passed to Update(), minimum 1ms.
/// Also tracks the number of iterations as well as milliseconds
/// passed.
class IntervalTimer {

public:
	/// Create a timer that hits every interval_ms milliseconds.
	/// Initial state is stopped.
	explicit IntervalTimer(int interval_ms = 0) noexcept
	: intv(interval_ms) { }

	void Start() noexcept {
		speed = 1;
	}
	void Stop() noexcept {
		value = 0;
		speed = 0;
	}
	void Reset() noexcept {
		value = 0;
	}

	bool Running() const noexcept {
		return speed != 0;
	}
	/// true if an interval boundary was passed by the last call to Update()
	bool Hit() const noexcept {
		return Running() && value % intv < last_dt;
	}
	bool HitOnce() const noexcept {
		return Running() && value >= intv;
	}
	int Elapsed() const noexcept {
		return value;
	}
	int Interval() const noexcept {
		return intv;
	}
	int Iteration() const noexcept {
		return value / intv;
	}
	void PopIteration() noexcept {
		value -= intv;
	}

	void Update(int dt) noexcept {
		value += dt * speed;
		last_dt = dt;
	}

private:
	int intv;
	int value = 0;
	int speed = 0;
	int last_dt = 0;

};

}

#endif
