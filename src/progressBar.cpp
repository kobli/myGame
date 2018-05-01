#include "progressBar.hpp"
#include <algorithm>
#include <cassert>

ProgressBar::ProgressBar(const irr::core::vector2di& size):
	_size{size}, _maxVal{0}, _valDisplayMode{ValueDisplayMode::None}
{
	setColors();
}

void ProgressBar::setProgress(float progress)
{
	_progress = std::max(0.f, std::min(1.f, progress));
}

float ProgressBar::getProgress()
{
	return _progress;
}

void ProgressBar::setColors(irr::video::SColor background, irr::video::SColor progress)
{
	_backgroundColor = background;
	_progressColor = progress;
}

void ProgressBar::setMaxValue(int maxVal)
{
	_maxVal = maxVal;
}

int ProgressBar::getMaxValue()
{
	return _maxVal;
}

void ProgressBar::setValueDisplayMode(ValueDisplayMode m)
{
	_valDisplayMode = m;
}

std::wstring ProgressBar::getValueString()
{
	std::wstring w;
	if(_valDisplayMode == None)
		w = L"";
	else if(_valDisplayMode  == Perc)
		w = std::to_wstring(int(getProgress()*100)) + L" %";
	else if(_valDisplayMode == Abs)
		w = std::to_wstring(int(getProgress()*getMaxValue())) + L" / " + std::to_wstring(int(getMaxValue()));
	else
		assert(false);
	return w;
}

irr::core::vector2di ProgressBar::getSize()
{
	return _size;
}

irr::video::SColor ProgressBar::getBackgroundColor()
{
	return _backgroundColor;
}

irr::video::SColor ProgressBar::getProgressColor()
{
	return _progressColor;
}
