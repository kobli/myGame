#ifndef PROGRESSBAR_HPP_17_07_15_13_34_08
#define PROGRESSBAR_HPP_17_07_15_13_34_08 
#include <string>
#include <irrlicht.h>

class ProgressBar
{
	public:
		enum ValueDisplayMode {
			None,
			Perc,
			Abs,
		};

		ProgressBar(const irr::core::vector2di& size);

		// progess must be in range 0. - 1. (otherwise clipped)
		void setProgress(float progress);
		float getProgress();

		void setColors(irr::video::SColor background=irr::video::SColor(255,255,255,255), irr::video::SColor progress=irr::video::SColor(255,0,0,0));

		virtual void setLabel(const wchar_t* label) = 0;
		void setMaxValue(int maxVal);
		int getMaxValue();
		void setValueDisplayMode(ValueDisplayMode m);
		std::wstring getValueString();
		irr::core::vector2di getSize();
		irr::video::SColor getBackgroundColor();
		irr::video::SColor getProgressColor();

	private:
		irr::core::vector2di _size;
		float _progress;
		int _maxVal;
		ValueDisplayMode _valDisplayMode;

		irr::video::SColor _backgroundColor;
		irr::video::SColor _progressColor;
};
#endif /* PROGRESSBAR_HPP_17_07_15_13_34_08 */
