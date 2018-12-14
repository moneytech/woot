#ifndef UI_H
#define UI_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct wmEvent;

struct uiControl;
struct uiLabel;
struct uiButton;
struct uiLineEdit;

typedef void (*uiEventHandler)(struct uiControl *sender);
typedef void (*uiWMEventHandler)(struct uiControl *sender, struct wmEvent *event);

struct uiControl *uiControlCreate(struct uiControl *parent, struct pmPixMap *parentPixMap, int x, int y, int width, int height, const char *text, uiEventHandler onCreate);
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
void uiControlSetOnMousePress(struct uiControl *control, uiWMEventHandler handler);
void uiControlSetOnMouseRelease(struct uiControl *control, uiWMEventHandler handler);
void uiControlSetOnMouseMove(struct uiControl *control, uiWMEventHandler handler);

struct uiLabel *uiLabelCreate(struct uiControl *parent, int x, int y, int width, int height, const char *text, struct fntFont *font, uiEventHandler onCreate);
void uiLabelDelete(struct uiLabel *control);

struct uiButton *uiButtonCreate(struct uiControl *parent, int x, int y, int width, int height, const char *text, struct fntFont *font, uiEventHandler onCreate);
void uiButtonDelete(struct uiButton *control);

struct uiLineEdit *uiLineEditCreate(struct uiControl *parent, int x, int y, int width, int height, const char *text, struct fntFont *font, uiEventHandler onCreate);
void uiLineEditDelete(struct uiLineEdit *control);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // UI_H
