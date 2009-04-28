CMake_Minimum_Required(VERSION 2.6)

Project(Fog C CXX)

# =============================================================================
# [Configuration]
# =============================================================================

Set(FOG_DIR ${CMAKE_SOURCE_DIR})
Set(FOG_AGG_DIR "${FOG_DIR}/thirdparty/agg")

Set(ASMJIT_DIR "${FOG_DIR}/thirdparty/AsmJit")
#Set(ASMJIT_BUILD_LIBRARY TRUE)

Set(BLITJIT_DIR "${FOG_DIR}/thirdparty/BlitJit")
#Set(BLITJIT_BUILD_LIBRARY TRUE)

Set(TEST_BUILD TRUE)

# =============================================================================
# [Thirdparty Libraries]
# =============================================================================

Add_Definitions(-DASMJIT_API= -DASMJIT_HIDDEN=)
Add_Definitions(-DBLITJIT_API= -DBLITJIT_HIDDEN=)

#If(ASMJIT_BUILD_LIBRARY)
  Include(${ASMJIT_DIR}/CMakeLists.txt NO_POLICY_SCOPE)
#EndIf(ASMJIT_BUILD_LIBRARY)

#If(BLITJIT_BUILD_LIBRARY)
  Include(${BLITJIT_DIR}/CMakeLists.txt NO_POLICY_SCOPE)
#EndIf(BLITJIT_BUILD_LIBRARY)

# =============================================================================
# [Fog]
# =============================================================================

# Include cmake helpers we need
Include(CheckIncludeFiles)
Include(CheckLibraryExists)
Include(CheckTypeSize)
Include(FindThreads)
Include(TestBigEndian)

# Directories
Include_Directories(${FOG_DIR})
Include_Directories(${FOG_AGG_DIR}/include)

# Dependency
Set(FOG_LIBRARIES "")

Set(FOG_BUILD_SOURCES
  ${FOG_DIR}/Fog/Build/Init.cpp
)

Set(FOG_BUILD_HEADERS
  ${FOG_DIR}/Fog/Build/Build.h
  ${FOG_DIR}/Fog/Build/Config.h
  ${FOG_DIR}/Fog/Build/Config.h.cmake
)

SOURCE_GROUP(Build FILES
  ${FOG_BUILD_SOURCES}
  ${FOG_BUILD_HEADERS})
  
# Static linking
SOURCE_GROUP(3rdparty_AsmJit FILES ${ASMJIT_SOURCES} ${ASMJIT_HEADERS})
SOURCE_GROUP(3rdparty_BlitJit FILES ${BLITJIT_SOURCES} ${BLITJIT_HEADERS})

# =============================================================================
# [Fog/Core] Part
# =============================================================================

# Fog/Core Sources
Set(FOG_CORE_SOURCES
  ${FOG_DIR}/Fog/Core/Algorithms.cpp
  ${FOG_DIR}/Fog/Core/Application.cpp
  ${FOG_DIR}/Fog/Core/BitArray.cpp
  ${FOG_DIR}/Fog/Core/Char.cpp
  ${FOG_DIR}/Fog/Core/Class.cpp
  ${FOG_DIR}/Fog/Core/Cpu.cpp
  ${FOG_DIR}/Fog/Core/DirIterator.cpp
  ${FOG_DIR}/Fog/Core/Event.cpp
  ${FOG_DIR}/Fog/Core/EventLoop.cpp
  ${FOG_DIR}/Fog/Core/EventLoop_Def.cpp
  ${FOG_DIR}/Fog/Core/EventLoop_libevent.cpp
  ${FOG_DIR}/Fog/Core/EventLoop_Win.cpp
  ${FOG_DIR}/Fog/Core/FileSystem.cpp
  ${FOG_DIR}/Fog/Core/FileUtil.cpp
  ${FOG_DIR}/Fog/Core/Flags.cpp
  ${FOG_DIR}/Fog/Core/Hash.cpp
  ${FOG_DIR}/Fog/Core/Lazy.cpp
  ${FOG_DIR}/Fog/Core/Library.cpp
  ${FOG_DIR}/Fog/Core/List.cpp
  ${FOG_DIR}/Fog/Core/Locale.cpp
  ${FOG_DIR}/Fog/Core/Lock.cpp
  ${FOG_DIR}/Fog/Core/MapFile.cpp
  ${FOG_DIR}/Fog/Core/Memory.cpp
  ${FOG_DIR}/Fog/Core/Misc.cpp
  ${FOG_DIR}/Fog/Core/OS.cpp
  ${FOG_DIR}/Fog/Core/Object.cpp
  ${FOG_DIR}/Fog/Core/ObserverList.cpp
  ${FOG_DIR}/Fog/Core/RefCounted.cpp
  ${FOG_DIR}/Fog/Core/RefData.cpp
  ${FOG_DIR}/Fog/Core/Sequence.cpp
  ${FOG_DIR}/Fog/Core/Std.cpp
  ${FOG_DIR}/Fog/Core/Stream.cpp
  ${FOG_DIR}/Fog/Core/String.cpp
  ${FOG_DIR}/Fog/Core/String_gen.cpp
  ${FOG_DIR}/Fog/Core/StringCache.cpp
  ${FOG_DIR}/Fog/Core/StringUtil.cpp
  ${FOG_DIR}/Fog/Core/StringUtil_dtoa.cpp
  ${FOG_DIR}/Fog/Core/StringUtil_gen.cpp
  ${FOG_DIR}/Fog/Core/StringUtil_itoa.cpp
  ${FOG_DIR}/Fog/Core/Strings.cpp
  ${FOG_DIR}/Fog/Core/Task.cpp
  ${FOG_DIR}/Fog/Core/TextCodec.cpp
  ${FOG_DIR}/Fog/Core/Thread.cpp
  ${FOG_DIR}/Fog/Core/ThreadCondition.cpp
  ${FOG_DIR}/Fog/Core/ThreadEvent.cpp
  ${FOG_DIR}/Fog/Core/ThreadLocalStorage.cpp
  ${FOG_DIR}/Fog/Core/Time.cpp
  ${FOG_DIR}/Fog/Core/Timer.cpp
  ${FOG_DIR}/Fog/Core/UserInfo.cpp
  ${FOG_DIR}/Fog/Core/Value.cpp
  ${FOG_DIR}/Fog/Core/Vector.cpp
)

Set(FOG_CORE_HEADERS
  ${FOG_DIR}/Fog/Core.h
  ${FOG_DIR}/Fog/Core/Algorithms.h
  ${FOG_DIR}/Fog/Core/Application.h
  ${FOG_DIR}/Fog/Core/Assert.h
  ${FOG_DIR}/Fog/Core/Atomic.h
  ${FOG_DIR}/Fog/Core/AutoLock.h
  ${FOG_DIR}/Fog/Core/AutoUnlock.h
  ${FOG_DIR}/Fog/Core/BitArray.h
  ${FOG_DIR}/Fog/Core/Char.h
  ${FOG_DIR}/Fog/Core/Class.h
  ${FOG_DIR}/Fog/Core/Constants.h
  ${FOG_DIR}/Fog/Core/Cpu.h
  ${FOG_DIR}/Fog/Core/Delegate.h
  ${FOG_DIR}/Fog/Core/DirIterator.h
  ${FOG_DIR}/Fog/Core/Error.h
  ${FOG_DIR}/Fog/Core/Event.h
  ${FOG_DIR}/Fog/Core/EventLoop.h
  ${FOG_DIR}/Fog/Core/EventLoop_Def.h
  ${FOG_DIR}/Fog/Core/EventLoop_libevent.h
  ${FOG_DIR}/Fog/Core/EventLoop_Win.h
  ${FOG_DIR}/Fog/Core/FileSystem.h
  ${FOG_DIR}/Fog/Core/FileUtil.h
  ${FOG_DIR}/Fog/Core/Flags.h
  ${FOG_DIR}/Fog/Core/Hash.h
  ${FOG_DIR}/Fog/Core/Lazy.h
  ${FOG_DIR}/Fog/Core/Library.h
  ${FOG_DIR}/Fog/Core/List.h
  ${FOG_DIR}/Fog/Core/Locale.h
  ${FOG_DIR}/Fog/Core/Lock.h
  ${FOG_DIR}/Fog/Core/LockedPtr.h
  ${FOG_DIR}/Fog/Core/MapFile.h
  ${FOG_DIR}/Fog/Core/Math.h
  ${FOG_DIR}/Fog/Core/Memory.h
  ${FOG_DIR}/Fog/Core/Misc.h
  ${FOG_DIR}/Fog/Core/OS.h
  ${FOG_DIR}/Fog/Core/Object.h
  ${FOG_DIR}/Fog/Core/ObserverList.h
  ${FOG_DIR}/Fog/Core/Pack.h
  ${FOG_DIR}/Fog/Core/RefCounted.h
  ${FOG_DIR}/Fog/Core/RefData.h
  ${FOG_DIR}/Fog/Core/ScopedHandle.h
  ${FOG_DIR}/Fog/Core/ScopedPtr.h
  ${FOG_DIR}/Fog/Core/Sequence.h
  ${FOG_DIR}/Fog/Core/SequenceInfo.h
  ${FOG_DIR}/Fog/Core/Static.h
  ${FOG_DIR}/Fog/Core/Std.h
  ${FOG_DIR}/Fog/Core/Stream.h
  ${FOG_DIR}/Fog/Core/String.h
  ${FOG_DIR}/Fog/Core/String_gen.h
  ${FOG_DIR}/Fog/Core/StringCache.h
  ${FOG_DIR}/Fog/Core/StringUtil.h
  ${FOG_DIR}/Fog/Core/StringUtil_gen.h
  ${FOG_DIR}/Fog/Core/Strings.h
  ${FOG_DIR}/Fog/Core/Task.h
  ${FOG_DIR}/Fog/Core/TextCodec.h
  ${FOG_DIR}/Fog/Core/Thread.h
  ${FOG_DIR}/Fog/Core/ThreadCondition.h
  ${FOG_DIR}/Fog/Core/ThreadEvent.h
  ${FOG_DIR}/Fog/Core/ThreadLocalStorage.h
  ${FOG_DIR}/Fog/Core/Time.h
  ${FOG_DIR}/Fog/Core/Timer.h
  ${FOG_DIR}/Fog/Core/TypeInfo.h
  ${FOG_DIR}/Fog/Core/Unpack.h
  ${FOG_DIR}/Fog/Core/UserInfo.h
  ${FOG_DIR}/Fog/Core/Value.h
  ${FOG_DIR}/Fog/Core/Vector.h
)

SOURCE_GROUP(Core FILES
  ${FOG_CORE_SOURCES}
  ${FOG_CORE_HEADERS})

# Endianness
Test_Big_Endian(WORDS_BIGENDIAN)

If(WORDS_BIGENDIAN)
  Set(FOG_BYTE_ORDER 0)
Else(WORDS_BIGENDIAN)
  Set(FOG_BYTE_ORDER 1)
EndIf(WORDS_BIGENDIAN)

# Size of POD types
Check_Type_Size("void*" FOG_SIZEOF_VOID)
Check_Type_Size("short" FOG_SIZEOF_SHORT)
Check_Type_Size("int" FOG_SIZEOF_INT)
Check_Type_Size("long" FOG_SIZEOF_LONG)
Set(CMAKE_EXTRA_INCLUDE_FILES wchar.h)
Check_Type_Size("wchar_t" FOG_SIZEOF_WCHAR_T)
Set(CMAKE_EXTRA_INCLUDE_FILES)

# Presence of headers
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

# Dynamic loadable library support
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

# Threads support
If(CMAKE_THREAD_LIBS_INIT)
  Set(FOG_LIBRARIES "${FOG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}")
EndIf(CMAKE_THREAD_LIBS_INIT)

# libevent support for unix platforms
If(NOT WIN32)
  Check_Library_Exists(event event_base_new "" FOG_LIBRARY_EVENT_EXISTS)
  If(FOG_LIBRARY_EVENT_EXISTS)
    Set(FOG_LIBRARIES "${FOG_LIBRARIES} -levent")
  Else(FOG_LIBRARY_EVENT_EXISTS)
    Message("Libevent not found!")
  EndIf(FOG_LIBRARY_EVENT_EXISTS)
Endif(NOT WIN32)

# =============================================================================
# [Fog/Graphics] Part
# =============================================================================

# Fog/Graphics Sources
Set(FOG_GRAPHICS_SOURCES
  ${FOG_DIR}/Fog/Graphics/AffineMatrix.cpp
  ${FOG_DIR}/Fog/Graphics/Converter.cpp
  ${FOG_DIR}/Fog/Graphics/Dither.cpp
  ${FOG_DIR}/Fog/Graphics/Font.cpp
  ${FOG_DIR}/Fog/Graphics/Font_FreeType.cpp
  ${FOG_DIR}/Fog/Graphics/Font_Win.cpp
  ${FOG_DIR}/Fog/Graphics/Geometry.cpp
  ${FOG_DIR}/Fog/Graphics/Glyph.cpp
  ${FOG_DIR}/Fog/Graphics/GlyphCache.cpp
  ${FOG_DIR}/Fog/Graphics/GlyphSet.cpp
  ${FOG_DIR}/Fog/Graphics/Gradient.cpp
  ${FOG_DIR}/Fog/Graphics/Image.cpp
  ${FOG_DIR}/Fog/Graphics/ImageFormat.cpp
  ${FOG_DIR}/Fog/Graphics/ImageIO.cpp
  ${FOG_DIR}/Fog/Graphics/ImageIO/ImageIO_BMP.cpp
  ${FOG_DIR}/Fog/Graphics/ImageIO/ImageIO_GIF.cpp
  ${FOG_DIR}/Fog/Graphics/ImageIO/ImageIO_JPEG.cpp
  ${FOG_DIR}/Fog/Graphics/ImageIO/ImageIO_PCX.cpp
  ${FOG_DIR}/Fog/Graphics/ImageIO/ImageIO_PNG.cpp
  ${FOG_DIR}/Fog/Graphics/ImageScale.cpp
  ${FOG_DIR}/Fog/Graphics/Painter.cpp
  ${FOG_DIR}/Fog/Graphics/PainterUtil.cpp
  ${FOG_DIR}/Fog/Graphics/Palette.cpp
  ${FOG_DIR}/Fog/Graphics/Path.cpp
  ${FOG_DIR}/Fog/Graphics/Pattern.cpp
  ${FOG_DIR}/Fog/Graphics/Raster.cpp
  ${FOG_DIR}/Fog/Graphics/Raster_MMX.cpp
  ${FOG_DIR}/Fog/Graphics/Raster_SSE2.cpp
  ${FOG_DIR}/Fog/Graphics/Reduce.cpp
  ${FOG_DIR}/Fog/Graphics/Region.cpp
  ${FOG_DIR}/Fog/Graphics/RegionPoly.cpp
  ${FOG_DIR}/Fog/Graphics/RegionRound.cpp
  ${FOG_DIR}/Fog/Graphics/Rgba.cpp
  ${FOG_DIR}/Fog/Graphics/RgbaAnalyzer.cpp
)

# Fog/Graphics Sources
Set(FOG_GRAPHICS_HEADERS
  ${FOG_DIR}/Fog/Graphics.h
  ${FOG_DIR}/Fog/Graphics/AffineMatrix.h
  ${FOG_DIR}/Fog/Graphics/Constants.h
  ${FOG_DIR}/Fog/Graphics/Converter.h
  ${FOG_DIR}/Fog/Graphics/Dither.h
  ${FOG_DIR}/Fog/Graphics/Fixed.h
  ${FOG_DIR}/Fog/Graphics/Font.h
  ${FOG_DIR}/Fog/Graphics/Font_Win.h
  ${FOG_DIR}/Fog/Graphics/Font_FreeType.h
  ${FOG_DIR}/Fog/Graphics/Geometry.h
  ${FOG_DIR}/Fog/Graphics/Glyph.h
  ${FOG_DIR}/Fog/Graphics/GlyphCache.h
  ${FOG_DIR}/Fog/Graphics/GlyphSet.h
  ${FOG_DIR}/Fog/Graphics/Gradient.h
  ${FOG_DIR}/Fog/Graphics/Image.h
  ${FOG_DIR}/Fog/Graphics/ImageFormat.h
  ${FOG_DIR}/Fog/Graphics/ImageIO.h
  ${FOG_DIR}/Fog/Graphics/ImageIO/ImageIO_BMP.h
  ${FOG_DIR}/Fog/Graphics/ImageIO/ImageIO_GIF.h
  ${FOG_DIR}/Fog/Graphics/ImageIO/ImageIO_JPEG.h
  ${FOG_DIR}/Fog/Graphics/ImageIO/ImageIO_PCX.h
  ${FOG_DIR}/Fog/Graphics/ImageIO/ImageIO_PNG.h
  ${FOG_DIR}/Fog/Graphics/ImageScale.h
  ${FOG_DIR}/Fog/Graphics/Painter.h
  ${FOG_DIR}/Fog/Graphics/PainterUtil.h
  ${FOG_DIR}/Fog/Graphics/Palette.h
  ${FOG_DIR}/Fog/Graphics/Path.h
  ${FOG_DIR}/Fog/Graphics/Pattern.h
  ${FOG_DIR}/Fog/Graphics/Raster.h
  ${FOG_DIR}/Fog/Graphics/Raster_MMX.h
  ${FOG_DIR}/Fog/Graphics/Raster_SSE2.h
  ${FOG_DIR}/Fog/Graphics/Reduce.h
  ${FOG_DIR}/Fog/Graphics/Region.h
  ${FOG_DIR}/Fog/Graphics/Rgba.h
  ${FOG_DIR}/Fog/Graphics/RgbaAnalyzer.h
)

# Fog/Agg Sources
Set(FOG_AGG_SOURCES
  ${FOG_AGG_DIR}/src/agg_arc.cpp
  ${FOG_AGG_DIR}/src/agg_arrowhead.cpp
  ${FOG_AGG_DIR}/src/agg_bezier_arc.cpp
  ${FOG_AGG_DIR}/src/agg_bspline.cpp
  ${FOG_AGG_DIR}/src/agg_curves.cpp
  ${FOG_AGG_DIR}/src/agg_image_filters.cpp
  ${FOG_AGG_DIR}/src/agg_line_aa_basics.cpp
  ${FOG_AGG_DIR}/src/agg_line_profile_aa.cpp
  ${FOG_AGG_DIR}/src/agg_rounded_rect.cpp
  ${FOG_AGG_DIR}/src/agg_sqrt_tables.cpp
  ${FOG_AGG_DIR}/src/agg_trans_affine.cpp
  ${FOG_AGG_DIR}/src/agg_trans_double_path.cpp
  ${FOG_AGG_DIR}/src/agg_trans_single_path.cpp
  ${FOG_AGG_DIR}/src/agg_trans_warp_magnifier.cpp
  ${FOG_AGG_DIR}/src/agg_vcgen_bspline.cpp
  ${FOG_AGG_DIR}/src/agg_vcgen_contour.cpp
  ${FOG_AGG_DIR}/src/agg_vcgen_dash.cpp
  ${FOG_AGG_DIR}/src/agg_vcgen_markers_term.cpp
  ${FOG_AGG_DIR}/src/agg_vcgen_smooth_poly1.cpp
  ${FOG_AGG_DIR}/src/agg_vcgen_stroke.cpp
  ${FOG_AGG_DIR}/src/agg_vpgen_clip_polygon.cpp
  ${FOG_AGG_DIR}/src/agg_vpgen_clip_polyline.cpp
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
)

SOURCE_GROUP(Graphics FILES 
  ${FOG_GRAPHICS_SOURCES}
  ${FOG_GRAPHICS_HEADERS})

SOURCE_GROUP(3rdparty_agg FILES
  ${FOG_AGG_SOURCES}
  ${FOG_AGG_HEADERS})

# Fontconfig and Freetype2 Support
If(WIN32)
  Set(FOG_FONT_WINDOWS TRUE)
Else(WIN32)
  Find_File(HAVE_FREETYPE freetype-config)
  If(HAVE_FREETYPE)
    Exec_Program(freetype-config ARGS --cflags OUTPUT_VARIABLE FREETYPE_CONFIG_OUT RETURN_VALUE FREETYPE_CONFIG_RET)
    Set(FOG_FONT_FREETYPE TRUE)
    Set_Source_Files_Properties({$FOG_DIR}/Fog/Graphics/Font_FreeType.cpp PROPERTIES COMPILE_FLAGS "${FREETYPE_CONFIG_OUT}")
  EndIf(HAVE_FREETYPE)

  If(NOT FOG_HAVE_FONTCONFIG)
    Check_Include_Files(fontconfig/fontconfig.h FOG_HAVE_FONTCONFIG)
  EndIf(NOT FOG_HAVE_FONTCONFIG)
EndIf(WIN32)

# JPEG Support
If(NOT FOG_HAVE_JPEGLIB_H)
  Check_Include_Files(jconfig.h FOG_HAVE_JPEGLIB_H)
EndIf(NOT FOG_HAVE_JPEGLIB_H)

# PNG Support
If(NOT FOG_HAVE_PNG_H)
  Check_Include_Files(png.h FOG_HAVE_PNG_H)
EndIf(NOT FOG_HAVE_PNG_H)

# =============================================================================
# [Fog/UI] Part
# =============================================================================

# Fog/UI Sources
Set(FOG_UI_SOURCES
  ${FOG_DIR}/Fog/UI/BoxLayout.cpp
  ${FOG_DIR}/Fog/UI/Button.cpp
  ${FOG_DIR}/Fog/UI/ButtonBase.cpp
  ${FOG_DIR}/Fog/UI/CheckBox.cpp
  ${FOG_DIR}/Fog/UI/Composite.cpp
  ${FOG_DIR}/Fog/UI/Event.cpp
  ${FOG_DIR}/Fog/UI/Layout.cpp
  ${FOG_DIR}/Fog/UI/LayoutItem.cpp
  ${FOG_DIR}/Fog/UI/Theme.cpp
  ${FOG_DIR}/Fog/UI/Theme_Fog.cpp
  ${FOG_DIR}/Fog/UI/UISystem.cpp
  ${FOG_DIR}/Fog/UI/UISystem_Def.cpp
  ${FOG_DIR}/Fog/UI/UISystem_Win.cpp
  ${FOG_DIR}/Fog/UI/Widget.cpp
  ${FOG_DIR}/Fog/UI/Window.cpp
)

Set(FOG_UI_HEADERS
  ${FOG_DIR}/Fog/UI.h
  ${FOG_DIR}/Fog/UI/BoxLayout.h
  ${FOG_DIR}/Fog/UI/Button.h
  ${FOG_DIR}/Fog/UI/ButtonBase.h
  ${FOG_DIR}/Fog/UI/CheckBox.h
  ${FOG_DIR}/Fog/UI/Composite.h
  ${FOG_DIR}/Fog/UI/Constants.h
  ${FOG_DIR}/Fog/UI/Event.h
  ${FOG_DIR}/Fog/UI/Layout.h
  ${FOG_DIR}/Fog/UI/LayoutItem.h
  ${FOG_DIR}/Fog/UI/Theme.h
  ${FOG_DIR}/Fog/UI/Theme_Fog.h
  ${FOG_DIR}/Fog/UI/UISystem.h
  ${FOG_DIR}/Fog/UI/UISystem_Def.h
  ${FOG_DIR}/Fog/UI/UISystem_Win.h
  ${FOG_DIR}/Fog/UI/Widget.h
  ${FOG_DIR}/Fog/UI/Window.h
  ${FOG_DIR}/Fog/UI/Xlib.h
)

SOURCE_GROUP(UI FILES
  ${FOG_UI_SOURCES}
  ${FOG_UI_HEADERS})

# UI-Subsystem and Libraries
If(WIN32)
  Set(FOG_UI_WINDOWS 1)
  if(MINGW)
    Set(FOG_LIBRARIES ${FOG_LIBRARIES} -luser32 -lgdi32)
  EndIf(MINGW)
Else(WIN32)
  Set(FOG_UI_X11 1)
EndIf(WIN32)

# =============================================================================
# [Finalize]
# =============================================================================

Set(FOG_SOURCES 
  ${FOG_BUILD_SOURCES}
  ${FOG_CORE_SOURCES}
  ${FOG_GRAPHICS_SOURCES}
  ${FOG_UI_SOURCES}
)

Set(FOG_HEADERS
  ${FOG_BUILD_HEADERS}
  ${FOG_CORE_HEADERS}
  ${FOG_GRAPHICS_HEADERS}
  ${FOG_UI_HEADERS}
)

# Write CMAKE_ #defines into Fog/Build/Config.h files
Configure_File(${FOG_DIR}/Fog/Build/Config.h.cmake ${FOG_DIR}/Fog/Build/Config.h)

# Trim
String(STRIP "${FOG_LIBRARIES}" FOG_LIBRARIES)

# Library
Add_Library(Fog SHARED
  ${FOG_SOURCES}
  ${FOG_HEADERS}
  ${FOG_AGG_SOURCES}
  ${FOG_AGG_HEADERS}
  ${ASMJIT_SOURCES}
  ${ASMJIT_HEADERS}
  ${BLITJIT_SOURCES}
  ${BLITJIT_HEADERS}
)
Target_Link_Libraries(Fog ${FOG_LIBRARIES})

# Install
ForEach(i ${FOG_HEADERS})
  Get_Filename_Component(dest ${i} PATH)  
  Install(FILES ${i} DESTINATION "include/${dest}")
EndForEach(i)

# Build test
If(TEST_BUILD)
  Add_Executable(test test/test.cpp)
  Target_Link_Libraries(test Fog ${FOG_LIBRARIES})
EndIf(TEST_BUILD)
