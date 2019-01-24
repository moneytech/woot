#ifndef UI_H
#define UI_H

#include <woot/font.h>
#include <woot/pixmap.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct wmEvent;

struct uiControl;
struct uiLabel;
struct uiButton;
struct uiLineEdit;
struct uiSlider;

typedef void (*uiEventHandler)(struct uiControl *sender);
typedef void (*uiWMEventHandler)(struct uiControl *sender, struct wmEvent *event);

struct uiControl *uiControlCreate(struct uiControl *parent, size_t structSize, struct pmPixMap *parentPixMap, int x, int y, int width, int height, const char *text, uiEventHandler onCreate);
void uiControlDelete(struct uiControl *control);
void uiControlRedraw(struct uiControl *control);
struct pmPixMap *uiControlGetPixMap(struct uiControl *control);
void *uiControlGetContext(struct uiControl *control);
void uiControlSetContext(struct uiControl *control, void *context);
char *uiControlGetText(struct uiControl *control);
void uiControlSetText(struct uiControl *control, const char *text);
int uiControlProcessEvent(struct uiControl *control, struct wmEvent event);
void uiControlSetTextColor(struct uiControl *control, union pmColor color);
void uiControlSetBackColor(struct uiControl *control, union pmColor color);
void uiControlSetOnPaint(struct uiControl *control, uiEventHandler handler);
void uiControlSetOnMousePress(struct uiControl *control, uiWMEventHandler handler);
void uiControlSetOnMouseRelease(struct uiControl *control, uiWMEventHandler handler);
void uiControlSetOnMouseMove(struct uiControl *control, uiWMEventHandler handler);

struct uiLabel *uiLabelCreate(struct uiControl *parent, int x, int y, int width, int height, const char *text, struct fntFont *font, uiEventHandler onCreate);
void uiLabelSetHorizontalCentering(struct uiLabel *control, int value);
void uiLabelDelete(struct uiLabel *control);

struct uiButton *uiButtonCreate(struct uiControl *parent, int x, int y, int width, int height, const char *text, struct fntFont *font, uiEventHandler onCreate);
void uiButtonDelete(struct uiButton *control);

struct uiLineEdit *uiLineEditCreate(struct uiControl *parent, int x, int y, int width, int height, const char *text, struct fntFont *font, uiEventHandler onCreate);
void uiLineEditDelete(struct uiLineEdit *control);

struct uiSlider *uiSliderCreate(struct uiControl *parent, int x, int y, int width, int height, int horizontal, int minVal, int maxVal, int val);
void uiSliderSetValue(struct uiSlider *control, int value);
int uiSliderGetValue(struct uiSlider *control);
void uiSliderSetMinValue(struct uiSlider *control, int value);
int uiSliderGetMinValue(struct uiSlider *control);
void uiSliderSetMaxValue(struct uiSlider *control, int value);
int uiSliderGetMaxValue(struct uiSlider *control);
void uiSliderDelete(struct uiSlider *control);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // UI_H
