#ifndef _DISPLAY_PANEL_H_
#define _DISPLAY_PANEL_H_

// LCDC Content Struct
typedef struct {
	UINT32	width;
	UINT32	height;
	UINT32	frequency;					// pixel clock frequency in Hz
	UINT32	Bpp;

	// LCD panel specific settings
	UINT32	dwHWidth;
	UINT32	dwHSyncPulseWidth;
	UINT32	dwHFrontPorch;
	UINT32	dwHBackPorch;				// including dwHSyncPulseWidth
	UINT32	dwVHeight;
	UINT32	dwVSyncPulseWidth;
	UINT32	dwVFrontPorch;
	UINT32	dwVBackPorch;				// including dwVSyncPulseWidth
} DISPLAY_PANEL_MODE, *PDISPLAY_PANEL_MODE;

////////////////////////////////////*/
#endif /* !_DISPLAY_PANEL_H_ */

