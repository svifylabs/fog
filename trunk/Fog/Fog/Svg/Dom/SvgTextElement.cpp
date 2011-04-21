// [Fog-Core]
//
// [License]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Tools/Strings.h>
#include <Fog/Svg/Dom/SvgTextElement_p.h>
#include <Fog/Svg/Render/SvgRender.h>
#include <Fog/Xml/Dom/XmlText.h>

namespace Fog {

// ============================================================================
// [Fog::SvgTextElement]
// ============================================================================

SvgTextElement::SvgTextElement() :
  SvgStyledElement(fog_strings->getString(STR_SVG_ELEMENT_text), SVG_ELEMENT_TEXT),
  a_x           (NULL, fog_strings->getString(STR_SVG_ATTRIBUTE_x         ), FOG_OFFSET_OF(SvgTextElement, a_x           )),
  a_y           (NULL, fog_strings->getString(STR_SVG_ATTRIBUTE_y         ), FOG_OFFSET_OF(SvgTextElement, a_y           )),
  a_dx          (NULL, fog_strings->getString(STR_SVG_ATTRIBUTE_dx        ), FOG_OFFSET_OF(SvgTextElement, a_dx          )),
  a_dy          (NULL, fog_strings->getString(STR_SVG_ATTRIBUTE_dy        ), FOG_OFFSET_OF(SvgTextElement, a_dy          )),
  a_textLength  (NULL, fog_strings->getString(STR_SVG_ATTRIBUTE_textLength), FOG_OFFSET_OF(SvgTextElement, a_textLength  ))
{
}

SvgTextElement::~SvgTextElement()
{
  _removeAttributes();
}

XmlAttribute* SvgTextElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_SVG_ATTRIBUTE_x           )) return (XmlAttribute*)&a_x;
  if (name == fog_strings->getString(STR_SVG_ATTRIBUTE_y           )) return (XmlAttribute*)&a_y;
  if (name == fog_strings->getString(STR_SVG_ATTRIBUTE_dx          )) return (XmlAttribute*)&a_dx;
  if (name == fog_strings->getString(STR_SVG_ATTRIBUTE_dy          )) return (XmlAttribute*)&a_dy;
  if (name == fog_strings->getString(STR_SVG_ATTRIBUTE_textLength  )) return (XmlAttribute*)&a_textLength;

  return base::_createAttribute(name);
}

err_t SvgTextElement::onRenderShape(SvgRenderContext* context) const
{
  err_t err = ERR_OK;
  XmlElement* e;

  float x = a_x.isAssigned() ? a_x.getCoordValue() : 0.0f;
  float y = a_y.isAssigned() ? a_y.getCoordValue() : 0.0f;

  context->_textCursor.set(x, y);

  for (e = firstChild(); e; e = e->nextSibling())
  {
    if (e->isSvgElement())
    {
      err = reinterpret_cast<SvgElement*>(e)->onRender(context);
      if (FOG_IS_ERROR(err)) break;
    }
    else if (e->isText())
    {
      String text = e->getTextContent();
      text.simplify();

      // TODO: Not optimal, just initial support for text rendering.
      PathD path;
      context->_font.getOutline(text, path);
      path.translate(PointD(x, y));
      context->drawPath(path);
    }
    else if (e->hasChildNodes())
    {
      _walkAndRender(e, context);
    }
  }

  return err;
}

err_t SvgTextElement::onCalcBoundingBox(RectF* box) const
{
  // TODO: SVGText bounding box.
  return ERR_RT_NOT_IMPLEMENTED;
}

} // Fog namespace