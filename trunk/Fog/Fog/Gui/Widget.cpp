// [Fog/Gui Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Application.h>
#include <Fog/Core/List.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Region.h>
#include <Fog/Gui/Constants.h>
#include <Fog/Gui/GuiEngine.h>
#include <Fog/Gui/Layout.h>
#include <Fog/Gui/LayoutItem.h>
#include <Fog/Gui/Widget_p.h>

FOG_IMPLEMENT_OBJECT(Fog::Widget)

namespace Fog {

// ============================================================================
// [Fog::Widget - Construction / Destruction]
// ============================================================================

Widget::Widget(uint32_t createFlags) :
  _parent(NULL),
  _uiWindow(NULL),
  _rect(0, 0, 0, 0),
  _origin(0, 0),
  _layout(NULL),
  _layoutPolicy(0),
  _hasHeightForWidth(false),
  _isLayoutDirty(true),
  _lastFocus(NULL),
  _focusLink(NULL),
  _uflags(0),
  _state(WIDGET_ENABLED),
  _visibility(WIDGET_HIDDEN),
  _focusPolicy(FOCUS_NONE),
  _hasFocus(false),
  _orientation(ORIENTATION_HORIZONTAL),
  _reserved(0),
  _tabOrder(0)
{
  _flags |= OBJ_IS_WIDGET;

  // TODO ?
  _focusLink = NULL;

  if (createFlags & GuiWindow::CreateUIWindow)
  {
    createWindow(createFlags);
  }
}

Widget::~Widget()
{
  deleteLayout();
  if (_uiWindow) destroyWindow();
}

// ============================================================================
// [Fog::Widget - Hierarchy]
// ============================================================================

bool Widget::setParent(Widget* p)
{
  if (p) return p->add(this);

  if (_parent == NULL)
    return true;
  else
    return _parent->remove(this);
}

bool Widget::add(Widget* w)
{
  // First remove the element from it's parent
  Widget* p = w->_parent;
  if (p != NULL && p != this) 
  {
    // Remove element can fail, so we return false in that case
    if (!p->remove(w)) return false;
  }

  // Now element can be added, call virtual method
  return _add(_children.getLength(), w);
}

bool Widget::remove(Widget* w)
{
  Widget* p = w->_parent;
  if (p != this) return false;

  return _remove(_children.indexOf(w), w);
}

bool Widget::_add(sysuint_t index, Widget* w)
{
  FOG_ASSERT(index <= _children.getLength());

  _children.insert(index, w);
  w->_parent = this;

  return true;
}

bool Widget::_remove(sysuint_t index, Widget* w)
{
  FOG_ASSERT(index < _children.getLength());
  FOG_ASSERT(_children.at(index) == w);

  _children.removeAt(index);
  w->_parent = NULL;

  return true;
}

// ============================================================================
// [Fog::Widget - UIWindow]
// ============================================================================

GuiWindow* Widget::getClosestUIWindow() const
{
  Widget* w = const_cast<Widget*>(this);
  do {
    if (w->_uiWindow) return w->_uiWindow;
    w = w->_parent;
  } while (w);

  return NULL;
}

err_t Widget::createWindow(uint32_t createFlags)
{
  if (_uiWindow) return ERR_GUI_WINDOW_ALREADY_EXISTS;

  GuiEngine* ge = Application::getInstance()->getGuiEngine();
  if (ge == NULL) return ERR_GUI_NOT_INITIALIZED;

  _uiWindow = ge->createUIWindow(this);

  err_t err = _uiWindow->create(createFlags);
  if (err) destroyWindow();

  return err;
}

err_t Widget::destroyWindow()
{
  if (!_uiWindow) return ERR_RT_INVALID_HANDLE;

  delete _uiWindow;
  _uiWindow = NULL;
  return true;
}

String Widget::getWindowTitle() const
{
  String title;
  if (_uiWindow) _uiWindow->getTitle(title);
  return title;
}

err_t Widget::setWindowTitle(const String& title)
{
  if (!_uiWindow) return ERR_RT_INVALID_HANDLE;
  return _uiWindow->setTitle(title);
}

Image Widget::getWindowIcon() const
{
  Image i;
  if (_uiWindow) _uiWindow->getIcon(i);
  return i;
}

err_t Widget::setWindowIcon(const Image& icon)
{
  if (!_uiWindow) return ERR_RT_INVALID_HANDLE;
  return _uiWindow->setIcon(icon);
}

Point Widget::getWindowGranularity() const
{
  Point sz(0, 0);
  if (_uiWindow) _uiWindow->getSizeGranularity(sz);
  return sz;
}

err_t Widget::setWindowGranularity(const Point& pt)
{
  if (!_uiWindow) return ERR_RT_INVALID_HANDLE;
  return _uiWindow->setSizeGranularity(pt);
}

// ============================================================================
// [Fog::Widget - Geometry]
// ============================================================================

void Widget::setPosition(const Point& pt)
{
  if (_rect.getPosition() == pt) return;

  if (_uiWindow)
  {
    _uiWindow->move(pt);
  }
  else
  {
    GuiEngine* ge = Application::getInstance()->getGuiEngine();
    if (!ge) return;

    ge->dispatchConfigure(this, Rect(pt.getX(), pt.getY(), getWidth(), getHeight()), false);
  }
}

void Widget::setSize(const Size& sz)
{
  if (_rect.getSize() == sz) return;

  if (_uiWindow)
  {
    _uiWindow->resize(sz);
  }
  else
  {
    GuiEngine* ge = Application::getInstance()->getGuiEngine();
    if (!ge) return;

    ge->dispatchConfigure(this, Rect(getX1(), getY1(), sz.getWidth(), sz.getHeight()), false);
  }
}

void Widget::setRect(const Rect& rect)
{
  if (_rect == rect) return;

  if (_uiWindow)
  {
    _uiWindow->reconfigure(rect);
  }
  else
  {
    GuiEngine* ge = Application::getInstance()->getGuiEngine();
    if (!ge) return;

    ge->dispatchConfigure(this, rect, false);
  }
}

void Widget::setOrigin(const Point& pt)
{
  if (_origin == pt) return;

  OriginEvent e;
  e._origin = pt;

  _origin = pt;
  sendEvent(&e);

  update(Widget::UFlagUpdateAll);
}

bool Widget::worldToClient(Point* coords) const
{
  Widget* w = const_cast<Widget*>(this);

  do {
    coords->translate(w->getOrigin());
    if (w->_uiWindow) return w->_uiWindow->worldToClient(coords);
    coords->translate(w->getPosition().negated());
    w = w->_parent;
  } while (w);

  return false;
}

bool Widget::clientToWorld(Point* coords) const
{
  Widget* w = const_cast<Widget*>(this);

  do {
    coords->translate(w->getOrigin());
    if (w->_uiWindow) return w->_uiWindow->clientToWorld(coords);
    coords->translate(w->getPosition());
    w = w->_parent;
  } while (w);

  return false;
}

bool Widget::translateCoordinates(Widget* to, Widget* from, Point* coords)
{
  Widget* w;
  int x, y;

  if (to == from) return true;

  // Traverse 'from' -> 'to'.
  w = from;
  x = coords->getX();
  y = coords->getY();

  while (w->_parent)
  {
    x += w->getOrigin().getX();
    y += w->getOrigin().getY();
    x += w->getRect().getX();
    y += w->getRect().getY();

    w = w->_parent;

    if (w == to)
    {
      coords->set(x, y);
      return true;
    }
  }

  // Traverse 'to' -> 'from'.
  w = to;
  x = coords->getX();
  y = coords->getY();

  while (w->_parent)
  {
    x -= w->getOrigin().getX();
    y -= w->getOrigin().getY();
    x -= w->getRect().getX();
    y -= w->getRect().getY();

    w = w->_parent;

    if (w == from)
    {
      coords->set(x, y);
      return true;
    }
  }

  // These widgets are not in relationship.
  return false;
}

// ============================================================================
// [Fog::Widget - Hit Testing]
// ============================================================================

Widget* Widget::hitTest(const Point& pt) const
{
  int x = pt.getX();
  int y = pt.getY();

  if (x < 0 || y < 0 || x > _rect.getWidth() || y > _rect.getHeight()) return NULL;

  List<Widget*>::ConstIterator it(_children);
  for (it.toEnd(); it.isValid(); it.toPrevious())
  {
    if (it.value()->_rect.contains(pt)) return it.value();
  }

  return const_cast<Widget*>(this);
}

Widget* Widget::getChildAt(const Point& pt, bool recursive) const
{
  int x = pt.getX();
  int y = pt.getY();

  if (x < 0 || y < 0 || x > _rect.getWidth() || y > _rect.getHeight()) return NULL;

  Widget* current = const_cast<Widget*>(this);

repeat:
  {
    List<Widget*>::ConstIterator it(current->_children);
    for (it.toEnd(); it.isValid(); it.toPrevious())
    {
      if (it.value()->_rect.contains(pt))
      {
        current = it.value();
        x -= current->getX();
        y -= current->getY();
        if (current->hasChildren()) goto repeat;
        break;
      }
    }
  }

  return current;
}

// ============================================================================
// [Fog::Widget - Layout]
// ============================================================================

void Widget::setLayout(Layout* lay)
{
  if (lay->_parentItem == this) return;
  if (lay->_parentItem)
  {
    fog_stderr_msg("Fog::Widget", "setLayout", "Can't set layout that has already parent");
    return;
  }

  deleteLayout();

  if (lay)
  {
    _layout = lay;
    lay->_parentItem = this;

    LayoutEvent e(EV_LAYOUT_SET);
    this->sendEvent(&e);
    lay->sendEvent(&e);

    invalidateLayout();
  }
}

void Widget::deleteLayout()
{
  Layout* lay = takeLayout();
  if (lay) lay->destroy();
}

Layout* Widget::takeLayout()
{
  Layout* lay = _layout;

  if (lay)
  {
    lay->_parentItem = NULL;
    _layout = NULL;

    invalidateLayout();

    LayoutEvent e(EV_LAYOUT_REMOVE);
    lay->sendEvent(&e);
    this->sendEvent(&e);
  }

  return lay;
}

// ============================================================================
// [Layout Hints]
// ============================================================================

Size Widget::getSizeHint() const
{
  return Size(-1, -1);
}

void Widget::setSizeHint(const Size& sizeHint)
{
}

Size Widget::getMinimumSize() const
{
  return Size(-1, -1);
}

void Widget::setMinimumSize(const Size& minSize)
{
}

Size Widget::getMaximumSize() const
{
  return Size(-1, -1);
}

void Widget::setMaximumSize(const Size& maxSize)
{
}

// ============================================================================
// [Layout Policy]
// ============================================================================

uint32_t Widget::getLayoutPolicy() const
{
  return _layoutPolicy;
}

void Widget::setLayoutPolicy(uint32_t policy)
{
  if (_layoutPolicy == policy) return;

  _layoutPolicy = policy;
  invalidateLayout();
}

// ============================================================================
// [Layout Height For Width]
// ============================================================================

bool Widget::hasHeightForWidth() const
{
  return _hasHeightForWidth;
}

int Widget::getHeightForWidth(int width) const
{
  return -1;
}

// ============================================================================
// [Layout State]
// ============================================================================

bool Widget::isLayoutDirty() const
{
  return _isLayoutDirty;
}

void Widget::invalidateLayout() const
{
  // Don't invalidate more times, it can be time consuming.
  if (_isLayoutDirty) return;

  // Invalidate widget that has layout.
  Widget* w = const_cast<Widget*>(this);
  while (w->_layout == NULL)
  {
    w = w->_parent;
    if (w == NULL) return;
  }

  w->_isLayoutDirty = true;
  w->_layout->invalidateLayout();

  if (w->_parent && w->_parent->_isLayoutDirty == false) w->_parent->invalidateLayout();
}

// ============================================================================
// [Fog::Widget - State]
// ============================================================================

void Widget::setEnabled(bool val)
{
  if ( val && _state != WIDGET_DISABLED) return;
  if (!val && _state == WIDGET_DISABLED) return;

  if (_uiWindow)
  {
    if (val)
      _uiWindow->enable();
    else
      _uiWindow->disable();
  }
  else
  {
    GuiEngine* ge = Application::getInstance()->getGuiEngine();
    if (!ge) return;

    ge->dispatchEnabled(this, val);
  }
}

// ============================================================================
// [Fog::Widget - Visibility]
// ============================================================================

void Widget::setVisible(bool val)
{
  if ( val && _visibility != WIDGET_HIDDEN) return;
  if (!val && _visibility == WIDGET_HIDDEN) return;

  if (_uiWindow)
  {
    if (val)
      _uiWindow->show();
    else
      _uiWindow->hide();
  }
  else
  {
    GuiEngine* ge = Application::getInstance()->getGuiEngine();
    if (!ge) return;

    ge->dispatchVisibility(this, val);
  }
}

// ============================================================================
// [Fog::Widget - Orientation]
// ============================================================================

void Widget::setOrientation(uint32_t val)
{
  if (orientation() == val) return;

  _orientation = val;

  GuiEngine* ge = Application::getInstance()->getGuiEngine();
  if (!ge) return;

  ge->dispatchConfigure(this, _rect, true);
}

// ============================================================================
// [Fog::Widget - Caret]
// ============================================================================

#if 0
bool Widget::showCaret()
{
  return 0;
}

bool Widget::showCaret(const CaretProperties& properties)
{
  return 0;
}

bool Widget::hideCaret()
{
  return 0;
}
#endif

// ============================================================================
// [Fog::Widget - TabOrder]
// ============================================================================

void Widget::setTabOrder(int tabOrder)
{
  _tabOrder = tabOrder;
}

// ============================================================================
// [Fog::Widget - Focus]
// ============================================================================

void Widget::setFocusPolicy(uint32_t val)
{
  _focusPolicy = val;
}

Widget* Widget::getFocusableWidget(int focusable)
{
  return NULL;
}

void Widget::takeFocus(uint32_t reason)
{
  if (!hasFocus() && getVisibility() == WIDGET_VISIBLE && getState() == WIDGET_ENABLED)
  {
    // TODO:
    //Application::getInstance()->getGuiEngine()->dispatchTakeFocus(this, reason);
  }
}

void Widget::giveFocusNext(uint32_t reason)
{
  Widget* w = getFocusableWidget(FOCUSABLE_NEXT);
  if (w) w->takeFocus(reason);
}

void Widget::giveFocusPrevious(uint32_t reason)
{
  Widget* w = getFocusableWidget(FOCUSABLE_PREVIOUS);
  if (w) w->takeFocus(reason);
}

Widget* Widget::_findFocus() const
{
  Widget* w = const_cast<Widget*>(this);
  while (w->_focusLink) w = w->_focusLink;
  return w;
}

// ============================================================================
// [Fog::Widget - Font]
// ============================================================================

void Widget::setFont(const Font& font)
{
  _font = font;
  update(UFlagRepaintWidget | UFlagUpdateGeometry);
}

// ============================================================================
// [Fog::Widget - Update]
// ============================================================================

void Widget::update(uint32_t updateFlags)
{
  uint32_t u = _uflags;
  if ((u & updateFlags) == updateFlags || (u & UFlagUpdateAll)) return;

  _uflags |= updateFlags | UFlagUpdate;

  if (u & UFlagUpdate) return;

  if (_uiWindow)
  {
    if (!_uiWindow->isDirty()) _uiWindow->setDirty();
    return;
  }

  // No UFlagUpdate nor UIWindow, traverse all parents up to UIWindow and
  // set UFlagUpdateChild to all parents.
  Widget* w = _parent;
  while (w && !(w->_uflags & (UFlagUpdateChild | UFlagUpdateAll)))
  {
    w->_uflags |= UFlagUpdateChild;
    if (w->_uiWindow)
    {
      if (!w->_uiWindow->isDirty()) w->_uiWindow->setDirty();
      return;
    }
    w = w->_parent;
  }
}

// ============================================================================
// [Fog::Widget - Repaint]
// ============================================================================

void Widget::repaint(uint32_t repaintFlags)
{
  update(repaintFlags);
}

// ============================================================================
// [Fog::Widget - Events]
// ============================================================================

void Widget::onChildAdd(ChildEvent* e)
{
}

void Widget::onChildRemove(ChildEvent* e)
{
}

void Widget::onEnable(StateEvent* e)
{
}

void Widget::onDisable(StateEvent* e)
{
}

void Widget::onShow(VisibilityEvent* e)
{
}

void Widget::onHide(VisibilityEvent* e)
{
}

void Widget::onConfigure(ConfigureEvent* e)
{
}

void Widget::onFocusIn(FocusEvent* e)
{
}

void Widget::onFocusOut(FocusEvent* e)
{
}

void Widget::onKeyPress(KeyEvent* e)
{
}

void Widget::onKeyRelease(KeyEvent* e)
{
}

void Widget::onMouseIn(MouseEvent* e)
{
}

void Widget::onMouseOut(MouseEvent* e)
{
}

void Widget::onMouseMove(MouseEvent* e)
{
}

void Widget::onMousePress(MouseEvent* e)
{
}

void Widget::onMouseRelease(MouseEvent* e)
{
}

void Widget::onClick(MouseEvent* e)
{
}

void Widget::onDoubleClick(MouseEvent* e)
{
}

void Widget::onWheel(MouseEvent* e)
{
}

void Widget::onSelection(SelectionEvent* e)
{
}

void Widget::onPaint(PaintEvent* e)
{
}

void Widget::onClose(CloseEvent* e)
{
}

void Widget::onThemeChange(ThemeEvent* e)
{
}

void Widget::onLayout(LayoutEvent* e)
{
}

} // Fog namespace
