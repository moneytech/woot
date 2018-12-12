#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <woot/font.h>
#include <woot/pixmap.h>
#include <woot/ui.h>
#include <woot/wm.h>

struct uiControl
{
    struct uiControl *Next;
    struct uiControl *Parent;
    struct uiControl *Children;

    int X, Y;
    struct pmPixMap *Content;
    void *Context;
    char *Text;
    union pmColor TextColor;
    union pmColor BackColor;

    uiEventHandler OnCreate;
    uiEventHandler OnDelete;
    uiEventHandler OnPaint;

    uiWMEventHandler OnKeyPress;
    uiWMEventHandler OnKeyRelease;

    uiWMEventHandler PreMouseMove;
    uiWMEventHandler OnMouseMove;
    uiWMEventHandler PostMouseMove;

    uiWMEventHandler PreMousePress;
    uiWMEventHandler OnMousePress;
    uiWMEventHandler PostMousePress;

    uiWMEventHandler PreMouseRelease;
    uiWMEventHandler OnMouseRelease;
    uiWMEventHandler PostMouseRelease;
};

struct uiLabel
{
    struct uiControl Control;
    struct fntFont *Font;
};

struct uiButton
{
    struct uiControl Control;
    struct fntFont *Font;
};

struct uiLineEdit
{
    struct uiControl Control;
    struct fntFont *Font;
};

struct uiControl *uiControlCreate(struct uiControl *parent, struct pmPixMap *parentPixMap, int x, int y, int width, int height, const char *text, uiEventHandler onCreate)
{
    struct uiControl *control = (struct uiControl *)calloc(1, sizeof(struct uiControl));
    if(!control) return NULL;
    control->Parent = parent;
    if(control->Parent)
    {   // bind new control with the rest of the ui
        struct uiControl *ctrl;
        for(ctrl = control->Parent->Children; ctrl && ctrl->Next; ctrl = ctrl->Next);
        if(!ctrl) control->Parent->Children = control;
        else ctrl->Next = control;
    }
    control->X = x;
    control->Y = y;
    control->Content = pmSubPixMap(parent ? parent->Content : parentPixMap, x, y, width, height);
    if(!control->Content)
    {
        uiControlDelete(control);
        return NULL;
    }
    control->Text = strdup(text ? text : "");
    control->TextColor = pmColorBlack;
    control->BackColor = pmColorTransparent;
    control->OnCreate = onCreate;
    if(control->OnCreate)
        control->OnCreate(control);
    return control;
}

void uiControlDelete(struct uiControl *control)
{
    if(!control) return;
    if(control->OnDelete)
        control->OnDelete(control);
    // delete child controls
    for(struct uiControl *ctrl = control->Children; ctrl; ctrl = ctrl->Next)
        uiControlDelete(ctrl);
    if(control->Parent)
    {   // unbind this control from the rest of the ui
        for(struct uiControl *ctrl = control->Parent->Children; ctrl; ctrl = ctrl->Next)
        {
            if(ctrl == control)
                control->Parent->Children = control->Next;
            else if(ctrl->Next == control)
            {
                ctrl->Next = control->Next;
                break;
            }
        }
    }
    if(control->Content)
        pmDelete(control->Content);
    if(control->Text)
        free(control->Text);
}

void uiControlRedraw(struct uiControl *control)
{
    if(control->BackColor.A != 0)
    {
        struct wmRectangle rect = pmGetRectangle(control->Content);
        pmFillRectangle(control->Content, 0, 0, rect.Width, rect.Height, control->BackColor);
    }
    if(control->OnPaint)
        control->OnPaint(control);
    for(struct uiControl *ctrl = control->Children; ctrl; ctrl = ctrl->Next)
        uiControlRedraw(ctrl);
}

struct pmPixMap *uiControlGetPixMap(struct uiControl *control)
{
    if(!control) return NULL;
    return control->Content;
}

char *uiControlGetText(struct uiControl *control)
{
    if(!control) return NULL;
    return control->Text;
}

int uiControlProcessEvent(struct uiControl *control, struct wmEvent event)
{
    if(!control) return -EINVAL;
    if(event.Type == ET_MOUSE)
    {
        for(struct uiControl *ctrl = control->Children; ctrl; ctrl = ctrl->Next)
        {
            struct wmRectangle rect = pmGetRectangle(ctrl->Content);
            if(wmRectangleContainsPoint(&rect, event.Mouse.X, event.Mouse.Y))
            {
                event.Mouse.X -= rect.X;
                event.Mouse.Y -= rect.Y;
                int res = uiControlProcessEvent(ctrl, event);
                if(res) return res;
            }
        }

        if((event.Mouse.DeltaX || event.Mouse.DeltaY))
        {
            if(control->PreMouseMove) control->PreMouseMove(control, &event);
            if(control->OnMouseMove) control->OnMouseMove(control, &event);
            if(control->PostMouseMove) control->PostMouseMove(control, &event);
        }

        if(event.Mouse.ButtonsPressed)
        {
            if(control->PreMousePress) control->PreMousePress(control, &event);
            if(control->OnMousePress) control->OnMousePress(control, &event);
            if(control->PostMousePress) control->PostMousePress(control, &event);
        }

        if(control->OnMouseRelease && event.Mouse.ButtonsReleased)
        {
            if(control->PreMouseRelease) control->PreMouseRelease(control, &event);
            if(control->OnMouseRelease) control->OnMouseRelease(control, &event);
            if(control->PostMouseRelease) control->PostMouseRelease(control, &event);
        }
    }
    return 0;
}

void uiControlSetTextColor(struct uiControl *control, union pmColor color)
{
    if(!control) return;
    control->TextColor = color;
}

void uiControlSetBackColor(struct uiControl *control, union pmColor color)
{
    if(!control) return;
    control->BackColor = color;
}

void uiControlSetOnMousePress(struct uiControl *control, uiWMEventHandler handler)
{
    if(!control) return;
    control->OnMousePress = handler;
}

void uiControlSetOnMouseRelease(struct uiControl *control, uiWMEventHandler handler)
{
    if(!control) return;
    control->OnMouseRelease = handler;
}

void uiControlSetOnMouseMove(struct uiControl *control, uiWMEventHandler handler)
{
    if(!control) return;
    control->OnMouseMove = handler;
}

static void labelPaint(struct uiControl *sender)
{
    struct uiLabel *label = (struct uiLabel *)sender;
    struct wmRectangle rect = pmGetRectangle(label->Control.Content);
    float height = fntGetPixelHeight(label->Font);
    fntDrawString(label->Font, label->Control.Content, 2, (rect.Height - height) / 2, label->Control.Text, label->Control.TextColor);
    pmInvalidateRect(label->Control.Content, rect);
}

struct uiLabel *uiLabelCreate(struct uiControl *parent, int x, int y, int width, int height, const char *text, struct fntFont *font, uiEventHandler onCreate)
{
    struct uiLabel *control = (struct uiLabel *)uiControlCreate(parent, NULL, x, y, width, height, text, onCreate);
    if(!control) return NULL;
    control->Font = font ? font : wmGetDefaultFont();
    if(!control->Font)
    {
        uiLabelDelete(control);
        return NULL;
    }
    control->Control.OnPaint = labelPaint;
    return control;
}

void uiLabelDelete(struct uiLabel *control)
{
    uiControlDelete((struct uiControl *)control);
}

static void buttonPaint(struct uiControl *sender)
{
    struct uiButton *button = (struct uiButton *)sender;
    struct wmRectangle rect = pmGetRectangle(button->Control.Content);
    float width = fntMeasureString(button->Font, button->Control.Text);
    float height = fntGetPixelHeight(button->Font);
    fntDrawString(button->Font, button->Control.Content, (rect.Width - width) / 2, (rect.Height - height) / 2, button->Control.Text, button->Control.TextColor);
    pmDrawFrame(button->Control.Content, 0, 0, rect.Width, rect.Height, 0);
    pmInvalidateRect(button->Control.Content, rect);
}

struct uiButton *uiButtonCreate(struct uiControl *parent, int x, int y, int width, int height, const char *text, struct fntFont *font, uiEventHandler onCreate)
{
    struct uiButton *control = (struct uiButton *)uiControlCreate(parent, NULL, x, y, width, height, text, onCreate);
    if(!control) return NULL;
    control->Font = font ? font : wmGetDefaultFont();
    if(!control->Font)
    {
        uiButtonDelete(control);
        return NULL;
    }
    control->Control.OnPaint = buttonPaint;
    return control;
}

void uiButtonDelete(struct uiButton *control)
{
    uiControlDelete((struct uiControl *)control);
}

static void lineEditPaint(struct uiControl *sender)
{
    struct uiLineEdit *edit = (struct uiLineEdit *)sender;
    struct wmRectangle rect = pmGetRectangle(edit->Control.Content);
    pmFillRectangle(edit->Control.Content, 0, 0, rect.Width, rect.Height, edit->Control.BackColor);
    float height = fntGetPixelHeight(edit->Font);
    fntDrawString(edit->Font, edit->Control.Content, 2, (rect.Height - height) / 2, edit->Control.Text, edit->Control.TextColor);
    pmDrawFrame(edit->Control.Content, 0, 0, rect.Width, rect.Height, 1);
    pmDrawFrame(edit->Control.Content, 1, 1, rect.Width - 2, rect.Height - 2, 1);
    pmInvalidateRect(edit->Control.Content, rect);
}

struct uiLineEdit *uiLineEditCreate(struct uiControl *parent, int x, int y, int width, int height, const char *text, struct fntFont *font, uiEventHandler onCreate)
{
    struct uiLineEdit *control = (struct uiLineEdit *)uiControlCreate(parent, NULL, x, y, width, height, text, onCreate);
    if(!control) return NULL;
    control->Font = font ? font : wmGetDefaultFont();
    if(!control->Font)
    {
        uiLineEditDelete(control);
        return NULL;
    }
    control->Control.BackColor = pmColorWhite;
    control->Control.OnPaint = lineEditPaint;
    return control;
}

void uiLineEditDelete(struct uiLineEdit *control)
{
    uiControlDelete((struct uiControl *)control);
}

