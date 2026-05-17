#pragma once

namespace OverlayLayout
{
	static const float TargetOffsetX = 87.0f;
	static const float MenuOffsetX = 58.0f;

	static float GetTargetCenterX(float displayWidth)
	{
		return displayWidth * 0.5f + TargetOffsetX;
	}

	static float GetMenuX(float displayWidth)
	{
		return GetTargetCenterX(displayWidth) + MenuOffsetX;
	}
}
