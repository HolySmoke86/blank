#ifndef BLANK_APP_FRAMECOUNTER_HPP_
#define BLANK_APP_FRAMECOUNTER_HPP_

#include <iosfwd>
#include <SDL.h>


namespace blank {

class FrameCounter {

public:
	void EnterFrame() noexcept;
	void EnterHandle() noexcept;
	void ExitHandle() noexcept;
	void EnterUpdate() noexcept;
	void ExitUpdate() noexcept;
	void EnterRender() noexcept;
	void ExitRender() noexcept;
	void ExitFrame() noexcept;

	float AvgHandle() const noexcept { return avg.handle; }
	float AvgUpdate() const noexcept { return avg.update; }
	float AvgRender() const noexcept { return avg.render; }
	float AvgFrame() const noexcept { return avg.total; }
	float AvgRunning() const noexcept { return avg.handle + avg.update + avg.render; }
	float AvgWaiting() const noexcept { return avg.total - AvgRunning(); }

	bool Changed() const noexcept { return changed; }

	void Print(std::ostream &) const;

private:
	int Tick() noexcept;

private:
	static constexpr int NUM_FRAMES = 32;
	static constexpr float factor = 1.0f / float(NUM_FRAMES);

	template<class T>
	struct Frame {
		T handle = T(0);
		T update = T(0);
		T render = T(0);
		T total = T(0);
	};

	Uint32 last_enter = 0;
	Uint32 last_tick = 0;

	int cur_frame = 0;
	Frame<int> running = Frame<int>{};
	Frame<float> avg = Frame<float>{};
	bool changed = false;

};

}

#endif
