// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Core/Application.h>
#include <Fog/Core/Vector.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/Region.h>
#include <Fog/Graphics/Rgba.h>
#include <Fog/UI/Event.h>
#include <Fog/UI/Widget.h>
#include <Fog/UI/UISystem_Def.h>

FOG_IMPLEMENT_OBJECT(Fog::UISystemDefault)
FOG_IMPLEMENT_OBJECT(Fog::UIWindowDefault)

namespace Fog {

// ============================================================================
// [Helpers]
// ============================================================================

#define UI_SYSTEM() \
  reinterpret_cast<UISystemDefault*>(Application::instance()->uiSystem())

static uint32_t fogButtonToId(uint32_t button)
{
  switch (button)
  {
    case ButtonLeft  : return 0;
    case ButtonMiddle: return 1;
    case ButtonRight : return 2;
    default          : return ButtonInvalid;
  }
}

// ============================================================================
// [Fog::UISystemDefault - Construction / Destruction]
// ============================================================================

UISystemDefault::UISystemDefault()
{
  // Clear all
  memset(&_displayInfo, 0, sizeof(_displayInfo));
  memset(&_paletteInfo, 0, sizeof(_paletteInfo));
  memset(&_caretStatus, 0, sizeof(_caretStatus));
  memset(&_keyboardStatus, 0, sizeof(_keyboardStatus));
  memset(&_mouseStatus, 0, sizeof(_mouseStatus));
  memset(&_systemMouseStatus, 0, sizeof(_systemMouseStatus));

  _mouseStatus.position.set(INT_MIN, INT_MIN);
  _systemMouseStatus.position.set(INT_MIN, INT_MIN);

  _wheelLines          = 3;
  _repeatingDelay      = TimeDelta::fromMilliseconds(200);
  _repeatingInterval   = TimeDelta::fromMilliseconds(50);
  _doubleClickInterval = TimeDelta::fromMilliseconds(200);

  _buttonRepeat[0].addListener(EvTimer, this, &UISystemDefault::_onButtonRepeatTimeOut);
  _buttonRepeat[1].addListener(EvTimer, this, &UISystemDefault::_onButtonRepeatTimeOut);
  _buttonRepeat[2].addListener(EvTimer, this, &UISystemDefault::_onButtonRepeatTimeOut);

  memset(&_updateStatus, 0, sizeof(_updateStatus));
}

UISystemDefault::~UISystemDefault()
{
  if (_updateStatus.task) _updateStatus.task->cancel();
}

// ============================================================================
// [Fog::UISystemDefault - ID <-> UIWindow]
// ============================================================================

bool UISystemDefault::mapHandle(void* handle, UIWindow* native)
{
  return _widgetMapper.put(handle, native, true) == Error::Ok;
}

bool UISystemDefault::unmapHandle(void* handle)
{
  return _widgetMapper.remove(handle);
}

UIWindow* UISystemDefault::handleToNative(void* handle) const
{
  return _widgetMapper.value(handle, NULL);
}

// ============================================================================
// [Fog::UISystemDefault - Display]
// ============================================================================

err_t UISystemDefault::getDisplayInfo(DisplayInfo* out) const
{
  memcpy(out, &_displayInfo, sizeof(DisplayInfo));
  return Error::Ok;
}

err_t UISystemDefault::getPaletteInfo(PaletteInfo* out) const
{
  memcpy(out, &_paletteInfo, sizeof(PaletteInfo));
  return Error::Ok;
}

// ============================================================================
// [Fog::UISystemDefault - Caret]
// ============================================================================

err_t UISystemDefault::getCaretStatus(CaretStatus* out) const
{
  memcpy(out, &_caretStatus, sizeof(CaretStatus));
  return Error::Ok;
}

// ============================================================================
// [Fog::UISystemDefault - Keyboard]
// ============================================================================

err_t UISystemDefault::getKeyboardStatus(KeyboardStatus* out) const
{
  memcpy(out, &_keyboardStatus, sizeof(KeyboardStatus));
  return Error::Ok;
}

uint32_t UISystemDefault::getKeyboardModifiers() const
{
  return _keyboardStatus.modifiers;
}

// ============================================================================
// [Fog::UISystemDefault - Mouse]
// ============================================================================

err_t UISystemDefault::getMouseStatus(MouseStatus* out) const
{
  memcpy(out, &_mouseStatus, sizeof(MouseStatus));
  return Error::Ok;
}

err_t UISystemDefault::getSystemMouseStatus(SystemMouseStatus* out) const
{
  memcpy(out, &_systemMouseStatus, sizeof(SystemMouseStatus));
  return Error::Ok;
}

#if 0
// clear mouse status
  if (mouseStatus.widget)
  {
    Widget* widget = mouseStatus.widget;
    uint32_t hover = mouseStatus.wHover;
    uint32_t inClientArea = mouseStatus.wInClientArea;

    mouseStatus.widget = NULL;
    mouseStatus.wPosition.set(INT_MIN, INT_MIN);
    mouseStatus.wHover = false;
    mouseStatus.buttons = 0;
    mouseStatus.valid = true;

    if (hover)
    {
      MouseEvent e(EvMouseOut);
      e._button = ButtonInvalid;
      e._modifiers = getKeyboardModifiers();
      e._position.set(INT_MIN, INT_MIN);
      widget->sendEvent(&e);
    }
  }
#endif

void UISystemDefault::invalidateMouseStatus()
{
  _mouseStatus.valid = false;
}

void UISystemDefault::updateMouseStatus()
{
}

void UISystemDefault::changeMouseStatus(Widget* w, const Point& pos)
{
  if (_mouseStatus.widget != w)
  {
    // Clear button press times (double click).
    _buttonTime[0].clear();
    _buttonTime[1].clear();
    _buttonTime[2].clear();

    Widget* before = _mouseStatus.widget;

    _mouseStatus.widget = w;
    _mouseStatus.position = pos;
    _mouseStatus.hover = true;
    _mouseStatus.buttons = 0;
    _mouseStatus.valid = true;

    // Send MouseOut to widget where mouse was before
    if (before)
    {
      MouseEvent e(EvMouseOut);
      e._button = ButtonInvalid;
      e._modifiers = getKeyboardModifiers();
      e._position = pos;
      Widget::translateCoordinates(before, w, &e._position);
      before->sendEvent(&e);
    }

    {
      MouseEvent e(EvMouseIn);
      e._button = ButtonInvalid;
      e._modifiers = getKeyboardModifiers();
      e._position = pos;
      w->sendEvent(&e);
    }
  }
  else if (!(_mouseStatus.position != pos))
  {
    // Clear button press times (double click).
    _buttonTime[0].clear();
    _buttonTime[1].clear();
    _buttonTime[2].clear();

    uint32_t hoverChange = !!_mouseStatus.hover;
    uint32_t code = 0; // be quite

    hoverChange |= (!(pos.x() < 0 || 
                      pos.y() < 0 || 
                      pos.x() >= w->width() || 
                      pos.y() >= w->height())) << 1;

    enum HoverChange
    {
      HoverChangeOutsideMove = 0x0, // 00b
      HoverChangeOut         = 0x1, // 01b
      HoverChangeIn          = 0x2, // 10b
      HoverChangeInsideMove  = 0x3  // 11b
    };

    switch (hoverChange)
    {
      case HoverChangeOutsideMove: code = EvOutsideMove; break;
      case HoverChangeOut:         code = EvMouseOut   ; break;
      case HoverChangeIn:          code = EvMouseIn    ; break;
      case HoverChangeInsideMove:  code = EvMouseMove  ; break;
    }

    _mouseStatus.position = pos;
    _mouseStatus.hover = (hoverChange & 0x2) != 0;

    MouseEvent e(code);
    e._button = ButtonInvalid;
    e._modifiers = getKeyboardModifiers();
    e._position = pos;
    w->sendEvent(&e);
  }
}

void UISystemDefault::clearSystemMouseStatus()
{
  _systemMouseStatus.uiWindow = NULL;
  _systemMouseStatus.position.set(INT_MIN, INT_MIN);
  _systemMouseStatus.hover = false;
  _systemMouseStatus.buttons = 0;
}

bool UISystemDefault::startButtonRepeat(uint32_t button, bool reset, TimeDelta delay, TimeDelta interval)
{
  uint32_t id = fogButtonToId(button);
  if (id == ButtonInvalid) return false;

  if (!_buttonRepeat[id].isRunning())
  {
    _buttonRepeat[id].setInterval(delay);
    _buttonRepeat[id].start();
    _buttonRepeatInterval[id] = interval;
  }
  else if (_buttonRepeat[id].isRunning() && reset)
  {
    _buttonRepeat[id].stop();
    _buttonRepeat[id].setInterval(interval);
    _buttonRepeat[id].start();
    _buttonRepeatInterval[id] = interval;
  }
  return true;
}

bool UISystemDefault::stopButtonRepeat(uint32_t button)
{
  uint32_t id = fogButtonToId(button);
  if (id == ButtonInvalid) return false;

  if (_buttonRepeat[id].isRunning())
  {
    _buttonRepeat[id].stop();
  }
  return true;
}

void UISystemDefault::clearButtonRepeat()
{
  for (sysuint_t i = 0; i < FOG_ARRAY_SIZE(_buttonRepeat); i++)
  {
    if (_buttonRepeat[i].isRunning()) _buttonRepeat[i].stop();
  }
}

// ============================================================================
// [Fog::UISystemDefault - Wheel]
// ============================================================================

int UISystemDefault::getWheelLines() const
{
  return _wheelLines;
}

void UISystemDefault::setWheelLines(int count)
{
  _wheelLines = count;
}

// ============================================================================
// [Fog::UISystemDefault - Timing]
// ============================================================================

TimeDelta UISystemDefault::getRepeatingDelay() const
{
  return _repeatingDelay;
}

TimeDelta UISystemDefault::getRepeatingInterval() const
{
  return _repeatingInterval;
}

TimeDelta UISystemDefault::getDoubleClickInterval() const
{
  return _doubleClickInterval;
}

// ============================================================================
// [Fog::UISystemDefault - Windowing System]
// ============================================================================

void UISystemDefault::dispatchEnabled(Widget* w, bool enabled)
{
  uint32_t state = w->state();

  // Dispatch 'Enable'.
  if (enabled)
  {
    if (state == Widget::Enabled) return;
    if (state == Widget::DisabledByParent && !w->isUIWindow()) return;

    w->_state = Widget::Enabled;

    StateEvent e(EvEnable);
    w->sendEvent(&e);

    FOG_WIDGET_TREE_ITERATOR(i1, w, true,
      // before traverse
      {
        // show only child that's hidden by parent
        if (child->state() != Widget::DisabledByParent) FOG_WIDGET_TREE_ITERATOR_NEXT(i1);

        child->_state = Widget::Enabled;
        child->sendEvent(&e);
      },
      // after traverse
      {
      }
    );

    w->update(Widget::UFlagUpdateAll);
  }
  // Dispatch 'Disable'.
  else
  {
    uint32_t state = w->state();
    if (state == Widget::Disabled) return;

    w->_state = Widget::Disabled;

    //StateEvent e(toState == Widget::Disabled
    //  ? EvDisable
    //  : EvDisableByParent);
    StateEvent e(EvDisable);
    w->sendEvent(&e);

    if (state != Widget::Enabled) return;

    e._code = EvDisableByParent;

    FOG_WIDGET_TREE_ITERATOR(i2, w, true,
      // before traverse
      {
        // mark 'DisableByParent' all childs that's visible
        if (child->state() != Widget::Enabled)
        {
          FOG_WIDGET_TREE_ITERATOR_NEXT(i2);
        }
        else
        {
          child->_state = Widget::DisabledByParent;
          child->sendEvent(&e);
        }
      },
      // after traverse
      {
      }
    );

    w->update(Widget::UFlagUpdateAll);
  }
}

void UISystemDefault::dispatchVisibility(Widget* w, bool visible)
{
  uint32_t visibility = w->visibility();

  // Dispatch 'Show'.
  if (visible)
  {
    if (visibility == Widget::Visible) return;
    if (visibility == Widget::HiddenByParent && !w->isUIWindow()) return;

    w->_visibility = Widget::Visible;

    VisibilityEvent e(EvShow);
    w->sendEvent(&e);

    FOG_WIDGET_TREE_ITERATOR(i1, w, true,
      // before traverse
      {
        // show only child that's hidden by parent
        if (child->visibility() != Widget::HiddenByParent) FOG_WIDGET_TREE_ITERATOR_NEXT(i1);

        child->_visibility = Widget::Visible;
        w->sendEvent(&e);
      },
      // after traverse
      {}
    );

    w->update(Widget::UFlagUpdateAll);
  }
  // Dispatch 'Hidden'.
  else
  {
    if (visibility == Widget::Disabled) return;
    // if (visibility == toVisibility) return;

    w->_visibility = Widget::Hidden;
    // w->_visibility = toVisibility;

    VisibilityEvent e(EvHide);
    //VisibilityEvent e(toVisibility == Widget::Hidden
    //  ? EvHide
    //  : EvHideByParent);
    w->sendEvent(&e);

    if (visibility == Widget::Visible)
    {
      e._code = EvHideByParent;

      FOG_WIDGET_TREE_ITERATOR(i2, w, true,
        // before traverse
        {
          // mark 'HiddenByParent' all childs that's visible
          if (child->visibility() != Widget::Visible)
          {
            FOG_WIDGET_TREE_ITERATOR_NEXT(i2);
          }
          else
          {
            child->_visibility = Widget::HiddenByParent;
            child->sendEvent(&e);
          }
        },
        // after traverse
        {
        }
      );
      
      if (w->parent())
      {
        w->parent()->update(Widget::UFlagUpdateAll);
      }
    }
  }
}

void UISystemDefault::dispatchConfigure(Widget* w, const Rect& rect, bool changedOrientation)
{
  uint32_t changed = 0;

  if (w->rect().point() != rect.point())
    changed |= ConfigureEvent::ChangedPosition;

  if (w->rect().size() != rect.size())
    changed |= ConfigureEvent::ChangedSize;

  // TODO:
  //if (w->clientRect().x1() != client.x1() || w->clientRect().y1() != client.y1())
  //  changed |= ConfigureEvent::ChangedClientPosition;

  //if (w->clientRect().width() != rect.width() || w->clientRect().height() != client.height())
  //  changed |= ConfigureEvent::ChangedClientSize;

  if (changedOrientation)
    changed |= ConfigureEvent::ChangedOrientation;

  if (!changed) return;

  ConfigureEvent e;
  e._rect = rect;
  e._changed = changed;

  w->_rect = rect;
  w->sendEvent(&e);

  if (w->parent())
  {
    if (w->visibility() == Widget::Visible)
    {
      w->parent()->update(Widget::UFlagUpdateAll);
    }
  }
  else if (changed & ConfigureEvent::ChangedSize)
  {
    w->update(Widget::UFlagUpdateAll);
  }
}

void UISystemDefault::widgetDestroyed(Widget* w)
{
  if (_mouseStatus.widget == w)
  {
    _mouseStatus.widget = NULL;
    _mouseStatus.position.set(INT_MIN, INT_MIN);
    _mouseStatus.hover = 0;
    _mouseStatus.valid = false;
  }

  if (_caretStatus.widget == w)
  {
    Memory::zero(&_caretStatus, sizeof(CaretStatus));
  }
}

// ============================================================================
// [Fog::UISystemDefault - Update]
// ============================================================================

// Working structure for parent and child, also it's record in stack for 
// traversing in widget-tree.
struct UpdateRec
{
  Box bounds;
  Box paintBounds;
  uint32_t uflags;
  uint32_t implicitFlags;
  uint32_t visible;
  uint32_t painted;
};

struct UpdateTask : public CancelableTask
{
  UpdateTask() : _valid(true) {}

  virtual void run()
  {
    if (!_valid) return;

    // Try to process all pending messages from event queue.
    EventLoop::current()->runAllPending();
    // Do update.
    UI_SYSTEM()->doUpdate();
  }

  virtual void cancel()
  {
    _valid = false;
  }

private:
  bool _valid;
};

void UISystemDefault::update()
{
  if (!_updateStatus.scheduled)
  {
    _updateStatus.scheduled = true;
    _updateStatus.task = new UpdateTask();
    Application::instance()->eventLoop()->postTask(_updateStatus.task);
  }
}

void UISystemDefault::doUpdate()
{
  _updateStatus.scheduled = false;
  _updateStatus.updating = true;
  _updateStatus.task = NULL;

  // Iterate over all dirty "UIWindow"s and clear the list.
  {
    Vector<UIWindowDefault*> dirty(_dirtyList);
    _dirtyList.free();

    Vector<UIWindowDefault*>::ConstIterator it(dirty);
    for (it.toStart(); it.isValid(); it.toNext())
    {
      UIWindow* uiWindow = it.value();
      if (uiWindow) doUpdateWindow(uiWindow);
    }
  }

  _updateStatus.updating = false;
}

void UISystemDefault::doUpdateWindow(UIWindow* window)
{
  window->_dirty = false;

  // UIWindow widget.
  Widget* top = window->_widget;

  // Hidden windows won't be updated.
  if (!window->_visible) 
  {
    top->_uflags = 0;
    return;
  }

  // =======================================================
  // Local variables and initialization.
  // =======================================================

  // 10 seconds is extra buffer timeout
  TimeTicks now = TimeTicks::now() - TimeDelta::fromSeconds(10);

  // Painting.
  Painter painter;
  PaintEvent e;
  e._painter = &painter;

  // Temporary regions. We are using more regions, because region operations
  // are most effective by Region::op(dest, src1, src2) way. So it's safe to 
  // use different destination regions. Memory will be copied, but no memory
  // will be allocated that is the goal.
  TemporaryRegion<32> rtmp1;
  TemporaryRegion<32> rtmp2;
  TemporaryRegion<32> rtmp3;
  TemporaryRegion<64> blitRegion;

  // Some temporary data.
  Size topSize(top->size());
  Box  topBox(0, 0, (int)topSize.width(), (int)topSize.height());

  uint32_t uflags = top->_uflags;
  uint32_t implicitFlags = 0;
  bool blitFull = false;
  bool paintAll = false;

  // Paint helper variables.
  uint topBytesPerPixel;

  if ((int)window->_backingStore->width()  < topSize.width() ||
      (int)window->_backingStore->height() < topSize.height())
  {
    window->_backingStore->resize((uint)topSize.width(), (uint)topSize.height(), true);

    // We can omit updating here, because if window size was changed,
    // all uflags are already set.

    // It should already be set, but nobody knows...
    uflags |= Widget::UFlagUpdateAll;
  }
  else if (
      ((int)window->_backingStore->width() != topSize.width() ||
      (int)window->_backingStore->height() != topSize.height()) &&
      window->_backingStore->expired(now))
  {
    window->_backingStore->resize(topSize.width(), topSize.height(), false);
    uflags |= Widget::UFlagUpdateAll;
  }

  topBytesPerPixel = window->_backingStore->depth() >> 3;

  // =======================================================
  // Update top level widget.
  // =======================================================

  // if there is nothing to do, continue. It's checked here,
  // because next steps needs to create drawing context, etc...
  if ((uflags & (
      Widget::UFlagUpdate         |
      Widget::UFlagUpdateChild    |
      Widget::UFlagUpdateAll      |
      Widget::UFlagUpdateGeometry |
      Widget::UFlagRepaintWidget  |
      Widget::UFlagRepaintCaret   )) == 0)
  {
    return;
  }

  // We will call Painter::begin() here, because it will be shared
  // between all repaints.
  painter.begin(
      window->_backingStore->pixels(),
      window->_backingStore->width(),
      window->_backingStore->height(),
      window->_backingStore->stride(),
      window->_backingStore->format());

  if ((uflags & Widget::UFlagUpdateAll) != 0)
  {
    implicitFlags |=
      Widget::UFlagUpdate         |
      Widget::UFlagUpdateChild    |
      Widget::UFlagUpdateGeometry |
      Widget::UFlagRepaintWidget  ;
  }

  uflags |= implicitFlags;

  if ((uflags & Widget::UFlagRepaintWidget) != 0)
  {
    e._code = EvPaint;
    e._receiver = top;
    e._parentPainted = 0;

    painter.setMetaVariables(
      Point(0, 0),
      TemporaryRegion<1>(Rect(0, 0, top->width(), top->height())),
      true,
      true);
    top->onEvent(&e);

    uflags |=
      Widget::UFlagUpdateChild    ;
    implicitFlags |=
      Widget::UFlagUpdateChild    |
      Widget::UFlagRepaintWidget  ;

    blitFull = true;
    paintAll = true;
  }

  // =======================================================
  // Update children.
  // =======================================================

  if (top->hasChildren() && (uflags & Widget::UFlagUpdateChild) != 0)
  {
    LocalStack<1024> stack;

    // manual object iterator
    Widget* const* ocur;
    Widget* const* oend;

    // parent and child widgets
    Widget* parent;
    Widget* child;

    UpdateRec parentRec;
    UpdateRec childRec;

    parent = top;
    parentRec.bounds.set(0, 0, parent->width(), parent->height());
    parentRec.paintBounds.set(0, 0, parent->width(), parent->height());
    parentRec.uflags = uflags;
    parentRec.implicitFlags = implicitFlags;
    parentRec.visible = true;
    parentRec.painted = blitFull | paintAll;

__pushed:
    ocur = parent->children().cData();
    oend = ocur + parent->children().length();

    child = (Widget*)*ocur;
    for (;;)
    {
      if (child->visibility() != Widget::Visible)
      {
        goto __next;
      }

      childRec.uflags = child->_uflags;
      childRec.implicitFlags = parentRec.implicitFlags;

      if ((childRec.uflags & Widget::UFlagUpdateAll) != 0)
      {
        childRec.implicitFlags |=
          Widget::UFlagUpdate         |
          Widget::UFlagUpdateChild    |
          Widget::UFlagUpdateGeometry |
          Widget::UFlagRepaintWidget  ;
      }
      childRec.uflags |= childRec.implicitFlags;

      if ((childRec.uflags & (
        Widget::UFlagUpdate         |
        Widget::UFlagUpdateChild    |
        Widget::UFlagUpdateGeometry |
        Widget::UFlagRepaintWidget  |
        Widget::UFlagRepaintCaret   |
        Widget::UFlagUpdateAll)) == 0)
      {
        goto __next;
      }

      childRec.bounds._x1 = parentRec.bounds.x1() + child->rect().x1();
      childRec.bounds._y1 = parentRec.bounds.y1() + child->rect().y1();
      childRec.bounds._x2 = childRec.bounds.x1() + child->rect().width();
      childRec.bounds._y2 = childRec.bounds.y1() + child->rect().height();

      childRec.bounds += parent->origin();

      childRec.visible = Box::intersect(childRec.paintBounds, parentRec.paintBounds, childRec.bounds);
      childRec.painted = false;

      if (childRec.visible)
      {
        // paint client area / caret
        if ((childRec.uflags & (Widget::UFlagRepaintWidget  | Widget::UFlagRepaintCaret)) != 0)
        {
          e._code = EvPaint;
          e._receiver = child;
          e._parentPainted = parentRec.painted | (!!(child->_uflags & Widget::UFlagPaintParentDone));

#if 0
          if (child->children().count() > 0 && child->children().length() <= 16)
          {
            rtmp2.set(childRec.paintBounds);
            TemporaryRegion<128> rtmp4;
            Vector<Widget*>::ConstIterator ci(child->children());
            int ox = childRec.bounds.x1() + child->origin().x();
            int oy = childRec.bounds.y1() + child->origin().y();

            for (ci.toStart(); ci.isValid(); ci.toNext())
            {
              Widget* cw = core_object_cast<Widget*>(ci.value());
              if (cw && cw->visibility() == Widget::Visible)
              {
                if ((cw->_uflags & Widget::UFlagPaintParentRequired))
                {
                  // this is simple optimization. If widget will call
                  // Widget::paintParent(), we will simply do it
                  // earlier.
                  cw->_uflags |= Widget::UFlagPaintParentDone;
                }
                else
                {
                  rtmp4.unite(Box(
                    cw->rect().x1() + ox,
                    cw->rect().y1() + oy,
                    cw->rect().x2() + ox,
                    cw->rect().y2() + oy));
                }
              }
            }
            Region::subtract(rtmp1, rtmp2, rtmp4);
          }
          else
          {
#endif
            rtmp1.set(childRec.paintBounds);
            childRec.painted = true;
#if 0
          }
#endif
          painter.setMetaVariables(
            Point(childRec.bounds.x1(), childRec.bounds.y1()),
            rtmp1,
            true,
            true);

          // FIXME: Repaint caret repaints whole control
          if ((childRec.uflags & (Widget::UFlagRepaintWidget | Widget::UFlagRepaintCaret)) != 0)
          {
            child->onEvent(&e);
            //blitFull = true;
            if (!parentRec.painted) blitRegion.unite(rtmp1);
          }

          /*
          if (Application::caretStatus().widget == child &&
            (childRec.uflags & (Widget::UFlagRepaintWidget | Widget::UFlagRepaintCaret)) != 0)
          {
            theme->paintCaret(&painter, Application::caretStatus());
            //blitFull = true;
            if (!parentRec.painted) blitRegion.unite(rtmp1);
          }
          */

          parentRec.implicitFlags |=
            Widget::UFlagUpdate         |
            Widget::UFlagUpdateChild    |
            Widget::UFlagRepaintWidget  ;
          childRec.uflags |=
            Widget::UFlagUpdateChild    ;
          childRec.implicitFlags |=
            Widget::UFlagUpdate         |
            Widget::UFlagUpdateChild    |
            Widget::UFlagRepaintWidget  ;
        }
      }

      if ((childRec.uflags & Widget::UFlagUpdateChild) != 0 && child->_children.length())
      {
        stack.push(parent);
        stack.push(ocur);
        stack.push(oend);
        stack.push(parentRec);

        parent = child;
        parentRec = childRec;

        goto __pushed;
__pop:
        childRec = parentRec;
        child = parent;
        stack.pop(parentRec);
        stack.pop(oend);
        stack.pop(ocur);
        stack.pop(parent);
      }

      // clear update uflags
      child->_uflags &= ~(
        Widget::UFlagUpdate         |
        Widget::UFlagUpdateChild    |
        Widget::UFlagUpdateAll      |
        Widget::UFlagUpdateGeometry |
        Widget::UFlagRepaintWidget  |
        Widget::UFlagRepaintCaret   |
        // clear if this was set
        Widget::UFlagPaintParentDone);

__next:
      // go to next child or to parent
      if (++ocur == oend)
      {
        if (!stack.isEmpty())
          goto __pop;
        else
          break;
      }

      child = (Widget*)*ocur;
    }
  }

  painter.end();

  // =======================================================
  // blit root widget content to screen
  // =======================================================

  const Box* rptr;
  sysuint_t rcount = 0;

  if (blitFull || window->_blit) 
  {
    rptr = &topBox;
    rcount = 1;
  }
  else
  {
    rptr = blitRegion.cData();
    rcount = blitRegion.count();
  }

  if (rcount)
  {
    window->_backingStore->updateRects(rptr, rcount);

    doBlitWindow(window, rptr, rcount);
  }

  // Clear update flags.
  top->_uflags &= ~(
    Widget::UFlagUpdate         |
    Widget::UFlagUpdateChild    |
    Widget::UFlagUpdateAll      |
    Widget::UFlagUpdateGeometry |
    Widget::UFlagRepaintWidget  |
    Widget::UFlagRepaintCaret   |
    // clear if this was set
    Widget::UFlagPaintParentDone);

  // Clear blit flag.
  window->_blit = false;
}

// ============================================================================
// [Fog::UISystemDefault - Event Handlers]
// ============================================================================

void UISystemDefault::_onButtonRepeatTimeOut(TimerEvent* e)
{
  if (_systemMouseStatus.uiWindow == NULL) 
  {
    e->timer()->stop();
  }
  else
  {
    uint32_t id;
    for (id = 0; id != 3; id++)
    {
      if (e->timer() == &_buttonRepeat[id])
      {
        _buttonRepeat[id].setInterval(_buttonRepeatInterval[id]);
        _systemMouseStatus.uiWindow->onMousePress(1 << id, true);
        break;
      }
    }
  }
}

// ============================================================================
// [Fog::UIWindowDefault]
// ============================================================================

UIWindowDefault::UIWindowDefault(Widget* widget) : 
  UIWindow(widget),
  _windowRect(0, 0, 1, 1),
  _clientRect(0, 0, 1, 1),
  _sizeGranularity(1, 1)
{
}

UIWindowDefault::~UIWindowDefault()
{
  UISystemDefault* uiSystem = UI_SYSTEM();

  // Remove UIWindow from system mouse status
  if (uiSystem->_systemMouseStatus.uiWindow == this)
  {
    uiSystem->clearSystemMouseStatus();
    uiSystem->clearButtonRepeat();
  }

  // Remove UIWindow from dirty list
  sysuint_t i = uiSystem->_dirtyList.indexOf(this);
  if (i != InvalidIndex) uiSystem->_dirtyList[i] = NULL;
}

// ============================================================================
// [Fog::UIWindowDefault - Windowing System]
// ============================================================================

void UIWindowDefault::onEnabled(bool enabled)
{
  _enabled = enabled;
  UI_SYSTEM()->dispatchEnabled(_widget, enabled);
}

void UIWindowDefault::onVisibility(bool visible)
{
  _visible = visible;

  UI_SYSTEM()->dispatchVisibility(_widget, visible);
}

void UIWindowDefault::onConfigure(const Rect& windowRect, const Rect& clientRect)
{
  _windowRect = windowRect;
  _clientRect = clientRect;

  UI_SYSTEM()->dispatchConfigure(_widget, Rect(
    windowRect.x1(),
    windowRect.y1(),
    clientRect.width(),
    clientRect.height()), false);
}

void UIWindowDefault::onMouseHover(int x, int y)
{
  UISystemDefault* uiSystem = UI_SYSTEM();

  if (uiSystem->_systemMouseStatus.uiWindow != this)
  {
    uiSystem->_systemMouseStatus.uiWindow = this;
    uiSystem->_systemMouseStatus.buttons = 0;
  }
  uiSystem->_systemMouseStatus.position.set(x, y);
  uiSystem->_systemMouseStatus.hover = true;

  onMouseMove(x, y);
}

void UIWindowDefault::onMouseMove(int x, int y)
{
  UISystemDefault* uiSystem = UI_SYSTEM();
  Widget* w;

  if (uiSystem->_systemMouseStatus.uiWindow != this)
  {
    uiSystem->_systemMouseStatus.uiWindow = this;
    uiSystem->_systemMouseStatus.buttons = 0;
    uiSystem->_systemMouseStatus.hover = false;
  }
  uiSystem->_systemMouseStatus.position.set(x, y);

  Point p(x, y);

  // ----------------------------------
  // Grabbing mode
  // ----------------------------------

  if (uiSystem->_mouseStatus.buttons)
  {
    w = uiSystem->_mouseStatus.widget;
    p -= _widget->origin();

    if (!Widget::translateCoordinates(w, _widget, &p))
    {
      FOG_ASSERT_NOT_REACHED();
    }

    uiSystem->changeMouseStatus(w, p);
    return;
  }

  // ----------------------------------
  // Motion mode
  // ----------------------------------

  // in motion mode, mouse should be in widget bounds (so check for it)
  if (x < 0 || y < 0 || x >= _widget->width() || y >= _widget->height())
  {
    return;
  }

  w = _widget;

__repeat:
  {
    // Add origin.
    p -= w->_origin;

    // Iterate over children and try to find child widget where  mouse
    // position is. Iteration must be done through end, becuase we want
    // widget with highest Z-Order to match mouse position first.
    Vector<Widget*>::ConstIterator it(w->_children);
    for (it.toEnd(); it.isValid(); it.toPrevious())
    {
      Widget* current = it.value();
      if (current->visibility() == Widget::Visible && current->_rect.contains(p))
      {
        w = current;
        p -= w->position();
        goto __repeat;
      }
    }

    uiSystem->changeMouseStatus(w, p);
  }
}

void UIWindowDefault::onMouseLeave(int x, int y)
{
  UISystemDefault* uiSystem = UI_SYSTEM();

  if (uiSystem->_systemMouseStatus.uiWindow == this)
  {
    uiSystem->clearSystemMouseStatus();
    uiSystem->clearButtonRepeat();
  }
}

void UIWindowDefault::onMousePress(uint32_t button, bool repeated)
{
  UISystemDefault* uiSystem = UI_SYSTEM();
  if (uiSystem->_systemMouseStatus.uiWindow != this) return;

  Widget* w = uiSystem->_mouseStatus.widget;
  if (!w) return;

  uiSystem->_mouseStatus.buttons |= button;

  TimeTicks now = TimeTicks::now();

  if (!repeated) uiSystem->startButtonRepeat(
    button, true, uiSystem->_repeatingDelay, uiSystem->_repeatingInterval);

  MouseEvent e(EvButtonPress + (uint32_t)repeated);
  e._button = button;
  e._modifiers = uiSystem->getKeyboardModifiers();
  e._position = uiSystem->_mouseStatus.position;
  w->sendEvent(&e);

  uint32_t buttonId = fogButtonToId(button);
  if (repeated || buttonId == ButtonInvalid) return;

  if ((now - uiSystem->_buttonTime[buttonId]) <= uiSystem->_doubleClickInterval)
  {
    uiSystem->_buttonTime[buttonId].clear();
    e._code = EvDoubleClick;
    w->sendEvent(&e);
  }
  else
  {
    uiSystem->_buttonTime[buttonId] = now;
  }
}

void UIWindowDefault::onMouseRelease(uint32_t button)
{
  UISystemDefault* uiSystem = UI_SYSTEM();
  if (uiSystem->_systemMouseStatus.uiWindow != this) return;

  uiSystem->_mouseStatus.buttons &= ~button;
  uiSystem->stopButtonRepeat(button);

  Widget* w = uiSystem->_mouseStatus.widget;
  if (!w) return;

  bool lastButtonRelease = (uiSystem->_mouseStatus.buttons == 0);

  MouseEvent e(EvButtonRelease);
  e._button = button;
  e._modifiers = uiSystem->getKeyboardModifiers();
  e._position = uiSystem->_mouseStatus.position;
  w->sendEvent(&e);

  e._code = EvClick;
  w->sendEvent(&e);

  if (lastButtonRelease)
  {
    onMouseMove(
      uiSystem->_systemMouseStatus.position.x(),
      uiSystem->_systemMouseStatus.position.y());
  }
}

void UIWindowDefault::onMouseWheel(uint32_t wheel)
{
  UISystemDefault* uiSystem = UI_SYSTEM();
  if (uiSystem->_systemMouseStatus.uiWindow != this) return;

  Widget* w = uiSystem->_mouseStatus.widget;
  if (!w) return;

  MouseEvent e(EvWheel);
  e._button = wheel;
  e._modifiers = uiSystem->getKeyboardModifiers();
  e._position = uiSystem->_mouseStatus.position;
  w->sendEvent(&e);
}

void UIWindowDefault::onFocus(bool focus)
{
  UISystemDefault* uiSystem = UI_SYSTEM();

  _focus = focus;
  if (focus)
  {
    Widget* w = _widget->_findFocus();
    if (!w->_hasFocus)
    {
      FocusEvent e(EvFocusIn);
      _widget->_hasFocus = true;
      _widget->sendEvent(&e);
    }
  }
  else
  {
    uiSystem->_keyboardStatus.modifiers = 0;
  }
}

bool UIWindowDefault::onKeyPress(uint32_t key, uint32_t modifier, uint32_t systemCode, Char32 unicode)
{
  UISystemDefault* uiSystem = UI_SYSTEM();

  KeyEvent e(EvKeyPress);
  e._key = key;
  e._modifiers = uiSystem->getKeyboardModifiers();
  e._systemCode = systemCode;
  e._unicode = unicode;

  if (isShiftMod(uiSystem->_keyboardStatus.modifiers)) e._key |= KeyShift;
  if (isCtrlMod (uiSystem->_keyboardStatus.modifiers)) e._key |= KeyCtrl;
  if (isAltMod  (uiSystem->_keyboardStatus.modifiers)) e._key |= KeyAlt;

  // Set this status after modifiers check.
  uiSystem->_keyboardStatus.modifiers |= modifier;

  if ((e.unicode().ch() >= 1 && e.unicode().ch() <= 31) ||
      e.unicode().ch() == 127 /* DEL key */)
  {
    e._unicode._ch = 0;
  }

  _widget->_findFocus()->sendEvent(&e);
  return e.isAccepted();
}

bool UIWindowDefault::onKeyRelease(uint32_t key, uint32_t modifier, uint32_t systemCode, Char32 unicode)
{
  UISystemDefault* uiSystem = UI_SYSTEM();
  uiSystem->_keyboardStatus.modifiers &= ~modifier;

  KeyEvent e(EvKeyRelease);
  e._key = key;
  e._modifiers = uiSystem->getKeyboardModifiers();
  e._systemCode = systemCode;
  e._unicode._ch = 0;

  if (isShiftMod(uiSystem->_keyboardStatus.modifiers)) e._key |= KeyShift;
  if (isCtrlMod (uiSystem->_keyboardStatus.modifiers)) e._key |= KeyCtrl;
  if (isAltMod  (uiSystem->_keyboardStatus.modifiers)) e._key |= KeyAlt;

  _widget->_findFocus()->sendEvent(&e);
  return e.isAccepted();
}

void UIWindowDefault::clearFocus()
{
  Widget* w = _widget;

  // Need to clear focus
  while (w->_focusLink)
  {
    Widget* t = w->_focusLink;
    w->_focusLink = NULL;
    w = t;
  }

  if (w->_hasFocus)
  {
    w->_hasFocus = false;
    FocusEvent e(EvFocusOut);
    w->sendEvent(&e);
  }
}

void UIWindowDefault::setFocus(Widget* w, uint32_t reason)
{
  if (_widget->_focusLink) clearFocus();

  Widget* t = w;
  while (t != _widget)
  {
    Widget* parent = t->parent();
    parent->_focusLink = t;
    t = parent;
    FOG_ASSERT(t != NULL);
  }

  if (!_focus) takeFocus();

  FocusEvent e(EvFocusIn, reason);
  w->_hasFocus = true;
  w->sendEvent(&e);
}

// ============================================================================
// [UIWindowDefault - Dirty]
// ============================================================================

void UIWindowDefault::setDirty()
{
  if (_dirty) return;
  _dirty = true;

  UISystemDefault* uiSystem = UI_SYSTEM();
  uiSystem->_dirtyList.append(this);
  uiSystem->update();
}

} // Fog namespace


































































#if 0
// [Windows Specific]

#if defined(FOG_UI_WINDOWS)

const char* Application::winMessageToAStr(uint message)
{
  struct MsgTable
  {
    uint message;
    const char *str;
  };

  static const MsgTable msgTable[] = {
    {  WM_NULL, "WM_NULL"  },
    {  WM_CREATE, "WM_CREATE"  },
    {  WM_DESTROY, "WM_DESTROY"  },
    {  WM_MOVE, "WM_MOVE"  },
    {  WM_SIZE, "WM_SIZE"  },
    {  WM_ACTIVATE, "WM_ACTIVATE"  },
    {  WM_SETFOCUS, "WM_SETFOCUS"  },
    {  WM_KILLFOCUS, "WM_KILLFOCUS"  },
    {  WM_ENABLE, "WM_ENABLE"  },
    {  WM_SETREDRAW, "WM_SETREDRAW"  },
    {  WM_SETTEXT, "WM_SETTEXT"  },
    {  WM_GETTEXT, "WM_GETTEXT"  },
    {  WM_GETTEXTLENGTH, "WM_GETTEXTLENGTH"  },
    {  WM_PAINT, "WM_PAINT"  },
    {  WM_CLOSE, "WM_CLOSE"  },
    {  WM_QUERYENDSESSION, "WM_QUERYENDSESSION"  },
    {  WM_QUIT, "WM_QUIT"  },
    {  WM_QUERYOPEN, "WM_QUERYOPEN"  },
    {  WM_ERASEBKGND, "WM_ERASEBKGND"  },
    {  WM_SYSCOLORCHANGE, "WM_SYSCOLORCHANGE"  },
    {  WM_ENDSESSION, "WM_ENDSESSION"  },
    {  WM_SHOWWINDOW, "WM_SHOWWINDOW"  },
    {  WM_WININICHANGE, "WM_WININICHANGE"  },
    {  WM_DEVMODECHANGE, "WM_DEVMODECHANGE"  },
    {  WM_ACTIVATEAPP, "WM_ACTIVATEAPP"  },
    {  WM_FONTCHANGE, "WM_FONTCHANGE"  },
    {  WM_TIMECHANGE, "WM_TIMECHANGE"  },
    {  WM_CANCELMODE, "WM_CANCELMODE"  },
    {  WM_SETCURSOR, "WM_SETCURSOR"  },
    {  WM_MOUSEACTIVATE, "WM_MOUSEACTIVATE"  },
    {  WM_CHILDACTIVATE, "WM_CHILDACTIVATE"  },
    {  WM_QUEUESYNC, "WM_QUEUESYNC"  },
    {  WM_GETMINMAXINFO, "WM_GETMINMAXINFO"  },
    {  WM_PAINTICON, "WM_PAINTICON"  },
    {  WM_ICONERASEBKGND, "WM_ICONERASEBKGND"  },
    {  WM_NEXTDLGCTL, "WM_NEXTDLGCTL"  },
    {  WM_SPOOLERSTATUS, "WM_SPOOLERSTATUS"  },
    {  WM_DRAWITEM, "WM_DRAWITEM"  },
    {  WM_MEASUREITEM, "WM_MEASUREITEM"  },
    {  WM_DELETEITEM, "WM_DELETEITEM"  },
    {  WM_VKEYTOITEM, "WM_VKEYTOITEM"  },
    {  WM_CHARTOITEM, "WM_CHARTOITEM"  },
    {  WM_SETFONT, "WM_SETFONT"  },
    {  WM_GETFONT, "WM_GETFONT"  },
    {  WM_SETHOTKEY, "WM_SETHOTKEY"  },
    {  WM_GETHOTKEY, "WM_GETHOTKEY"  },
    {  WM_QUERYDRAGICON, "WM_QUERYDRAGICON"  },
    {  WM_COMPAREITEM, "WM_COMPAREITEM"  },
    {  WM_COMPACTING, "WM_COMPACTING"  },
    {  WM_WINDOWPOSCHANGING, "WM_WINDOWPOSCHANGING"  },
    {  WM_WINDOWPOSCHANGED, "WM_WINDOWPOSCHANGED"  },
    {  WM_POWER, "WM_POWER"  },
    {  WM_COPYDATA, "WM_COPYDATA"  },
    {  WM_CANCELJOURNAL, "WM_CANCELJOURNAL"  },
    {  WM_NCCREATE, "WM_NCCREATE"  },
    {  WM_NCDESTROY, "WM_NCDESTROY"  },
    {  WM_NCCALCSIZE, "WM_NCCALCSIZE"  },
    {  WM_NCHITTEST, "WM_NCHITTEST"  },
    {  WM_NCPAINT, "WM_NCPAINT"  },
    #if defined(WM_SYNCPAINT)
    {  WM_SYNCPAINT, "WM_SYNCPAINT"  },
    #endif
    {  WM_NCACTIVATE, "WM_NCACTIVATE"  },
    {  WM_GETDLGCODE, "WM_GETDLGCODE"  },
    {  WM_NCMOUSEMOVE, "WM_NCMOUSEMOVE"  },
    {  WM_NCLBUTTONDOWN, "WM_NCLBUTTONDOWN"  },
    {  WM_NCLBUTTONUP, "WM_NCLBUTTONUP"  },
    {  WM_NCLBUTTONDBLCLK, "WM_NCLBUTTONDBLCLK"  },
    {  WM_NCRBUTTONDOWN, "WM_NCRBUTTONDOWN"  },
    {  WM_NCRBUTTONUP, "WM_NCRBUTTONUP"  },
    {  WM_NCRBUTTONDBLCLK, "WM_NCRBUTTONDBLCLK"  },
    {  WM_NCMBUTTONDOWN, "WM_NCMBUTTONDOWN"  },
    {  WM_NCMBUTTONUP, "WM_NCMBUTTONUP"  },
    {  WM_NCMBUTTONDBLCLK, "WM_NCMBUTTONDBLCLK"  },
    {  WM_KEYFIRST, "WM_KEYFIRST"  },
    {  WM_KEYDOWN, "WM_KEYDOWN"  },
    {  WM_KEYUP, "WM_KEYUP"  },
    {  WM_CHAR, "WM_CHAR"  },
    {  WM_DEADCHAR, "WM_DEADCHAR"  },
    {  WM_SYSKEYDOWN, "WM_SYSKEYDOWN"  },
    {  WM_SYSKEYUP, "WM_SYSKEYUP"  },
    {  WM_SYSCHAR, "WM_SYSCHAR"  },
    {  WM_SYSDEADCHAR, "WM_SYSDEADCHAR"  },
    {  WM_KEYLAST, "WM_KEYLAST"  },
    {  WM_INITDIALOG, "WM_INITDIALOG"  },
    {  WM_COMMAND, "WM_COMMAND"  },
    {  WM_SYSCOMMAND, "WM_SYSCOMMAND"  },
    {  WM_TIMER, "WM_TIMER"  },
    {  WM_HSCROLL, "WM_HSCROLL"  },
    {  WM_VSCROLL, "WM_VSCROLL"  },
    {  WM_INITMENU, "WM_INITMENU"  },
    {  WM_INITMENUPOPUP, "WM_INITMENUPOPUP"  },
    {  WM_MENUSELECT, "WM_MENUSELECT"  },
    {  WM_MENUCHAR, "WM_MENUCHAR"  },
    {  WM_ENTERIDLE, "WM_ENTERIDLE"  },
    {  WM_CTLCOLORMSGBOX, "WM_CTLCOLORMSGBOX"  },
    {  WM_CTLCOLOREDIT, "WM_CTLCOLOREDIT"  },
    {  WM_CTLCOLORLISTBOX, "WM_CTLCOLORLISTBOX"  },
    {  WM_CTLCOLORBTN, "WM_CTLCOLORBTN"  },
    {  WM_CTLCOLORDLG, "WM_CTLCOLORDLG"  },
    {  WM_CTLCOLORSCROLLBAR, "WM_CTLCOLORSCROLLBAR"  },
    {  WM_CTLCOLORSTATIC, "WM_CTLCOLORSTATIC"  },
    {  WM_MOUSEFIRST, "WM_MOUSEFIRST"  },
    {  WM_MOUSEMOVE, "WM_MOUSEMOVE"  },
    {  WM_LBUTTONDOWN, "WM_LBUTTONDOWN"  },
    {  WM_LBUTTONUP, "WM_LBUTTONUP"  },
    {  WM_LBUTTONDBLCLK, "WM_LBUTTONDBLCLK"  },
    {  WM_RBUTTONDOWN, "WM_RBUTTONDOWN"  },
    {  WM_RBUTTONUP, "WM_RBUTTONUP"  },
    {  WM_RBUTTONDBLCLK, "WM_RBUTTONDBLCLK"  },
    {  WM_MBUTTONDOWN, "WM_MBUTTONDOWN"  },
    {  WM_MBUTTONUP, "WM_MBUTTONUP"  },
    {  WM_MBUTTONDBLCLK, "WM_MBUTTONDBLCLK"  },
    {  WM_MOUSELAST, "WM_MOUSELAST"  },
    {  WM_PARENTNOTIFY, "WM_PARENTNOTIFY"  },
    {  WM_ENTERMENULOOP, "WM_ENTERMENULOOP"  },
    {  WM_EXITMENULOOP, "WM_EXITMENULOOP"  },
    {  WM_MDICREATE, "WM_MDICREATE"  },
    {  WM_MDIDESTROY, "WM_MDIDESTROY"  },
    {  WM_MDIACTIVATE, "WM_MDIACTIVATE"  },
    {  WM_MDIRESTORE, "WM_MDIRESTORE"  },
    {  WM_MDINEXT, "WM_MDINEXT"  },
    {  WM_MDIMAXIMIZE, "WM_MDIMAXIMIZE"  },
    {  WM_MDITILE, "WM_MDITILE"  },
    {  WM_MDICASCADE, "WM_MDICASCADE"  },
    {  WM_MDIICONARRANGE, "WM_MDIICONARRANGE"  },
    {  WM_MDIGETACTIVE, "WM_MDIGETACTIVE"  },
    {  WM_MDISETMENU, "WM_MDISETMENU"  },
    {  WM_DROPFILES, "WM_DROPFILES"  },
    {  WM_MDIREFRESHMENU, "WM_MDIREFRESHMENU"  },
    {  WM_CUT, "WM_CUT"  },
    {  WM_COPY, "WM_COPY"  },
    {  WM_PASTE, "WM_PASTE"  },
    {  WM_CLEAR, "WM_CLEAR"  },
    {  WM_UNDO, "WM_UNDO"  },
    {  WM_RENDERFORMAT, "WM_RENDERFORMAT"  },
    {  WM_RENDERALLFORMATS, "WM_RENDERALLFORMATS"  },
    {  WM_DESTROYCLIPBOARD, "WM_DESTROYCLIPBOARD"  },
    {  WM_DRAWCLIPBOARD, "WM_DRAWCLIPBOARD"  },
    {  WM_PAINTCLIPBOARD, "WM_PAINTCLIPBOARD"  },
    {  WM_VSCROLLCLIPBOARD, "WM_VSCROLLCLIPBOARD"  },
    {  WM_SIZECLIPBOARD, "WM_SIZECLIPBOARD"  },
    {  WM_ASKCBFORMATNAME, "WM_ASKCBFORMATNAME"  },
    {  WM_CHANGECBCHAIN, "WM_CHANGECBCHAIN"  },
    {  WM_HSCROLLCLIPBOARD, "WM_HSCROLLCLIPBOARD"  },
    {  WM_QUERYNEWPALETTE, "WM_QUERYNEWPALETTE"  },
    {  WM_PALETTEISCHANGING, "WM_PALETTEISCHANGING"  },
    {  WM_PALETTECHANGED, "WM_PALETTECHANGED"  },
    {  WM_HOTKEY, "WM_HOTKEY"  },
    {  WM_PENWINFIRST, "WM_PENWINFIRST"  },
    {  WM_PENWINLAST, "WM_PENWINLAST"  }
    #if defined(WM_SIZING)
    ,{  WM_SIZING, "WM_SIZING"  }
    #endif
    #if defined(WM_MOVING)
    ,{  WM_MOVING, "WM_MOVING"  }
    #if defined(WM_ENTERSIZEMOVE)
    ,{  WM_ENTERSIZEMOVE, "WM_ENTERSIZEMOVE"  }
    #endif
    #if defined(WM_EXITSIZEMOVE)
    ,{  WM_EXITSIZEMOVE, "WM_EXITSIZEMOVE"  }
    #endif
    #endif
    #if defined(WM_ASYNCIO)
    ,{  WM_ASYNCIO, "WM_ASYNCIO"  }
    #endif
  };

  register sysuint_t i;

  for (i = 0; i != FOG_ARRAY_SIZE(msgTable); i++)
  {
    if (msgTable[i].message == message) return msgTable[i].str;
  }

  return "Unknown";
}
#endif // FOG_UI_WINDOWS

}
#endif
