// [Fog/Graphics Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/FileUtil.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/OS.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/UserInfo.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/FontEngine.h>
#include <Fog/Graphics/FontManager.h>
#include <Fog/Graphics/Glyph.h>
#include <Fog/Graphics/GlyphCache.h>

namespace Fog {

// ============================================================================
// [Fog::FontFace]
// ============================================================================

FontFace::FontFace()
{
  refCount.init(1);
  memset(&metrics, 0, sizeof(FontMetrics));
  memset(&caps, 0, sizeof(FontCaps));
  scalable = false;
}

FontFace::~FontFace()
{
}

FontFace* FontFace::ref()
{
  refCount.inc();
  return this;
}

void FontFace::deref()
{
  if (refCount.deref()) delete this;
}

// ============================================================================
// [Fog::FontEngine]
// ============================================================================

FontEngine::FontEngine(const String& name) :
  _name(name)
{
}

FontEngine::~FontEngine()
{
}

// TODO: Move this out.
List<String> FontEngine::getDefaultFontDirectories()
{
  List<String> paths;

#if defined(FOG_OS_WINDOWS)
  // Add WIN font directory.
  String winFonts = OS::getWindowsDirectory();
  FileUtil::joinPath(winFonts, winFonts, Ascii8("fonts"));
  paths.append(winFonts);
#elif defined(FOG_OS_MAC)
  // Add MAC font directories.
  String home;

  if (UserInfo::getDirectory(home, UserInfo::DIRECTORY_HOME) == ERR_OK)
  {
    paths.append(home + Ascii8("/Library/Fonts"));
  }

  paths.append(Ascii8("/System/Library/Fonts"));
  paths.append(Ascii8("/Library/Fonts"));
#elif defined(FOG_OS_POSIX)
  // add $HOME and $HOME/fonts directories.
  {
    String home;

    if (UserInfo::getDirectory(home, UserInfo::DIRECTORY_HOME) == ERR_OK)
    {
      paths.append(home);
      paths.append(home + Ascii8("/fonts"));
    }
  }

  // Add font directories found in linux/unix systems...

  // Gentoo default font paths:
  paths.append(Ascii8("/usr/share/fonts"));
  paths.append(Ascii8("/usr/share/fonts/TTF"));
  paths.append(Ascii8("/usr/share/fonts/corefonts"));
  paths.append(Ascii8("/usr/share/fonts/local"));
  paths.append(Ascii8("/usr/share/fonts/ttf-bitstream-vera"));
  paths.append(Ascii8("/usr/local/share/fonts"));
  paths.append(Ascii8("/usr/local/share/fonts/TTF"));
  paths.append(Ascii8("/usr/local/share/fonts/corefonts"));
  paths.append(Ascii8("/usr/local/share/fonts/local"));
  paths.append(Ascii8("/usr/local/share/fonts/ttf-bitstream-vera"));

  // Ubuntu default truetype font paths:
  paths.append(Ascii8("/usr/share/fonts/truetype/msttcorefonts"));
#else
# warning "Fog::FontEngine::Unknown operating system target"
#endif // FOG_OS_POSIX

  return paths;
}

} // Fog namespace
