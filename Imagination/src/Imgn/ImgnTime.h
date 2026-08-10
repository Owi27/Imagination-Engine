#pragma once

namespace Imgn
{
	class IMGN_API Time
	{
		std::chrono::duration<float> _time;

	public:
		Time(std::chrono::duration<float> pTime)
		{
			_time = pTime;
		}

		operator float() const { return _time.count(); }

		float Seconds() const { return _time.count(); }
		float MiliSeconds() const { return _time.count() * 1000.f; }
	};

}