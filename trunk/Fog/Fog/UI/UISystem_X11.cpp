



void UISystemX11::doBlitWindow(UIWindow* window, const Box* rects, sysuint_t count)
{
  window->_backingStore->blitRects(window->handle(), rects, count);
}



// [Fog::UIBackingStoreX11]

bool UIBackingStoreX11::resize(uint width, uint height, bool cache)
{
  uint32_t targetWidth = width;
  uint32_t targetHeight = height;
  sysint_t targetSize;
  sysint_t targetStride;

  bool destroyImage = false;
  bool createImage = false;

  if (width == 0 || height == 0)
  {
    destroyImage = true;
  }
  else
  {
    if (cache)
    {
      if (width <= _widthOrig && height <= _heightOrig)
      {
        // Cached, here can be debug counter to create
        // statistics about usability of that
        _width = width;
        _height = height;
        return true;
      }

      // Don't create smaller buffer that previous!
      targetWidth  = width;
      targetHeight = height;

      if (targetWidth  < _width)  targetWidth  = _width;
      if (targetHeight < _height) targetHeight = _height;

      targetWidth  = (targetWidth  + 127) & ~127;
      targetHeight = (targetHeight + 127) & ~127;
    }

    destroyImage = true;
    createImage = (targetWidth > 0 && targetHeight > 0);
  }

  // Destroy image buffer
  if (destroyImage)
  {
    switch (_type)
    {
      case TypeNone:
        break;

      case TypeXShmPixmap:
        FOG_XAPI->pXShmDetach(FOG_Application::x11().display(), &_shmi);
        FOG_XAPI->pXSync(FOG_Application::x11().display(), False);
    
        shmdt(_shmi.shmaddr);
        shmctl(_shmi.shmid, IPC_RMID, NULL);
    
        FOG_XAPI->pXFreePixmap(FOG_Application::x11().display(), _pixmap);

        if (_pixelsSecondary) Fog::Memory_free(_pixelsSecondary);
        break;

      case TypeXImage:
      case TypeXImageWithPixmap:
        // We want to free image data ourselves
        Fog::Memory_free(_pixelsPrimary);
        _xi->data = NULL;
        
        FOG_XAPI->pXDestroyImage(_xi);
#if defined(FOG_UIBACKINGSTORE_FORCE_PIXMAP)
        if (_type == TypeXImageWithPixmap)
        {
          FOG_XAPI->pXFreePixmap(FOG_Application::x11().display(), _pixmap);
        }
#endif
        if (_pixelsSecondary) Fog::Memory::free(_pixelsSecondary);
        break;
#endif // FOG_UI_X11
    }
  }

  // Create image buffer
  if (createImage)
  {
    // correct target BPP, some X server settings can be amazing:-)
    uint targetBPP = FOG_Application::displayInfo().depth();
    if (targetBPP > 4 && targetBPP < 8) targetBPP = 8;
    else if (targetBPP == 15) targetBPP = 16;
    else if (targetBPP == 24) targetBPP = 32;

    targetStride = Fog_Image_strideFromWidth(targetWidth, targetBPP);
    targetSize = targetStride * targetHeight;

    // TypeXShmPixmap
    if (FOG_Application::x11().xShm())
    {
      if ((_shmi.shmid = shmget(IPC_PRIVATE, targetSize, IPC_CREAT | IPC_EXCL | 0666)) < 0)
      {
        fog_stderr_msg("Fog::UIBackingStoreX11", "resize", "shmget() failed: %s", strerror(errno));
        goto __tryImage;
      }

      if ((_shmi.shmaddr = (char *)shmat(_shmi.shmid, NULL, 0)) == NULL)
      {
        fog_stderr_msg("Fog::UIBackingStoreX11", "resize", "shmat() failed: %s", strerror(errno));
        shmctl(_shmi.shmid, IPC_RMID, NULL);
        goto __tryImage;
      }

      _shmi.readOnly = False;

      // Get the X server to attach this segment to a pixmap
      FOG_XAPI->pXShmAttach(FOG_Application::x11().display(), &_shmi);
      FOG_XAPI->pXSync(FOG_Application::x11().display(), False);

      _pixmap = FOG_XAPI->pXShmCreatePixmap(
        FOG_Application::x11().display(),
        FOG_Application::x11().root(),
        _shmi.shmaddr,
        &_shmi,
        targetWidth, targetHeight,
        FOG_Application::displayInfo().depth());

      _type = TypeXShmPixmap;
      _pixelsPrimary = (uint8_t*)_shmi.shmaddr;
    }

    // TypeXImage
    else
    {
__tryImage:
      // try to alloc image data
      _pixelsPrimary = (uint8_t*)Fog::Memory_alloc(targetSize);
      if (!_pixelsPrimary)
      {
        fog_stderr_msg("Fog::UIBackingStoreX11", "resize", "Memory allocation error %s", strerror(errno));
        goto fail;
      }

      // try to create XImage
      _xi = FOG_XAPI->pXCreateImage(
        FOG_Application::x11().display(),
        FOG_Application::x11().visual(),
        FOG_Application::displayInfo().depth(),
        ZPixmap, 0, (char *)_pixelsPrimary,
        targetWidth, targetHeight,
        32, targetStride);

      if (!_xi)
      {
        fog_stderr_msg("Fog::UIBackingStoreX11", "resize", "XCreateImage() failed");
        Fog::Memory_free(_pixelsPrimary);
        goto fail;
      }
#if defined(FOG_UIBACKINGSTORE_FORCE_PIXMAP)
      // this step should be optional, but can increase performance
      // on remote machines (local machines should use XSHM extension)
      _pixmap = FOG_XAPI->pXCreatePixmap(
        FOG_Application::x11().display(),
        FOG_Application::x11().root(),
        targetWidth, targetHeight,
        FOG_Application::displayInfo().depth());

      if (_pixmap)
        _type = TypeXImageWithPixmap;
      else
#endif
        _type = TypeXImage;
    }

    if (_type != TypeNone)
    {
      _created = Fog::TimeTicks::now();
      _expire = _created + Fog::TimeDelta::fromSeconds(15);

      _format.set(Fog::ImageFormat::XRGB32);

      _stridePrimary = targetStride;
      _widthOrig = targetWidth;
      _heightOrig = targetHeight;

      // Now image is created and we must check if we have correct
      // depth and pixel format, if not, we must create secondary
      // buffer that will be used for conversion
      sysint_t secondaryStride = (sysint_t)targetWidth << 2;
      
      if (targetStride != secondaryStride)
      {
        FOG_PixelConverterDescription pcdTarget;
        FOG_PixelConverterDescription pcdSource;

        pcdTarget.depth = targetBPP;
        pcdTarget.rMask = FOG_Application::displayInfo().redMask();
        pcdTarget.gMask = FOG_Application::displayInfo().greenMask();
        pcdTarget.bMask = FOG_Application::displayInfo().blueMask();
        pcdTarget.aMask = 0x00000000;
        pcdTarget.byteSwapped = FOG_Application::displayInfo().isByteSwapped();

        pcdSource.depth = 32;
        pcdSource.rMask = 0x00FF0000;
        pcdSource.gMask = 0x0000FF00;
        pcdSource.bMask = 0x000000FF;
        pcdSource.aMask = 0x00000000;
        pcdSource.byteSwapped = false;

        // need extra buffer
        _pixelsSecondary = (uint8_t*)Fog::Memory_alloc(secondaryStride * targetHeight);
        if (!_pixelsSecondary)
        {
          fog_stderr_msg("Fog::UIBackingStoreX11", "resize", "can't create secondary buffer for converting");
        }

        _strideSecondary = secondaryStride;

        _usingConverter = true;
        _converter.dither = 1;
        _converter.init(pcdTarget, pcdSource);

        _pixels = _pixelsSecondary;
        _width = width;
        _height = height;
        _stride = _strideSecondary;
      }
      else
      {
        // Extra buffer not needed.
        _pixelsSecondary = NULL;
        _strideSecondary = 0;

        _usingConverter = false;

        _pixels = _pixelsPrimary;
        _width = width;
        _height = height;
        _stride = _stridePrimary;
      }
      return true;
    }
  }

fail:
  _clear();
  return false;
}

void UIBackingStoreX11::destroy()
{
  resize(0, 0, false);
}

void UIBackingStoreX11::updateRects(const Box* rects, sysuint_t count)
{
  // If there is secondary buffer, we need to convert it to primary
  // one that has same depth and pixel format as X display.
  if (_pixelsSecondary && _usingConverter)
  {
    _converter.palConv = FOG_Application::displayInfo().palConv();

    for (sysuint_t i = 0; i != count; i++)
    {
      int x1 = rects[i].x1();
      int y1 = rects[i].y1();
      int x2 = rects[i].x2();
      int y2 = rects[i].y2();

      // Apply clip. In rare cases rectangles can contain bigger
      // coordinates that buffers are (reason can be resizing).
      if (x1 < 0) x1 = 0;
      if (y1 < 0) y1 = 0;
      if (x2 > (int)width()) x2 = (int)width();
      if (y2 > (int)height()) y2 = (int)height();

      if (x1 >= x2 || y1 >= y2) continue;

      uint w = uint(x2 - x1);
      uint h = uint(y2 - y1);

      _converter.ditherOrigin.set(x1, y1);
      _converter.convertRect(
        _pixelsPrimary + (y1 * _stridePrimary), _stridePrimary, x1,
        _pixelsSecondary + (y1 * _strideSecondary), _strideSecondary, x1,
        w, h);
    }
  }

  // Possible secondary step is to put XImage to Pixmap here, because
  // it can increase performance on remote machines. Idea is to do 
  // put XImage here instead in blitRects() method.
  if (_type == TypeXImageWithPixmap)
  {
    for (sysuint_t i = 0; i != count; i++)
    {
      int x = rects[i].x1();
      int y = rects[i].y1();
      uint w = uint(rects[i].width());
      uint h = uint(rects[i].height());

      FOG_XAPI->pXPutImage(
        FOG_Application::x11().display(),
        _pixmap,
        FOG_Application::x11().gc(),
        _xi,
        x, y,
        x, y,
        w, h);
    }
  }
}

void UIBackingStoreX11::blitRects(XID target, const Fog::Box* rects, sysuint_t count)
{
  register sysuint_t i;

  switch (type())
  {
    case TypeNone:
      break;

    // These ones uses pixmap as X resource
    case TypeXShmPixmap:
    case TypeXImageWithPixmap:
      for (i = 0; i != count; i++)
      {
        int x = rects[i].x1();
        int y = rects[i].y1();
        int w = rects[i].width();
        int h = rects[i].height();

        FOG_XAPI->pXCopyArea(
          FOG_Application::x11().display(),
          _pixmap,
          target,
          FOG_Application::x11().gc(),
          x, y,
          w, h,
          x, y);
      }
      break;

    case TypeXImage:
      for (i = 0; i != count; i++)
      {
        int x = rects[i].x1();
        int y = rects[i].y1();
        int w = rects[i].width();
        int h = rects[i].height();
        
        FOG_XAPI->pXPutImage(
          FOG_Application::x11().display(),
          target,
          FOG_Application::x11().gc(),
          _xi,
          x, y,
          x, y,
          w, h);
      }
      break;
  }
}

