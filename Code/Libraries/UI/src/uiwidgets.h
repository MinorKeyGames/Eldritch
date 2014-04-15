#ifndef ADDUIWIDGETFACTORY

#include "Widgets/uiwidget-text.h"
#include "Widgets/uiwidget-image.h"
#include "Widgets/uiwidget-slider.h"
#include "Widgets/uiwidget-frame.h"
#include "Widgets/uiwidget-spacer.h"
#include "Widgets/uiwidget-composite.h"

#else

ADDUIWIDGETFACTORY( Text );
ADDUIWIDGETFACTORY( Image );
ADDUIWIDGETFACTORY( Slider );
ADDUIWIDGETFACTORY( Frame );
ADDUIWIDGETFACTORY( Spacer );
ADDUIWIDGETFACTORY( Composite );

#endif