CMake_Minimum_Required(VERSION 2.6)

Project(Fog C CXX)

# =============================================================================
# [Configuration]
# =============================================================================

Set(FOG_DIR ${CMAKE_SOURCE_DIR})
Set(FOG_LIBRARIES "")
Set(FOG_AGG_DIR "${FOG_DIR}/thirdparty/agg")
Set(FOG_AGG_CONTRIB_DIR "${FOG_DIR}/thirdparty/agg-contrib")

Set(TEST_BUILD TRUE)

Set(FOG_BUILD_MODULE_X11_EXTERNAL TRUE)
Set(FOG_BUILD_MODULE_X11_INTERNAL FALSE)

Set(CMAKE_CXX_STANDARD_LIBRARIES "")

# =============================================================================
# [3rdparty Libraries]
# =============================================================================

# =============================================================================
# [Include cmake helpers we need]
# =============================================================================

Include(CheckIncludeFiles)
Include(CheckLibraryExists)
Include(CheckTypeSize)
Include(FindThreads)

# =============================================================================
# [Include Directories]
# =============================================================================

Include_Directories(${FOG_DIR})
Include_Directories(${FOG_AGG_DIR}/include)
Include_Directories(${FOG_AGG_CONTRIB_DIR}/include)

# =============================================================================
# [CFlags]
# =============================================================================

Set(FOG_CFLAGS_OPTIMIZE "")
Set(FOG_CFLAGS_MMX "")
Set(FOG_CFLAGS_SSE "")
Set(FOG_CFLAGS_SSE2 "")
Set(FOG_CFLAGS_SSE3 "")
Set(FOG_CFLAGS_SSSE3 "")

If(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fvisibility-inlines-hidden -fstrict-aliasing -fno-math-errno -Winline -Wstrict-aliasing")
  If(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
    Set(FOG_CFLAGS_OPTIMIZE "-O3 -frename-registers")
  EndIf(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
  Set(FOG_CFLAGS_MMX "${FOG_CFLAGS_OPTIMIZE} -mmmx")
  Set(FOG_CFLAGS_SSE "${FOG_CFLAGS_OPTIMIZE} -msse")
  Set(FOG_CFLAGS_SSE2 "${FOG_CFLAGS_OPTIMIZE} -msse2")
  Set(FOG_CFLAGS_SSE3 "${FOG_CFLAGS_OPTIMIZE} -msse3")
  Set(FOG_CFLAGS_SSSE3 "${FOG_CFLAGS_OPTIMIZE} -mssse3")
EndIf(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)

If(MSVC)
  Set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /Oy /GF /GS- /GR-")
  Set(FOG_CFLAGS_OPTIMIZE "/Ot")
  Set(FOG_CFLAGS_MMX "")
  Set(FOG_CFLAGS_SSE "/arch:SSE")
  Set(FOG_CFLAGS_SSE2 "/arch:SSE2")
  Set(FOG_CFLAGS_SSE3 "/arch:SSE2")
  Set(FOG_CFLAGS_SSSE3 "/arch:SSE2")
EndIf(MSVC)

# =============================================================================
# [Endianness]
# =============================================================================

Include(TestBigEndian)
Test_Big_Endian(WORDS_BIGENDIAN)

If(WORDS_BIGENDIAN)
  Set(FOG_BYTE_ORDER 0)
Else(WORDS_BIGENDIAN)
  Set(FOG_BYTE_ORDER 1)
EndIf(WORDS_BIGENDIAN)

# =============================================================================
# [Size of POD types]
# =============================================================================

Check_Type_Size("void*" FOG_SIZEOF_VOID)
Check_Type_Size("short" FOG_SIZEOF_SHORT)
Check_Type_Size("int" FOG_SIZEOF_INT)
Check_Type_Size("long" FOG_SIZEOF_LONG)
Set(CMAKE_EXTRA_INCLUDE_FILES wchar.h)
Check_Type_Size("wchar_t" FOG_SIZEOF_WCHAR_T)
Set(CMAKE_EXTRA_INCLUDE_FILES)

# =============================================================================
# [Presence of headers]
# =============================================================================

If(NOT FOG_HAVE_FLOAT_H)
  Check_Include_Files(float.h FOG_HAVE_FLOAT_H)
EndIf(NOT FOG_HAVE_FLOAT_H)

If(NOT FOG_HAVE_LIMITS_H)
  Check_Include_Files(limits.h FOG_HAVE_LIMITS_H)
EndIf(NOT FOG_HAVE_LIMITS_H)

If(NOT FOG_HAVE_STDINT_H)
  Check_Include_Files(stdint.h FOG_HAVE_STDINT_H)
EndIf(NOT FOG_HAVE_STDINT_H)

If(UNIX)
  If(NOT FOG_HAVE_UNISTD_H)
    Check_Include_Files(unistd.h FOG_HAVE_UNISTD_H)
  EndIf(NOT FOG_HAVE_UNISTD_H)
  
  If(NOT FOG_HAVE_FENV_H)
    Check_Include_Files(fenv.h FOG_HAVE_FENV_H)
  EndIf(NOT FOG_HAVE_FENV_H)
EndIf(UNIX)


# =============================================================================
# [Dynamic loadable library support]
# =============================================================================

If(WIN32)
  # Windows dll support
  Set(FOG_LIBRARY_WINDOWS 1)
Else(WIN32)
  # Unix libdl 
  Check_Include_Files(dlfcn.h FOG_HAVE_DLFCN_H)
  If(FOG_HAVE_DLFCN_H)
    Check_Library_Exists(dl dlopen "" FOG_LIBRARY_DL_EXISTS)
    If(FOG_LIBRARY_DL_EXISTS)
      Set(FOG_LIBRARY_DL 1)
      Set(FOG_LIBRARIES "${FOG_LIBRARIES} -ldl")
    EndIf(FOG_LIBRARY_DL_EXISTS)
  EndIf(FOG_HAVE_DLFCN_H)
EndIf(WIN32)

# =============================================================================
# [Threads support]
# =============================================================================

If(CMAKE_THREAD_LIBS_INIT)
  Set(FOG_LIBRARIES "${FOG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}")
EndIf(CMAKE_THREAD_LIBS_INIT)

# =============================================================================
# [Include rt library under linux]
# =============================================================================

If(NOT WIN32)
  Set(FOG_LIBRARIES "${FOG_LIBRARIES} -lrt")
EndIf(NOT WIN32)

# =============================================================================
# [Fontconfig and Freetype2 support]
# =============================================================================

If(WIN32)
  Set(FOG_FONT_WINDOWS TRUE)
Else(WIN32)
  Find_File(HAVE_FREETYPE freetype-config)
  If(HAVE_FREETYPE)
    Exec_Program(freetype-config ARGS --cflags OUTPUT_VARIABLE FREETYPE_CONFIG_OUT RETURN_VALUE FREETYPE_CONFIG_RET)
    Set(FOG_FONT_FREETYPE TRUE)
    Set_Source_Files_Properties(${FOG_DIR}/Fog/Graphics/Font_FreeType.cpp PROPERTIES COMPILE_FLAGS "${FREETYPE_CONFIG_OUT}")
  EndIf(HAVE_FREETYPE)

  If(NOT FOG_HAVE_FONTCONFIG)
    Check_Include_Files(fontconfig/fontconfig.h FOG_HAVE_FONTCONFIG)
  EndIf(NOT FOG_HAVE_FONTCONFIG)
EndIf(WIN32)

# =============================================================================
# [JPEG support]
# =============================================================================

If(NOT FOG_HAVE_JPEGLIB_H)
  Check_Include_Files(jconfig.h FOG_HAVE_JPEGLIB_H)
EndIf(NOT FOG_HAVE_JPEGLIB_H)

# =============================================================================
# [PNG support]
# =============================================================================

If(NOT FOG_HAVE_PNG_H)
  Check_Include_Files(png.h FOG_HAVE_PNG_H)
EndIf(NOT FOG_HAVE_PNG_H)

# =============================================================================
# [Fog/Build part]
# =============================================================================

Set(FOG_BUILD_SOURCES
  Fog/Build/Init.cpp
)

Set(FOG_BUILD_HEADERS
  Fog/Build/Build.h
  Fog/Build/Config.h
)

SOURCE_GROUP(Build FILES ${FOG_BUILD_SOURCES} ${FOG_BUILD_HEADERS})

# =============================================================================
# [Fog/Core] Part
# =============================================================================

Set(FOG_CORE_SOURCES
  Fog/Core/Algorithms.cpp
  Fog/Core/Application.cpp
  Fog/Core/Assert.cpp
  Fog/Core/BitArray.cpp
  Fog/Core/Basics.cpp
  Fog/Core/Byte.cpp
  Fog/Core/ByteArray.cpp
  Fog/Core/ByteArrayFilter.cpp
  Fog/Core/ByteArrayMatcher.cpp
  Fog/Core/Char.cpp
  Fog/Core/CharUtil.cpp
  Fog/Core/Class.cpp
  Fog/Core/CpuInfo.cpp
  Fog/Core/DirIterator.cpp
  Fog/Core/Event.cpp
  Fog/Core/EventLoop.cpp
  Fog/Core/EventLoop_Def.cpp
  Fog/Core/EventLoop_Win.cpp
  Fog/Core/FileSystem.cpp
  Fog/Core/FileUtil.cpp
  Fog/Core/Hash.cpp
  Fog/Core/HashUtil.cpp
  Fog/Core/Lazy.cpp
  Fog/Core/Library.cpp
  Fog/Core/List.cpp
  Fog/Core/Locale.cpp
  Fog/Core/Lock.cpp
  Fog/Core/ManagedString.cpp
  Fog/Core/MapFile.cpp
  Fog/Core/Math.cpp
  Fog/Core/Memory.cpp
  Fog/Core/Misc.cpp
  Fog/Core/OS.cpp
  Fog/Core/Object.cpp
  Fog/Core/ObserverList.cpp
  Fog/Core/Properties.cpp
  Fog/Core/RefCounted.cpp
  Fog/Core/Sequence.cpp
  Fog/Core/Std.cpp
  Fog/Core/Stream.cpp
  Fog/Core/String.cpp
  Fog/Core/StringFilter.cpp
  Fog/Core/StringMatcher.cpp
  Fog/Core/StringUtil.cpp
  Fog/Core/StringUtil_gen.cpp
  Fog/Core/Strings.cpp
  Fog/Core/Task.cpp
  Fog/Core/TextCodec.cpp
  Fog/Core/Thread.cpp
  Fog/Core/ThreadCondition.cpp
  Fog/Core/ThreadEvent.cpp
  Fog/Core/ThreadLocalStorage.cpp
  Fog/Core/ThreadPool.cpp
  Fog/Core/Time.cpp
  Fog/Core/Timer.cpp
  Fog/Core/UserInfo.cpp
  Fog/Core/Value.cpp
  Fog/Core/Vector.cpp
  Fog/Core/WinUtil.cpp
)

Set(FOG_CORE_HEADERS
  Fog/Core.h
  Fog/Core/Algorithms.h
  Fog/Core/Application.h
  Fog/Core/Assert.h
  Fog/Core/Atomic.h
  Fog/Core/AutoLock.h
  Fog/Core/AutoUnlock.h
  Fog/Core/Basics.h
  Fog/Core/BitArray.h
  Fog/Core/Byte.h
  Fog/Core/ByteArray.h
  Fog/Core/ByteArrayFilter.h
  Fog/Core/ByteArrayMatcher.h
  Fog/Core/Char.h
  Fog/Core/Class.h
  Fog/Core/Constants.h
  Fog/Core/CpuInfo.h
  Fog/Core/CharUtil.h
  Fog/Core/Delegate.h
  Fog/Core/DirIterator.h
  Fog/Core/Error.h
  Fog/Core/Event.h
  Fog/Core/EventLoop.h
  Fog/Core/EventLoop_Def.h
  Fog/Core/EventLoop_Win.h
  Fog/Core/FileSystem.h
  Fog/Core/FileUtil.h
  Fog/Core/Hash.h
  Fog/Core/HashUtil.h
  Fog/Core/Intrin_3dNow.h
  Fog/Core/Intrin_MMX.h
  Fog/Core/Intrin_SSE.h
  Fog/Core/Intrin_SSE2.h
  Fog/Core/Intrin_SSE3.h
  Fog/Core/Lazy.h
  Fog/Core/Library.h
  Fog/Core/List.h
  Fog/Core/Locale.h
  Fog/Core/Lock.h
  Fog/Core/LockedPtr.h
  Fog/Core/ManagedString.h
  Fog/Core/MapFile.h
  Fog/Core/Math.h
  Fog/Core/Memory.h
  Fog/Core/Misc.h
  Fog/Core/OS.h
  Fog/Core/Object.h
  Fog/Core/ObserverList.h
  Fog/Core/Pack.h
  Fog/Core/RefCounted.h
  Fog/Core/Properties.h
  Fog/Core/ScopedHandle.h
  Fog/Core/ScopedPtr.h
  Fog/Core/Sequence.h
  Fog/Core/SequenceInfo.h
  Fog/Core/Static.h
  Fog/Core/Std.h
  Fog/Core/Stream.h
  Fog/Core/String.h
  Fog/Core/StringFilter.h
  Fog/Core/StringMatcher.h
  Fog/Core/StringUtil.h
  Fog/Core/Strings.h
  Fog/Core/Task.h
  Fog/Core/TextCodec.h
  Fog/Core/Thread.h
  Fog/Core/ThreadCondition.h
  Fog/Core/ThreadEvent.h
  Fog/Core/ThreadLocalStorage.h
  Fog/Core/ThreadPool.h
  Fog/Core/Time.h
  Fog/Core/Timer.h
  Fog/Core/TypeInfo.h
  Fog/Core/Unpack.h
  Fog/Core/UserInfo.h
  Fog/Core/Value.h
  Fog/Core/Vector.h
  Fog/Core/WinUtil.h
)

SOURCE_GROUP(Core FILES ${FOG_CORE_SOURCES} ${FOG_CORE_HEADERS})

# =============================================================================
# [Fog/Xml] Part
# =============================================================================

Set(FOG_XML_SOURCES
  Fog/Xml/XmlDom.cpp
  Fog/Xml/XmlEntity.cpp
  Fog/Xml/XmlReader.cpp
  Fog/Xml/XmlWriter.cpp
)

Set(FOG_XML_HEADERS
  Fog/Xml.h
  Fog/Xml/Error.h
  Fog/Xml/XmlDom.h
  Fog/Xml/XmlEntity.h
  Fog/Xml/XmlReader.h
  Fog/Xml/XmlWriter.h
)

SOURCE_GROUP(Xml FILES ${FOG_XML_SOURCES} ${FOG_XML_HEADERS})

# =============================================================================
# [Fog/Graphics] Part
# =============================================================================

Set(FOG_GRAPHICS_SOURCES
  Fog/Graphics/ColorFilter.cpp
  Fog/Graphics/ColorLut.cpp
  Fog/Graphics/ColorMatrix.cpp
  Fog/Graphics/DitherMatrix.cpp
  Fog/Graphics/Font.cpp
  Fog/Graphics/Font_FreeType.cpp
  Fog/Graphics/Font_Win.cpp
  Fog/Graphics/Geometry.cpp
  Fog/Graphics/Glyph.cpp
  Fog/Graphics/GlyphCache.cpp
  Fog/Graphics/GlyphSet.cpp
  Fog/Graphics/Gradient.cpp
  Fog/Graphics/Image.cpp
  Fog/Graphics/ImageFilter.cpp
  Fog/Graphics/ImageIO.cpp
  Fog/Graphics/ImageIO/ImageIO_BMP.cpp
  Fog/Graphics/ImageIO/ImageIO_GIF.cpp
  Fog/Graphics/ImageIO/ImageIO_ICO.cpp
  Fog/Graphics/ImageIO/ImageIO_JPEG.cpp
  Fog/Graphics/ImageIO/ImageIO_PCX.cpp
  Fog/Graphics/ImageIO/ImageIO_PNG.cpp
  Fog/Graphics/Matrix.cpp
  Fog/Graphics/Painter.cpp
  Fog/Graphics/PainterEngine.cpp
  Fog/Graphics/PainterEngine_Null.cpp
  Fog/Graphics/PainterEngine_Raster.cpp
  Fog/Graphics/PainterUtil.cpp
  Fog/Graphics/Palette.cpp
  Fog/Graphics/Path.cpp
  Fog/Graphics/Pattern.cpp
  Fog/Graphics/Raster.cpp
  Fog/Graphics/Raster/Raster_C.cpp
  Fog/Graphics/Raster/Raster_C_base.cpp.h
  Fog/Graphics/Raster/Raster_C_adaptor.cpp.h
  Fog/Graphics/Raster/Raster_C_composite.cpp.h
  Fog/Graphics/Raster/Raster_C_convert.cpp.h
  Fog/Graphics/Raster/Raster_C_filters.cpp.h
  Fog/Graphics/Raster/Raster_C_gradient.cpp.h
  Fog/Graphics/Raster/Raster_C_pattern.cpp.h
  Fog/Graphics/Raster/Raster_C_vector.cpp.h
  Fog/Graphics/Raster/Raster_MMX.cpp
  Fog/Graphics/Raster/Raster_MMX_adaptor.cpp.h
  Fog/Graphics/Raster/Raster_MMX_base.cpp.h
  Fog/Graphics/Raster/Raster_MMX_composite.cpp.h
  Fog/Graphics/Raster/Raster_MMX_convert.cpp.h
  Fog/Graphics/Raster/Raster_MMX_filters.cpp.h
  Fog/Graphics/Raster/Raster_MMX_gradient.cpp.h
  Fog/Graphics/Raster/Raster_MMX_pattern.cpp.h
  Fog/Graphics/Raster/Raster_MMX_vector.cpp.h
  Fog/Graphics/Raster/Raster_SSE2.cpp
  Fog/Graphics/Raster/Raster_SSE2_adaptor.cpp.h
  Fog/Graphics/Raster/Raster_SSE2_base.cpp.h
  Fog/Graphics/Raster/Raster_SSE2_composite.cpp.h
  Fog/Graphics/Raster/Raster_SSE2_convert.cpp.h
  Fog/Graphics/Raster/Raster_SSE2_filters.cpp.h
  Fog/Graphics/Raster/Raster_SSE2_gradient.cpp.h
  Fog/Graphics/Raster/Raster_SSE2_pattern.cpp.h
  Fog/Graphics/Raster/Raster_SSE2_vector.cpp.h
  Fog/Graphics/Rasterizer.cpp
  Fog/Graphics/Reduce.cpp
  Fog/Graphics/Region.cpp
  Fog/Graphics/RegionBuilder.cpp
  Fog/Graphics/RegionPoly.cpp
  Fog/Graphics/RegionRound.cpp
  Fog/Graphics/Rgba.cpp
  Fog/Graphics/RgbaAnalyzer.cpp
  Fog/Graphics/Scanline.cpp
)

Set(FOG_GRAPHICS_HEADERS
  Fog/Graphics.h
  Fog/Graphics/ColorFilter.h
  Fog/Graphics/ColorLut.h
  Fog/Graphics/ColorMatrix.h
  Fog/Graphics/Constants.h
  Fog/Graphics/DitherMatrix.h
  Fog/Graphics/Error.h
  Fog/Graphics/Fixed.h
  Fog/Graphics/Font.h
  Fog/Graphics/Font_Win.h
  Fog/Graphics/Font_FreeType.h
  Fog/Graphics/Geometry.h
  Fog/Graphics/Glyph.h
  Fog/Graphics/GlyphCache.h
  Fog/Graphics/GlyphSet.h
  Fog/Graphics/Gradient.h
  Fog/Graphics/Image.h
  Fog/Graphics/ImageFilter.h
  Fog/Graphics/ImageIO.h
  Fog/Graphics/ImageIO/ImageIO_BMP.h
  Fog/Graphics/ImageIO/ImageIO_GIF.h
  Fog/Graphics/ImageIO/ImageIO_JPEG.h
  Fog/Graphics/ImageIO/ImageIO_PCX.h
  Fog/Graphics/ImageIO/ImageIO_PNG.h
  Fog/Graphics/Matrix.h
  Fog/Graphics/Painter.h
  Fog/Graphics/PainterEngine.h
  Fog/Graphics/PainterEngine_Null.h
  Fog/Graphics/PainterEngine_Raster.h
  Fog/Graphics/PainterUtil.h
  Fog/Graphics/Palette.h
  Fog/Graphics/Path.h
  Fog/Graphics/Path_p.h
  Fog/Graphics/Pattern.h
  Fog/Graphics/Raster.h
  Fog/Graphics/Raster/Raster_Bresenham.h
  Fog/Graphics/Raster/Raster_C.h
  Fog/Graphics/Rasterizer.h
  Fog/Graphics/Reduce.h
  Fog/Graphics/Region.h
  Fog/Graphics/RegionBuilder.h
  Fog/Graphics/Rgba.h
  Fog/Graphics/RgbaAnalyzer.h
  Fog/Graphics/Scanline.h
)

# TODO: REMOVE
# Fog/Agg Sources
Set(FOG_AGG_SOURCES
  ${FOG_AGG_DIR}/src/agg_arc.cpp
  ${FOG_AGG_DIR}/src/agg_line_aa_basics.cpp
  ${FOG_AGG_DIR}/src/agg_rounded_rect.cpp
  ${FOG_AGG_DIR}/src/agg_sqrt_tables.cpp
  ${FOG_AGG_DIR}/src/agg_trans_affine.cpp
  ${FOG_AGG_DIR}/src/agg_vcgen_contour.cpp
  ${FOG_AGG_DIR}/src/agg_vcgen_dash.cpp
  ${FOG_AGG_DIR}/src/agg_vcgen_markers_term.cpp
  ${FOG_AGG_DIR}/src/agg_vcgen_stroke.cpp
  ${FOG_AGG_DIR}/src/agg_vpgen_segmentator.cpp
)

Set(FOG_AGG_HEADERS
  ${FOG_AGG_DIR}/include/agg_alpha_mask_u8.h
  ${FOG_AGG_DIR}/include/agg_arc.h
  ${FOG_AGG_DIR}/include/agg_array.h
  ${FOG_AGG_DIR}/include/agg_arrowhead.h
  ${FOG_AGG_DIR}/include/agg_basics.h
  ${FOG_AGG_DIR}/include/agg_bezier_arc.h
  ${FOG_AGG_DIR}/include/agg_bitset_iterator.h
  ${FOG_AGG_DIR}/include/agg_blur.h
  ${FOG_AGG_DIR}/include/agg_bounding_rect.h
  ${FOG_AGG_DIR}/include/agg_bspline.h
  ${FOG_AGG_DIR}/include/agg_clip_liang_barsky.h
  ${FOG_AGG_DIR}/include/agg_color_gray.h
  ${FOG_AGG_DIR}/include/agg_color_rgba.h
  ${FOG_AGG_DIR}/include/agg_config.h
  ${FOG_AGG_DIR}/include/agg_conv_adaptor_vcgen.h
  ${FOG_AGG_DIR}/include/agg_conv_adaptor_vpgen.h
  ${FOG_AGG_DIR}/include/agg_conv_bspline.h
  ${FOG_AGG_DIR}/include/agg_conv_clip_polygon.h
  ${FOG_AGG_DIR}/include/agg_conv_clip_polyline.h
  ${FOG_AGG_DIR}/include/agg_conv_close_polygon.h
  ${FOG_AGG_DIR}/include/agg_conv_concat.h
  ${FOG_AGG_DIR}/include/agg_conv_contour.h
  ${FOG_AGG_DIR}/include/agg_conv_curve.h
  ${FOG_AGG_DIR}/include/agg_conv_dash.h
  ${FOG_AGG_DIR}/include/agg_conv_gpc.h
  ${FOG_AGG_DIR}/include/agg_conv_marker.h
  ${FOG_AGG_DIR}/include/agg_conv_marker_adaptor.h
  ${FOG_AGG_DIR}/include/agg_conv_segmentator.h
  ${FOG_AGG_DIR}/include/agg_conv_shorten_path.h
  ${FOG_AGG_DIR}/include/agg_conv_smooth_poly1.h
  ${FOG_AGG_DIR}/include/agg_conv_stroke.h
  ${FOG_AGG_DIR}/include/agg_conv_transform.h
  ${FOG_AGG_DIR}/include/agg_conv_unclose_polygon.h
  ${FOG_AGG_DIR}/include/agg_curves.h
  ${FOG_AGG_DIR}/include/agg_dda_line.h
  ${FOG_AGG_DIR}/include/agg_ellipse.h
  ${FOG_AGG_DIR}/include/agg_ellipse_bresenham.h
  ${FOG_AGG_DIR}/include/agg_embedded_raster_fonts.h
  ${FOG_AGG_DIR}/include/agg_font_cache_manager.h
  ${FOG_AGG_DIR}/include/agg_gamma_functions.h
  ${FOG_AGG_DIR}/include/agg_gamma_lut.h
  ${FOG_AGG_DIR}/include/agg_glyph_raster_bin.h
  ${FOG_AGG_DIR}/include/agg_gradient_lut.h
  ${FOG_AGG_DIR}/include/agg_gsv_text.h
  ${FOG_AGG_DIR}/include/agg_image_accessors.h
  ${FOG_AGG_DIR}/include/agg_image_filters.h
  ${FOG_AGG_DIR}/include/agg_line_aa_basics.h
  ${FOG_AGG_DIR}/include/agg_math.h
  ${FOG_AGG_DIR}/include/agg_math_stroke.h
  ${FOG_AGG_DIR}/include/agg_path_length.h
  ${FOG_AGG_DIR}/include/agg_path_storage.h
  ${FOG_AGG_DIR}/include/agg_path_storage_integer.h
  ${FOG_AGG_DIR}/include/agg_pattern_filters_rgba.h
  ${FOG_AGG_DIR}/include/agg_pixfmt_amask_adaptor.h
  ${FOG_AGG_DIR}/include/agg_pixfmt_gray.h
  ${FOG_AGG_DIR}/include/agg_pixfmt_rgb.h
  ${FOG_AGG_DIR}/include/agg_pixfmt_rgba.h
  ${FOG_AGG_DIR}/include/agg_pixfmt_rgb_packed.h
  ${FOG_AGG_DIR}/include/agg_pixfmt_transposer.h
  ${FOG_AGG_DIR}/include/agg_rasterizer_cells_aa.h
  ${FOG_AGG_DIR}/include/agg_rasterizer_compound_aa.h
  ${FOG_AGG_DIR}/include/agg_rasterizer_outline.h
  ${FOG_AGG_DIR}/include/agg_rasterizer_outline_aa.h
  ${FOG_AGG_DIR}/include/agg_rasterizer_scanline_aa.h
  ${FOG_AGG_DIR}/include/agg_rasterizer_sl_clip.h
  ${FOG_AGG_DIR}/include/agg_renderer_base.h
  ${FOG_AGG_DIR}/include/agg_renderer_markers.h
  ${FOG_AGG_DIR}/include/agg_renderer_mclip.h
  ${FOG_AGG_DIR}/include/agg_renderer_outline_aa.h
  ${FOG_AGG_DIR}/include/agg_renderer_outline_image.h
  ${FOG_AGG_DIR}/include/agg_renderer_primitives.h
  ${FOG_AGG_DIR}/include/agg_renderer_raster_text.h
  ${FOG_AGG_DIR}/include/agg_renderer_scanline.h
  ${FOG_AGG_DIR}/include/agg_rendering_buffer.h
  ${FOG_AGG_DIR}/include/agg_rendering_buffer_dynarow.h
  ${FOG_AGG_DIR}/include/agg_rounded_rect.h
  ${FOG_AGG_DIR}/include/agg_scanline_bin.h
  ${FOG_AGG_DIR}/include/agg_scanline_boolean_algebra.h
  ${FOG_AGG_DIR}/include/agg_scanline_p.h
  ${FOG_AGG_DIR}/include/agg_scanline_storage_aa.h
  ${FOG_AGG_DIR}/include/agg_scanline_storage_bin.h
  ${FOG_AGG_DIR}/include/agg_scanline_u.h
  ${FOG_AGG_DIR}/include/agg_shorten_path.h
  ${FOG_AGG_DIR}/include/agg_simul_eq.h
  ${FOG_AGG_DIR}/include/agg_span_allocator.h
  ${FOG_AGG_DIR}/include/agg_span_converter.h
  ${FOG_AGG_DIR}/include/agg_span_gouraud.h
  ${FOG_AGG_DIR}/include/agg_span_gouraud_gray.h
  ${FOG_AGG_DIR}/include/agg_span_gouraud_rgba.h
  ${FOG_AGG_DIR}/include/agg_span_gradient.h
  ${FOG_AGG_DIR}/include/agg_span_gradient_alpha.h
  ${FOG_AGG_DIR}/include/agg_span_gradient_contour.h
  ${FOG_AGG_DIR}/include/agg_span_gradient_image.h
  ${FOG_AGG_DIR}/include/agg_span_image_filter.h
  ${FOG_AGG_DIR}/include/agg_span_image_filter_gray.h
  ${FOG_AGG_DIR}/include/agg_span_image_filter_rgb.h
  ${FOG_AGG_DIR}/include/agg_span_image_filter_rgba.h
  ${FOG_AGG_DIR}/include/agg_span_interpolator_adaptor.h
  ${FOG_AGG_DIR}/include/agg_span_interpolator_linear.h
  ${FOG_AGG_DIR}/include/agg_span_interpolator_persp.h
  ${FOG_AGG_DIR}/include/agg_span_interpolator_trans.h
  ${FOG_AGG_DIR}/include/agg_span_pattern_gray.h
  ${FOG_AGG_DIR}/include/agg_span_pattern_rgb.h
  ${FOG_AGG_DIR}/include/agg_span_pattern_rgba.h
  ${FOG_AGG_DIR}/include/agg_span_solid.h
  ${FOG_AGG_DIR}/include/agg_span_subdiv_adaptor.h
  ${FOG_AGG_DIR}/include/agg_trans_affine.h
  ${FOG_AGG_DIR}/include/agg_trans_bilinear.h
  ${FOG_AGG_DIR}/include/agg_trans_double_path.h
  ${FOG_AGG_DIR}/include/agg_trans_perspective.h
  ${FOG_AGG_DIR}/include/agg_trans_single_path.h
  ${FOG_AGG_DIR}/include/agg_trans_viewport.h
  ${FOG_AGG_DIR}/include/agg_trans_warp_magnifier.h
  ${FOG_AGG_DIR}/include/agg_vcgen_bspline.h
  ${FOG_AGG_DIR}/include/agg_vcgen_contour.h
  ${FOG_AGG_DIR}/include/agg_vcgen_dash.h
  ${FOG_AGG_DIR}/include/agg_vcgen_markers_term.h
  ${FOG_AGG_DIR}/include/agg_vcgen_smooth_poly1.h
  ${FOG_AGG_DIR}/include/agg_vcgen_stroke.h
  ${FOG_AGG_DIR}/include/agg_vcgen_vertex_sequence.h
  ${FOG_AGG_DIR}/include/agg_vertex_sequence.h
  ${FOG_AGG_DIR}/include/agg_vpgen_clip_polygon.h
  ${FOG_AGG_DIR}/include/agg_vpgen_clip_polyline.h
  ${FOG_AGG_DIR}/include/agg_vpgen_segmentator.h
  ${FOG_AGG_DIR}/include/util/agg_color_conv.h
  ${FOG_AGG_DIR}/include/util/agg_color_conv_rgb16.h
  ${FOG_AGG_DIR}/include/util/agg_color_conv_rgb8.h
  ${FOG_AGG_CONTRIB_DIR}/include/agg_rasterizer_scanline_aa_custom.h
)

SOURCE_GROUP(Graphics FILES ${FOG_GRAPHICS_SOURCES} ${FOG_GRAPHICS_HEADERS})

SOURCE_GROUP(3rdparty_agg FILES ${FOG_AGG_SOURCES} ${FOG_AGG_HEADERS})

# Optimize specific files for speed
Set_Source_Files_Properties(Fog/Graphics/Image.cpp               PROPERTIES COMPILE_FLAGS "${FOG_CFLAGS_OPTIMIZE}")
Set_Source_Files_Properties(Fog/Graphics/Painter.cpp             PROPERTIES COMPILE_FLAGS "${FOG_CFLAGS_OPTIMIZE}")
Set_Source_Files_Properties(Fog/Graphics/Raster/Raster_C.cpp     PROPERTIES COMPILE_FLAGS "${FOG_CFLAGS_OPTIMIZE}")
Set_Source_Files_Properties(Fog/Graphics/Raster/Raster_MMX.cpp   PROPERTIES COMPILE_FLAGS "${FOG_CFLAGS_MMX}")
Set_Source_Files_Properties(Fog/Graphics/Raster/Raster_SSE2.cpp  PROPERTIES COMPILE_FLAGS "${FOG_CFLAGS_SSE2}")
Set_Source_Files_Properties(Fog/Graphics/Rasterizer.cpp          PROPERTIES COMPILE_FLAGS "${FOG_CFLAGS_OPTIMIZE}")
Set_Source_Files_Properties(Fog/Graphics/Region.cpp              PROPERTIES COMPILE_FLAGS "${FOG_CFLAGS_OPTIMIZE}")
Set_Source_Files_Properties(Fog/Graphics/Scanline.cpp            PROPERTIES COMPILE_FLAGS "${FOG_CFLAGS_OPTIMIZE}")

# =============================================================================
# [Fog/Svg] Part
# =============================================================================

Set(FOG_SVG_SOURCES
  Fog/Svg/SvgBase.cpp
  Fog/Svg/SvgContext.cpp
  Fog/Svg/SvgDom.cpp
  Fog/Svg/SvgUtil.cpp
)

Set(FOG_SVG_HEADERS
  Fog/Svg.h
  Fog/Svg/Error.h
  Fog/Svg/SvgBase.h
  Fog/Svg/SvgContext.h
  Fog/Svg/SvgDom.h
  Fog/Svg/SvgUtil.h
)

SOURCE_GROUP(Svg FILES ${FOG_SVG_SOURCES} ${FOG_SVG_HEADERS})

# =============================================================================
# [Fog/UI] Part
# =============================================================================

Set(FOG_UI_SOURCES
  Fog/UI/BoxLayout.cpp
  Fog/UI/Button.cpp
  Fog/UI/ButtonBase.cpp
  Fog/UI/CheckBox.cpp
  Fog/UI/Composite.cpp
  Fog/UI/Event.cpp
  Fog/UI/Layout.cpp
  Fog/UI/LayoutItem.cpp
  Fog/UI/Theme.cpp
  Fog/UI/Theme_Fog.cpp
  Fog/UI/UISystem.cpp
  Fog/UI/UISystem_Def.cpp
  Fog/UI/Widget.cpp
  Fog/UI/Window.cpp
)

Set(FOG_UI_HEADERS
  Fog/UI.h
  Fog/UI/BoxLayout.h
  Fog/UI/Button.h
  Fog/UI/ButtonBase.h
  Fog/UI/CheckBox.h
  Fog/UI/Composite.h
  Fog/UI/Constants.h
  Fog/UI/Error.h
  Fog/UI/Event.h
  Fog/UI/Layout.h
  Fog/UI/LayoutItem.h
  Fog/UI/Theme.h
  Fog/UI/Theme_Fog.h
  Fog/UI/UISystem.h
  Fog/UI/UISystem_Def.h
  Fog/UI/Widget.h
  Fog/UI/Window.h
  Fog/UI/Xlib.h
)

Set(FOG_UI_WIN_SOURCES
  Fog/UI/UISystem_Win.cpp
)

Set(FOG_UI_WIN_HEADERS
  Fog/UI/UISystem_Win.h
)

Set(FOG_UI_X11_SOURCES
  Fog/UI/UISystem_X11.cpp
)

Set(FOG_UI_X11_HEADERS
  Fog/UI/UISystem_X11.h
)

SOURCE_GROUP(UI FILES
  ${FOG_UI_SOURCES} ${FOG_UI_HEADERS}
  ${FOG_UI_X11_SOURCES} ${FOG_UI_X11_HEADERS}
  ${FOG_UI_WIN_SOURCES} ${FOG_UI_WIN_HEADERS})

# UI-Subsystem and Libraries
If(WIN32)
  Set(FOG_UI_WINDOWS 1)
Else(WIN32)
  Set(FOG_UI_X11 1)
EndIf(WIN32)

# =============================================================================
# [Finalize]
# =============================================================================

Set(FOG_SOURCES 
  ${FOG_BUILD_SOURCES}
  ${FOG_CORE_SOURCES}
  ${FOG_XML_SOURCES}
  ${FOG_GRAPHICS_SOURCES}
  ${FOG_SVG_SOURCES}
  ${FOG_UI_SOURCES}
)

Set(FOG_HEADERS
  ${FOG_BUILD_HEADERS}
  ${FOG_CORE_HEADERS}
  ${FOG_XML_HEADERS}
  ${FOG_GRAPHICS_HEADERS}
  ${FOG_SVG_HEADERS}
  ${FOG_UI_HEADERS}
)

If(FOG_BUILD_MODULE_X11_INTERNAL AND NOT WIN32)
  Set(FOG_SOURCES ${FOG_SOURCES} ${FOG_UI_X11_SOURCES})
  Set(FOG_HEADERS ${FOG_HEADERS} ${FOG_UI_X11_HEADERS})
EndIf(FOG_BUILD_MODULE_X11_INTERNAL AND NOT WIN32)

If(WIN32)
  Set(FOG_SOURCES ${FOG_SOURCES} ${FOG_UI_WIN_SOURCES})
  Set(FOG_SOURCES ${FOG_HEADERS} ${FOG_UI_WIN_HEADERS})
EndIf(WIN32)

# Write CMAKE_ #defines into Fog/Build/Config.h files
Configure_File(${FOG_DIR}/Fog/Build/Config.h.cmake ${FOG_DIR}/Fog/Build/Config.h)

# Trim
String(STRIP "${FOG_LIBRARIES}" FOG_LIBRARIES)

# Unicode
If(WIN32)
  Add_Definitions(-DUNICODE -D_UNICODE)
EndIf(WIN32)

# TODO: Remove antigrain...
# Library
Add_Library(Fog SHARED
  ${FOG_SOURCES}
  ${FOG_HEADERS}
  ${FOG_AGG_SOURCES}
  ${FOG_AGG_HEADERS}
)
Install(TARGETS Fog DESTINATION lib)

Target_Link_Libraries(Fog ${FOG_LIBRARIES})
If(WIN32)
  Target_Link_Libraries(Fog user32 gdi32 advapi32 winmm)
EndIf(WIN32)

If(FOG_BUILD_MODULE_X11_EXTERNAL AND NOT WIN32)
  Add_Library(FogUI_X11 SHARED ${FOG_UI_X11_SOURCES})
  Install(TARGETS FogUI_X11 DESTINATION lib)

  Target_Link_Libraries(FogUI_X11 Fog ${FOG_LIBRARIES})
EndIf(FOG_BUILD_MODULE_X11_EXTERNAL AND NOT WIN32)

# Install header files.
ForEach(i ${FOG_HEADERS})
  Get_Filename_Component(path ${i} PATH)
  Install(FILES ${i} DESTINATION "include/${path}")
EndForEach(i)

# Build test
If(TEST_BUILD)
  Add_Executable(test WIN32 test/test.cpp)
  Target_Link_Libraries(test Fog ${FOG_LIBRARIES})
EndIf(TEST_BUILD)

If(MAKE_BENCH)
  If(NOT WIN32)
    Include_Directories(/usr/include/cairo)
  EndIf(NOT WIN32)

  Add_Executable(bench test/bench.cpp)
  Target_Link_Libraries(bench Fog ${FOG_LIBRARIES})

  If(NOT WIN32)
    Target_Link_Libraries(bench cairo)
  EndIf(NOT WIN32)
EndIf(MAKE_BENCH)
