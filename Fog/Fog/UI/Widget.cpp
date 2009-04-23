// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Application.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Vector.h>
#include <Fog/Graphics/Region.h>
#include <Fog/UI/Layout.h>
#include <Fog/UI/LayoutItem.h>
#include <Fog/UI/UISystem.h>
#include <Fog/UI/Widget.h>

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
  _lastFocus(NULL),
  _focusLink(NULL),
  _uflags(0),
  _state(Enabled),
  _visibility(Hidden),
  _focusPolicy(NoFocus),
  _hasFocus(false),
  _orientation(OrientationHorizontal),
  _reserved(0),
  _tabOrder(0)
{
  _flags |= IsWidget;

  // TODO ?
  _focusLink = NULL;

  if (createFlags & UIWindow::CreateUIWindow) 
  {
    createWindow(createFlags);
  }
}

Widget::~Widget()
{
  if (_uiWindow) destroyWindow();
}

// ============================================================================
// [Fog::Widget - Hierarchy]
// ============================================================================

bool Widget::setParent(Widget* p)
{
  if (p) return p->add(this);

  if (parent() == NULL)
    return true;
  else
    return parent()->remove(this);
}

bool Widget::add(Widget* w)
{
  // First remove the element from it's parent
  Widget* p = w->parent();
  if (p != NULL && p != this) 
  {
    // Remove element can fail, so we return false in that case
    if (!p->remove(w)) return false;
  }

  // Now element can be added, call virtual method
  return _add(_children.length(), w);
}

bool Widget::remove(Widget* w)
{
  Widget* p = w->parent();
  if (p != this) return false;

  return _remove(_children.indexOf(w), w);
}

bool Widget::_add(sysuint_t index, Widget* w)
{
  FOG_ASSERT(index <= _children.length());

  _children.insert(index, w);
  w->_parent = this;

  return true;
}

bool Widget::_remove(sysuint_t index, Widget* w)
{
  FOG_ASSERT(index < _children.length());
  FOG_ASSERT(_children.cAt(index) == w);

  _children.removeAt(index);
  w->_parent = NULL;

  return true;
}

// ============================================================================
// [Fog::Widget - UIWindow]
// ============================================================================

UIWindow* Widget::closestUIWindow() const
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
  if (_uiWindow) return Error::UIWindowAlreadyExists;

  _uiWindow = Application::instance()->uiSystem()->createUIWindow(this);

  err_t err = _uiWindow->create(createFlags);
  if (err) destroyWindow();

  return err;
}

err_t Widget::destroyWindow()
{
  if (!_uiWindow) return Error::InvalidHandle;

  delete _uiWindow;
  _uiWindow = NULL;
  return true;
}

String32 Widget::windowTitle() const
{
  String32 title;
  if (_uiWindow) _uiWindow->getTitle(title);
  return title;
}

Image Widget::windowIcon() const
{
  Image i;
  if (_uiWindow) _uiWindow->getIcon(i);
  return i;
}

Point Widget::windowGranularity() const
{
  Point sz(0, 0);
  if (_uiWindow) _uiWindow->getSizeGranularity(sz);
  return sz;
}

err_t Widget::setWindowTitle(const String32& title)
{
  if (!_uiWindow) return Error::InvalidHandle;
  return _uiWindow->setTitle(title);
}

err_t Widget::setWindowIcon(const Image& icon)
{
  if (!_uiWindow) return Error::InvalidHandle;
  return _uiWindow->setIcon(icon);
}

err_t Widget::setWindowGranularity(const Point& pt)
{
  if (!_uiWindow) return Error::InvalidHandle;
  return _uiWindow->setSizeGranularity(pt);
}

// ============================================================================
// [Fog::Widget - Geometry]
// ============================================================================

void Widget::setRect(const Rect& rect)
{
  if (_rect == rect) return;

  Application::instance()->uiSystem()->dispatchConfigure(this,
    rect, false);
}

void Widget::setPosition(const Point& pt)
{
  if (_rect.point() == pt) return;

  Application::instance()->uiSystem()->dispatchConfigure(this,
    Rect(pt.x(), pt.y(), width(), height()), false);
}

void Widget::setSize(const Size& sz)
{
  if (_rect.size() == sz) return;

  Application::instance()->uiSystem()->dispatchConfigure(this,
    Rect(x1(), y1(), sz.width(), sz.height()), false);
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
    coords->translate(w->origin());
    if (w->_uiWindow) return w->_uiWindow->worldToClient(coords);
    coords->translate(w->position().negated());
    w = w->parent();
  } while (w);

  return false;
}

bool Widget::clientToWorld(Point* coords) const
{
  Widget* w = const_cast<Widget*>(this);

  do {
    coords->translate(w->origin());
    if (w->_uiWindow) return w->_uiWindow->clientToWorld(coords);
    coords->translate(w->position());
    w = w->parent();
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
  x = coords->x();
  y = coords->y();

  while (w->parent())
  {
    x += w->_origin.x();
    y += w->_origin.y();
    x += w->_rect.x1();
    y += w->_rect.y1();

    w = w->parent();

    if (w == to)
    {
      coords->set(x, y);
      return true;
    }
  }

  // Traverse 'to' -> 'from'.
  w = to;
  x = coords->x();
  y = coords->y();

  while (w->parent())
  {
    x -= w->_origin.x();
    y -= w->_origin.y();
    x -= w->_rect.x1();
    y -= w->_rect.y1();

    w = w->parent();

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
  int x = pt.x();
  int y = pt.y();

  if (x < 0 || y < 0 || x > _rect.width() || y > _rect.height()) return NULL;

  Vector<Widget*>::ConstIterator it(_children);
  for (it.toEnd(); it.isValid(); it.toPrevious())
  {
    if (it.value()->_rect.contains(pt)) return it.value();
  }
  return const_cast<Widget*>(this);
}

// ============================================================================
// [Fog::Widget - Layout]
// ============================================================================

void Widget::setLayout(Layout* layout)
{
}

void Widget::removeLayout()
{
}

// ============================================================================
// [Fog::Widget - LayoutItem]
// ============================================================================

void Widget::invalidateLayout() const
{
}

void Widget::setSizeHint(const Fog::Size& sizeHint)
{
}

void Widget::setMinimumSize(const Fog::Size& sizeHint)
{
}

void Widget::setMaximumSize(const Fog::Size& sizeHint)
{
}

int Widget::heightForWidth(int width) const
{
  return 0;
}

// ============================================================================
// [Fog::Widget - State]
// ============================================================================

void Widget::setEnabled(bool val)
{
  if (val && (_state == Enabled || _state == DisabledByParent)) return;
  if (!val && _state == Disabled) return;

  if (_uiWindow)
  {
    if (val)
      _uiWindow->enable();
    else
      _uiWindow->disable();
  }
  else
  {
    Application::instance()->uiSystem()->dispatchEnabled(this, val);
  }
}

// ============================================================================
// [Fog::Widget - Visibility]
// ============================================================================

void Widget::setVisible(bool val)
{
  if (val && (_visibility == Visible || _visibility == HiddenByParent)) return;
  if (!val && _visibility == Hidden) return;

  if (_uiWindow)
  {
    if (val)
      _uiWindow->show();
    else
      _uiWindow->hide();
  }
  else
  {
    Application::instance()->uiSystem()->dispatchVisibility(this, val);
  }
}

// ============================================================================
// [Fog::Widget - Orientation]
// ============================================================================

void Widget::setOrientation(uint32_t val)
{
  if (orientation() == val) return;

  _orientation = val;
  Application::instance()->uiSystem()->dispatchConfigure(this, _rect, true);
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

Widget* Widget::getFocusableWidget(uint32_t focusable)
{
  return NULL;
}

void Widget::takeFocus(uint32_t reason)
{
  if (!hasFocus() && visibility() == Visible && state() == Enabled)
  {
    // TODO:
    //Application::instance()->uiSystem()->dispatchTakeFocus(this, reason);
  }
}

void Widget::giveFocusNext(uint32_t reason)
{
  Widget* w = getFocusableWidget(NextFocusable);
  if (w) w->takeFocus(reason);
}

void Widget::giveFocusPrevious(uint32_t reason)
{
  Widget* w = getFocusableWidget(PreviousFocusable);
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
    if (!_uiWindow->dirty()) _uiWindow->setDirty();
    return;
  }

  // No UFlagUpdate nor UIWindow, traverse all parents up to UIWindow and
  // set UFlagUpdateChild to all parents.
  Widget* w = this->parent();
  while (w && !(w->_uflags & (UFlagUpdateChild | UFlagUpdateAll)))
  {
    w->_uflags |= UFlagUpdateChild;
    if (w->_uiWindow)
    {
      if (!w->_uiWindow->dirty()) w->_uiWindow->setDirty();
      return;
    }
    w = w->parent();
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

void Widget::onStateChange(StateEvent* e)
{
}

void Widget::onVisibilityChange(VisibilityEvent* e)
{
}

void Widget::onConfigure(ConfigureEvent* e)
{
}

void Widget::onFocus(FocusEvent* e)
{
}

void Widget::onKey(KeyEvent* e)
{
}

void Widget::onMouse(MouseEvent* e)
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

} // Fog namespace
