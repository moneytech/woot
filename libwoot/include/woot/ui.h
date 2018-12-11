#ifndef UI_H
#define UI_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct wmEvent;

struct uiControl;
struct uiLabel;
struct uiButton;

typedef void (*uiEventHandler)(struct uiControl *sender);
typedef void (*uiWMEventHandler)(struct uiControl *sender, struct wmEvent *event);

struct uiControl *uiControlCreate(struct uiControl *parent, struct pmPixMap *parentPixMap, int x, int y, int width, int height, const char *text, uiEventHandler onCreate);
void uiControlDelete(struct uiControl *control);
void uiControlRedraw(struct uiControl *control);
struct pmPixMap *uiControlGetPixMap(struct uiControl *control);
char *uiControlGetText(struct uiControl *control);
int uiControlProcessEvent(struct uiControl *control, struct wmEvent event);
void uiControlSetOnMousePress(struct uiControl *control, uiWMEventHandler handler);
void uiControlSetOnMouseRelease(struct uiControl *control, uiWMEventHandler handler);
void uiControlSetOnMouseMove(struct uiControl *control, uiWMEventHandler handler);

struct uiLabel *uiLabelCreate(struct uiControl *parent, int x, int y, int width, int height, const char *text, struct fntFont *font, uiEventHandler onCreate);
void uiLabelDelete(struct uiLabel *control);

struct uiButton *uiButtonCreate(struct uiControl *parent, int x, int y, int width, int height, const char *text, struct fntFont *font, uiEventHandler onCreate);
void uiButtonDelete(struct uiButton *control);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // UI_H
