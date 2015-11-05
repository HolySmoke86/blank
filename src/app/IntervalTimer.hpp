#ifndef BLANK_APP_INTERVALTIMER_HPP
#define BLANK_APP_INTERVALTIMER_HPP

#include <cmath>


namespace blank {

/// Timer that hits every n Time units. Resolution is that of the
/// delta values passed to Update().
/// Also tracks the number of iterations as well as Time units
/// passed.
template<class Time = int>
class IntervalTimer {

public:
	/// Create a timer that hits every interval Time units.
	/// Initial state is stopped.
	explicit IntervalTimer(Time interval_ms = Time(0)) noexcept
	: intv(interval_ms) { }

	void Start() noexcept {
		speed = Time(1);
	}
	void Stop() noexcept {
		value = Time(0);
		speed = Time(0);
	}
	void Reset() noexcept {
		value = Time(0);
	}

	bool Running() const noexcept {
		return speed != Time(0);
	}
	/// true if an interval boundary was passed by the last call to Update()
	bool Hit() const noexcept {
		return Running() && IntervalElapsed() < last_dt;
	}
	bool HitOnce() const noexcept {
		return Running() && value >= intv;
	}
	Time Elapsed() const noexcept {
		return value;
	}
	Time Interval() const noexcept {
		return intv;
	}
	Time IntervalElapsed() const noexcept {
		return mod(value, intv);
	}
	Time IntervalRemain() const noexcept {
		return intv - IntervalElapsed();
	}
	int Iteration() const noexcept {
		return value / intv;
	}
	void PopIteration() noexcept {
		value -= intv;
	}

	void Update(Time dt) noexcept {
		value += dt * speed;
		last_dt = dt;
	}

	static Time mod(Time val, Time m) noexcept {
		return val % m;
	}

private:
	Time intv;
	Time value = Time(0);
	Time speed = Time(0);
	Time last_dt = Time(0);

};

using CoarseTimer = IntervalTimer<int>;
using FineTimer = IntervalTimer<float>;

template<>
inline float IntervalTimer<float>::mod(float val, float m) noexcept {
	return std::fmod(val, m);
}

template<>
inline int IntervalTimer<float>::Iteration() const noexcept {
	return std::floor(value / intv);
}

}

#endif
