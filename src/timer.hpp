#ifndef BLANK_TIMER_HPP
#define BLANK_TIMER_HPP


namespace blank {

class IntervalTimer {

public:
	explicit IntervalTimer(int interval_ms) noexcept
	: intv(interval_ms) { }

	void Start() noexcept {
		speed = 1;
	}
	void Stop() noexcept {
		value = 0;
		speed = 0;
	}

	bool Running() const noexcept {
		return speed != 0;
	}
	bool Hit() const noexcept {
		return Running() && value % intv < last_dt;
	}
	int Elapsed() const noexcept {
		return value;
	}
	int Iteration() const noexcept {
		return value / intv;
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
