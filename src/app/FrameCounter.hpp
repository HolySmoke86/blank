#ifndef BLANK_APP_FRAMECOUNTER_HPP_
#define BLANK_APP_FRAMECOUNTER_HPP_

#include <iosfwd>
#include <SDL.h>


namespace blank {

class FrameCounter {

public:
	template<class T>
	struct Frame {
		T handle = T(0);
		T update = T(0);
		T render = T(0);
		T running = T(0);
		T waiting = T(0);
		T total = T(0);
	};


public:
	void EnterFrame() noexcept;
	void EnterHandle() noexcept;
	void ExitHandle() noexcept;
	void EnterUpdate() noexcept;
	void ExitUpdate() noexcept;
	void EnterRender() noexcept;
	void ExitRender() noexcept;
	void ExitFrame() noexcept;

	const Frame<int> &Peak() const noexcept { return peak; }
	const Frame<float> &Average() const noexcept { return avg; }

	bool Changed() const noexcept { return changed; }

	void Print(std::ostream &) const;

private:
	int Tick() noexcept;

	void Accumulate() noexcept;
	void Push() noexcept;

private:
	static constexpr int NUM_FRAMES = 32;
	static constexpr float factor = 1.0f / float(NUM_FRAMES);

	Uint32 last_enter = 0;
	Uint32 last_tick = 0;

	int cur_frame = 0;
	Frame<int> current = Frame<int>{};
	Frame<int> sum = Frame<int>{};
	Frame<int> max = Frame<int>{};

	Frame<int> peak = Frame<int>{};
	Frame<float> avg = Frame<float>{};

	bool changed = false;

};

}

#endif
