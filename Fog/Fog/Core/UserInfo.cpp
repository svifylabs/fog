// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/OS.h>
#include <Fog/Core/String.h>
#include <Fog/Core/TextCodec.h>
#include <Fog/Core/UserInfo.h>

#if defined(FOG_OS_POSIX)
#include <stdlib.h>
#include <pwd.h>
// for getuid() and getgid()
#if defined(FOG_HAVE_UNISTD_H)
#include <unistd.h>
#endif // FOG_HAVE_UNISTD_H
#endif

namespace Fog {

static bool homeDirectory(String32& to);

#if defined(FOG_OS_POSIX)

// TODO: Caching?
static bool xdgLookupDirWithFallback(const char *type, const String32& fallback, String32& result)
{
  FILE *file;
  char *home_dir, *config_home, *config_file;
  char buffer[512];
  char *user_dir;
  char *p, *d;
  int len;
  int relative;
  
  home_dir = getenv("HOME");

  if (home_dir == NULL)
     goto error;

  config_home = getenv ("XDG_CONFIG_HOME");
  if (config_home == NULL || config_home[0] == 0)
  {
    config_file = (char*)malloc (strlen (home_dir) + strlen ("/.config/user-dirs.dirs") + 1);
    strcpy (config_file, home_dir);
    strcat (config_file, "/.config/user-dirs.dirs");
  }
  else
  {
    config_file = (char*)malloc (strlen (config_home) + strlen ("/user-dirs.dirs") + 1);
    strcpy (config_file, config_home);
    strcat (config_file, "/user-dirs.dirs");
  }

  file = fopen (config_file, "r");
  free (config_file);
  if (file == NULL)
    goto error;

  user_dir = NULL;
  while (fgets (buffer, sizeof (buffer), file))
  {
    /* Remove newline at end */
    len = strlen (buffer);
    if (len > 0 && buffer[len-1] == '\n')
    buffer[len-1] = 0;
    
    p = buffer;
    while (*p == ' ' || *p == '\t')
      p++;
      
    if (strncmp (p, "XDG_", 4) != 0)
      continue;
    p += 4;
    if (strncmp (p, type, strlen (type)) != 0)
      continue;
    p += strlen (type);
    if (strncmp (p, "_DIR", 4) != 0)
      continue;
    p += 4;

    while (*p == ' ' || *p == '\t')
      p++;

    if (*p != '=')
      continue;
    p++;
      
    while (*p == ' ' || *p == '\t')
      p++;

    if (*p != '"')
      continue;
    p++;
      
    relative = 0;
    if (strncmp (p, "$HOME/", 6) == 0)
    {
      p += 6;
      relative = 1;
    }
    else if (*p != '/')
    {
      continue;
    }
      
    if (relative)
    {
      user_dir = (char*)malloc (strlen (home_dir) + 1 + strlen (p) + 1);
      strcpy (user_dir, home_dir);
      strcat (user_dir, "/");
    }
    else
    {
      user_dir = (char*)malloc (strlen (p) + 1);
      *user_dir = 0;
    }
      
    d = user_dir + strlen (user_dir);
    while (*p && *p != '"')
    {
      if ((*p == '\\') && (*(p+1) != 0))
        p++;
      *d++ = *p++;
    }
    *d = 0;
  }  
  fclose (file);

  if (user_dir)
  {
    result.set(user_dir, DetectLength, TextCodec::local8());
    free(user_dir);
    return true;
  }

error:
  result = fallback;
  return false;
}

static bool xdgLookupDir(const char *type, String32& result)
{
  if (!xdgLookupDirWithFallback(type, UserInfo::directory(UserInfo::Home), result))
  {
    if (strcmp(type, "DESKTOP") == 0)
    {
      result.append(Ascii8("/Desktop"));
    }
    return false;
  }
  else
    return true;
}

#elif defined(FOG_OS_WINDOWS)

// fixed: documents, music and videos returns as home directory
static bool registryLookupDir(const char *type, String32& to)
{
  HKEY k;
  DWORD d = 0, v_type, size;
  const char *reg_path;
  bool tryShellFolders = true;

  MemoryBuffer<1024> bufStorage;
  MemoryBuffer<1024> expandedStorage;

  char* buf;
  char* expanded;
  
  homeDirectory(to);

  reg_path = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders";
  
_back:    
  if (RegCreateKeyExA( HKEY_CURRENT_USER, reg_path,
      0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE |
      KEY_QUERY_VALUE, NULL, &k, &d ) != ERROR_SUCCESS )
  {
    return false;
  }
  
  if (RegQueryValueExA( k, type, NULL, &v_type, NULL, &size ) == ERROR_SUCCESS && size != 0 && (v_type == REG_SZ || v_type == REG_EXPAND_SZ ) )
  {
    buf = (char*)bufStorage.alloc((sysuint_t)size);
    RegQueryValueExA( k, type, NULL, NULL, (LPBYTE)buf, &size );
    if( *buf )
    {
      to.set(buf, DetectLength, TextCodec::local8());
      if( v_type == REG_EXPAND_SZ )
      {
        size = ExpandEnvironmentStringsA( buf, NULL, 0 );
        if( size )
        {
          size += 2;
          expanded = (char*)expandedStorage.alloc(size);

          ExpandEnvironmentStringsA(buf, expanded, size);
          if(expanded[0]) to.set(expanded, DetectLength, TextCodec::local8());
        }
      }
    }
  }
  else if(tryShellFolders)
  {
    tryShellFolders = false;
    RegCloseKey(k);
    reg_path = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";
    goto _back;
  }

  RegCloseKey(k);
  return true;
}
#endif // FOG_OS

static bool homeDirectory(String32& to)
{
#if defined(FOG_OS_WINDOWS)
  OS::getEnv(TemporaryString32<12>(Ascii8("USERPROFILE")), to);
  return true;
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  struct passwd *pwd = getpwuid(UserInfo::uid());
  if (pwd)
  {
    to.set(Local8(pwd->pw_dir));
    return true;
  }
  else
  {
    char* s;

    if ((s = getenv("HOME")) != 0)
    {
      to.set(s, DetectLength, TextCodec::local8());
      return true;
    }
    else if ((s = getenv("TMPDIR")) != 0)
    {
      to.set(s, DetectLength, TextCodec::local8());
      return false;
    }
    else
    {
      to.set(Ascii8("/tmp"));
      return false;
    }
  }
#endif // FOG_OS_POSIX
}

// [Fog::UserInfo]

#if defined(FOG_OS_WINDOWS)
static const char* directoryNames[] =
{
  "",
  "Desktop",
  "Personal",
  "My Music",
  "My Pictures",
  "My Video"
};
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
static const char* directoryNames[] =
{
  "",
  "DESKTOP",
  "DOCUMENTS",
  "MUSIC",
  "PICTURES",
  "VIDEOS"
};
#endif // FOG_OS_POSIX

uint32_t UserInfo::uid()
{
#if defined(FOG_OS_WINDOWS)
  return 0;
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  return (uint32_t)(::getuid());
#endif // FOG_OS_POSIX
}

uint32_t UserInfo::gid()
{
#if defined(FOG_OS_WINDOWS)
  return 0;
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  return (uint32_t)(::getgid());
#endif // FOG_OS_POSIX
}

String32 UserInfo::directory(uint32_t dir)
{
  String32 to;
  directoryTo(dir, to);
  return to;
}

bool UserInfo::directoryTo(uint32_t dir, String32& to)
{
  bool result;

  if (dir >= 6)
  {
    to.clear();
    return false;
  }

  switch (dir)
  {
    case UserInfo::Home:
      result = homeDirectory(to);
      break;
    default:
#if  defined(FOG_OS_POSIX)
      result = xdgLookupDir(directoryNames[dir], to);
#elif defined(FOG_OS_WINDOWS)
      result = registryLookupDir(directoryNames[dir], to);
#endif
      break;
  }

#if defined(FOG_OS_WINDOWS)
  to.slashesToPosix();
#endif // FOG_OS_WINDOWS

  return result;
}

} // Fog namespace
