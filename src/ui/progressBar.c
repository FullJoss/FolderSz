#include "theme.h"
#include "progressBar.h"
#include "dpi.h"


void PaintProgressBar(HDC hdc, ProgressBar* prBr) {

	int heightPrBr = DPI_CONST(11);
	RECT rcPrBrBg = { 
		prBr->bounds.left, 
		prBr->bounds.bottom - heightPrBr,
		prBr->bounds.right, 
		prBr->bounds.bottom 
	};

	FillRect(hdc, &rcPrBrBg, g_brushs[IC_LV_TRACK]);

	int widthPrBr = rcPrBrBg.right - rcPrBrBg.left;
	int widthValue = (float)(widthPrBr) * prBr->value;
	RECT rcValue = {
		rcPrBrBg.left,
		rcPrBrBg.top,
		rcPrBrBg.left + widthValue,
		rcPrBrBg.bottom
	};

	FillRect(hdc, &rcValue, g_brushs[IC_LV_THUMB]);
	FrameRect(hdc, &rcPrBrBg, g_brushs[IC_LV_BORDER]);

	ExtTextOutW(hdc,
		prBr->bounds.left,
		prBr->bounds.top,
		ETO_CLIPPED,
		&prBr->bounds,
		prBr->text,
		(UINT)wcslen(prBr->text),
		NULL);
}